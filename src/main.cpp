#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/OverlayManager.hpp>
#include "button.hpp"
#include "updater.hpp"

using namespace geode::prelude;

class $modify(NCMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        // Runs once per game launch.
        static bool s_started = false;
        if (!s_started) {
            s_started = true;

            // The floating button lives in Geode's overlay, so it shows on every screen.
            if (auto btn = nc::FloatingButton::create()) {
                OverlayManager::get()->addChild(btn);
            }

            if (Mod::get()->getSettingValue<bool>("check-updates")) {
                nc::updater::check(false);
            }
        }
        return true;
    }
};
