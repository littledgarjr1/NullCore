#pragma once
#include "ui.hpp"
#include "updater.hpp"

// =====================================================================
// NullCore main menu — Home and Player.
// Single-column cards are sized for the new sidebar content area.
// =====================================================================
class NullCorePopup : public NCWindow {
protected:
    inline static NullCorePopup* s_open = nullptr;

    static float parse(std::string const& s, float fallback) {
        return geode::utils::numFromString<float>(s).unwrapOr(fallback);
    }

    static std::string numText(float v) {
        return fmt::format("{}", v);
    }

    bool init() override {
        s_open = this;
        return initWindow(380.f, 270.f, {
            {"Home", "nullcore-homeicon.png"},
            {"Player", "nullcore-player_2-icon.png"},
            {"Macro", "nullcore-homeicon.png"},
        });
    }

    void buildTab(int index) override {
        if (index == 0) buildHome();
        else if (index == 1) buildPlayer();
        else buildMacro();
    }

    void buildHome() {
        addNumber(0, 0, "TPS Bypass", fmt::format("{}", static_cast<int>(nc::tps())),
            [](std::string const& s, TextInput* in) {
                float v = parse(s, 0.f);
                if (v < 1.f) return;
                float cap = nc::safeMode() ? nc::SAFE_MODE_MAX_TPS : 100000.f;
                if (v > cap) {
                    v = cap;
                    in->setString(fmt::format("{}", static_cast<int>(v)));
                }
                nc::setTps(std::max(v, 10.f));
            });

        addNumber(0, 1, "Speed Hack", numText(nc::speed()),
            [](std::string const& s, TextInput* in) {
                float v = parse(s, 0.f);
                if (v <= 0.f) return;
                if (v > 20.f) {
                    v = 20.f;
                    in->setString("20");
                }
                nc::setSpeed(std::max(v, 0.05f));
            });

        addCheckbox(0, 2, "Safe Mode", nc::safeMode(), [this](bool on) {
            if (on) {
                nc::set("safe-mode", true);
                this->refreshLater();
                return;
            }

            Ref<NullCorePopup> self = this;
            createQuickPopup(
                "Warning",
                "Turning off <cr>Safe Mode</c> means your progress will be saved while using hacks.\n"
                "This can lead to <cr>leaderboard bans</c>, or a <cr>ban on your account</c> if you look suspicious.\n\n"
                "Are you sure?",
                "Cancel", "Turn Off",
                [self](FLAlertLayer*, bool confirmed) {
                    if (confirmed) nc::set("safe-mode", false);
                    self->refreshLater();
                }
            );
        });

        addNote(nc::safeMode()
            ? "Safe Mode ON  |  Progress is not saved; TPS capped at 240."
            : "Safe Mode OFF  |  Progress will be saved.");
    }

    void buildPlayer() {
        addCheckbox(0, 0, "Noclip", nc::get("noclip"),
            [](bool on) { nc::set("noclip", on); });

        addCheckboxWithGear(0, 1, "Auto Clicker", nc::get("auto-clicker"),
            [](bool on) { nc::set("auto-clicker", on); },
            [] {
                createQuickPopup(
                    "Auto Clicker Settings",
                    fmt::format(
                        "The Auto Clicker follows your current TPS rate.\n\n"
                        "Current rate: <cy>{} CPS</c>\n"
                        "Change TPS on the Home tab to change the click rate.",
                        static_cast<int>(std::round(nc::tps()))
                    ),
                    "Close", nullptr, [](FLAlertLayer*, bool) {}
                );
            }
        );

        addNote("Auto Clicker CPS follows your current TPS.");
    }
    void buildMacro() {
        addNote("Macro tools are coming soon.");
    }

    void onClose(CCObject* sender) override {
        s_open = nullptr;
        Popup::onClose(sender);
    }

public:
    ~NullCorePopup() {
        if (s_open == this) s_open = nullptr;
    }

    static NullCorePopup* create() {
        auto ret = new NullCorePopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    static void toggle() {
        if (s_open) {
            s_open->onClose(nullptr);
            return;
        }
        if (auto popup = create()) popup->show();
    }
};
