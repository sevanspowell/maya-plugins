#include <maya/MFnPlugin.h>

#include "HelloWorldCmd.h"

void *HelloWorld::creator() { return new HelloWorld; }

MStatus HelloWorld::doIt(const MArgList &argList) {
    MGlobal::displayInfo("Hello Chad Vernon!");
    return MS::kSuccess;
}

MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "Samuel Evans-Powell", "1.0", "Any");
    MStatus status = plugin.registerCommand("helloWorld", HelloWorld::creator);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);
    MStatus status = plugin.deregisterCommand("helloWorld");
    CHECK_MSTATUS_AND_RETURN_IT(status);
    return status;
}
