#pragma once

#include <maya/MArgDatabase.h>
#include <maya/MDGModifier.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MMeshIntersector.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MSyntax.h>

#include <maya/MPxCommand.h>

class MeshSnapCommand : public MPxCommand {
  public:
    MeshSnapCommand(){};
    ~MeshSnapCommand(){};
    virtual MStatus doIt(const MArgList &argList);
    virtual MStatus redoIt();
    virtual MStatus undoIt();
    virtual bool isUndoable() const;
    static void *creator();
    static MSyntax newSyntax();

  private:
    /**
     * Attempt to move the MDagPath to a shape node by first checking the
     * current path, then checking all of it's children.
     */
    static MStatus getShapeNode(MDagPath &path);
    /**
     * Calculate the vertex mapping between the base mesh and the snap mesh.
     */
    MStatus calculateVertexMapping();
    MStatus getSnapDeformerFromBaseMesh(MObject &oSnapDeformer);

    MDagPath m_pathBaseMesh;
    MDagPath m_pathSnapMesh;
    MIntArray m_vertexMapping;
    MString m_name;
    MDGModifier m_dgMod;
};
