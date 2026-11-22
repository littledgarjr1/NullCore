#pragma once
#include <Geode/Geode.hpp>
#include "settings.hpp"
#include "updater.hpp"

using namespace geode::prelude;

// ---------------------------------------------------------------- Theme
// Black and white, with grey outlines.
namespace nc::theme {
    inline ccColor4F black() { return {0.f, 0.f, 0.f, 1.f}; }
    inline ccColor4F white() { return {1.f, 1.f, 1.f, 1.f}; }
    inline ccColor4F grey()  { return {0.45f, 0.45f, 0.45f, 1.f}; }
    inline ccColor3B dimText() { return {130, 130, 130}; }

    // A filled rectangle with a grey outline, drawn from (0,0) to (w,h).
    inline CCDrawNode* rect(float w, float h, ccColor4F fill, float border = 1.5f) {
        auto d = CCDrawNode::create();
        CCPoint v[4] = {CCPoint(0.f, 0.f), CCPoint(w, 0.f), CCPoint(w, h), CCPoint(0.f, h)};
        d->drawPolygon(v, 4, fill, border, grey());
        return d;
    }

    // A black button face with centered white text.
    inline CCNode* labelBox(float w, float h, char const* text, float scale, CCLabelBMFont** outLabel = nullptr) {
        auto node = CCNode::create();
        node->setContentSize(CCSize(w, h));
        node->addChild(rect(w, h, black()));
        auto label = CCLabelBMFont::create(text, "bigFont.fnt");
        label->setScale(scale);
        label->setPosition(CCPoint(w / 2.f, h / 2.f));
        node->addChild(label);
        if (outLabel) *outLabel = label;
        return node;
    }

    // Flat switch: black with a grey knob when off, white with a black knob when on.
    inline CCNode* toggleSwitch(bool on) {
        constexpr float W = 44.f, H = 22.f;
        auto node = CCNode::create();
        node->setContentSize(CCSize(W, H));
        node->addChild(rect(W, H, on ? white() : black()));
        float kx = on ? W - 19.f : 3.f;
        CCPoint k[4] = {CCPoint(kx, 3.f), CCPoint(kx + 16.f, 3.f), CCPoint(kx + 16.f, H - 3.f), CCPoint(kx, H - 3.f)};
        auto knob = CCDrawNode::create();
        knob->drawPolygon(k, 4, on ? black() : grey(), 0.5f, on ? black() : grey());
        node->addChild(knob);
        return node;
    }
}

// Short alias so the menu code can write theme::black() etc.
namespace theme = nc::theme;

// ---------------------------------------------------------------- Menu
class NullCorePopup : public geode::Popup {
protected:
    struct Row { char const* key; char const* label; bool wip; };
    static constexpr Row ROWS[] = {
        {"noclip",      "Noclip",                  false},
        {"speedhack",   "Speedhack",               false},
        {"autoclicker", "Auto Clicker (soon)",     true},
        {"tps-bypass",  "TPS Bypass",              false},
        {"macro",       "Macro (soon)",            true},
        {"safe-mode",   "Safe Mode",               false},
    };

    CCLabelBMFont* m_speedLabel = nullptr;
    CCLabelBMFont* m_tpsLabel = nullptr;
    CCLabelBMFont* m_status = nullptr;
    inline static NullCorePopup* s_open = nullptr;

