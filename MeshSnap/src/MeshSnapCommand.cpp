#include <cstdio>

#include "MeshSnap.h"
#include "MeshSnapCommand.h"

MStatus MeshSnapCommand::doIt(const MArgList &argList) {
    MStatus status;

    // Read all the flag arguments
    MArgDatabase argData(syntax(), argList,
                         &status); // syntax() calls newSyntax
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MSelectionList selection;
    status = argData.getObjects(selection);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = selection.getDagPath(0, m_pathBaseMesh);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = selection.getDagPath(1, m_pathSnapMesh);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = MeshSnapCommand::getShapeNode(m_pathBaseMesh);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = MeshSnapCommand::getShapeNode(m_pathSnapMesh);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = calculateVertexMapping();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (argData.isFlagSet("-n")) {
        m_name = argData.flagArgumentString("-n", 0, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    } else {
        m_name = "meshSnap#";
    }

    // Fill DG modifier with commands
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "deformer -type meshSnap -n \"%s\" %s",
             m_name.asChar(), m_pathBaseMesh.partialPathName().asChar());
    status = m_dgMod.commandToExecute(buffer);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return redoIt();
}

MStatus MeshSnapCommand::calculateVertexMapping() {
    MStatus status;

    MFnMesh fnBaseMesh(m_pathBaseMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MPointArray basePoints;
    status = fnBaseMesh.getPoints(basePoints, MSpace::kWorld);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    unsigned int numBasePoints = basePoints.length();
    // Accelerated object for finding closest point to a mesh
    MMeshIntersector intersector;
    MObject oBaseMesh = m_pathBaseMesh.node();
    // Inclusive matrix is all transforms in the path, the inverse of this
    // matrix will transform to local space
    intersector.create(oBaseMesh, m_pathBaseMesh.inclusiveMatrixInverse());


    MFnMesh fnSnapMesh(m_pathSnapMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MPointArray snapPoints;
    status = fnBaseMesh.getPoints(snapPoints, MSpace::kWorld);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MPointOnMesh meshPoint;

    MIntArray vertexList;

    double minDistance;
    int closestVertId, faceIndex;
    double thisDistance;

    m_vertexMapping =
        MIntArray(numBasePoints,
                  -1); // -1 means vertex has no corresponding vertex to map to
    MDoubleArray distances(numBasePoints, 9999999.0);

    for (unsigned int ii = 0; ii < snapPoints.length(); ++ii) {
        // Get point on base mesh closest to snap mesh point
        status = intersector.getClosestPoint(snapPoints[ii], meshPoint);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // Get list of vertices that make up the face that the closest point
        // rests on
        faceIndex = meshPoint.faceIndex();
        fnBaseMesh.getPolygonVertices(faceIndex, vertexList);

        // Find the closest of the vertices
        minDistance = 9999999.0;
        for (unsigned int jj = 0; jj < vertexList.length(); ++jj) {
            // Get distance from potential closest point to snap point
            thisDistance = basePoints[vertexList[jj]].distanceTo(snapPoints[ii]);
            if (thisDistance < minDistance) {
                minDistance   = thisDistance;
                closestVertId = vertexList[jj];
            }
            if (minDistance < 0.00001) {
                // Vertex right on top so use it
                break;
            }
        }

        if (m_vertexMapping[closestVertId] != -1 &&
            minDistance > distances[closestVertId]) {
            // This vertex has already been assigned a closer vertex - covers
            // the case that a given vertex is the closest to multiple potential
            // snap points
            continue;
        }

        m_vertexMapping[closestVertId] = ii;
        distances[closestVertId]       = minDistance;
    }

    return MS::kSuccess;
}

MStatus MeshSnapCommand::redoIt() {
    MStatus status;

    // Create the deformer
    status = m_dgMod.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnIntArrayData fnIntData;
    MObject oData = fnIntData.create(m_vertexMapping, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Traverse dependency graph of base mesh to find snap deformer
    MObject oSnapDeformer;
    status = getSnapDeformerFromBaseMesh(oSnapDeformer);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Put vertex mapping data into attribute
    MPlug plugVertexMapping(oSnapDeformer, MeshSnap::aMapping);
    plugVertexMapping.setMObject(oData);

    // Get world mesh of the snap mesh object
    MFnDagNode fnSnapMesh(m_pathSnapMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MPlug plugWorldMesh = fnSnapMesh.findPlug("worldMesh", false, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    // NOTE: world mesh is array, we just use first index
    status = plugWorldMesh.selectAncestorLogicalIndex(0, plugWorldMesh.attribute());
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Fill the snap mesh attribute with the snap mesh object data
    MPlug plugSnapMesh(oSnapDeformer, MeshSnap::aSnapMesh);
    MDGModifier dgMod;
    status = dgMod.connect(plugWorldMesh, plugSnapMesh);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = dgMod.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}

MStatus MeshSnapCommand::undoIt() {
    MStatus status;

    // Restore the initial state
    status = m_dgMod.undoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}

bool MeshSnapCommand::isUndoable() const { return true; }

void *MeshSnapCommand::creator() { return new MeshSnapCommand(); }

MSyntax MeshSnapCommand::newSyntax() {
    MSyntax syntax;

    // Name to give mesh snap deformer node
    syntax.addFlag("-n", "-name", MSyntax::kString);
    // Arguments to command, in this case the mesh to snap and the mesh to snap
    // to
    syntax.setObjectType(MSyntax::kSelectionList, 2, 2);
    // Allow the user to select these arguments, rather than having to type in
    // their names
    syntax.useSelectionAsDefault(true);

    syntax.enableEdit(false);
    syntax.enableQuery(false);

    return syntax;
}

MStatus MeshSnapCommand::getShapeNode(MDagPath &path) {
    MStatus status;

    // If already a mesh, don't need to do anything
    if (path.apiType() == MFn::kMesh) {
        return MS::kSuccess;
    }

    // Get shape nodes of transform
    unsigned int numShapes;
    status = path.numberOfShapesDirectlyBelow(numShapes);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Find correct shape node
    for (unsigned int i = 0; i < numShapes; ++i) {
        status = path.extendToShapeDirectlyBelow(i);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        if (!path.hasFn(MFn::kMesh)) {
            // Not a mesh - keep looking
            path.pop(); // Go 'up' - back to transform node
            continue;
        }

        // Is a mesh...
        // Is it an intermediate object?
        MFnDagNode fnNode(path, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        if (!fnNode.isIntermediateObject()) {
            // Not an intermediate object, therefore is the shape node we're
            // looking for
            return MS::kSuccess;
        }
        path.pop();
    }

    return MS::kFailure;
}

MStatus MeshSnapCommand::getSnapDeformerFromBaseMesh(MObject &oSnapDeformer) {
    MStatus status;

    MObject oBaseMesh = m_pathBaseMesh.node();
    MItDependencyGraph itGraph(oBaseMesh, MFn::kInvalid, // No filter
                               MItDependencyGraph::kUpstream,
                               MItDependencyGraph::kDepthFirst,
                               MItDependencyGraph::kNodeLevel, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    while (!itGraph.isDone()) {
        oSnapDeformer = itGraph.currentItem();

        MFnDependencyNode fnNode(oSnapDeformer, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // If is a mesh snap deformer
        if (fnNode.typeId() == MeshSnap::id) {
            // Found our mesh snap deformer!
            return MS::kSuccess;
        }

        itGraph.next();
    }

    return MS::kFailure;
}
