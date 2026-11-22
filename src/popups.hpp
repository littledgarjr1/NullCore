#pragma once
#include "ui.hpp"
#include "updater.hpp"

// =====================================================================
//  The NullCore mod menu (opened by the floating button).
//  Tabs: Home, Player.
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
            {"Home",   "nullcore-homeicon.png"},
            {"Player", "nullcore-player_2-icon.png"},
        });
    }

    void buildTab(int index) override {
        if (index == 0) buildHome();
        else buildPlayer();
    }

    void buildHome() {
        // TPS Bypass: 240 is the game's normal rate. Safe Mode caps it at 240.
        addNumber(0, 0, "TPS\nBypass", fmt::format("{}", static_cast<int>(nc::tps())),
            [](std::string const& s, TextInput* in) {
                float v = parse(s, 0.f);
                if (v < 1.f) return;               // empty or invalid: ignore while typing
                float cap = nc::safeMode() ? nc::SAFE_MODE_MAX_TPS : 100000.f;
                if (v > cap) {
                    v = cap;
                    in->setString(fmt::format("{}", static_cast<int>(v)));
                }
                nc::setTps(std::max(v, 10.f));
            });

        // Speedhack: 1 is normal speed.
        addNumber(1, 0, "Speed\nHack", numText(nc::speed()),
            [](std::string const& s, TextInput* in) {
                float v = parse(s, 0.f);
                if (v <= 0.f) return;
                if (v > 20.f) {
                    v = 20.f;
                    in->setString("20");
                }
                nc::setSpeed(std::max(v, 0.05f));
            });

        addCheckbox(0, 1, "Safe Mode", nc::safeMode(), [this](bool on) {
            if (on) {
                nc::set("safe-mode", true);
                this->refreshLater(); // TPS box shows the capped value
                return;
            }
            // Turning Safe Mode OFF needs confirmation.
            Ref<NullCorePopup> self = this;
            createQuickPopup(
                "Warning",
                "Turning off <cr>Safe Mode</c> means your progress will be saved while using hacks.\n"
                "This can lead to <cr>leaderboard bans</c>, or a <cr>ban on your account</c> if you look suspicious.\n\n"
                "Are you sure?",
                "Cancel", "Turn Off",
                [self](FLAlertLayer*, bool confirmed) {
                    if (confirmed) nc::set("safe-mode", false);
                    self->refreshLater(); // redraws the checkbox in its real state
                }
            );
        });

        addNote(nc::safeMode()
            ? "Safe Mode is ON: progress is not saved and TPS is capped at 240."
            : "Safe Mode is OFF: progress WILL be saved.");
    }

    void buildPlayer() {
        addCheckbox(0, 0, "Noclip", nc::get("noclip"), [](bool on) { nc::set("noclip", on); });
        addCheckbox(1, 0, "Auto\nClicker", false, [](bool) {}, false);
        addCheckbox(0, 1, "Macro", false, [](bool) {}, false);
        addNote("Auto Clicker and Macro are coming soon.");
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
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }

    // Opens the menu, or closes it if it's already open.
    static void toggle() {
        if (s_open) { s_open->onClose(nullptr); return; }
        if (auto p = create()) p->show();
    }
};
