#pragma once

#include "Globals.h"
#include "Vertex.h"
#include "LogManager.h"
#include "Material.h"

class OBJLoader {
public:
	bool LoadOBJ(const char* filename, std::vector<Vertex>& vertices, std::vector<UINT>& indices, std::unordered_map<std::string, Material>& materials);
	bool LoadMTL(const std::string& mtlFilename, std::unordered_map<std::string, Material>& materials);
};