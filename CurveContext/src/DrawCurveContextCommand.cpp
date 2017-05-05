#include "DrawCurveContextCommand.h"

void *DrawCurveContextCommand::creator() {
    return new DrawCurveContextCommand();
}

MStatus DrawCurveContextCommand::doEditFlags() {
    MStatus status;

    MArgParser argData = parser();

    if (argData.isFlagSet(LENGTH_FLAG)) {
        float length = (float)argData.flagArgumentDouble(
            DrawCurveContextCommand::LENGTH_FLAG, 0, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        m_context->setLength(length);
    }

    return MS::kSuccess;
}

MStatus DrawCurveContextCommand::doQueryFlags() {
    MArgParser argData = parser();

    if (argData.isFlagSet(DrawCurveContextCommand::LENGTH_FLAG)) {
        setResult(m_context->getLength());
    }

    return MS::kSuccess;
}

MStatus DrawCurveContextCommand::appendSyntax() {
    MStatus status;
    MSyntax mSyntax = syntax();

    status = mSyntax.addFlag(DrawCurveContextCommand::LENGTH_FLAG,
                             DrawCurveContextCommand::LENGTH_FLAG_LONG,
                             MSyntax::kDouble);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}

MPxContext *DrawCurveContextCommand::makeObj() {
    m_context = new DrawCurveContext();

    return m_context;
}
