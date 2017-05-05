#pragma once

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MItGeometry.h>
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>
#include <maya/MStatus.h>

#include <maya/MFnMesh.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MPxDeformerNode.h>

/**
 * A node that blends two meshes according to a weight.
 *
 * Attributes:
 *   blendMesh (bm) - Mesh 
 *   blendWeight (bw) - float
 */
class BlendNode : public MPxDeformerNode {
  public:
    BlendNode(){};
    virtual ~BlendNode(){};
    static void *creator();
    static MStatus initialize();
    virtual MStatus deform(MDataBlock &data, MItGeometry &itGeo,
                           const MMatrix &localToWorldMatrix,
                           unsigned int geomIndex) override;

    static MTypeId id;
    static MObject blendMesh;
    static MObject blendWeight;
};
