#pragma once

#include "ll/api/mod/NativeMod.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <optional>

namespace wooden_axe {

// Forward declarations
struct BlockPos {
    int x, y, z;
    BlockPos(int x = 0, int y = 0, int z = 0) : x(x), y(y), z(z) {}
};

struct PlayerSelection {
    std::optional<BlockPos> pos1;
    std::optional<BlockPos> pos2;
    int dimension = 0;
};

class WoodenAxeMod {
public:
    static WoodenAxeMod& getInstance();

    WoodenAxeMod() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();

    // Selection management
    void setPos1(const std::string& playerName, const BlockPos& pos, int dim);
    void setPos2(const std::string& playerName, const BlockPos& pos, int dim);
    std::optional<PlayerSelection> getSelection(const std::string& playerName) const;
    void clearSelection(const std::string& playerName);

    // Config
    std::string getSchematicDir() const;

private:
    ll::mod::NativeMod& mSelf;
    std::unordered_map<std::string, PlayerSelection> mSelections;
};

} // namespace wooden_axe
