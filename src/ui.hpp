#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <cmath>
#include <functional>
#include <vector>
#include "settings.hpp"

using namespace geode::prelude;

// =====================================================================
//  Theme: two palettes (Light is the default) and the chosen font.
// =====================================================================
namespace nc::theme {
    inline bool light() {
        return Mod::get()->getSettingValue<std::string>("theme") != "Dark";
    }

    inline ccColor4F rgb(int r, int g, int b) {
        return {r / 255.f, g / 255.f, b / 255.f, 1.f};
    }

    struct Palette {
        ccColor4F frame, frameBorder;   // the window itself
        ccColor4F panel, panelBorder;   // the big content area
        ccColor4F bar, barBorder;       // the tab bar
        ccColor4F tabOn, tabOff, tabBorder;
        ccColor4F box, boxBorder;       // setting rows
        ccColor4F field, fieldBorder;   // value boxes
        float stroke, strokeThin;       // outline widths
    };

    inline Palette pal() {
        if (light()) {
            return {
                rgb(0, 0, 0), rgb(255, 255, 255),
                rgb(255, 255, 255), rgb(0, 0, 0),
                rgb(0, 0, 0), rgb(255, 255, 255),
                rgb(190, 190, 190), rgb(255, 255, 255), rgb(0, 0, 0),
                rgb(255, 255, 255), rgb(0, 0, 0),
                rgb(0, 0, 0), rgb(0, 0, 0),
                2.f, 1.5f
            };
        }
        return {
            rgb(12, 12, 12), rgb(80, 80, 80),
            rgb(32, 32, 32), rgb(80, 80, 80),
            rgb(12, 12, 12), rgb(80, 80, 80),
            rgb(80, 80, 80), rgb(12, 12, 12), rgb(80, 80, 80),
            rgb(48, 48, 48), rgb(80, 80, 80),
            rgb(0, 0, 0), rgb(80, 80, 80),
            1.f, 0.8f
        };
    }

    inline ccColor3B noteColor() { return light() ? ccColor3B{110, 110, 110} : ccColor3B{170, 170, 170}; }

    // ----- fonts --------------------------------------------------------------
    // Pusab is GD's own font. The others are bundled with the mod (see mod.json "fonts").
    inline std::string fontFile() {
        auto n = Mod::get()->getSettingValue<std::string>("font");
        if (n == "FredokaOne")    return "FredokaOne.fnt"_spr;
        if (n == "Rubik")         return "Rubik.fnt"_spr;
        if (n == "Arial")         return "Arial.fnt"_spr;
        if (n == "PressStart2P")  return "PressStart2P.fnt"_spr;
        if (n == "Chopin Script") return "ChopinScript.fnt"_spr;
        return "bigFont.fnt"; // Pusab
    }

    inline bool isPusab() {
        return Mod::get()->getSettingValue<std::string>("font") == "Pusab";
    }

    // Per-font fine tuning on top of the automatic size matching below.
    inline float fontTweak(std::string const& n) {
        if (n == "PressStart2P")  return 0.85f;
        if (n == "Chopin Script") return 1.05f;
        return 1.f;
    }

    // Scales every font so its line height matches Pusab's. This is measured in-game,
    // so it stays correct whatever size the font was generated at.
    inline float fontScale() {
        static std::string s_name;
        static float s_value = 1.f;
        auto n = Mod::get()->getSettingValue<std::string>("font");
        if (n == s_name) return s_value;

        float v = 1.f;
        if (n != "Pusab") {
            auto ref = CCLabelBMFont::create("Hg", "bigFont.fnt");
            auto mine = CCLabelBMFont::create("Hg", fontFile().c_str());
            float rh = ref->getContentSize().height;
            float mh = mine->getContentSize().height;
            if (rh > 1.f && mh > 1.f) v = rh / mh;
            v *= fontTweak(n);
        }
        s_name = n;
        s_value = v;
        return v;
    }

    // Text color for fonts other than Pusab: the opposite of the theme, so it can always be read.
    inline ccColor3B oppositeText() {
        return light() ? ccColor3B{0, 0, 0} : ccColor3B{255, 255, 255};
    }

    // A label in the chosen font. maxW > 0 shrinks it to fit that width.
    // onField = true is for text on a black value box (always white).
    inline CCLabelBMFont* text(std::string const& s, float scale, float maxW = 0.f, bool onField = false) {
        auto l = CCLabelBMFont::create(s.c_str(), fontFile().c_str());
        float sc = scale * fontScale();
        l->setScale(sc);
        if (maxW > 0.f) l->limitLabelWidth(maxW, sc, 0.1f);
        if (onField) l->setColor(ccColor3B{255, 255, 255});
        else if (!isPusab()) l->setColor(oppositeText());
        return l;
    }

