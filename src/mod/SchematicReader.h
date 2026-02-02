#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <cstdint>
#include <memory>

namespace wooden_axe {

// Simple block state representation
struct SchematicBlock {
    std::string name;  // e.g. "minecraft:stone"
    std::unordered_map<std::string, std::string> properties;  // Block states
    
    std::string toString() const {
        if (properties.empty()) {
            return name;
        }
        std::string result = name + "[";
        bool first = true;
        for (const auto& [key, value] : properties) {
            if (!first) result += ",";
            result += key + "=" + value;
            first = false;
        }
        result += "]";
        return result;
    }
};

// Schematic data structure
struct Schematic {
    int width = 0;
    int height = 0;
    int length = 0;
    
    // Offset from origin (for paste operations)
    int offsetX = 0;
    int offsetY = 0;
    int offsetZ = 0;
    
    // Block palette: index -> block
    std::vector<SchematicBlock> palette;
    
    // Block data: 3D array stored as 1D (index = y * width * length + z * width + x)
    std::vector<int> blocks;
    
    // Get block at position
    std::optional<SchematicBlock> getBlock(int x, int y, int z) const {
        if (x < 0 || x >= width || y < 0 || y >= height || z < 0 || z >= length) {
            return std::nullopt;
        }
        int index = y * width * length + z * width + x;
        if (index >= 0 && static_cast<size_t>(index) < blocks.size()) {
            int paletteIndex = blocks[index];
            if (paletteIndex >= 0 && static_cast<size_t>(paletteIndex) < palette.size()) {
                return palette[paletteIndex];
            }
        }
        return std::nullopt;
    }
    
    // Get total block count
    size_t getBlockCount() const {
        return static_cast<size_t>(width) * height * length;
    }
};

class SchematicReader {
public:
    // Load schematic from file
    static std::optional<Schematic> loadFromFile(const std::string& filePath);
    
    // List available schematics in directory
    static std::vector<std::string> listSchematics(const std::string& directory);

private:
    // Parse NBT data from uncompressed bytes
    static std::optional<Schematic> parseNBT(const std::vector<uint8_t>& data);
    
    // Decompress gzip data
    static std::vector<uint8_t> decompressGzip(const std::vector<uint8_t>& compressed);
    
    // Read file to bytes
    static std::vector<uint8_t> readFile(const std::string& filePath);
};

} // namespace wooden_axe
