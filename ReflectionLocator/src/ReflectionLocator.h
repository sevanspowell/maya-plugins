#pragma once

#include <maya/MArgList.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFloatPointArray.h>
#include <maya/MGlobal.h>
#include <maya/MGlobal.h>
#include <maya/MMatrix.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MStatus.h>
#include <maya/MTypeId.h>
#include <maya/MVector.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>

#include <maya/MPxLocatorNode.h>

/**
 * A locator that draws the line of reflection from a locator, to this locator,
 * with this locator acting as a 'mirror'.
 *
 * Name:
 *   reflection
 *
 * Attributes:
 *   planeMatrix [in] -
 *   point [in] -
 *   scale [in] -
 *   reflectedParentInverse [in] -
 *
 *   reflectedPoint [out] -
 */
class ReflectionLocator : public MPxLocatorNode {
  public:
    ReflectionLocator();
    virtual void postConstructor() override;
    virtual ~ReflectionLocator();

    virtual MStatus compute(const MPlug &plug, MDataBlock &data) override;
    virtual void draw(M3dView &view, const MDagPath &dagPath,
                      M3dView::DisplayStyle style,
                      M3dView::DisplayStatus status) override;
    virtual bool isBounded() const override;
    virtual bool isTransparent() const override;
    virtual MBoundingBox boundingBox() const override;

    void drawDisc(float radius, int divisions, bool filled);
    void drawReflection(const MPoint &src, const MPoint &dst);

    static void *creator();
    static MStatus initialize();

    static MObject aPlaneMatrix;
    static MObject aPoint;
    static MObject aReflectedPoint;
    static MObject aReflectedParentInverse;
    static MObject aScale;

    static MTypeId id;

  private:
    MPoint m_srcPoint, m_dstPoint, m_planePoint;
};
