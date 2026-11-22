#pragma once
#include "ui.hpp"

// =====================================================================
// NullCore Settings — Interface and Sound Effects.
// Settings callbacks continue to update the same Geode mod settings.
// =====================================================================
class NullCoreSettingsPopup : public NCWindow {
protected:
    inline static NullCoreSettingsPopup* s_open = nullptr;

    static std::string str(char const* key) {
        return Mod::get()->getSettingValue<std::string>(key);
    }

    static std::string nextOption(
        std::vector<std::string> const& options,
        std::string const& current
    ) {
        for (size_t i = 0; i < options.size(); ++i) {
            if (options[i] == current)
                return options[(i + 1) % options.size()];
        }
        return options[0];
    }

    void reopen() {
        this->onClose(nullptr);
        queueInMainThread([] { NullCoreSettingsPopup::open(); });
    }

    bool init() override {
        s_open = this;
        return initWindow(380.f, 370.f, {
            {"Interface", "nullcore-interface-icon.png"},
            {"Sound Effects", "nullcore-soundeffecticon.png"},
        });
    }

    void buildTab(int index) override {
        if (index == 0) buildInterface();
        else buildSound();
    }

    void buildInterface() {
        addChoice(0, 0, "Theme", str("theme"), [this]() {
            Mod::get()->setSettingValue<std::string>(
                "theme", nextOption({ "Light", "Dark" }, str("theme"))
            );
            this->reopen();
            return str("theme");
        });

        addChoice(0, 1, "Font", str("font"), [this]() {
            Mod::get()->setSettingValue<std::string>(
                "font",
                nextOption(
                    { "Pusab", "FredokaOne", "Rubik", "Arial", "PressStart2P", "Chopin Script" },
                    str("font")
                )
            );
            this->reopen();
            return str("font");
        });

        addCheckbox(0, 2, "Floating Button",
            Mod::get()->getSettingValue<bool>("show-button"),
            [](bool on) {
                Mod::get()->setSettingValue<bool>("show-button", on);
            }
        );

        addChoice(0, 3, "Button Size", str("button-size"), []() {
            auto next = nextOption(
                { "Small", "Medium", "Large" }, str("button-size")
            );
            Mod::get()->setSettingValue<std::string>("button-size", next);
            return next;
        });

        double opacity = Mod::get()->getSettingValue<double>("button-opacity");
        addNumber(0, 4, "Button Opacity",
            fmt::format("{}", static_cast<int>(std::round(opacity * 100.0))),
            [](std::string const& value, TextInput* input) {
                float v = geode::utils::numFromString<float>(value).unwrapOr(0.f);
                if (v <= 0.f) return;
                if (v > 100.f) {
                    v = 100.f;
                    input->setString("100");
                }
                v = std::max(v, 20.f);
                Mod::get()->setSettingValue<double>(
                    "button-opacity", static_cast<double>(v) / 100.0
                );
            }
        );

        addCheckbox(0, 5, "Auto-Check Updates",
            Mod::get()->getSettingValue<bool>("check-updates"),
            [](bool on) {
                Mod::get()->setSettingValue<bool>("check-updates", on);
            }
        );

        addNote("Changes apply to the NullCore interface.");
    }

    void buildSound() {
        auto label = theme::text("Sound effects are coming soon.", .36f, m_w - 130.f);
        if (label) {
            label->setColor(theme::noteColor());
            addNode(label, { 10.f + 96.f + 8.f + (m_w - 10.f - 96.f - 8.f) / 2.f, m_h / 2.f });
        }
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
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    static void open() {
        if (s_open) return;
        if (auto popup = create()) popup->show();
    }
};
