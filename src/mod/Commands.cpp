#include "mod/Commands.h"
#include "mod/WoodenAxeMod.h"
#include "mod/SchematicReader.h"
#include "mod/SchematicPlacer.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/player/Player.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"

#include <string>
#include <filesystem>

namespace wooden_axe {

// Command parameter structures
struct WaListParams {};

struct WaLoadParams {
    std::string filename;
};

struct WaPasteParams {};

struct WaPosParams {};

struct WaClearParams {};

void registerCommands() {
    auto& logger = WoodenAxeMod::getInstance().getSelf().getLogger();
    
    // Register /wa command group
    auto& cmdRegistrar = ll::command::CommandRegistrar::getInstance();
    
    // /wa list - List available schematics
    auto& listCmd = cmdRegistrar.getOrCreateCommand("walist", "List available schematics", CommandPermissionLevel::GameDirectors);
    listCmd.overload<WaListParams>()
        .execute([&logger](CommandOrigin const& origin, CommandOutput& output, WaListParams const&) {
            auto schemDir = WoodenAxeMod::getInstance().getSchematicDir();
            auto schematics = SchematicReader::listSchematics(schemDir);
            
            if (schematics.empty()) {
                output.success("No schematics found in: " + schemDir);
                return;
            }
            
            output.success("§eAvailable schematics:");
            for (const auto& name : schematics) {
                output.success("  §7- §f" + name);
            }
        });
    
    // /wa load <filename> - Load a schematic
    auto& loadCmd = cmdRegistrar.getOrCreateCommand("waload", "Load a schematic file", CommandPermissionLevel::GameDirectors);
    loadCmd.overload<WaLoadParams>()
        .required("filename")
        .execute([&logger](CommandOrigin const& origin, CommandOutput& output, WaLoadParams const& params) {
            // Get player
            auto* entity = origin.getEntity();
            if (!entity || !entity->isPlayer()) {
                output.error("This command can only be used by players");
                return;
            }
            
            Player* player = static_cast<Player*>(entity);
            std::string playerName = player->getRealName();
            
            // Build full path
            auto schemDir = WoodenAxeMod::getInstance().getSchematicDir();
            std::string filename = params.filename;
            
            // Add .schem extension if not present
            if (filename.find('.') == std::string::npos) {
                filename += ".schem";
            }
            
            std::string fullPath = (std::filesystem::path(schemDir) / filename).string();
            
            // Load schematic
            auto schem = SchematicReader::loadFromFile(fullPath);
            if (!schem) {
                output.error("Failed to load schematic: " + filename);
                return;
            }
            
            // Store in placer
            SchematicPlacer::getInstance().setLoadedSchematic(playerName, std::move(*schem));
            
            output.success("§aLoaded schematic: §f" + filename);
            output.success("§7Size: " + std::to_string(schem->width) + "x" + 
                          std::to_string(schem->height) + "x" + std::to_string(schem->length));
        });
    
    // /wa paste - Paste loaded schematic at pos1
    auto& pasteCmd = cmdRegistrar.getOrCreateCommand("wapaste", "Paste schematic at pos1", CommandPermissionLevel::GameDirectors);
    pasteCmd.overload<WaPasteParams>()
        .execute([&logger](CommandOrigin const& origin, CommandOutput& output, WaPasteParams const&) {
            // Get player
            auto* entity = origin.getEntity();
            if (!entity || !entity->isPlayer()) {
                output.error("This command can only be used by players");
                return;
            }
            
            Player* player = static_cast<Player*>(entity);
            std::string playerName = player->getRealName();
            
            // Get selection
            auto selection = WoodenAxeMod::getInstance().getSelection(playerName);
            if (!selection || !selection->pos1) {
                output.error("Please set pos1 first (left-click with wooden axe)");
                return;
            }
            
            // Get schematic
            auto* schem = SchematicPlacer::getInstance().getLoadedSchematic(playerName);
            if (!schem) {
                output.error("No schematic loaded. Use /waload <filename> first");
                return;
            }
            
            // Paste
            auto& pos = *selection->pos1;
            int dim = selection->dimension;
            
            int placed = SchematicPlacer::getInstance().placeSchematic(
                *schem, pos.x, pos.y, pos.z, dim
            );
            
            if (placed > 0) {
                output.success("§aPasted " + std::to_string(placed) + " blocks");
            } else {
                output.error("Failed to paste schematic");
            }
        });
    
    // /wa pos - Show current selection
    auto& posCmd = cmdRegistrar.getOrCreateCommand("wapos", "Show current selection", CommandPermissionLevel::GameDirectors);
    posCmd.overload<WaPosParams>()
        .execute([&logger](CommandOrigin const& origin, CommandOutput& output, WaPosParams const&) {
            auto* entity = origin.getEntity();
            if (!entity || !entity->isPlayer()) {
                output.error("This command can only be used by players");
                return;
            }
            
            Player* player = static_cast<Player*>(entity);
            std::string playerName = player->getRealName();
            
            auto selection = WoodenAxeMod::getInstance().getSelection(playerName);
            if (!selection) {
                output.success("No selection set");
                return;
            }
            
            if (selection->pos1) {
                auto& p = *selection->pos1;
                output.success("§ePos1: §f(" + std::to_string(p.x) + ", " + 
                              std::to_string(p.y) + ", " + std::to_string(p.z) + ")");
            } else {
                output.success("§ePos1: §7Not set");
            }
            
            if (selection->pos2) {
                auto& p = *selection->pos2;
                output.success("§ePos2: §f(" + std::to_string(p.x) + ", " + 
                              std::to_string(p.y) + ", " + std::to_string(p.z) + ")");
            } else {
                output.success("§ePos2: §7Not set");
            }
        });
    
    // /wa clear - Clear selection
    auto& clearCmd = cmdRegistrar.getOrCreateCommand("waclear", "Clear selection", CommandPermissionLevel::GameDirectors);
    clearCmd.overload<WaClearParams>()
        .execute([&logger](CommandOrigin const& origin, CommandOutput& output, WaClearParams const&) {
            auto* entity = origin.getEntity();
            if (!entity || !entity->isPlayer()) {
                output.error("This command can only be used by players");
                return;
            }
            
            Player* player = static_cast<Player*>(entity);
            std::string playerName = player->getRealName();
            
            WoodenAxeMod::getInstance().clearSelection(playerName);
            SchematicPlacer::getInstance().clearLoadedSchematic(playerName);
            
            output.success("§aSelection and loaded schematic cleared");
        });
    
    logger.info("Commands registered: /walist, /waload, /wapaste, /wapos, /waclear");
}

} // namespace wooden_axe
