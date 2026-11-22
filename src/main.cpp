#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/OverlayManager.hpp>
#include "button.hpp"
#include "settings_popup.hpp"
#include "updater.hpp"

using namespace geode::prelude;

class $modify(NCMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        // Settings button in the main menu's bottom row.
        if (auto bottomMenu = this->getChildByID("bottom-menu")) {
            if (auto icon = CCSprite::create("nullcore-settings.png"_spr)) {
                // match the height of the other bottom-menu buttons
                icon->setScale(50.f / icon->getContentSize().height);

                auto button = CCMenuItemSpriteExtra::create(
                    icon, this, menu_selector(NCMenuLayer::onNullCoreSettings)
                );
                button->setID("nullcore-settings-button"_spr);
                bottomMenu->addChild(button);
                bottomMenu->updateLayout();
            }
            else {
                log::error("NullCore: icon not found");
            }
        }

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

    void onNullCoreSettings(CCObject*) {
        NullCoreSettingsPopup::open();
    }
};
