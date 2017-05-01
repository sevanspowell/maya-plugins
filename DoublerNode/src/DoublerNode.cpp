#include "DoublerNode.h"

#include <maya/MFnPlugin.h>

MTypeId DoublerNode::id(0x00000001);
MObject DoublerNode::input;
MObject DoublerNode::output;

void *DoublerNode::creator() { return new DoublerNode; }

MStatus DoublerNode::initialize() {
    MFnNumericAttribute numericAttribute;

    output = numericAttribute.create("output", "out", MFnNumericData::kFloat);
    numericAttribute.setWritable(false);
    numericAttribute.setStorable(false);
    addAttribute(output);

    input = numericAttribute.create("input", "in", MFnNumericData::kFloat);
    numericAttribute.setKeyable(true);
    addAttribute(input);
    attributeAffects(input, output);

    return MS::kSuccess;
}

// Called when a node attribute's flag is dirty and Maya requests an output
// value from the node.
MStatus DoublerNode::compute(const MPlug &plug, MDataBlock &data) {
    // Check that the data we are being asked to calculate is the output attrib.
    if (plug != output) {
        return MS::kUnknownParameter;
    }

    // Get the input
    float inputValue = data.inputValue(input).asFloat();

    // Double it
    inputValue *= 2.0f;

    // Set the output
    MDataHandle outputValueHandle = data.outputValue(output);
    outputValueHandle.setFloat(inputValue);

    // Indicate to Maya that this attribute is no longer dirty
    data.setClean(plug);

    return MS::kSuccess;
}

MStatus initializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj, "Samuel Evans-Powell", "1.0", "Any");

    status = plugin.registerNode("doublerNode", DoublerNode::id,
                                 DoublerNode::creator, DoublerNode::initialize);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterNode(DoublerNode::id);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}
