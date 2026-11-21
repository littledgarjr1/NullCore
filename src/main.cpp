#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "popups.hpp"

using namespace geode::prelude;

class $modify(NCMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto bottomMenu = this->getChildByID("bottom-menu");
        if (!bottomMenu) return true;

        auto icon = CCSprite::create("nullcore-icon.png"_spr);
        if (!icon) {
            log::error("NullCore: icon not found");
            return true;
        }

        // match the height of the other bottom-menu buttons (54), keep the icon round
        icon->setScale(54.f / icon->getContentSize().height);

        auto button = CCMenuItemSpriteExtra::create(
            icon, this, menu_selector(NCMenuLayer::onNullCore));
        button->setID("nullcore-button"_spr);

        bottomMenu->addChild(button);
        bottomMenu->updateLayout();
        return true;
    }

    void onNullCore(CCObject*) {
        NullCorePopup::create()->show();
    }
};