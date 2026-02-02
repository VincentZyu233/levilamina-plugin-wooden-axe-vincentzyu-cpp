#include "mod/SchematicReader.h"
#include "mod/WoodenAxeMod.h"

#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <cstring>
#include <zlib.h>

namespace wooden_axe {

// Simple NBT Tag Types
enum class TagType : uint8_t {
    End = 0,
    Byte = 1,
    Short = 2,
    Int = 3,
    Long = 4,
    Float = 5,
    Double = 6,
    ByteArray = 7,
    String = 8,
    List = 9,
    Compound = 10,
    IntArray = 11,
    LongArray = 12
};

// Simple NBT parser class
class NBTParser {
public:
    NBTParser(const std::vector<uint8_t>& data) : mData(data), mPos(0) {}
    
    std::optional<Schematic> parseSchematic() {
        // Read root compound tag
        if (readByte() != static_cast<uint8_t>(TagType::Compound)) {
            return std::nullopt;
        }
        
        // Read root name (usually "Schematic")
        readString();
        
        Schematic schem;
        std::unordered_map<std::string, int> paletteMap;
        std::vector<uint8_t> blockData;
        bool inMetadata = false;
        
        // Parse compound contents
        while (mPos < mData.size()) {
            uint8_t tagType = readByte();
            if (tagType == static_cast<uint8_t>(TagType::End)) {
                if (inMetadata) {
                    inMetadata = false;
                    continue;
                }
                break;
            }
            
            std::string tagName = readString();
            
            // Handle specific tags for Sponge Schematic format
            if (tagName == "Width" && tagType == static_cast<uint8_t>(TagType::Short)) {
                schem.width = readShort();
            }
            else if (tagName == "Height" && tagType == static_cast<uint8_t>(TagType::Short)) {
                schem.height = readShort();
            }
            else if (tagName == "Length" && tagType == static_cast<uint8_t>(TagType::Short)) {
                schem.length = readShort();
            }
            else if (tagName == "Palette" && tagType == static_cast<uint8_t>(TagType::Compound)) {
                parsePalette(paletteMap, schem);
            }
            else if (tagName == "BlockData" && tagType == static_cast<uint8_t>(TagType::ByteArray)) {
                blockData = readByteArray();
            }
            else if (tagName == "Metadata" && tagType == static_cast<uint8_t>(TagType::Compound)) {
                inMetadata = true;
                // Parse metadata for offset
                parseMetadata(schem);
            }
            else if (tagName == "Offset" && tagType == static_cast<uint8_t>(TagType::IntArray)) {
                auto offset = readIntArray();
                if (offset.size() >= 3) {
                    schem.offsetX = offset[0];
                    schem.offsetY = offset[1];
                    schem.offsetZ = offset[2];
                }
            }
            else {
                // Skip unknown tags
                skipTag(static_cast<TagType>(tagType));
            }
        }
        
        // Convert block data using VarInt encoding (Sponge Schematic v2)
        if (!blockData.empty() && !paletteMap.empty()) {
            schem.blocks = parseVarIntBlocks(blockData, schem.width, schem.height, schem.length);
        }
        
        // Build palette vector from map
        if (!paletteMap.empty()) {
            int maxIndex = 0;
            for (const auto& [name, idx] : paletteMap) {
                maxIndex = std::max(maxIndex, idx);
            }
            schem.palette.resize(maxIndex + 1);
            for (const auto& [name, idx] : paletteMap) {
                schem.palette[idx] = parseBlockState(name);
            }
        }
        
        return schem;
    }

private:
    const std::vector<uint8_t>& mData;
    size_t mPos;
    
    uint8_t readByte() {
        if (mPos >= mData.size()) throw std::runtime_error("NBT: Unexpected end of data");
        return mData[mPos++];
    }
    
    int16_t readShort() {
        uint8_t b1 = readByte();
        uint8_t b2 = readByte();
        return static_cast<int16_t>((b1 << 8) | b2);
    }
    
    int32_t readInt() {
        uint8_t b1 = readByte();
        uint8_t b2 = readByte();
        uint8_t b3 = readByte();
        uint8_t b4 = readByte();
        return static_cast<int32_t>((b1 << 24) | (b2 << 16) | (b3 << 8) | b4);
    }
    
    int64_t readLong() {
        int64_t result = 0;
        for (int i = 0; i < 8; i++) {
            result = (result << 8) | readByte();
        }
        return result;
    }
    
