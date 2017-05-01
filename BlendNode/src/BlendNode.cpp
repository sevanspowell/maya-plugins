#include <maya/MFnPlugin.h>

#include "BlendNode.h"

MTypeId BlendNode::id(0x00000002);
MObject BlendNode::blendMesh;
MObject BlendNode::blendWeight;

void *BlendNode::creator() { return new BlendNode; }

MStatus BlendNode::initialize() {
    MFnTypedAttribute typedAttribute;
    MFnNumericAttribute numericAttribute;

    blendMesh = typedAttribute.create("blendMesh", "blendMesh", MFnData::kMesh);
    addAttribute(blendMesh);
    attributeAffects(blendMesh,
                     outputGeom); // outputGeom is provided by MPxDeformerNode

    blendWeight =
        numericAttribute.create("blendWeight", "bw", MFnNumericData::kFloat);
    numericAttribute.setKeyable(true);
    numericAttribute.setMin(0.0);
    numericAttribute.setMax(1.0);
    addAttribute(blendWeight);
    attributeAffects(blendWeight, outputGeom);

    // Make the deformer weights paintable
    MGlobal::executeCommand(
        "makePaintable -attrType multiFloat -sm deformer blendNode weights;");

    return MS::kSuccess;
}

MStatus BlendNode::deform(MDataBlock &data, MItGeometry &itGeo,
                                  const MMatrix &localToWorldMatrix,
                                  unsigned int geomIndex) {
    MStatus status;

    // Get the envelope and blend weight. The envelope is a magnifier provided
    // by the MPxDeformerNode that allows user to scale deformation.
    float env = data.inputValue(envelope).asFloat();
    float bw = data.inputValue(blendWeight).asFloat();
    bw *= env;

    // Get the blend mesh
    MObject mesh = data.inputValue(blendMesh).asMesh();
    if (mesh.isNull()) {
        // No blend mesh attached, do nothing.
        return MS::kSuccess;
    }

    // Get the blend points
    MFnMesh fnBlendMesh(mesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MPointArray blendPoints;
    fnBlendMesh.getPoints(blendPoints);

    // Iterate over all points in mesh and perform the deformation
    MPoint pt;
    float w = 0.0f;
    for (; !itGeo.isDone(); itGeo.next()) {
        // Get the input point
        pt = itGeo.position();
        // Get the painted weight value
        w = weightValue(data, geomIndex, itGeo.index());

        // Perform the deformation
        pt = pt + (blendPoints[itGeo.index()] - pt) * bw * w;

        // Set the new output point
        itGeo.setPosition(pt);
    }

    // No need to set clean flag, MPxDeformerNode does this for us
    return MS::kSuccess;
}

MStatus initializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj, "Samuel Evans-Powell", "1.0", "Any");

    // Specify we are making a deformer node
    status = plugin.registerNode("blendNode", BlendNode::id, BlendNode::creator,
                                 BlendNode::initialize, MPxNode::kDeformerNode);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterNode(BlendNode::id);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}
