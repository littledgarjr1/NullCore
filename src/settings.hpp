#pragma once
#include <Geode/Geode.hpp>
#include <algorithm>

// Central place for every NullCore setting. Values persist via Geode's saved values.
namespace nc {
    constexpr float SAFE_MODE_MAX_TPS = 240.f;

    inline bool get(char const* key) {
        return geode::Mod::get()->getSavedValue<bool>(key, false);
    }
    inline void set(char const* key, bool value) {
        geode::Mod::get()->setSavedValue<bool>(key, value);
    }

    // Safe Mode defaults to ON so new users are protected out of the box.
    inline bool safeMode() {
        return geode::Mod::get()->getSavedValue<bool>("safe-mode", true);
    }

    inline float speed() {
        return geode::Mod::get()->getSavedValue<float>("speed-value", 1.f);
    }
    inline void setSpeed(float v) {
        geode::Mod::get()->setSavedValue<float>("speed-value", v);
    }

    // The TPS the game should actually use (Safe Mode caps it at 240).
    inline float tps() {
        float v = geode::Mod::get()->getSavedValue<float>("tps-value", 240.f);
        return safeMode() ? std::min(v, SAFE_MODE_MAX_TPS) : v;
    }
}