#pragma once

#include "mod/SchematicReader.h"
#include <unordered_map>
#include <string>
#include <optional>

namespace wooden_axe {

class SchematicPlacer {
public:
    static SchematicPlacer& getInstance() {
        static SchematicPlacer instance;
        return instance;
    }
    
    // Store loaded schematic for a player
    void setLoadedSchematic(const std::string& playerName, Schematic schem);
    
    // Get loaded schematic for a player
    Schematic* getLoadedSchematic(const std::string& playerName);
    
    // Clear loaded schematic
    void clearLoadedSchematic(const std::string& playerName);
    
    // Place schematic at position
    // Returns number of blocks placed, or -1 on error
    int placeSchematic(const Schematic& schem, int x, int y, int z, int dimension);

private:
    std::unordered_map<std::string, Schematic> mLoadedSchematics;
    
    // Convert Java block name to Bedrock format
    static std::string convertBlockName(const std::string& javaName);
    
    // Build block state string for setblock command
    static std::string buildBlockState(const SchematicBlock& block);
};

} // namespace wooden_axe