    // ----- drawing helpers ----------------------------------------------------
    // A rounded rectangle from (0,0) to (w,h).
    inline CCDrawNode* roundRect(float w, float h, float r, ccColor4F fill, float border, ccColor4F borderColor) {
        auto d = CCDrawNode::create();
        std::vector<CCPoint> pts;
        constexpr int N = 6;
        constexpr float HALF_PI = 1.5707963f;
        auto arc = [&](float cx, float cy, float a0) {
            for (int i = 0; i <= N; i++) {
                float a = a0 + HALF_PI * static_cast<float>(i) / N;
                pts.push_back(CCPoint(cx + r * std::cos(a), cy + r * std::sin(a)));
            }
        };
        arc(w - r, r, -HALF_PI);   // bottom right
        arc(w - r, h - r, 0.f);    // top right
        arc(r, h - r, HALF_PI);    // top left
        arc(r, r, 2.f * HALF_PI);  // bottom left
        d->drawPolygon(pts.data(), static_cast<unsigned int>(pts.size()), fill, border, borderColor);
        return d;
    }

    // White X with a black outline (the window's close button).
    inline CCNode* closeX(float s) {
        auto node = CCNode::create();
        node->setContentSize(CCSize(s, s));
        auto d = CCDrawNode::create();
        CCPoint a(s * 0.22f, s * 0.22f), b(s * 0.78f, s * 0.78f);
        CCPoint c(s * 0.22f, s * 0.78f), e(s * 0.78f, s * 0.22f);
        ccColor4F black = rgb(0, 0, 0), white = rgb(255, 255, 255);
        d->drawSegment(a, b, s * 0.15f, black);
        d->drawSegment(c, e, s * 0.15f, black);
        d->drawSegment(a, b, s * 0.09f, white);
        d->drawSegment(c, e, s * 0.09f, white);
        node->addChild(d);
        return node;
    }

    // A square checkbox: black with a white check when on.
    inline CCNode* checkBox(bool on, bool enabled = true) {
        constexpr float S = 26.f;
        auto p = pal();
        auto node = CCNode::create();
        node->setContentSize(CCSize(S, S));
        ccColor4F fill = enabled ? p.field : rgb(110, 110, 110);
        node->addChild(roundRect(S, S, 5.f, fill, p.strokeThin, p.fieldBorder));
        if (on) {
            auto d = CCDrawNode::create();
            ccColor4F white = rgb(255, 255, 255);
            d->drawSegment(CCPoint(S * 0.22f, S * 0.52f), CCPoint(S * 0.42f, S * 0.28f), 1.8f, white);
            d->drawSegment(CCPoint(S * 0.42f, S * 0.28f), CCPoint(S * 0.80f, S * 0.76f), 1.8f, white);
            node->addChild(d);
        }
        return node;
    }
}

namespace theme = nc::theme;

// =====================================================================
//  NCWindow: a window with a content area and a tab bar at the bottom.
//  Derived classes list their tabs and fill each one in buildTab().
// =====================================================================
class NCWindow : public geode::Popup {
protected:
    struct TabDef { std::string name; std::string icon; };

    static constexpr float PAD = 8.f;
    static constexpr float BAR_H = 52.f;
    static constexpr float BOX_H = 40.f;
    static constexpr float ROW_GAP = 46.f;
    static constexpr float COL_GAP = 10.f;

    float m_w = 0.f, m_h = 0.f;
    int m_tab = 0;
    std::vector<TabDef> m_tabs;
    std::vector<Ref<CCNode>> m_barNodes;   // tab bar buttons
    std::vector<Ref<CCNode>> m_tabNodes;   // everything inside the current tab

    virtual void buildTab(int index) = 0;