    float readFloat() {
        int32_t bits = readInt();
        float result;
        std::memcpy(&result, &bits, sizeof(float));
        return result;
    }
    
    double readDouble() {
        int64_t bits = readLong();
        double result;
        std::memcpy(&result, &bits, sizeof(double));
        return result;
    }
    
    std::string readString() {
        int16_t length = readShort();
        if (length < 0 || mPos + length > mData.size()) {
            throw std::runtime_error("NBT: Invalid string length");
        }
        std::string result(reinterpret_cast<const char*>(&mData[mPos]), length);
        mPos += length;
        return result;
    }
    
    std::vector<uint8_t> readByteArray() {
        int32_t length = readInt();
        if (length < 0 || mPos + length > mData.size()) {
            throw std::runtime_error("NBT: Invalid byte array length");
        }
        std::vector<uint8_t> result(mData.begin() + mPos, mData.begin() + mPos + length);
        mPos += length;
        return result;
    }
    
    std::vector<int32_t> readIntArray() {
        int32_t length = readInt();
        if (length < 0) {
            throw std::runtime_error("NBT: Invalid int array length");
        }
        std::vector<int32_t> result;
        result.reserve(length);
        for (int32_t i = 0; i < length; i++) {
            result.push_back(readInt());
        }
        return result;
    }
    
    void skipTag(TagType type) {
        switch (type) {
            case TagType::End: break;
            case TagType::Byte: mPos++; break;
            case TagType::Short: mPos += 2; break;
            case TagType::Int: mPos += 4; break;
            case TagType::Long: mPos += 8; break;
            case TagType::Float: mPos += 4; break;
            case TagType::Double: mPos += 8; break;
            case TagType::ByteArray: {
                int32_t len = readInt();
                mPos += len;
                break;
            }
            case TagType::String: {
                int16_t len = readShort();
                mPos += len;
                break;
            }
            case TagType::List: {
                uint8_t elemType = readByte();
                int32_t len = readInt();
                for (int32_t i = 0; i < len; i++) {
                    skipTag(static_cast<TagType>(elemType));
                }
                break;
            }
            case TagType::Compound: {
                while (true) {
                    uint8_t subType = readByte();
                    if (subType == static_cast<uint8_t>(TagType::End)) break;
                    readString(); // name
                    skipTag(static_cast<TagType>(subType));
                }
                break;
            }
            case TagType::IntArray: {
                int32_t len = readInt();
                mPos += len * 4;
                break;
            }
            case TagType::LongArray: {
                int32_t len = readInt();
                mPos += len * 8;
                break;
            }
        }
    }
    
    void parsePalette(std::unordered_map<std::string, int>& paletteMap, Schematic& schem) {
        while (true) {
            uint8_t tagType = readByte();
            if (tagType == static_cast<uint8_t>(TagType::End)) break;
            
            std::string blockName = readString();
            if (tagType == static_cast<uint8_t>(TagType::Int)) {
                int index = readInt();
                paletteMap[blockName] = index;
            } else {
                skipTag(static_cast<TagType>(tagType));
            }
        }
    }
    
    void parseMetadata(Schematic& schem) {
        while (true) {
            uint8_t tagType = readByte();
            if (tagType == static_cast<uint8_t>(TagType::End)) break;
            
            std::string tagName = readString();
            if (tagName == "WEOffsetX" && tagType == static_cast<uint8_t>(TagType::Int)) {
                schem.offsetX = readInt();
            } else if (tagName == "WEOffsetY" && tagType == static_cast<uint8_t>(TagType::Int)) {
                schem.offsetY = readInt();
            } else if (tagName == "WEOffsetZ" && tagType == static_cast<uint8_t>(TagType::Int)) {
                schem.offsetZ = readInt();
            } else {
                skipTag(static_cast<TagType>(tagType));
            }
        }
    }
    
    std::vector<int> parseVarIntBlocks(const std::vector<uint8_t>& data, int width, int height, int length) {
        std::vector<int> blocks;
        size_t expectedSize = static_cast<size_t>(width) * height * length;
        blocks.reserve(expectedSize);
        
        size_t pos = 0;
        while (pos < data.size() && blocks.size() < expectedSize) {
            int value = 0;
            int shift = 0;
            while (true) {
                if (pos >= data.size()) break;
                uint8_t b = data[pos++];
                value |= (b & 0x7F) << shift;
                if ((b & 0x80) == 0) break;
                shift += 7;
            }
            blocks.push_back(value);
        }
        
        return blocks;
    }
    
