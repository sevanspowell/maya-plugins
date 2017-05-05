#include "DrawCurveContext.h"

void *DrawCurveToolCommand::creator() { return new DrawCurveToolCommand(); }

MStatus DrawCurveToolCommand::doIt(const MArgList &args) { return redoIt(); }

MStatus DrawCurveToolCommand::redoIt() {
    MStatus status;

    // Create a NURBS curve
    MFnNurbsCurve fnCurve;
    MObject oCurve = fnCurve.createWithEditPoints(
        m_editPoints, 3, MFnNurbsCurve::kOpen, false, true, true,
        MObject::kNullObj, // No parent - create new transform
        &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Get DAG path to curve and store it so we can delete the curve later
    // Don't hold onto MObjects in your plugins - may not be valid
    MFnDagNode fnDag(oCurve, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = fnDag.getPath(m_pathCurve);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}

MStatus DrawCurveToolCommand::undoIt() {
    MStatus status;

    if (m_pathCurve.isValid()) {
        // Use command to delete curve
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "delete %s",
                 m_pathCurve.partialPathName().asChar());
        status = MGlobal::executeCommand(buffer);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    return MS::kSuccess;
}

bool DrawCurveToolCommand::isUndoable() const { return true; }

MStatus DrawCurveToolCommand::finalize() {
    // Run script command
    MArgList command;
    command.addArg(commandString());
    return MPxToolCommand::doFinalize(command);
}

void DrawCurveToolCommand::setEditPoints(MPointArray &editPoints) {
    m_editPoints = editPoints;
}

void DrawCurveContext::toolOnSetup(MEvent &event) {
    MStatus status;

    // Get active 3D view
    m_view = M3dView::active3dView();
    // Get camera DAG path
    status = m_view.getCamera(m_pathCamera);
    CHECK_MSTATUS(status);

    updateCameraNormal();

    // Get panel to attach callback to
    MString modelPanel;
    status = MGlobal::executeCommand("getPanel -wf", modelPanel);
    CHECK_MSTATUS(status);
    // Attach callback
    if (m_postRenderId == 0) {
        m_postRenderId = MUiMessage::add3dViewPostRenderMsgCallback(
            modelPanel, &DrawCurveContext::postRenderCallback, (void *)this,
            &status);
        CHECK_MSTATUS(status);
    }

    // Get selected geometry
    m_geometry.clear();
    status = getSelection(m_geometry);
    CHECK_MSTATUS(status);
}

void DrawCurveContext::toolOffCleanup() {
    // Deregister callback
    if (m_postRenderId) {
        MMessage::removeCallback(m_postRenderId);
        m_postRenderId = 0;
    }
}

void DrawCurveContext::getClassName(MString &name) const {
    // Set the name of the context
    name.set("drawCurveContext");
}

MStatus DrawCurveContext::doPress(MEvent &event) {
    MStatus status;

    short mouseX, mouseY;
    status = event.getPosition(mouseX, mouseY);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    m_editPoints.clear();
    status = getMeshIntersection(mouseX, mouseY);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Edit points start at mouse intersection point with mesh
    m_distanceFromLastEditPoint = 0.0;
    m_lastProjectedPoint        = m_intersectionPoint;

    return MS::kSuccess;
}

MStatus DrawCurveContext::doDrag(MEvent &event) {
    MStatus status;

    if (!m_intersects) {
        // Didn't intersect with anything, don't need to do any work.
        return MS::kSuccess;
    }

    short mouseX, mouseY;
    event.getPosition(mouseX, mouseY);
    MPoint near, far;
    status = m_view.viewToWorld(mouseX, mouseY, near, far);

    MPoint projectedPoint =
        planeLineIntersection(near, far, m_intersectionPoint, m_cameraNormal);
    m_distanceFromLastEditPoint +=
        projectedPoint.distanceTo(m_lastProjectedPoint);

    if (m_distanceFromLastEditPoint >= m_length) {
        m_editPoints.append(projectedPoint);
        status = createCurve();
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // Reset cumulative distance
        m_distanceFromLastEditPoint = 0.0f;
    }
    m_lastProjectedPoint = projectedPoint;

    return MS::kSuccess;
}

MStatus DrawCurveContext::doRelease(MEvent &event) {
    MStatus status;

    // Don't need to do any work if mouse didn't intersect with a mesh
    if (!m_intersects) {
        return MS::kSuccess;
    }

    short mouseX, mouseY;
    event.getPosition(mouseX, mouseY);
    MPoint near, far;
    status = m_view.viewToWorld(mouseX, mouseY, near, far);

    MPoint projectedPoint =
        planeLineIntersection(near, far, m_intersectionPoint, m_cameraNormal);
    m_editPoints.append(projectedPoint);

    status = deleteCurve();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Create the tool command so we can undo the curve creation - the tool
    // command will recreate the curve for us, so we can take advantage of it's
    // undo features.
    DrawCurveToolCommand *cmd = (DrawCurveToolCommand *)newToolCommand();
    cmd->setEditPoints(m_editPoints);
    cmd->redoIt();
    cmd->finalize();

    return MS::kSuccess;
}

float DrawCurveContext::getLength() const { return m_length; }

void DrawCurveContext::setLength(float length) { m_length = length; }

MStatus DrawCurveContext::getSelection(MDagPathArray &geometry) {
    MStatus status;

    MSelectionList selectionList;
    MGlobal::getActiveSelectionList(selectionList);

    // Get the shape nodes of the selection
    MItSelectionList itList(selectionList);
    MDagPath pathNode;
    unsigned int numShapes;
    // Iterate through each selection object
    for (; !itList.isDone(); itList.next()) {
        // Get the path of the object
        status = itList.getDagPath(pathNode);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // Iterate thru each child
        numShapes = pathNode.childCount();
        for (unsigned int i = 0; i < numShapes; ++i) {
            status = pathNode.push(pathNode.child(i));
            CHECK_MSTATUS_AND_RETURN_IT(status);

            if (pathNode.node().hasFn(MFn::kMesh)) {
                // Is a mesh
                MFnDagNode fnDag(pathNode);
                // Make sure is not an intermediate object
                if (!fnDag.isIntermediateObject()) {
                    // Found shape node we want
                    geometry.append(MDagPath(pathNode));
                }
            }
            pathNode.pop();
        }
    }

    return MS::kSuccess;
}

MStatus DrawCurveContext::getMeshIntersection(short mouseX, short mouseY) {
    MStatus status;

    MPoint near, far;
    m_view.viewToWorld(mouseX, mouseY, near, far);

    MVector rayDirection = (far - near).normal();
    MFloatPoint raySource, hitPoint;
    raySource.setCast(near); // cast MPoint to MFloatPoint
    float intersectionDistance = 0.0f;
    int hitFace, hitTriangle;
    float hitBary1, hitBary2;
    bool initialClosestIntersection = true;
    float minIntersectionDistance   = 0.0f;
    m_intersects = false; // Reset every time mouse is pressed

    for (unsigned int i = 0; i < m_geometry.length(); ++i) {
        MFnMesh fnMesh(m_geometry[i], &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        if (!fnMesh.closestIntersection(
                raySource, rayDirection, nullptr, nullptr, false,
                MSpace::kWorld, 1000.0f, false, NULL, hitPoint,
                &intersectionDistance, &hitFace, &hitTriangle, &hitBary1,
                &hitBary2, 0.00001f, &status)) {
            // No intersection with this mesh
            continue;
        }
        // Else, did intersect
        CHECK_MSTATUS_AND_RETURN_IT(status);

        if (initialClosestIntersection ||
            intersectionDistance < minIntersectionDistance) {
            m_intersects               = true;
            minIntersectionDistance    = intersectionDistance;
            initialClosestIntersection = false;
            m_intersectionPoint        = MPoint(hitPoint);
        }
    }

    if (m_intersects) {
        // Make first edit point the intersection point
        m_editPoints.append(m_intersectionPoint);
    }

    return MS::kSuccess;
}

void DrawCurveContext::postRenderCallback(const MString &panelName,
                                          void *data) {
    DrawCurveContext *context = (DrawCurveContext *)data;

    if (context == nullptr) {
        return;
    }

    context->updateCameraNormal();
}

void DrawCurveContext::updateCameraNormal() {
    m_cameraNormal =
        MVector(0.0, 0.0, 1.0); // Maya camera starts looking down positive Z
    m_cameraNormal *=
        m_pathCamera.inclusiveMatrix(); // Transform by world matrix of camera.
}

MPoint DrawCurveContext::planeLineIntersection(const MPoint &lineA,
                                               const MPoint &lineB,
                                               const MPoint &planeP,
                                               const MVector &planeN) {
    MVector v = lineB - lineA;
    return lineA + v * ((planeN * (planeP - lineA)) / (planeN * v));
}

MStatus DrawCurveContext::createCurve() {
    MStatus status;

    // We don't edit an existing curve, we delete the old curve and create a new
    // one.
    status = deleteCurve();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Create a NURBS curve
    MFnNurbsCurve fnCurve;
    MObject oCurve = fnCurve.createWithEditPoints(
        m_editPoints, 3, MFnNurbsCurve::kOpen, false, true, true,
        MObject::kNullObj, // No parent - create new transform
        &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Get DAG path to curve and store it so we can delete the curve later
    // Don't hold onto MObjects in your plugins - may not be valid
    MFnDagNode fnDag(oCurve, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = fnDag.getPath(m_pathCurve);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    m_view.refresh(false, true);

    return MS::kSuccess;
}

MStatus DrawCurveContext::deleteCurve() {
    MStatus status;

    if (m_pathCurve.isValid()) {
        // Use command to delete curve
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "delete %s",
                 m_pathCurve.partialPathName().asChar());
        status = MGlobal::executeCommand(buffer);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    return MS::kSuccess;
}
