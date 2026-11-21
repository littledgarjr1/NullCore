#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GJGameLevel.hpp>
#include "settings.hpp"

using namespace geode::prelude;

// ---------------------------------------------------------------- Noclip
class $modify(NCPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Never swallow the anti-cheat spike, only normal deaths.
        if (nc::get("noclip") && object != m_anticheatSpike) return;
        PlayLayer::destroyPlayer(player, object);
    }
};

// ------------------------------------------------------------- Speedhack
class $modify(NCScheduler, CCScheduler) {
    void update(float dt) {
        if (nc::get("speedhack")) dt *= nc::speed();
        CCScheduler::update(dt);
    }
};

// ------------------------------------------------------------- Safe Mode
// While Safe Mode is on, progress is never written to the level.
class $modify(NCGameLevel, GJGameLevel) {
    void savePercentage(int percent, bool isPracticeMode, int clicks, int attempts, bool isChkValid) {
        if (nc::safeMode()) return;
        GJGameLevel::savePercentage(percent, isPracticeMode, clicks, attempts, isChkValid);
    }
};

// TODO next: TPS Bypass (use nc::tps() so the 240 cap applies in Safe Mode),
//            Auto Clicker, Macro (record/replay).