    bool init() override {
        if (!Popup::init(300.f, 300.f)) return false;
        s_open = this;

        // Replace the stock GD look with our own panel.
        m_bgSprite->setVisible(false);
        m_closeBtn->setVisible(false);
        m_mainLayer->addChildAtPosition(
            theme::rect(m_size.width, m_size.height, theme::black(), 2.f),
            Anchor::BottomLeft, ccp(0.f, 0.f)
        );

        auto title = CCLabelBMFont::create("NullCore", "bigFont.fnt");
        title->setScale(0.7f);
        m_mainLayer->addChildAtPosition(title, Anchor::Top, ccp(0.f, -26.f));

        m_status = CCLabelBMFont::create("", "bigFont.fnt");
        m_status->setScale(0.28f);
        m_status->setColor(theme::dimText());
        m_mainLayer->addChildAtPosition(m_status, Anchor::Top, ccp(0.f, -48.f));

        float y = m_size.height - 82.f;
        for (auto const& r : ROWS) {
            std::string key = r.key;
            bool on = !r.wip && (key == "safe-mode" ? nc::safeMode() : nc::get(r.key));

            auto toggler = CCMenuItemToggler::create(
                theme::toggleSwitch(false), theme::toggleSwitch(true),
                this, menu_selector(NullCorePopup::onToggle)
            );
            toggler->setID(r.key);
            toggler->toggle(on);
            toggler->setEnabled(!r.wip);
            m_buttonMenu->addChildAtPosition(toggler, Anchor::BottomLeft, ccp(44.f, y));

            auto label = CCLabelBMFont::create(r.label, "bigFont.fnt");
            label->setScale(0.42f);
            if (r.wip) label->setColor(theme::dimText());
            m_mainLayer->addChildAtPosition(label, Anchor::BottomLeft, ccp(74.f, y), ccp(0.f, 0.5f));

            if (key == "speedhack") {
                auto box = theme::labelBox(70.f, 22.f, speedText().c_str(), 0.35f, &m_speedLabel);
                auto btn = CCMenuItemSpriteExtra::create(box, this, menu_selector(NullCorePopup::onSpeed));
                m_buttonMenu->addChildAtPosition(btn, Anchor::BottomLeft, ccp(m_size.width - 52.f, y));
            }
            if (key == "tps-bypass") {
                auto box = theme::labelBox(70.f, 22.f, tpsText().c_str(), 0.3f, &m_tpsLabel);
                auto btn = CCMenuItemSpriteExtra::create(box, this, menu_selector(NullCorePopup::onTps));
                m_buttonMenu->addChildAtPosition(btn, Anchor::BottomLeft, ccp(m_size.width - 52.f, y));
            }
            y -= 32.f;
        }

        // Check for updates
        auto upd = theme::labelBox(180.f, 24.f, "Check for updates", 0.32f);
        auto updBtn = CCMenuItemSpriteExtra::create(upd, this, menu_selector(NullCorePopup::onCheckUpdates));
        m_buttonMenu->addChildAtPosition(updBtn, Anchor::Bottom, ccp(0.f, 26.f));

        // Close button (top right)
        auto x = theme::labelBox(24.f, 24.f, "X", 0.4f);
        auto closeBtn = CCMenuItemSpriteExtra::create(x, this, menu_selector(NullCorePopup::onClose));
        m_buttonMenu->addChildAtPosition(closeBtn, Anchor::TopRight, ccp(-20.f, -20.f));

        refreshLabels();
        return true;
    }

    std::string speedText() { return fmt::format("{:.2f}x", nc::speed()); }
    std::string tpsText()   { return fmt::format("{} TPS", static_cast<int>(nc::tps())); }

    void refreshLabels() {
        if (m_speedLabel) m_speedLabel->setString(speedText().c_str());
        if (m_tpsLabel) m_tpsLabel->setString(tpsText().c_str());
        if (m_status) {
            m_status->setString(nc::safeMode()
                ? "SAFE MODE ON - progress is not saved"
                : "SAFE MODE OFF - progress WILL be saved");
        }
    }

    void onSpeed(CCObject*) {
        static constexpr float presets[] = {0.25f, 0.5f, 0.75f, 1.f, 1.5f, 2.f, 3.f};
        float cur = nc::speed();
        float next = presets[0];
        for (float p : presets) {
            if (p > cur + 0.001f) { next = p; break; }
        }
        nc::setSpeed(next);
        refreshLabels();
    }

    void onTps(CCObject*) {
        static constexpr float presets[] = {60.f, 120.f, 240.f, 480.f, 960.f};
        float limit = nc::safeMode() ? nc::SAFE_MODE_MAX_TPS : 100000.f;
        float cur = nc::tps();
        float next = presets[0];
        for (float p : presets) {
            if (p > cur + 0.5f && p <= limit) { next = p; break; }
        }
        nc::setTps(next);
        refreshLabels();
    }

    void onCheckUpdates(CCObject*) {
        nc::updater::check(true);
    }

    void onToggle(CCObject* sender) {
        auto toggler = static_cast<CCMenuItemToggler*>(sender);
        std::string key = toggler->getID();
        bool turningOn = !toggler->isToggled(); // the callback runs before the visual flip

        if (key == "safe-mode" && !turningOn) {
            // Turning Safe Mode OFF needs confirmation.
            Ref<CCMenuItemToggler> ref = toggler;
            Ref<NullCorePopup> self = this;
            createQuickPopup(
                "Warning",
                "Turning off <cr>Safe Mode</c> means your progress will be saved while using hacks.\n"
                "This can lead to <cr>leaderboard bans</c>, or a <cr>ban on your account</c> if you look suspicious.\n\n"
                "Are you sure?",
                "Cancel", "Turn Off",
                [ref, self](FLAlertLayer*, bool confirmed) {
                    if (confirmed) {
                        nc::set("safe-mode", false);
                        self->refreshLabels();
                    }
                    else {
                        ref->toggle(true); // put the switch back to ON
                    }
                }
            );
            return;
        }

        nc::set(key.c_str(), turningOn);
        if (key == "safe-mode") refreshLabels();
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