    // ---- window frame -------------------------------------------------------
    bool initWindow(float w, float h, std::vector<TabDef> tabs, int startTab = 0) {
        if (!Popup::init(w, h)) return false;
        m_w = w; m_h = h;
        m_tabs = std::move(tabs);
        m_tab = startTab;

        // Replace the stock GD look.
        m_bgSprite->setVisible(false);
        m_closeBtn->setVisible(false);

        auto p = theme::pal();
        m_mainLayer->addChildAtPosition(theme::roundRect(w, h, 14.f, p.frame, p.stroke, p.frameBorder), Anchor::BottomLeft, ccp(0.f, 0.f));
        // tab bar
        m_mainLayer->addChildAtPosition(theme::roundRect(w - 2 * PAD, BAR_H, 10.f, p.bar, p.stroke, p.barBorder), Anchor::BottomLeft, ccp(PAD, PAD));
        // content panel
        float panelY = PAD * 2 + BAR_H;
        m_mainLayer->addChildAtPosition(theme::roundRect(w - 2 * PAD, h - panelY - PAD, 10.f, p.panel, p.stroke, p.panelBorder), Anchor::BottomLeft, ccp(PAD, panelY));

        // close button, sticking out of the top right corner
        auto closeBtn = CCMenuItemExt::createSpriteExtra(theme::closeX(30.f), [this](CCMenuItemSpriteExtra*) {
            this->onClose(nullptr);
        });
        m_buttonMenu->addChildAtPosition(closeBtn, Anchor::TopRight, ccp(-4.f, -4.f));

        buildBar();
        buildTab(m_tab);
        return true;
    }

    // ---- tabs -----------------------------------------------------------------
    void buildBar() {
        auto p = theme::pal();
        float barW = m_w - 2 * PAD;
        float n = static_cast<float>(m_tabs.size());
        float tw = std::min(150.f, (barW - (n + 1.f) * 8.f) / n);
        float totalW = tw * n + 8.f * (n - 1.f);
        float x0 = (m_w - totalW) / 2.f + tw / 2.f;

        for (int i = 0; i < static_cast<int>(m_tabs.size()); i++) {
            bool selected = (i == m_tab);
            auto node = CCNode::create();
            node->setContentSize(CCSize(tw, 38.f));
            node->addChild(theme::roundRect(tw, 38.f, 8.f, selected ? p.tabOn : p.tabOff, p.stroke, p.tabBorder));

            float textX = 12.f;
            auto icon = CCSprite::create(Mod::get()->expandSpriteName(m_tabs[i].icon).c_str());
            if (icon) {
                icon->setScale(24.f / icon->getContentSize().height);
                icon->setPosition(CCPoint(24.f, 19.f));
                node->addChild(icon);
                textX = 42.f;
            }
            auto label = theme::text(m_tabs[i].name, 0.4f, tw - textX - 8.f);
            label->setAnchorPoint(CCPoint(0.f, 0.5f));
            label->setPosition(CCPoint(textX, 19.f));
            node->addChild(label);

            auto btn = CCMenuItemExt::createSpriteExtra(node, [this, i](CCMenuItemSpriteExtra*) {
                if (i == m_tab) return;
                // switch after this tap finishes (the tapped button gets replaced)
                queueInMainThread([self = Ref<NCWindow>(this), i] { self->selectTab(i); });
            });
            m_buttonMenu->addChildAtPosition(btn, Anchor::BottomLeft, ccp(x0 + i * (tw + 8.f), PAD + BAR_H / 2.f));
            m_barNodes.push_back(btn);
        }
    }

    void clearNodes() {
        for (auto& n : m_barNodes) n->removeFromParent();
        for (auto& n : m_tabNodes) n->removeFromParent();
        m_barNodes.clear();
        m_tabNodes.clear();
    }

    void refresh() {
        clearNodes();
        buildBar();
        buildTab(m_tab);
    }

    void refreshLater() {
        queueInMainThread([self = Ref<NCWindow>(this)] { self->refresh(); });
    }

    void selectTab(int i) {
        m_tab = i;
        refresh();
    }

    // ---- layout helpers ---------------------------------------------------------
    float boxW() const { return (m_w - 2 * PAD - 3 * COL_GAP) / 2.f; }

    // bottom-left corner of the setting box at (col, row)
    CCPoint cell(int col, int row) const {
        float x = PAD + COL_GAP + col * (boxW() + COL_GAP);
        float yTop = m_h - PAD - 10.f - row * ROW_GAP;
        return CCPoint(x, yTop - BOX_H);
    }

    void track(CCNode* n) { m_tabNodes.push_back(n); }

    void addNode(CCNode* n, CCPoint pos) {
        m_mainLayer->addChildAtPosition(n, Anchor::BottomLeft, ccp(pos.x, pos.y));
        track(n);
    }

    void addMenuItem(CCNode* n, CCPoint pos) {
        m_buttonMenu->addChildAtPosition(n, Anchor::BottomLeft, ccp(pos.x, pos.y));
        track(n);
    }