    SchematicBlock parseBlockState(const std::string& blockString) {
        SchematicBlock block;
        
        // Parse format: "minecraft:stone[facing=north,half=top]"
        size_t bracketStart = blockString.find('[');
        if (bracketStart == std::string::npos) {
            block.name = blockString;
            return block;
        }
        
        block.name = blockString.substr(0, bracketStart);
        
        size_t bracketEnd = blockString.find(']', bracketStart);
        if (bracketEnd == std::string::npos) {
            return block;
        }
        
        std::string propsStr = blockString.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
        
        // Parse properties
        size_t start = 0;
        while (start < propsStr.size()) {
            size_t equalPos = propsStr.find('=', start);
            if (equalPos == std::string::npos) break;
            
            size_t commaPos = propsStr.find(',', equalPos);
            if (commaPos == std::string::npos) commaPos = propsStr.size();
            
            std::string key = propsStr.substr(start, equalPos - start);
            std::string value = propsStr.substr(equalPos + 1, commaPos - equalPos - 1);
            
            block.properties[key] = value;
            start = commaPos + 1;
        }
        
        return block;
    }
};

std::vector<uint8_t> SchematicReader::readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return {};
    }
    
    return buffer;
}

std::vector<uint8_t> SchematicReader::decompressGzip(const std::vector<uint8_t>& compressed) {
    if (compressed.size() < 2) {
        return {};
    }
    
    // Check for gzip magic number
    if (compressed[0] != 0x1F || compressed[1] != 0x8B) {
        // Not gzip compressed, return as-is
        return compressed;
    }
    
    std::vector<uint8_t> decompressed;
    decompressed.reserve(compressed.size() * 10); // Estimate
    
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = static_cast<uInt>(compressed.size());
    stream.next_in = const_cast<Bytef*>(compressed.data());
    
    // 15 + 16 for gzip format
    if (inflateInit2(&stream, 15 + 16) != Z_OK) {
        return {};
    }
    
    uint8_t outBuffer[8192];
    int ret;
    
    do {
        stream.avail_out = sizeof(outBuffer);
        stream.next_out = outBuffer;
        
        ret = inflate(&stream, Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
            inflateEnd(&stream);
            return {};
        }
        
        size_t have = sizeof(outBuffer) - stream.avail_out;
        decompressed.insert(decompressed.end(), outBuffer, outBuffer + have);
        
    } while (ret != Z_STREAM_END);
    
    inflateEnd(&stream);
    return decompressed;
}

std::optional<Schematic> SchematicReader::parseNBT(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return std::nullopt;
    }
    
    try {
        NBTParser parser(data);
        return parser.parseSchematic();
    } catch (const std::exception& e) {
        WoodenAxeMod::getInstance().getSelf().getLogger().error("NBT parse error: {}", e.what());
        return std::nullopt;
    }
}

std::optional<Schematic> SchematicReader::loadFromFile(const std::string& filePath) {
    auto& logger = WoodenAxeMod::getInstance().getSelf().getLogger();
    
    logger.debug("Loading schematic from: {}", filePath);
    
    // Read file
    auto compressed = readFile(filePath);
    if (compressed.empty()) {
        logger.error("Failed to read file: {}", filePath);
        return std::nullopt;
    }
    
    logger.debug("Read {} bytes", compressed.size());
    
    // Decompress
    auto decompressed = decompressGzip(compressed);
    if (decompressed.empty()) {
        logger.error("Failed to decompress file");
        return std::nullopt;
    }
    
    logger.debug("Decompressed to {} bytes", decompressed.size());
    
    // Parse NBT
    auto schem = parseNBT(decompressed);
    if (!schem) {
        logger.error("Failed to parse NBT data");
        return std::nullopt;
    }
    
    logger.info("Loaded schematic: {}x{}x{} ({} blocks, {} palette entries)",
                schem->width, schem->height, schem->length,
                schem->blocks.size(), schem->palette.size());
    
    return schem;
}

std::vector<std::string> SchematicReader::listSchematics(const std::string& directory) {
    std::vector<std::string> result;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                auto ext = entry.path().extension().string();
                // Convert to lowercase for comparison
                for (auto& c : ext) c = static_cast<char>(std::tolower(c));
                
                if (ext == ".schem" || ext == ".schematic") {
                    result.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const std::exception& e) {
        WoodenAxeMod::getInstance().getSelf().getLogger().error(
            "Error listing schematics: {}", e.what());
    }
    
    return result;
}

} // namespace wooden_axe
