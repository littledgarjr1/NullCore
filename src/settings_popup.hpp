#pragma once
#include "ui.hpp"

// =====================================================================
//  NullCore Settings (opened from the main menu's bottom row).
//  Tabs: Interface, Sound Effects.
//  Edits the same values as the mod's page in Geode, so both always agree.
// =====================================================================
class NullCoreSettingsPopup : public NCWindow {
protected:
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

    // Close and reopen so the whole window redraws (used when the theme or font changes).
    void reopen() {
        this->onClose(nullptr);
        queueInMainThread([] { NullCoreSettingsPopup::open(); });
    }

    bool init() override {
        s_open = this;
        return initWindow(380.f, 270.f, {
            {"Interface",     "nullcore-interface-icon.png"},
            {"Sound Effects", "nullcore-soundeffecticon.png"},
        });
    }

    void buildTab(int index) override {
        if (index == 0) buildInterface();
        else buildSound();
    }

    void buildInterface() {
        // row 0: Theme | Font
        addChoice(0, 0, "Theme", str("theme"), [this]() {
            Mod::get()->setSettingValue<std::string>("theme", nextOption({"Light", "Dark"}, str("theme")));
            this->reopen();
            return str("theme");
        });
        addChoice(1, 0, "Font", str("font"), [this]() {
            Mod::get()->setSettingValue<std::string>("font",
                nextOption({"Pusab", "FredokaOne", "Rubik", "Arial", "PressStart2P", "Chopin Script"}, str("font")));
            this->reopen();
            return str("font");
        });

        // row 1: Floating Button | Button Size
        addCheckbox(0, 1, "Floating\nButton", Mod::get()->getSettingValue<bool>("show-button"), [](bool on) {
            Mod::get()->setSettingValue<bool>("show-button", on);
        });
        addChoice(1, 1, "Button\nSize", str("button-size"), []() {
            auto next = nextOption({"Small", "Medium", "Large"}, str("button-size"));
            Mod::get()->setSettingValue<std::string>("button-size", next);
            return next;
        });

        // row 2: Button Opacity (percent) | Auto-Check Updates
        double opacity = Mod::get()->getSettingValue<double>("button-opacity");
        addNumber(0, 2, "Button\nOpacity", fmt::format("{}", static_cast<int>(std::round(opacity * 100.0))),
            [](std::string const& s, TextInput* in) {
                float v = geode::utils::numFromString<float>(s).unwrapOr(0.f);
                if (v <= 0.f) return;
                if (v > 100.f) {
                    v = 100.f;
                    in->setString("100");
                }
                v = std::max(v, 20.f); // keep the button visible
                Mod::get()->setSettingValue<double>("button-opacity", static_cast<double>(v) / 100.0);
            });
        addCheckbox(1, 2, "Auto-Check\nUpdates", Mod::get()->getSettingValue<bool>("check-updates"), [](bool on) {
            Mod::get()->setSettingValue<bool>("check-updates", on);
        });
    }

    void buildSound() {
        auto l = theme::text("Sound effects are coming soon.", 0.4f, m_w - 60.f);
        l->setColor(theme::noteColor());
        addNode(l, CCPoint(m_w / 2.f, m_h / 2.f + 14.f));
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