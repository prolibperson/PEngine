#include "OBJLoader.h"

bool OBJLoader::LoadMTL(const std::string& mtlFilename, std::unordered_map<std::string, Material>& materials) {
    std::ifstream file(mtlFilename);
    if (!file.is_open()) {
        std::cerr << "Failed to open MTL file: " << mtlFilename << std::endl;
        return false;
    }

    std::string line, currentMaterial;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "newmtl") {
            iss >> currentMaterial;
            if (!currentMaterial.empty()) {
                materials[currentMaterial] = Material();
                materials[currentMaterial].name = currentMaterial;
            }
        }
        else if (token == "map_Kd") {
            std::string textureFilename;
            iss >> textureFilename;
            if (!currentMaterial.empty()) {
                materials[currentMaterial].diffuseMap = textureFilename;
                std::cout << "Material " << currentMaterial << " has diffuse map: " << textureFilename << std::endl;
            }
        }
        else if (token == "map_Disp") {
            std::string textureFilename;
            iss >> textureFilename;
            if (!currentMaterial.empty()) {
                materials[currentMaterial].displacementMap = textureFilename;
                std::cout << "Material " << currentMaterial << " has displacement map: " << textureFilename << std::endl;
            }
        }
        else if (token == "map_Ka") {
            std::string textureFilename;
            iss >> textureFilename;
            if (!currentMaterial.empty()) {
                materials[currentMaterial].ambientMap = textureFilename;
                std::cout << "Material " << currentMaterial << " has ambient map: " << textureFilename << std::endl;
            }
        }
    }

    return true;
}

bool OBJLoader::LoadOBJ(const char* filename, std::vector<Vertex>& vertices, std::vector<UINT>& indices, std::unordered_map<std::string, Material>& materials) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open OBJ file: " << filename << std::endl;
        return false;
    }

    LogManager::pPrint(true, true, "Loading file %s\n", filename);

    bool isUsemtl = false;

    std::vector<XMFLOAT3> positions;
    std::vector<XMFLOAT3> normals;
    std::vector<XMFLOAT2> texCoords;
    std::unordered_map<std::string, int> materialIndices;
    int currentMaterialIndex = -1;

    int index = 0;
    for (const auto& material : materials) {
        materialIndices[material.first] = index++;
    }

    std::string line;
    auto start = std::chrono::steady_clock::now();
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        if (prefix == "mtllib") {
            isUsemtl = true;
        }
        else if (prefix == "v") {
            XMFLOAT3 position;
            iss >> position.x >> position.y >> position.z;
            positions.push_back(position);
        }
        else if (prefix == "vt") {
            XMFLOAT2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            texCoords.push_back(texCoord);
        }
        else if (prefix == "vn") {
            XMFLOAT3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (prefix == "usemtl") {
            std::string materialName;
            iss >> materialName;
            if (materialIndices.find(materialName) != materialIndices.end()) {
                currentMaterialIndex = materialIndices[materialName];
            }
            else {
                LogManager::pPrint(true, true, "Material not found: %s", materialName.c_str());
            }
        }
        else if (prefix == "f") {
            std::string vertexStr;
            for (int i = 0; i < 3; ++i) {
                iss >> vertexStr;
                std::istringstream viss(vertexStr);
                std::string indexStr;
                int posIndex, texIndex = -1, normIndex = -1;
                try {
                    std::getline(viss, indexStr, '/');
                    posIndex = std::stoi(indexStr) - 1;

                    if (std::getline(viss, indexStr, '/')) {
                        texIndex = std::stoi(indexStr) - 1;
                    }

                    if (std::getline(viss, indexStr, '/')) {
                        normIndex = std::stoi(indexStr) - 1;
                    }
                }
                catch (const std::invalid_argument&) {
                    LogManager::pPrint(true, true, "Invalid face data: %s", vertexStr.c_str());
                    continue;
                }

                Vertex vertex;
                vertex.position = positions[posIndex];
                if (!texCoords.empty() && texIndex >= 0 && texIndex < texCoords.size()) {
                    vertex.texCoord = texCoords[texIndex];
                }
                if (!normals.empty() && normIndex >= 0 && normIndex < normals.size()) {
                    vertex.normal = normals[normIndex];
                }
                vertex.materialIndex = static_cast<UINT>(currentMaterialIndex);
                vertices.push_back(vertex);
                indices.push_back(static_cast<UINT>(indices.size()));
            }
        }
        else {
            if (prefix != "mtllib" && prefix != "usemtl" && prefix != "s" && prefix != "o" && prefix != "g" && prefix != "#") {
                LogManager::pPrint(true, true, "Unrecognized prefix: %s", prefix.c_str());
            }
        }
    }

    auto end = std::chrono::steady_clock::now(); // End timing
    std::chrono::duration<double> elapsed_seconds = end - start;

    LogManager::pPrint(true, true, "Loaded %i vertices and %i indices in %f seconds.\n", static_cast<int>(vertices.size()), static_cast<int>(indices.size()), elapsed_seconds.count());
    return true;
}


