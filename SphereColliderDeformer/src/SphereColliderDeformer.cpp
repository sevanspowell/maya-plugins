#include <maya/MFnPlugin.h>

#include "SphereColliderDeformer.h"

MTypeId SphereColliderDeformer::id(0x00000424);
MObject SphereColliderDeformer::aCollideMatrix;

void *SphereColliderDeformer::creator() { return new SphereColliderDeformer; }

MStatus SphereColliderDeformer::initialize() {
    MStatus status;

    MFnMatrixAttribute matrixAttribute;

    aCollideMatrix = matrixAttribute.create("collideMatrix", "col");
    addAttribute(aCollideMatrix);
    attributeAffects(aCollideMatrix, outputGeom);

    return MS::kSuccess;
}

MStatus SphereColliderDeformer::deform(MDataBlock &data, MItGeometry &itGeo,
                                       const MMatrix &localToWorldMatrix,
                                       unsigned int geomIndex) {
    MStatus status;

    float env                    = data.inputValue(envelope).asFloat();
    MMatrix collideMatrix        = data.inputValue(aCollideMatrix).asMatrix();
    MMatrix collideMatrixInverse = collideMatrix.inverse();
    MMatrix worldToLocalMatrix   = localToWorldMatrix.inverse();

    MPoint point;
    for (; !itGeo.isDone(); itGeo.next()) {
        point = itGeo.position(); // These positions are in local space (always
                                  // true for deform method)
        point *= localToWorldMatrix; // Now point is in world space
        point *=
            collideMatrixInverse; // Put point into coordinate space of locator

        // Measure length of point (remember we're now in coordinate space of
        // locator)
        float length = MVector(point).length();
        if (length < 1.0f && length > 0.0f) {
            // Point is within collider sphere

            // Normalize point vector so that point is exactly 1 unit away from
            // the origin. In a unit sphere this means that the point is forced
            // to the edge of the sphere.
            point = MVector(point).normal();

            point *= collideMatrix;      // Put point back into world space
            point *= worldToLocalMatrix; // Put point back into object space

            itGeo.setPosition(point);
        } else if (length == 0.0f) {
            // Don't try to normalize vector of 0 length
            return MS::kSuccess;
        }
    }

    return MS::kSuccess;
}

// Return which attribute is our accessory attribute
MObject &SphereColliderDeformer::accessoryAttribute() const {
    return aCollideMatrix;
}

MStatus SphereColliderDeformer::accessoryNodeSetup(MDagModifier &dagModifier) {
    MStatus status;

    // Create locator (x, y, z axes)
    MObject oLocator =
        dagModifier.createNode("locator", MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Get plug of world matrix of locator (note that the worldMatrix plug is an
    // array, objects may have multiple worldMatrices - we just use the first at
    // index 0).
    MFnDependencyNode fnLocator(oLocator, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MPlug plugWorldMatrix = fnLocator.findPlug("worldMatrix", false, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = plugWorldMatrix.selectAncestorLogicalIndex(
        0, plugWorldMatrix.attribute());

    // Get plug of collide matrix attribute
    MObject oThis = thisMObject();
    MPlug plugCollideMatrix(oThis, aCollideMatrix);

    // Connect the locator's world matrix plug to the collide matrix plug
    status = dagModifier.connect(plugWorldMatrix, plugCollideMatrix);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // MDagModifiers usually require us to call the 'doIt' method to actually
    // execute the operations given to the modifier. However, because we are
    // handed this MDagModifier, we just need to 'fill it up'.

    return MS::kSuccess;
}

MStatus initializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj, "Samuel Evans-Powell", "1.0", "Any");

    // Specify we are making a deformer node
    status = plugin.registerNode("sphereCollide", SphereColliderDeformer::id,
                                 SphereColliderDeformer::creator,
                                 SphereColliderDeformer::initialize,
                                 MPxNode::kDeformerNode);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterNode(SphereColliderDeformer::id);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}
