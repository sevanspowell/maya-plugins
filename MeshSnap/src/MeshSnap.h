#pragma once

#include <maya/MDagModifier.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MGlobal.h>
#include <maya/MItGeometry.h>
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>
#include <maya/MStatus.h>

#include <maya/MFnIntArrayData.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MPxDeformerNode.h>

/**
 * Snap the vertices of one mesh to another's.
 *
 * Name:
 *   meshSnap
 *
 * Attributes:
 *   snapMesh (snapMesh) - Mesh to snap to.
 *   mappint (mapping) - Used to decide which vertex on the snap mesh should
 *   correspond to which vertex on the snapped mesh.
 */
class MeshSnap : public MPxDeformerNode {
  public:
    MeshSnap(){};
    virtual ~MeshSnap(){};
    static void *creator();
    static MStatus initialize();

    virtual MStatus deform(MDataBlock &data, MItGeometry &itGeo,
                           const MMatrix &localToWorldMatrix,
                           unsigned int geomIndex) override;

    static MTypeId id;
    static MObject aSnapMesh;
    static MObject aMapping;
};
