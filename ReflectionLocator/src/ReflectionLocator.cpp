#include <cmath>

#include <maya/MFnPlugin.h>

#include "ReflectionLocator.h"

MTypeId ReflectionLocator::id(0x00000425);

MObject ReflectionLocator::aPlaneMatrix;
MObject ReflectionLocator::aPoint;
MObject ReflectionLocator::aReflectedPoint;
MObject ReflectionLocator::aReflectedParentInverse;
MObject ReflectionLocator::aScale;

ReflectionLocator::ReflectionLocator() {}

void ReflectionLocator::postConstructor() {
    MObject oThis = thisMObject();
    MFnDependencyNode fnNode(oThis);
    fnNode.setName("reflectionShape#");
}

ReflectionLocator::~ReflectionLocator() {}

MStatus ReflectionLocator::compute(const MPlug &plug, MDataBlock &data) {
    MStatus status;

    if (plug != aReflectedPoint && plug.parent() != aReflectedPoint) {
        return MS::kInvalidParameter;
    }
    MMatrix planeMatrix = data.inputValue(aPlaneMatrix).asMatrix();
    MVector planePos    = MTransformationMatrix(planeMatrix)
                           .getTranslation(MSpace::kPostTransform);
    MMatrix reflectedParentInverse =
        data.inputValue(aReflectedParentInverse).asMatrix();
    MMatrix inputMatrix = data.inputValue(aPoint).asMatrix();
    MVector inputPoint  = MTransformationMatrix(inputMatrix)
                             .getTranslation(MSpace::kPostTransform);
    double scale = data.inputValue(aScale).asDouble();

    // Compute reflection normal using transformation of the plane
    MVector normal(0.0, 1.0, 0.0);
    normal *= planeMatrix;
    normal.normalize();

    // Compute vector to reflect
    MVector L = inputPoint - planePos;

    // Reflect the vector
    MVector reflectedVector = 2 * ((normal * L) * normal) - L;
    reflectedVector.normalize();
    reflectedVector *= scale;

    // Calculate the reflected point position
    m_dstPoint = planePos + reflectedVector;

    // Put into local space
    m_dstPoint *= reflectedParentInverse;

    // Set output
    MDataHandle hOutput = data.outputValue(aReflectedPoint);
    hOutput.set3Float((float)m_dstPoint.x, (float)m_dstPoint.y,
                      (float)m_dstPoint.z);
    hOutput.setClean();
    data.setClean(plug);

    // Revert transform to coordinate space of locator
    MMatrix planeMatrixInverse = planeMatrix.inverse();

    m_srcPoint   = MPoint(inputPoint) * planeMatrixInverse;
    m_planePoint = MPoint(planePos) * planeMatrixInverse;
    m_dstPoint *= planeMatrixInverse;

    return MS::kSuccess;
}

void ReflectionLocator::draw(M3dView &view, const MDagPath &dagPath,
                             M3dView::DisplayStyle style,
                             M3dView::DisplayStatus status) {
    view.beginGL();

    glPushAttrib(GL_CURRENT_BIT);
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Disable depth mask
    glDepthMask(GL_FALSE);

    MColor solidColor, wireColor;
    if (status == M3dView::kActive) {
        // Active but not the primary selection
        solidColor = MColor(1.0f, 1.0f, 1.0f, 0.1f);
        wireColor  = MColor(1.0f, 1.0f, 1.0f, 1.0f);

    } else if (status == M3dView::kLead) {
        // Is the primary selection
        solidColor = MColor(.26f, 1.0f, .64f, 0.1f);
        wireColor  = MColor(.26f, 1.0f, .64f, 1.0f);
    } else {
        // Not selected
        solidColor = MColor(1.0f, 1.0f, 0.0f, 0.1f);
        wireColor  = MColor(1.0f, 1.0f, 0.0f, 1.0f);
    }

    // Draw solid
    glColor4f(solidColor.r, solidColor.g, solidColor.b, solidColor.a);
    drawDisc(1.0f, 32, true); // true = filled

    // Draw wireframe
    glColor4f(wireColor.r, wireColor.g, wireColor.b, wireColor.a);
    drawReflection(m_srcPoint, m_dstPoint);
    drawDisc(1.0f, 32, false); // false = not filled

    // Restore touched states
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glPopAttrib();

    view.endGL();
}

