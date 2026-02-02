#include "mod/SchematicPlacer.h"
#include "mod/WoodenAxeMod.h"

#include "ll/api/service/Bedrock.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/registry/BlockTypeRegistry.h"
#include "mc/world/level/BlockSource.h"
#include "mc/server/commands/CommandUtils.h"

#include <algorithm>
#include <cctype>

namespace wooden_axe {

void SchematicPlacer::setLoadedSchematic(const std::string& playerName, Schematic schem) {
    mLoadedSchematics[playerName] = std::move(schem);
}

Schematic* SchematicPlacer::getLoadedSchematic(const std::string& playerName) {
    auto it = mLoadedSchematics.find(playerName);
    if (it != mLoadedSchematics.end()) {
        return &it->second;
    }
    return nullptr;
}

void SchematicPlacer::clearLoadedSchematic(const std::string& playerName) {
    mLoadedSchematics.erase(playerName);
}

std::string SchematicPlacer::convertBlockName(const std::string& javaName) {
    // Basic Java -> Bedrock block name mapping
    // Most blocks have the same name, but some need conversion
    
    static const std::unordered_map<std::string, std::string> mapping = {
        // Logs
        {"minecraft:oak_log", "minecraft:log"},
        {"minecraft:spruce_log", "minecraft:log"},
        {"minecraft:birch_log", "minecraft:log"},
        {"minecraft:jungle_log", "minecraft:log"},
        {"minecraft:acacia_log", "minecraft:log2"},
        {"minecraft:dark_oak_log", "minecraft:log2"},
        
        // Planks (in newer Bedrock versions these might be the same)
        {"minecraft:oak_planks", "minecraft:planks"},
        {"minecraft:spruce_planks", "minecraft:planks"},
        {"minecraft:birch_planks", "minecraft:planks"},
        {"minecraft:jungle_planks", "minecraft:planks"},
        {"minecraft:acacia_planks", "minecraft:planks"},
        {"minecraft:dark_oak_planks", "minecraft:planks"},
        
        // Slabs
        {"minecraft:oak_slab", "minecraft:wooden_slab"},
        {"minecraft:spruce_slab", "minecraft:wooden_slab"},
        {"minecraft:birch_slab", "minecraft:wooden_slab"},
        {"minecraft:jungle_slab", "minecraft:wooden_slab"},
        {"minecraft:acacia_slab", "minecraft:wooden_slab"},
        {"minecraft:dark_oak_slab", "minecraft:wooden_slab"},
        
        // Stairs
        {"minecraft:oak_stairs", "minecraft:oak_stairs"},
        {"minecraft:spruce_stairs", "minecraft:spruce_stairs"},
        {"minecraft:birch_stairs", "minecraft:birch_stairs"},
        {"minecraft:jungle_stairs", "minecraft:jungle_stairs"},
        {"minecraft:acacia_stairs", "minecraft:acacia_stairs"},
        {"minecraft:dark_oak_stairs", "minecraft:dark_oak_stairs"},
        
        // Fences
        {"minecraft:oak_fence", "minecraft:fence"},
        {"minecraft:spruce_fence", "minecraft:fence"},
        {"minecraft:birch_fence", "minecraft:fence"},
        {"minecraft:jungle_fence", "minecraft:fence"},
        {"minecraft:acacia_fence", "minecraft:fence"},
        {"minecraft:dark_oak_fence", "minecraft:fence"},
        
        // Walls
        {"minecraft:cobblestone_wall", "minecraft:cobblestone_wall"},
        {"minecraft:mossy_cobblestone_wall", "minecraft:cobblestone_wall"},
        
        // Terracotta
        {"minecraft:terracotta", "minecraft:hardened_clay"},
        {"minecraft:white_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:orange_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:magenta_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:light_blue_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:yellow_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:lime_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:pink_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:gray_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:light_gray_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:cyan_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:purple_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:blue_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:brown_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:green_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:red_terracotta", "minecraft:stained_hardened_clay"},
        {"minecraft:black_terracotta", "minecraft:stained_hardened_clay"},
        
        // Concrete
        {"minecraft:white_concrete", "minecraft:concrete"},
        {"minecraft:orange_concrete", "minecraft:concrete"},
        {"minecraft:magenta_concrete", "minecraft:concrete"},
        {"minecraft:light_blue_concrete", "minecraft:concrete"},
        {"minecraft:yellow_concrete", "minecraft:concrete"},
        {"minecraft:lime_concrete", "minecraft:concrete"},
        {"minecraft:pink_concrete", "minecraft:concrete"},
        {"minecraft:gray_concrete", "minecraft:concrete"},
        {"minecraft:light_gray_concrete", "minecraft:concrete"},
        {"minecraft:cyan_concrete", "minecraft:concrete"},
        {"minecraft:purple_concrete", "minecraft:concrete"},
        {"minecraft:blue_concrete", "minecraft:concrete"},
        {"minecraft:brown_concrete", "minecraft:concrete"},
        {"minecraft:green_concrete", "minecraft:concrete"},
        {"minecraft:red_concrete", "minecraft:concrete"},
        {"minecraft:black_concrete", "minecraft:concrete"},
        
        // Wool  
        {"minecraft:white_wool", "minecraft:wool"},
        {"minecraft:orange_wool", "minecraft:wool"},
        {"minecraft:magenta_wool", "minecraft:wool"},
        {"minecraft:light_blue_wool", "minecraft:wool"},
        {"minecraft:yellow_wool", "minecraft:wool"},
        {"minecraft:lime_wool", "minecraft:wool"},
        {"minecraft:pink_wool", "minecraft:wool"},
        {"minecraft:gray_wool", "minecraft:wool"},
        {"minecraft:light_gray_wool", "minecraft:wool"},
        {"minecraft:cyan_wool", "minecraft:wool"},
        {"minecraft:purple_wool", "minecraft:wool"},
        {"minecraft:blue_wool", "minecraft:wool"},
        {"minecraft:brown_wool", "minecraft:wool"},
        {"minecraft:green_wool", "minecraft:wool"},
        {"minecraft:red_wool", "minecraft:wool"},
        {"minecraft:black_wool", "minecraft:wool"},
        
        // Glass panes
        {"minecraft:white_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:orange_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:magenta_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:light_blue_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:yellow_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:lime_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:pink_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:gray_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:light_gray_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:cyan_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:purple_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:blue_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:brown_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:green_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:red_stained_glass_pane", "minecraft:stained_glass_pane"},
        {"minecraft:black_stained_glass_pane", "minecraft:stained_glass_pane"},
        
        // Misc
        {"minecraft:grass_block", "minecraft:grass"},
        {"minecraft:dirt_path", "minecraft:grass_path"},
        {"minecraft:rooted_dirt", "minecraft:dirt_with_roots"},
        {"minecraft:infested_stone", "minecraft:monster_egg"},
        {"minecraft:bricks", "minecraft:brick_block"},
        {"minecraft:snow_block", "minecraft:snow"},
        {"minecraft:melon", "minecraft:melon_block"},
        {"minecraft:lily_pad", "minecraft:waterlily"},
        {"minecraft:nether_bricks", "minecraft:nether_brick"},
        {"minecraft:end_stone_bricks", "minecraft:end_bricks"},
        {"minecraft:red_nether_bricks", "minecraft:red_nether_brick"},
        {"minecraft:magma_block", "minecraft:magma"},
        {"minecraft:sea_lantern", "minecraft:seaLantern"},
        {"minecraft:jack_o_lantern", "minecraft:lit_pumpkin"},
    };
    
    auto it = mapping.find(javaName);
    if (it != mapping.end()) {
        return it->second;
    }
    
    // Return original name if no mapping found
    return javaName;
}

std::string SchematicPlacer::buildBlockState(const SchematicBlock& block) {
    std::string bedrockName = convertBlockName(block.name);
    
    // For now, return just the block name
    // TODO: Add proper block state conversion for properties
    // This is complex because Java and Bedrock have different property systems
    
    return bedrockName;
}

int SchematicPlacer::placeSchematic(const Schematic& schem, int baseX, int baseY, int baseZ, int dimension) {
    auto& logger = WoodenAxeMod::getInstance().getSelf().getLogger();
    
    auto* level = ll::service::getLevel();
    if (!level) {
        logger.error("Failed to get level");
        return -1;
    }
    
    // Get dimension
    auto* dim = level->getDimension(dimension).get();
    if (!dim) {
        logger.error("Failed to get dimension {}", dimension);
        return -1;
    }
    
    auto& blockSource = dim->getBlockSourceFromMainChunkSource();
    
    int placed = 0;
    int skipped = 0;
    int failed = 0;
    
    logger.info("Placing schematic {}x{}x{} at ({}, {}, {})", 
                schem.width, schem.height, schem.length, baseX, baseY, baseZ);
    
    // Place blocks
    for (int y = 0; y < schem.height; y++) {
        for (int z = 0; z < schem.length; z++) {
            for (int x = 0; x < schem.width; x++) {
                auto blockOpt = schem.getBlock(x, y, z);
                if (!blockOpt) {
                    skipped++;
                    continue;
                }
                
                const auto& block = *blockOpt;
                
                // Skip air blocks
                if (block.name == "minecraft:air" || block.name == "air") {
                    skipped++;
                    continue;
                }
                
                // Calculate world position
                int worldX = baseX + x + schem.offsetX;
                int worldY = baseY + y + schem.offsetY;
                int worldZ = baseZ + z + schem.offsetZ;
                
                // Get block to place
                std::string blockName = buildBlockState(block);
                
                try {
                    // Try to get the block from registry
                    const Block* bedrockBlock = BlockTypeRegistry::lookupByName(blockName, false);
                    
                    if (bedrockBlock) {
                        BlockPos pos(worldX, worldY, worldZ);
                        blockSource.setBlock(pos, *bedrockBlock, 3, nullptr, nullptr);
                        placed++;
                    } else {
                        // Fallback: try with just minecraft:stone if block not found
                        logger.debug("Block not found: {}, original: {}", blockName, block.name);
                        failed++;
                    }
                } catch (const std::exception& e) {
                    logger.debug("Failed to place block at ({}, {}, {}): {}", worldX, worldY, worldZ, e.what());
                    failed++;
                }
            }
        }
        
        // Log progress every 16 layers
        if ((y + 1) % 16 == 0 || y == schem.height - 1) {
            logger.debug("Progress: layer {}/{}, placed: {}", y + 1, schem.height, placed);
        }
    }
    
    logger.info("Schematic placement complete: {} placed, {} skipped (air), {} failed", 
                placed, skipped, failed);
    
    return placed;
}

} // namespace wooden_axe
