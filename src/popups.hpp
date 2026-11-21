#pragma once
#include <Geode/Geode.hpp>
#include "settings.hpp"

using namespace geode::prelude;

class NullCorePopup : public geode::Popup {
protected:
    struct Entry { char const* key; char const* label; bool wip; };
    static constexpr Entry ENTRIES[] = {
        {"noclip",      "Noclip",       false},
        {"speedhack",   "Speedhack",    false},
        {"autoclicker", "Auto Clicker", true},
        {"tps-bypass",  "TPS Bypass",   true},
        {"macro",       "Macro",        true},
        {"safe-mode",   "Safe Mode",    false},
    };

    ButtonSprite* m_speedSprite = nullptr;

    bool init() {
        if (!Popup::init(300.f, 260.f)) return false;
        this->setTitle("NullCore");

        auto size = m_mainLayer->getContentSize();
        float y = size.height - 55.f;

        for (auto const& e : ENTRIES) {
            auto toggler = CCMenuItemToggler::createWithStandardSprites(
                this, menu_selector(NullCorePopup::onToggle), 0.6f);
            toggler->setID(e.key);
            bool on = std::string(e.key) == "safe-mode" ? nc::safeMode() : nc::get(e.key);
            toggler->toggle(on);
            toggler->setPosition({40.f, y});
            m_buttonMenu->addChild(toggler);

            std::string text = e.label;
            if (e.wip) text += " (WIP)";
            auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
            label->setScale(0.5f);
            label->setAnchorPoint({0.f, 0.5f});
            label->setPosition({65.f, y});
            m_mainLayer->addChild(label);

            y -= 32.f;
        }

        // Speed preset button (cycles through values)
        m_speedSprite = ButtonSprite::create(speedText().c_str(), "bigFont.fnt", "GJ_button_04.png", 0.6f);
        auto speedBtn = CCMenuItemSpriteExtra::create(
            m_speedSprite, this, menu_selector(NullCorePopup::onSpeed));
        speedBtn->setPosition({size.width - 65.f, size.height - 87.f});
        m_buttonMenu->addChild(speedBtn);

        return true;
    }

    std::string speedText() {
        return fmt::format("{:.2f}x", nc::speed());
    }

    void onSpeed(CCObject*) {
        static constexpr float presets[] = {0.25f, 0.5f, 0.75f, 1.f, 1.5f, 2.f, 3.f};
        float cur = nc::speed();
        float next = presets[0];
        for (size_t i = 0; i < std::size(presets); i++) {
            if (presets[i] > cur + 0.001f) { next = presets[i]; break; }
        }
        nc::setSpeed(next);
        m_speedSprite->setString(speedText().c_str());
    }

    void onToggle(CCObject* sender) {
        auto toggler = static_cast<CCMenuItemToggler*>(sender);
        std::string key = toggler->getID();
        bool turningOn = !toggler->isToggled(); // callback runs before the visual flip

        if (key == "safe-mode" && !turningOn) {
            // Turning Safe Mode OFF needs explicit confirmation.
            Ref<CCMenuItemToggler> ref = toggler;
            createQuickPopup(
                "Warning",
                "Turning off <cr>Safe Mode</c> means your progress will be saved while using hacks.\n"
                "This can lead to <cr>leaderboard bans</c>, or a <cr>ban on your account</c> if you look suspicious.\n\n"
                "Are you sure?",
                "Cancel", "Turn Off",
                [ref](auto*, bool confirmed) {
                    if (confirmed) nc::set("safe-mode", false);
                    else ref->toggle(true); // put the toggle back to ON
                }
            );
            return;
        }

        nc::set(key.c_str(), turningOn);
    }

public:
    static NullCorePopup* create() {
        auto ret = new NullCorePopup();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};