#pragma once

#include <maya/M3dView.h>
#include <maya/MArgList.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MStatus.h>
#include <maya/MMessage.h>
#include <maya/MUiMessage.h>

#include <maya/MPxContext.h>
#include <maya/MPxToolCommand.h>

class DrawCurveContext;

class DrawCurveToolCommand : public MPxToolCommand {
  public:
    DrawCurveToolCommand() { setCommandString("drawCurveTool"); };
    virtual ~DrawCurveToolCommand(){};
    static void *creator();

    virtual MStatus doIt(const MArgList &args) override;
    virtual MStatus redoIt() override;
    virtual MStatus undoIt() override;
    virtual bool isUndoable() const override;
    virtual MStatus finalize() override;

    void setEditPoints(MPointArray &editPoints);

  private:
    MPointArray m_editPoints;
    MDagPath m_pathCurve;
};

class DrawCurveContext : public MPxContext {
  public:
    DrawCurveContext() : m_length(1.0f), m_intersects(false){};
    virtual ~DrawCurveContext(){};

    virtual void toolOnSetup(MEvent &event) override;
    virtual void toolOffCleanup() override;
    virtual void getClassName(MString &name) const override;

    virtual MStatus doPress(MEvent &event) override;
    virtual MStatus doDrag(MEvent &event) override;
    virtual MStatus doRelease(MEvent &event) override;

    float getLength() const;
    void setLength(float length);

    static void postRenderCallback(const MString &panelName, void *data);

    void updateCameraNormal();

  private:
    MStatus getSelection(MDagPathArray &geometry);
    MStatus getMeshIntersection(short mouseX, short mouseY);
    MPoint planeLineIntersection(const MPoint &lineA, const MPoint &lineB,
                                 const MPoint &planeP, const MVector &planeN);
    MStatus createCurve();
    MStatus deleteCurve();

    float m_length;
    MDagPathArray m_geometry;
    M3dView m_view;
    bool m_intersects;
    MPoint m_intersectionPoint;
    MVector m_cameraNormal;
    MDagPath m_pathCamera;

    double m_distanceFromLastEditPoint;
    MPoint m_lastProjectedPoint;
    MPointArray m_editPoints;

    MDagPath m_pathCurve;

    MCallbackId m_postRenderId;
};
