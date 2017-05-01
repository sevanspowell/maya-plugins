#include <maya/MFnPlugin.h>

#include "BulgeDeformer.h"

MTypeId BulgeDeformer::id(0x00000423);
MObject BulgeDeformer::aBulgeAmount;

void *BulgeDeformer::creator() { return new BulgeDeformer; }

MStatus BulgeDeformer::initialize() {
    MStatus status;

    MFnNumericAttribute numericAttribute;

    aBulgeAmount =
        numericAttribute.create("bulgeAmount", "amt", MFnNumericData::kFloat);
    numericAttribute.setKeyable(true);
    addAttribute(aBulgeAmount);
    attributeAffects(aBulgeAmount, outputGeom);

    MGlobal::executeCommand(
        "makePaintable -attrType multiFloat -sm deformer bulgeMesh weights;");

    return MS::kSuccess;
}

MStatus BulgeDeformer::deform(MDataBlock &data, MItGeometry &itGeo,
                              const MMatrix &localToWorldMatrix,
                              unsigned int geomIndex) {
    MStatus status;

    // We use output*Value instead of input*Value here because we don't want
    // Maya to perform a dependency graph (DG) evaluation, we already know the
    // value returned from the output*Value functions is valid for reading
    // because the MPxDeformerNode already performs a DG evaluation for us in
    // it's compute function.
    // From the documentation: "When implementing the compute method for a
    // deformer, another consideration is that the input geometry attribute is
    // not cached. This means that all of the inputs will evaluate each time
    // MDataBlock::inputArrayValue is called on "inputGeom". If you only want a
    // single inputGeometry, you can prevent unneeded evaluations by avoiding
    // calls to MDataBlock.inputArrayValue. For example, use the technique shown
    // in the above example or use MDataBlock::outputArrayValue."
    // (http://download.autodesk.com/us/maya/2011help/api/class_m_px_deformer_node.html)

    // Get handle to array
    MArrayDataHandle hInput = data.outputArrayValue(input, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    // Move array handle to appropriate element
    status = hInput.jumpToElement(geomIndex);
    // Get data handle to array element
    MDataHandle hInputElement = hInput.outputValue(&status);
    // Get input geometry of array element
    MObject oInputGeom = hInputElement.child(inputGeom).asMesh();

    MFnMesh fnMesh(oInputGeom, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MFloatVectorArray normals;
    fnMesh.getVertexNormals(false, normals);

    // Perform deformation
    float bulgeAmount = data.inputValue(aBulgeAmount).asFloat();
    float env = data.inputValue(envelope).asFloat();
    MPoint point;
    float w;
    for (; !itGeo.isDone(); itGeo.next()) {
        w = weightValue(data, geomIndex, itGeo.index());

        point = itGeo.position();

        // Deformation algorithm
        point += normals[itGeo.index()] * bulgeAmount * w * env;

        itGeo.setPosition(point);
    }

    return MS::kSuccess;
}

MStatus initializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj, "Samuel Evans-Powell", "1.0", "Any");

    // Specify we are making a deformer node
    status = plugin.registerNode(
        "bulgeMesh", BulgeDeformer::id, BulgeDeformer::creator,
        BulgeDeformer::initialize, MPxNode::kDeformerNode);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterNode(BulgeDeformer::id);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}
