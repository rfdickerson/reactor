#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <spdlog/spdlog.h>

#include <fstream>
#include <iostream>
#include <vector>

// Include the Vertex definition from your project
#include "../vulkan/Vertex.hpp"

// A structure to hold mesh data, which is what we'll load back in the engine.
struct MeshData {
    std::vector<reactor::Vertex> vertices;
    std::vector<uint32_t> indices;
};

// Processes the assimp scene and writes it to our custom binary format.
bool processAndExportScene(const aiScene* scene, const std::string& outputPath)
{
    // Open the output file in binary mode.
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        spdlog::error("Failed to open output file for writing: {}", outputPath);
        return false;
    }

    // --- Write File Header ---
    const char magic[8] = "R_MESH";
    uint32_t version = 1;
    uint32_t meshCount = scene->mNumMeshes;

    outFile.write(magic, sizeof(magic));
    outFile.write(reinterpret_cast<const char*>(&version), sizeof(version));
    outFile.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));

    spdlog::info("Exporting {} meshes to {}", meshCount, outputPath);

    // --- Write Each Mesh's Data ---
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* pMesh = scene->mMeshes[i];

        std::vector<reactor::Vertex> vertices;
        std::vector<uint32_t> indices;

        // Extract vertex data
        vertices.reserve(pMesh->mNumVertices);
        for (unsigned int v = 0; v < pMesh->mNumVertices; ++v) {
            reactor::Vertex vertex{};
            vertex.pos = {pMesh->mVertices[v].x, pMesh->mVertices[v].y, pMesh->mVertices[v].z};

            if (pMesh->HasNormals()) {
                vertex.normal = {pMesh->mNormals[v].x, pMesh->mNormals[v].y, pMesh->mNormals[v].z};
            }

            // Set a default color, or extract from mesh if available
            vertex.color = {1.0f, 1.0f, 1.0f};
            if (pMesh->HasVertexColors(0)) {
                vertex.color = {pMesh->mColors[0][v].r, pMesh->mColors[0][v].g, pMesh->mColors[0][v].b};
            }

            if (pMesh->HasTextureCoords(0)) {
                vertex.texCoord = {pMesh->mTextureCoords[0][v].x, pMesh->mTextureCoords[0][v].y};
            }
            vertices.push_back(vertex);
        }

        // Extract index data
        indices.reserve(pMesh->mNumFaces * 3);
        for (unsigned int f = 0; f < pMesh->mNumFaces; ++f) {
            aiFace face = pMesh->mFaces[f];
            // We assume the mesh is triangulated (aiProcess_Triangulate)
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // --- Write Mesh Chunk to File ---
        uint64_t vertexCount = vertices.size();
        uint64_t indexCount = indices.size();

        outFile.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
        outFile.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
        outFile.write(reinterpret_cast<const char*>(vertices.data()), vertexCount * sizeof(reactor::Vertex));
        outFile.write(reinterpret_cast<const char*>(indices.data()), indexCount * sizeof(uint32_t));

        spdlog::info("  - Mesh {}: {} vertices, {} indices", i, vertexCount, indexCount);
    }

    outFile.close();
    spdlog::info("Successfully exported model to {}", outputPath);
    return true;
}

// Example of how you would load the data back in your engine.
std::vector<MeshData> loadModelFromBinary(const std::string& path) {
    std::vector<MeshData> allMeshes;

    std::ifstream inFile(path, std::ios::binary);
    if (!inFile.is_open()) {
        spdlog::error("Failed to open model file for reading: {}", path);
        return allMeshes;
    }

    // --- Read and Validate Header ---
    char magic[8];
    uint32_t version;
    uint32_t meshCount;

    inFile.read(magic, sizeof(magic));
    inFile.read(reinterpret_cast<char*>(&version), sizeof(version));
    inFile.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));

    if (std::string(magic) != "R_MESH" || version != 1) {
        spdlog::error("Invalid model file or version mismatch: {}", path);
        return allMeshes;
    }

    allMeshes.resize(meshCount);
    spdlog::info("Loading {} meshes from {}", meshCount, path);

    // --- Read Each Mesh's Data ---
    for (uint32_t i = 0; i < meshCount; ++i) {
        uint64_t vertexCount;
        uint64_t indexCount;

        inFile.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
        inFile.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));

        allMeshes[i].vertices.resize(vertexCount);
        allMeshes[i].indices.resize(indexCount);

        inFile.read(reinterpret_cast<char*>(allMeshes[i].vertices.data()), vertexCount * sizeof(reactor::Vertex));
        inFile.read(reinterpret_cast<char*>(allMeshes[i].indices.data()), indexCount * sizeof(uint32_t));

        spdlog::info("  - Mesh {}: {} vertices, {} indices", i, vertexCount, indexCount);
    }

    inFile.close();
    return allMeshes;
}


bool importAndExport(const std::string& importPath, const std::string& exportPath) {
    Assimp::Importer importer;

    // Use aiProcess_FlipUVs since Vulkan's coordinate system is different from OpenGL's
    const aiScene* scene = importer.ReadFile(importPath,
      aiProcess_CalcTangentSpace       |
      aiProcess_Triangulate            |
      aiProcess_JoinIdenticalVertices  |
      aiProcess_SortByPType            |
      aiProcess_FlipUVs);

    if (nullptr == scene) {
        spdlog::error("Assimp import error: {}", importer.GetErrorString());
        return false;
    }

    processAndExportScene(scene, exportPath);

    return true;
}

int main()
{
    // Define input and output paths
    const std::string inputModel = "../workspace/monkey.obj";
    const std::string outputModel = "./monkey.mesh";

    // Run the import and export process
    if(importAndExport(inputModel, outputModel)) {
        // Optional: Test the loader to verify the export
        spdlog::info("--- Verifying export by loading file back ---");
        loadModelFromBinary(outputModel);
    }

    return 0;
}
