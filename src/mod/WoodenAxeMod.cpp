#include "mod/WoodenAxeMod.h"
#include "mod/Commands.h"
#include "mod/EventHandlers.h"

#include "ll/api/mod/RegisterHelper.h"

#include <filesystem>

namespace wooden_axe {

WoodenAxeMod& WoodenAxeMod::getInstance() {
    static WoodenAxeMod instance;
    return instance;
}

bool WoodenAxeMod::load() {
    auto& logger = getSelf().getLogger();
    
    logger.info("");
    logger.info(R"( __    __                 _              ___              )");
    logger.info(R"(/ / /\ \ \___   ___   __| | ___ _ __   / _ \__  _____    )");
    logger.info(R"(\ \/  \/ / _ \ / _ \ / _` |/ _ \ '_ \ / /_)/ / / / _ \   )");
    logger.info(R"( \  /\  / (_) | (_) | (_| |  __/ | | / ___/\ V /  __/    )");
    logger.info(R"(  \/  \/ \___/ \___/ \__,_|\___|_| |_\/     \_/ \___|    )");
    logger.info("");
    logger.info("  Author: VincentZyu");
    logger.info("  A simple schematic loader for LeviLamina");
    logger.info("");

    // Create schematic directory if not exists
    auto schematicPath = std::filesystem::path(getSelf().getDataDir().string()) / "schematics";
    if (!std::filesystem::exists(schematicPath)) {
        std::filesystem::create_directories(schematicPath);
        logger.info("Created schematics directory: {}", schematicPath.string());
    }

    logger.info("WoodenAxe mod loaded!");
    return true;
}

bool WoodenAxeMod::enable() {
    auto& logger = getSelf().getLogger();
    logger.info("Enabling WoodenAxe...");

    // Register event handlers
    registerEventHandlers();
    
    // Register commands
    registerCommands();

    logger.info("WoodenAxe enabled!");
    return true;
}

bool WoodenAxeMod::disable() {
    auto& logger = getSelf().getLogger();
    logger.info("Disabling WoodenAxe...");

    // Cleanup
    mSelections.clear();

    logger.info("WoodenAxe disabled!");
    return true;
}

void WoodenAxeMod::setPos1(const std::string& playerName, const BlockPos& pos, int dim) {
    auto& sel = mSelections[playerName];
    sel.pos1 = pos;
    sel.dimension = dim;
}

void WoodenAxeMod::setPos2(const std::string& playerName, const BlockPos& pos, int dim) {
    auto& sel = mSelections[playerName];
    sel.pos2 = pos;
    sel.dimension = dim;
}

std::optional<PlayerSelection> WoodenAxeMod::getSelection(const std::string& playerName) const {
    auto it = mSelections.find(playerName);
    if (it != mSelections.end()) {
        return it->second;
    }
    return std::nullopt;
}

void WoodenAxeMod::clearSelection(const std::string& playerName) {
    mSelections.erase(playerName);
}

std::string WoodenAxeMod::getSchematicDir() const {
    return (std::filesystem::path(getSelf().getDataDir().string()) / "schematics").string();
}

} // namespace wooden_axe

LL_REGISTER_MOD(wooden_axe::WoodenAxeMod, wooden_axe::WoodenAxeMod::getInstance());
