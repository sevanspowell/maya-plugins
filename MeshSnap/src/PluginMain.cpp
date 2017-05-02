#include "MeshSnap.h"
#include "MeshSnapCommand.h"

#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj, "Samuel Evans-Powell", "1.0", "Any");

    status = plugin.registerCommand("meshSnap", MeshSnapCommand::creator,
                                    MeshSnapCommand::newSyntax);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.registerNode("meshSnap", MeshSnap::id, MeshSnap::creator,
                                 MeshSnap::initialize, MPxNode::kDeformerNode);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterCommand("meshSnap");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.deregisterNode(MeshSnap::id);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}
