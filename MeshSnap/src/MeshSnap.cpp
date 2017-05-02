#include <cstdint>

#include "MeshSnap.h"

MTypeId MeshSnap::id(0x00000426);
MObject MeshSnap::aSnapMesh;
MObject MeshSnap::aMapping;

void *MeshSnap::creator() { return new MeshSnap(); }

MStatus MeshSnap::initialize() {
    MFnTypedAttribute typedAttribute;

    aSnapMesh = typedAttribute.create("snapMesh", "snapMesh", MFnData::kMesh);
    addAttribute(aSnapMesh);
    attributeAffects(aSnapMesh, outputGeom);

    aMapping = typedAttribute.create("vertexMapping", "vertexMapping",
                                     MFnData::kIntArray);
    typedAttribute.setHidden(true);
    typedAttribute.setConnectable(false);
    addAttribute(aMapping);
    attributeAffects(aMapping, outputGeom);

    return MS::kSuccess;
}

MStatus MeshSnap::deform(MDataBlock &data, MItGeometry &itGeo,
                         const MMatrix &localToWorldMatrix,
                         unsigned int geomIndex) {
    MStatus status;

    float env = data.inputValue(envelope).asFloat();

    // Get the snap mesh
    MObject oMesh = data.inputValue(aSnapMesh).asMesh();

    // Get the vertex mapping
    MObject oMapData = data.inputValue(aMapping).data();

    // Can't perform deformation in these cases
    if (oMesh.isNull() || env == 0.0f || oMapData.isNull()) {
        return MS::kSuccess;
    }

    MFnIntArrayData intData(oMapData, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MIntArray vertexMapping = intData.array();

    // Get vertices to snap to from snap mesh
    MFnMesh fnMesh(oMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MPointArray snapVertices;
    fnMesh.getPoints(snapVertices, MSpace::kWorld);
    unsigned int numSnapVerts = snapVertices.length();

    MMatrix worldToLocalMatrix = localToWorldMatrix.inverse();
    MPoint pt;
    int snapVertexIndex;
    for (itGeo.reset(); !itGeo.isDone(); itGeo.next()) {
        pt              = itGeo.position();
        snapVertexIndex = vertexMapping[itGeo.index()];
        if (snapVertexIndex != -1) {
            // Transform snap point from world-space to local space and perform
            // snapping in local space (itGeo.setPosition performed in local
            // space)
            pt = pt +
                 (((snapVertices[snapVertexIndex] * worldToLocalMatrix) - pt) *
                  env);
        }
        itGeo.setPosition(pt);
    }

    return MS::kSuccess;
}
