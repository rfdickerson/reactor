#include "../core/ModelIO.hpp"

#include <spdlog/spdlog.h>

int main()
{
    // Define input and output paths
    const std::string inputModel = "../workspace/monkey.obj";
    const std::string outputModel = "./monkey.mesh";

    // Run the import and export process
    if(reactor::importAndExport(inputModel, outputModel)) {
        // Optional: Test the loader to verify the export
        spdlog::info("--- Verifying export by loading file back ---");
        reactor::loadModelFromBinary(outputModel);
    }

    return 0;
}
