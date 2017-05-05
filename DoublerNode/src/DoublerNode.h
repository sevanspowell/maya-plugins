#pragma once

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MStatus.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MPxNode.h>

/**
 * A node that simply doubles the input value.
 *
 * Attributes:
 * input (in) - float
 * output (out) - float
 */
class DoublerNode : public MPxNode {
public:
    DoublerNode() {};
    virtual ~DoublerNode() {};
    static void *creator();
    static MStatus initialize();
    virtual MStatus compute(const MPlug &plug, MDataBlock &data) override;

    static MTypeId id;
    static MObject input;
    static MObject output;
};
