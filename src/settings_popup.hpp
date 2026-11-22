#pragma once
#include <Geode/Geode.hpp>
#include "popups.hpp"
#include "button.hpp"

using namespace geode::prelude;

// NullCore's own settings screen (opened from the main menu's bottom row).
// It edits the same values as the mod's page in Geode, so both always agree.
class NullCoreSettingsPopup : public geode::Popup {
protected:
    CCLabelBMFont* m_sizeLabel = nullptr;
    CCLabelBMFont* m_opacityLabel = nullptr;
    inline static NullCoreSettingsPopup* s_open = nullptr;

    static std::string str(char const* key) {
        return Mod::get()->getSettingValue<std::string>(key);
    }

    static std::string nextOption(std::vector<std::string> const& opts, std::string const& cur) {
        for (size_t i = 0; i < opts.size(); i++) {
            if (opts[i] == cur) return opts[(i + 1) % opts.size()];
        }
        return opts[0];
    }

    static std::string opacityText() {
        double o = Mod::get()->getSettingValue<double>("button-opacity");
        return fmt::format("{}%", static_cast<int>(std::round(o * 100.0)));
    }

    void addLabel(char const* text, float y) {
        auto label = CCLabelBMFont::create(text, "bigFont.fnt");
        label->setScale(0.42f);
        label->setColor(theme::fgText());
        m_mainLayer->addChildAtPosition(label, Anchor::BottomLeft, ccp(20.f, y), ccp(0.f, 0.5f));
    }

    CCLabelBMFont* addCycle(std::string const& text, SEL_MenuHandler callback, float y) {
        CCLabelBMFont* lbl = nullptr;
        auto box = theme::labelBox(90.f, 24.f, text.c_str(), 0.34f, &lbl);
        auto btn = CCMenuItemSpriteExtra::create(box, this, callback);
        m_buttonMenu->addChildAtPosition(btn, Anchor::BottomLeft, ccp(m_size.width - 62.f, y));
        return lbl;
    }

    void addToggle(char const* key, bool value, float y) {
        auto toggler = CCMenuItemToggler::create(
            theme::toggleSwitch(false), theme::toggleSwitch(true),
            this, menu_selector(NullCoreSettingsPopup::onToggle)
        );
        toggler->setID(key);
        toggler->toggle(value);
        m_buttonMenu->addChildAtPosition(toggler, Anchor::BottomLeft, ccp(m_size.width - 40.f, y));
    }

    bool init() override {
        if (!Popup::init(300.f, 290.f)) return false;
        s_open = this;

        m_bgSprite->setVisible(false);
        m_closeBtn->setVisible(false);
        m_mainLayer->addChildAtPosition(
            theme::rect(m_size.width, m_size.height, theme::bg(), 2.f),
            Anchor::BottomLeft, ccp(0.f, 0.f)
        );

        auto title = CCLabelBMFont::create("NullCore Settings", "bigFont.fnt");
        title->setScale(0.6f);
        title->setColor(theme::fgText());
        m_mainLayer->addChildAtPosition(title, Anchor::Top, ccp(0.f, -26.f));

        float y = m_size.height - 74.f;

        addLabel("Theme", y);
        addCycle(str("theme"), menu_selector(NullCoreSettingsPopup::onTheme), y);
        y -= 36.f;

        addLabel("Floating button", y);
        addToggle("show-button", Mod::get()->getSettingValue<bool>("show-button"), y);
        y -= 36.f;

        addLabel("Button size", y);
        m_sizeLabel = addCycle(str("button-size"), menu_selector(NullCoreSettingsPopup::onSize), y);
        y -= 36.f;

        addLabel("Button opacity", y);
        m_opacityLabel = addCycle(opacityText(), menu_selector(NullCoreSettingsPopup::onOpacity), y);
        y -= 36.f;

        addLabel("Auto-check updates", y);
        addToggle("check-updates", Mod::get()->getSettingValue<bool>("check-updates"), y);

        // Reset the floating button's position
        auto reset = theme::labelBox(200.f, 24.f, "Reset button position", 0.32f);
        auto resetBtn = CCMenuItemSpriteExtra::create(reset, this, menu_selector(NullCoreSettingsPopup::onReset));
        m_buttonMenu->addChildAtPosition(resetBtn, Anchor::Bottom, ccp(0.f, 26.f));

        // Close button (top right)
        auto x = theme::labelBox(24.f, 24.f, "X", 0.4f);
        auto closeBtn = CCMenuItemSpriteExtra::create(x, this, menu_selector(NullCoreSettingsPopup::onClose));
        m_buttonMenu->addChildAtPosition(closeBtn, Anchor::TopRight, ccp(-20.f, -20.f));

        return true;
    }

    void onTheme(CCObject*) {
        Mod::get()->setSettingValue<std::string>("theme", nextOption({"Dark", "Light"}, str("theme")));
        // Reopen so the whole screen redraws in the new theme.
        this->onClose(nullptr);
        queueInMainThread([] { NullCoreSettingsPopup::open(); });
    }

    void onSize(CCObject*) {
        auto next = nextOption({"Small", "Medium", "Large"}, str("button-size"));
        Mod::get()->setSettingValue<std::string>("button-size", next);
        m_sizeLabel->setString(next.c_str());
    }

    void onOpacity(CCObject*) {
        static constexpr double presets[] = {1.0, 0.85, 0.7, 0.5};
        double cur = Mod::get()->getSettingValue<double>("button-opacity");
        double next = presets[0];
        for (double p : presets) {
            if (p < cur - 0.001) { next = p; break; }
        }
        Mod::get()->setSettingValue<double>("button-opacity", next);
        m_opacityLabel->setString(opacityText().c_str());
    }

    void onToggle(CCObject* sender) {
        auto toggler = static_cast<CCMenuItemToggler*>(sender);
        bool turningOn = !toggler->isToggled(); // the callback runs before the visual flip
        Mod::get()->setSettingValue<bool>(toggler->getID(), turningOn);
    }

    void onReset(CCObject*) {
        nc::FloatingButton::resetPosition();
    }

    void onClose(CCObject* sender) override {
        s_open = nullptr;
        Popup::onClose(sender);
    }

public:
    ~NullCoreSettingsPopup() {
        if (s_open == this) s_open = nullptr;
    }

    static NullCoreSettingsPopup* create() {
        auto ret = new NullCoreSettingsPopup();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }

    static void open() {
        if (s_open) return;
        if (auto p = create()) p->show();
    }
};
