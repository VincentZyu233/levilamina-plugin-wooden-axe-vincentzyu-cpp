#include "mod/EventHandlers.h"
#include "mod/WoodenAxeMod.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/ListenerBase.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStack.h"

namespace wooden_axe {

static ll::event::ListenerPtr interactListener;
static ll::event::ListenerPtr destroyListener;

// Check if player is holding wooden axe
static bool isHoldingWoodenAxe(Player& player) {
    // Get the item in main hand
    const auto& item = player.getSelectedItem();
    
    // Check if it's a wooden axe
    std::string itemName = item.getTypeName();
    return itemName == "minecraft:wooden_axe";
}

void registerEventHandlers() {
    auto& logger = WoodenAxeMod::getInstance().getSelf().getLogger();
    auto& eventBus = ll::event::EventBus::getInstance();
    
    // Right-click event - set pos2
    interactListener = eventBus.emplaceListener<ll::event::player::PlayerInteractBlockEvent>(
        [&logger](ll::event::player::PlayerInteractBlockEvent& event) {
            auto& player = event.self();
            
            if (!isHoldingWoodenAxe(player)) {
                return;
            }
            
            // Check permission (OP level)
            if (player.getPlayerPermissionLevel() < PlayerPermissionLevel::Operator) {
                return;
            }
            
            auto blockPos = event.blockPos();
            std::string playerName = player.getRealName();
            int dim = player.getDimensionId().id;
            
            BlockPos pos(blockPos.x, blockPos.y, blockPos.z);
            WoodenAxeMod::getInstance().setPos2(playerName, pos, dim);
            
            logger.debug("Player {} set pos2: ({}, {}, {})", playerName, pos.x, pos.y, pos.z);
            
            // Send message to player
            player.sendMessage("§aPos2 set to ({}, {}, {})", pos.x, pos.y, pos.z);
            
            // Cancel the event to prevent normal block interaction
            event.cancel();
        }
    );
    
    // Left-click (destroy attempt) event - set pos1
    destroyListener = eventBus.emplaceListener<ll::event::player::PlayerDestroyBlockEvent>(
        [&logger](ll::event::player::PlayerDestroyBlockEvent& event) {
            auto& player = event.self();
            
            if (!isHoldingWoodenAxe(player)) {
                return;
            }
            
            // Check permission (OP level)
            if (player.getPlayerPermissionLevel() < PlayerPermissionLevel::Operator) {
                return;
            }
            
            auto blockPos = event.pos();
            std::string playerName = player.getRealName();
            int dim = player.getDimensionId().id;
            
            BlockPos pos(blockPos.x, blockPos.y, blockPos.z);
            WoodenAxeMod::getInstance().setPos1(playerName, pos, dim);
            
            logger.debug("Player {} set pos1: ({}, {}, {})", playerName, pos.x, pos.y, pos.z);
            
            // Send message to player
            player.sendMessage("§aPos1 set to ({}, {}, {})", pos.x, pos.y, pos.z);
            
            // Cancel the event to prevent block destruction
            event.cancel();
        }
    );
    
    logger.info("Event handlers registered");
}

void unregisterEventHandlers() {
    auto& eventBus = ll::event::EventBus::getInstance();
    
    if (interactListener) {
        eventBus.removeListener(interactListener);
        interactListener = nullptr;
    }
    
    if (destroyListener) {
        eventBus.removeListener(destroyListener);
        destroyListener = nullptr;
    }
    
    WoodenAxeMod::getInstance().getSelf().getLogger().info("Event handlers unregistered");
}

} // namespace wooden_axe
