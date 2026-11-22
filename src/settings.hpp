#pragma once
#include <Geode/Geode.hpp>
#include <algorithm>
#include <string>

// Central place for NullCore's saved state (persists between launches).
namespace nc {
    constexpr float SAFE_MODE_MAX_TPS = 240.f;

    inline bool get(char const* key) {
        return geode::Mod::get()->getSavedValue<bool>(key, false);
    }
    inline void set(char const* key, bool value) {
        geode::Mod::get()->setSavedValue<bool>(key, value);
    }

    // Auto Clicker target: Player 1, Player 2, or Both.
    inline std::string autoClickerTarget() {
        auto target = geode::Mod::get()->getSavedValue<std::string>("auto-clicker-target", "Player 1");
        if (target != "Player 1" && target != "Player 2" && target != "Both")
            return "Player 1";
        return target;
    }
    inline void setAutoClickerTarget(std::string const& target) {
        if (target == "Player 1" || target == "Player 2" || target == "Both")
            geode::Mod::get()->setSavedValue<std::string>("auto-clicker-target", target);
    }

    // Safe Mode defaults to ON.
    inline bool safeMode() {
        return geode::Mod::get()->getSavedValue<bool>("safe-mode", true);
    }

    inline float speed() {
        return geode::Mod::get()->getSavedValue<float>("speed-value", 1.f);
    }
    inline void setSpeed(float v) {
        geode::Mod::get()->setSavedValue<float>("speed-value", v);
    }

    // The TPS the user picked, ignoring Safe Mode.
    inline float tpsRaw() {
        return geode::Mod::get()->getSavedValue<float>("tps-value", 240.f);
    }
    inline void setTps(float v) {
        geode::Mod::get()->setSavedValue<float>("tps-value", v);
    }
    // The TPS actually used. Safe Mode caps it at 240.
    inline float tps() {
        float v = tpsRaw();
        return safeMode() ? std::min(v, SAFE_MODE_MAX_TPS) : v;
    }
}