    // The rounded background and the label of a setting box.
    void addBoxBase(CCPoint c, std::string const& label, bool enabled = true) {
        auto p = theme::pal();
        addNode(theme::roundRect(boxW(), BOX_H, 8.f, p.box, p.stroke, p.boxBorder), c);
        auto l = theme::text(label, 0.34f, boxW() - 10.f - 78.f);
        l->setAnchorPoint(CCPoint(0.f, 0.5f));
        if (!enabled) l->setOpacity(130);
        addNode(l, CCPoint(c.x + 10.f, c.y + BOX_H / 2.f));
    }

    // A small centered note line at the bottom of the content area.
    void addNote(std::string const& s) {
        auto l = theme::text(s, 0.3f, m_w - 2 * PAD - 30.f);
        l->setColor(theme::noteColor());
        addNode(l, CCPoint(m_w / 2.f, PAD * 2 + BAR_H + 14.f));
    }

    // ---- components ---------------------------------------------------------------
    // Checkbox: click to toggle.
    void addCheckbox(int col, int row, std::string const& label, bool value, std::function<void(bool)> onChange, bool enabled = true) {
        auto c = cell(col, row);
        addBoxBase(c, label, enabled);
        auto toggler = CCMenuItemExt::createToggler(
            theme::checkBox(true, enabled), theme::checkBox(false, enabled),
            [onChange](CCMenuItemToggler* t) { onChange(!t->isToggled()); } // runs before the visual flip
        );
        toggler->toggle(value);
        toggler->setEnabled(enabled);
        addMenuItem(toggler, CCPoint(c.x + boxW() - 24.f, c.y + BOX_H / 2.f));
    }

    // Choice box: click to switch to the next choice. onNext does the switching and returns the new text.
    void addChoice(int col, int row, std::string const& label, std::string const& value, std::function<std::string()> onNext) {
        auto c = cell(col, row);
        addBoxBase(c, label);
        auto p = theme::pal();

        constexpr float FW = 70.f, FH = 28.f;
        auto face = CCNode::create();
        face->setContentSize(CCSize(FW, FH));
        face->addChild(theme::roundRect(FW, FH, 6.f, p.field, p.strokeThin, p.fieldBorder));
        auto lbl = theme::text(value, 0.36f, FW - 8.f, true);
        lbl->setPosition(CCPoint(FW / 2.f, FH / 2.f));
        face->addChild(lbl);

        Ref<CCLabelBMFont> lblRef = lbl;
        float baseScale = lbl->getScale();
        auto btn = CCMenuItemExt::createSpriteExtra(face, [onNext, lblRef, baseScale](CCMenuItemSpriteExtra*) {
            auto s = onNext();
            lblRef->setString(s.c_str());
            lblRef->setScale(baseScale);
            lblRef->limitLabelWidth(FW - 8.f, baseScale, 0.1f);
        });
        addMenuItem(btn, CCPoint(c.x + boxW() - 6.f - FW / 2.f, c.y + BOX_H / 2.f));
    }

    // Number box: type a value. onChange gets the text and the input (so it can rewrite the text if it needs to).
    void addNumber(int col, int row, std::string const& label, std::string const& value,
                   std::function<void(std::string const&, TextInput*)> onChange) {
        auto c = cell(col, row);
        addBoxBase(c, label);
        auto input = TextInput::create(78.f, "0", "bigFont.fnt");
        input->setCommonFilter(CommonFilter::Float);
        input->setMaxCharCount(8);
        input->setScale(0.85f);
        input->setString(value);
        TextInput* raw = input;
        input->setCallback([onChange, raw](std::string const& s) { onChange(s, raw); });
        addNode(input, CCPoint(c.x + boxW() - 6.f - 33.f, c.y + BOX_H / 2.f));
    }

    // Action box: the whole box is a button.
    void addAction(int col, int row, std::string const& label, std::function<void()> onTap) {
        auto c = cell(col, row);
        auto p = theme::pal();
        auto face = CCNode::create();
        face->setContentSize(CCSize(boxW(), BOX_H));
        face->addChild(theme::roundRect(boxW(), BOX_H, 8.f, p.box, p.stroke, p.boxBorder));
        auto l = theme::text(label, 0.34f, boxW() - 16.f);
        l->setPosition(CCPoint(boxW() / 2.f, BOX_H / 2.f));
        face->addChild(l);
        auto btn = CCMenuItemExt::createSpriteExtra(face, [onTap](CCMenuItemSpriteExtra*) { onTap(); });
        addMenuItem(btn, CCPoint(c.x + boxW() / 2.f, c.y + BOX_H / 2.f));
    }
};