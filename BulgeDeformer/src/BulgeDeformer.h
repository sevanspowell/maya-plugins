#pragma once

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MItGeometry.h>
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>
#include <maya/MStatus.h>

#include <maya/MFnMesh.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MPxDeformerNode.h>

/**
 * A node that 'pushes' a mesh out along it's normals.
 *
 * Name:
 *   bulgeMesh
 *
 * Attributes:
 *   bulgeAmount (amt) - float
 */
class BulgeDeformer : public MPxDeformerNode {
  public:
    BulgeDeformer(){};
    virtual ~BulgeDeformer(){};
    static void *creator();
    static MStatus initialize();
    virtual MStatus deform(MDataBlock &data, MItGeometry &itGeo,
                           const MMatrix &localToWorldMatrix,
                           unsigned int geomIndex) override;

    static MTypeId id;
    static MObject aBulgeAmount;
};
