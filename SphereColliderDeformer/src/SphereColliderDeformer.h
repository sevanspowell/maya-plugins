#pragma once

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MItGeometry.h>
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>
#include <maya/MStatus.h>
#include <maya/MDagModifier.h>

#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MPxDeformerNode.h>

/**
 * A node that allows you to deform a mesh by 'colliding' it with a sphere.
 *
 * Name:
 *   sphereCollide 
 */
class SphereColliderDeformer : public MPxDeformerNode {
  public:
    SphereColliderDeformer(){};
    virtual ~SphereColliderDeformer(){};
    static void *creator();
    static MStatus initialize();

    virtual MStatus deform(MDataBlock &data, MItGeometry &itGeo,
                           const MMatrix &localToWorldMatrix,
                           unsigned int geomIndex) override;
    virtual MObject &accessoryAttribute() const override;
    virtual MStatus accessoryNodeSetup(MDagModifier &dagModifier) override;

    static MTypeId id;
    static MObject aCollideMatrix;
};
