#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GJGameLevel.hpp>
#include "settings.hpp"

using namespace geode::prelude;

// ---------------------------------------------------------------- Noclip
// Works with Safe Mode on or off.
class $modify(NCPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Never swallow the anti-cheat spike, only normal deaths.
        if (nc::get("noclip") && object != m_anticheatSpike) return;
        PlayLayer::destroyPlayer(player, object);
    }
};

// ------------------------------------------------- Speedhack + TPS Bypass
// Speedhack: 1 = normal speed; any other value scales the time passed to the game.
// TPS Bypass: 240 = the game's normal rate. Any other value splits each frame's time into
// fixed slices of 1/TPS seconds and runs the game's update once per slice (only inside a level).
// Both work with Safe Mode on or off; Safe Mode only caps TPS at 240.
class $modify(NCScheduler, CCScheduler) {
    void update(float dt) {
        float speed = nc::speed();
        if (std::abs(speed - 1.f) > 0.0001f) dt *= speed;

        static double s_accumulator = 0.0;
        constexpr int MAX_STEPS_PER_FRAME = 48; // protects slow devices from freezing

        float tps = nc::tps();
        bool slicing = std::abs(tps - 240.f) > 0.5f && PlayLayer::get();
        if (!slicing) {
            s_accumulator = 0.0;
            CCScheduler::update(dt);
            return;
        }

        double step = 1.0 / static_cast<double>(tps);
        s_accumulator += dt;

        int steps = static_cast<int>(s_accumulator / step);
        s_accumulator -= steps * step;
        if (steps > MAX_STEPS_PER_FRAME) steps = MAX_STEPS_PER_FRAME; // drop the overflow

        for (int i = 0; i < steps; i++) {
            CCScheduler::update(static_cast<float>(step));
        }
    }
};

// ------------------------------------------------------------- Safe Mode
// While Safe Mode is on, level progress is never saved. Hacks still work.
class $modify(NCGameLevel, GJGameLevel) {
    void savePercentage(int percent, bool isPracticeMode, int clicks, int attempts, bool isChkValid) {
        if (nc::safeMode()) return;
        GJGameLevel::savePercentage(percent, isPracticeMode, clicks, attempts, isChkValid);
    }
};

// TODO next: Auto Clicker, Macro.
