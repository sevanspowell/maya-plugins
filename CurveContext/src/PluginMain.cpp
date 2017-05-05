#include "DrawCurveContext.h"
#include "DrawCurveContextCommand.h"

#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj, "Samuel Evans-Powell", "1.0", "Any");

    status = plugin.registerContextCommand("drawCurveContext", DrawCurveContextCommand::creator, "drawCurveTool", DrawCurveToolCommand::creator);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Create a new context when you load the plugin
    MGlobal::executeCommand("drawCurveContext drawCurveContext1;");

    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterContextCommand("drawCurveContext");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}