// Maya uses bounding box to determine if locator is selected
bool ReflectionLocator::isBounded() const { return true; }

// True because we are using alpha-blending in our drawing code - so Maya can
// decide draw order (e.g. draw after all opaque geometry)
bool ReflectionLocator::isTransparent() const { return true; }

MBoundingBox ReflectionLocator::boundingBox() const {
    MBoundingBox bbox;

    // Expands the bounding box to contain the given point
    bbox.expand(m_srcPoint);
    bbox.expand(m_dstPoint);
    bbox.expand(m_planePoint);
    // Make sure it includes size of disc
    bbox.expand(m_planePoint + MVector(1.0, 0.0, 0.0));
    bbox.expand(m_planePoint + MVector(-1.0, 0.0, 0.0));
    bbox.expand(m_planePoint + MVector(0.0, 0.0, 1.0));
    bbox.expand(m_planePoint + MVector(0.0, 0.0, -1.0));

    return bbox;
}

void ReflectionLocator::drawDisc(float radius, int divisions, bool filled) {
    int renderState = filled ? GL_POLYGON : GL_LINE_LOOP;

    // Calculate points of circle
    float degreesPerDiv = 360.0f / divisions;
    float radiansPerDiv = degreesPerDiv * (M_PI / 180.0f);
    MFloatPointArray points(divisions);
    for (uint32_t i = 0; i < divisions; ++i) {
        float angle = i * radiansPerDiv;
        float x     = cos(angle) * radius;
        float z     = sin(angle) * radius;
        points[i].x = x;
        points[i].z = z;
    }

    // Draw disc
    glBegin(renderState);
    for (uint32_t i = 0; i < divisions; ++i) {
        // Drawing is in locator space
        glVertex3f(points[i].x, 0.0f, points[i].z);
    }
    glEnd();
}

void ReflectionLocator::drawReflection(const MPoint &src, const MPoint &dst) {
    glBegin(GL_LINES);
    // Drawing is in locator space
    glVertex3f((float)src.x, (float)src.y, (float)src.z);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f((float)dst.x, (float)dst.y, (float)dst.z);
    glEnd();
}

MStatus ReflectionLocator::initialize() {
    MFnMatrixAttribute matrixAttribute;
    MFnNumericAttribute numericAttribute;

    // Reflected point output
    aReflectedPoint =
        numericAttribute.createPoint("reflectedPoint", "reflectedPoint");
    numericAttribute.setWritable(false);
    numericAttribute.setStorable(false);
    addAttribute(aReflectedPoint);

    aPlaneMatrix = matrixAttribute.create("planeMatrix", "planeMatrix");
    addAttribute(aPlaneMatrix);
    attributeAffects(aPlaneMatrix, aReflectedPoint);

    aPoint = matrixAttribute.create("point", "point");
    addAttribute(aPoint);
    attributeAffects(aPoint, aReflectedPoint);

    aReflectedParentInverse = matrixAttribute.create("reflectedParentInverse",
                                                     "reflectedParentInverse");
    matrixAttribute.setDefault(MMatrix::identity);
    addAttribute(aReflectedParentInverse);
    attributeAffects(aReflectedParentInverse, aReflectedPoint);

    aScale =
        numericAttribute.create("scale", "scale", MFnNumericData::kDouble, 1.0);
    numericAttribute.setKeyable(true);
    addAttribute(aScale);
    attributeAffects(aScale, aReflectedPoint);

    return MS::kSuccess;
}

void *ReflectionLocator::creator() { return new ReflectionLocator(); }

MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "Samuel Evans-Powell", "1.0", "Any");

    MStatus status = plugin.registerNode(
        "reflection", ReflectionLocator::id, ReflectionLocator::creator,
        ReflectionLocator::initialize, MPxNode::kLocatorNode);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);

    MStatus status = plugin.deregisterNode(ReflectionLocator::id);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}
