
#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <cmath>
#include <functional>
#include <vector>
#include "settings.hpp"

using namespace geode::prelude;

// =====================================================================
// Theme: monochrome Light and Dark palettes.
// =====================================================================
namespace nc::theme {
    inline bool light() {
        return Mod::get()->getSettingValue<std::string>("theme") != "Dark";
    }

    inline ccColor4F rgb(int r, int g, int b) {
        return {r / 255.f, g / 255.f, b / 255.f, 1.f};
    }

    struct Palette {
        ccColor4F frame, frameBorder;
        ccColor4F panel, panelBorder;
        ccColor4F bar, barBorder;
        ccColor4F tabOn, tabOff, tabBorder;
        ccColor4F box, boxBorder;
        ccColor4F field, fieldBorder;
        float stroke, strokeThin;
    };

    inline Palette pal() {
        if (light()) {
            return {
                rgb(255, 255, 255), rgb(128, 128, 128),
                rgb(255, 255, 255), rgb(160, 160, 160),
                rgb(255, 255, 255), rgb(160, 160, 160),
                rgb(0, 0, 0), rgb(235, 235, 235), rgb(128, 128, 128),
                rgb(255, 255, 255), rgb(160, 160, 160),
                rgb(0, 0, 0), rgb(128, 128, 128),
                1.25f, 0.8f
            };
        }

        return {
            rgb(18, 18, 18), rgb(112, 112, 112),
            rgb(28, 28, 28), rgb(92, 92, 92),
            rgb(18, 18, 18), rgb(92, 92, 92),
            rgb(55, 55, 55), rgb(32, 32, 32), rgb(112, 112, 112),
            rgb(28, 28, 28), rgb(92, 92, 92),
            rgb(0, 0, 0), rgb(112, 112, 112),
            1.25f, 0.8f
        };
    }

    inline ccColor3B noteColor() {
        return light()
            ? ccColor3B{110, 110, 110}
            : ccColor3B{170, 170, 170};
    }

    // Fonts
    inline std::string fontFile() {
        auto n = Mod::get()->getSettingValue<std::string>("font");

        if (n == "FredokaOne")    return "FredokaOne.fnt"_spr;
        if (n == "Rubik")         return "Rubik.fnt"_spr;
        if (n == "Arial")         return "Arial.fnt"_spr;
        if (n == "PressStart2P")  return "PressStart2P.fnt"_spr;
        if (n == "Chopin Script") return "ChopinScript.fnt"_spr;

        return "bigFont.fnt";
    }

    inline bool isPusab() {
        return Mod::get()->getSettingValue<std::string>("font") == "Pusab";
    }

    inline float fontTweak(std::string const& n) {
        if (n == "PressStart2P")  return 0.85f;
        if (n == "Chopin Script") return 1.05f;
        return 1.f;
    }

    inline float fontScale() {
        static std::string s_name;
        static float s_value = 1.f;

        auto n = Mod::get()->getSettingValue<std::string>("font");

        if (n == s_name)
            return s_value;

        float v = 1.f;

        if (n != "Pusab") {
            auto ref = CCLabelBMFont::create("Hg", "bigFont.fnt");
            auto mine = CCLabelBMFont::create("Hg", fontFile().c_str());

            float rh = ref->getContentSize().height;
            float mh = mine->getContentSize().height;

            if (rh > 1.f && mh > 1.f)
                v = rh / mh;

            v *= fontTweak(n);
        }

        s_name = n;
        s_value = v;

        return v;
    }

    inline ccColor3B oppositeText() {
        return light()
            ? ccColor3B{0, 0, 0}
            : ccColor3B{255, 255, 255};
    }

    inline CCLabelBMFont* text(
        std::string const& s,
        float scale,
        float maxW = 0.f,
        bool onField = false
    ) {
        auto l = CCLabelBMFont::create(s.c_str(), fontFile().c_str());

        float sc = scale * fontScale();
        l->setScale(sc);

        if (maxW > 0.f)
            l->limitLabelWidth(maxW, sc, 0.1f);

        if (onField)
            l->setColor(ccColor3B{255, 255, 255});
        else if (!isPusab())
            l->setColor(oppositeText());

        return l;
    }

    // Rounded rectangle drawing
    inline CCDrawNode* roundRect(
        float w,
        float h,
        float r,
        ccColor4F fill,
        float border,
        ccColor4F borderColor
    ) {
        auto d = CCDrawNode::create();

        std::vector<CCPoint> pts;

        constexpr int N = 6;
        constexpr float HALF_PI = 1.5707963f;

        auto arc = [&](float cx, float cy, float a0) {
            for (int i = 0; i <= N; i++) {
                float a = a0 + HALF_PI * static_cast<float>(i) / N;
                pts.push_back(
                    CCPoint(
                        cx + r * std::cos(a),
                        cy + r * std::sin(a)
                    )
                );
            }
        };

        arc(w - r, r, -HALF_PI);
        arc(w - r, h - r, 0.f);
        arc(r, h - r, HALF_PI);
        arc(r, r, 2.f * HALF_PI);

        d->drawPolygon(
            pts.data(),
            static_cast<unsigned int>(pts.size()),
            fill,
            border,
            borderColor
        );

        return d;
    }

    // Close button
    inline CCNode* closeX(float s) {
        auto node = CCNode::create();
        node->setContentSize(CCSize(s, s));

        auto d = CCDrawNode::create();

        CCPoint a(s * 0.22f, s * 0.22f);
        CCPoint b(s * 0.78f, s * 0.78f);
        CCPoint c(s * 0.22f, s * 0.78f);
        CCPoint e(s * 0.78f, s * 0.22f);

        ccColor4F black = rgb(0, 0, 0);
        ccColor4F white = rgb(255, 255, 255);

        d->drawSegment(a, b, s * 0.15f, black);
        d->drawSegment(c, e, s * 0.15f, black);
        d->drawSegment(a, b, s * 0.09f, white);
        d->drawSegment(c, e, s * 0.09f, white);

        node->addChild(d);

        return node;
    }

    // Checkbox
    inline CCNode* checkBox(bool on, bool enabled = true) {
        constexpr float S = 26.f;

        auto p = pal();

        auto node = CCNode::create();
        node->setContentSize(CCSize(S, S));

        ccColor4F fill = enabled
            ? p.field
            : rgb(110, 110, 110);

        node->addChild(
            roundRect(S, S, 5.f, fill, p.strokeThin, p.fieldBorder)
        );

        if (on) {
            auto d = CCDrawNode::create();

            ccColor4F white = rgb(255, 255, 255);

            d->drawSegment(
                CCPoint(S * 0.22f, S * 0.52f),
                CCPoint(S * 0.42f, S * 0.28f),
                1.8f,
                white
            );

            d->drawSegment(
                CCPoint(S * 0.42f, S * 0.28f),
                CCPoint(S * 0.80f, S * 0.76f),
                1.8f,
                white
            );

            node->addChild(d);
        }

        return node;
    }
}

namespace theme = nc::theme;

// =====================================================================
// NCWindow
// Redesigned window with top navigation and compact controls.
// =====================================================================
class NCWindow : public geode::Popup {
protected:
    struct TabDef {
        std::string name;
        std::string icon;
    };

    static constexpr float PAD = 10.f;
    static constexpr float BAR_H = 42.f;
    static constexpr float BOX_H = 36.f;
    static constexpr float ROW_GAP = 42.f;
    static constexpr float COL_GAP = 10.f;

    float m_w = 0.f;
    float m_h = 0.f;

    int m_tab = 0;

    std::vector<TabDef> m_tabs;
    std::vector<Ref<CCNode>> m_barNodes;
    std::vector<Ref<CCNode>> m_tabNodes;

    virtual void buildTab(int index) = 0;

    // Window initialization
    bool initWindow(
        float w,
        float h,
        std::vector<TabDef> tabs,
        int startTab = 0
    ) {
        if (!Popup::init(w, h))
            return false;

        m_w = w;
        m_h = h;
        m_tabs = std::move(tabs);
        m_tab = startTab;

        m_bgSprite->setVisible(false);
        m_closeBtn->setVisible(false);

        auto p = theme::pal();

        // Main window frame
        m_mainLayer->addChildAtPosition(
            theme::roundRect(
                w, h, 14.f,
                p.frame,
                p.stroke,
                p.frameBorder
            ),
            Anchor::BottomLeft,
            ccp(0.f, 0.f)
        );

        // Top navigation
        m_mainLayer->addChildAtPosition(
            theme::roundRect(
                w - 2 * PAD,
                BAR_H,
                9.f,
                p.bar,
                p.stroke,
                p.barBorder
            ),
            Anchor::BottomLeft,
            ccp(PAD, h - PAD - BAR_H)
        );

        // Main content panel
        float panelH = h - BAR_H - 3.f * PAD;

        m_mainLayer->addChildAtPosition(
            theme::roundRect(
                w - 2 * PAD,
                panelH,
                9.f,
                p.panel,
                p.stroke,
                p.panelBorder
            ),
            Anchor::BottomLeft,
            ccp(PAD, PAD)
        );

        // Close button
        auto closeBtn = CCMenuItemExt::createSpriteExtra(
            theme::closeX(30.f),
            [this](CCMenuItemSpriteExtra*) {
                this->onClose(nullptr);
            }
        );

        m_buttonMenu->addChildAtPosition(
            closeBtn,
            Anchor::TopRight,
            ccp(-4.f, -4.f)
        );

        buildBar();
        buildTab(m_tab);

        return true;
    }

    // Build navigation
    void buildBar() {
        auto p = theme::pal();

        float barW = m_w - 2 * PAD;
        float n = static_cast<float>(m_tabs.size());

        float tw = std::min(
            150.f,
            (barW - (n + 1.f) * 8.f) / n
        );

        float totalW = tw * n + 8.f * (n - 1.f);
        float x0 = (m_w - totalW) / 2.f + tw / 2.f;

        for (int i = 0; i < static_cast<int>(m_tabs.size()); i++) {
            bool selected = (i == m_tab);

            auto node = CCNode::create();
            node->setContentSize(CCSize(tw, 34.f));

            node->addChild(
                theme::roundRect(
                    tw,
                    34.f,
                    7.f,
                    selected ? p.tabOn : p.tabOff,
                    p.strokeThin,
                    p.tabBorder
                )
            );

            float textX = 12.f;

            auto icon = CCSprite::create(
                Mod::get()->expandSpriteName(m_tabs[i].icon).c_str()
            );

            if (icon) {
                icon->setScale(
                    21.f / icon->getContentSize().height
                );

                icon->setPosition(CCPoint(22.f, 17.f));
                node->addChild(icon);

                textX = 40.f;
            }

            auto label = theme::text(
                m_tabs[i].name,
                0.38f,
                tw - textX - 8.f
            );

            label->setAnchorPoint(CCPoint(0.f, 0.5f));
            label->setPosition(CCPoint(textX, 17.f));

            node->addChild(label);

            auto btn = CCMenuItemExt::createSpriteExtra(
                node,
                [this, i](CCMenuItemSpriteExtra*) {
                    if (i == m_tab)
                        return;

                    queueInMainThread(
                        [self = Ref<NCWindow>(this), i] {
                            self->selectTab(i);
                        }
                    );
                }
            );

            m_buttonMenu->addChildAtPosition(
                btn,
                Anchor::BottomLeft,
                ccp(
                    x0 + i * (tw + 8.f),
                    m_h - PAD - BAR_H / 2.f
                )
            );

            m_barNodes.push_back(btn);
        }
    }

    // Remove old tab contents
    void clearNodes() {
        for (auto& n : m_barNodes)
            n->removeFromParent();

        for (auto& n : m_tabNodes)
            n->removeFromParent();

        m_barNodes.clear();
        m_tabNodes.clear();
    }

    void refresh() {
        clearNodes();
        buildBar();
        buildTab(m_tab);
    }

    void refreshLater() {
        queueInMainThread(
            [self = Ref<NCWindow>(this)] {
                self->refresh();
            }
        );
    }

    void selectTab(int i) {
        m_tab = i;
        refresh();
    }

    // Layout helpers
    float boxW() const {
        return (m_w - 2 * PAD - 3 * COL_GAP) / 2.f;
    }

    CCPoint cell(int col, int row) const {
        float x =
            PAD + COL_GAP + col * (boxW() + COL_GAP);

        float yTop =
            m_h - PAD - BAR_H - 12.f - row * ROW_GAP;

        return CCPoint(x, yTop - BOX_H);
    }

    void track(CCNode* n) {
        m_tabNodes.push_back(n);
    }

    void addNode(CCNode* n, CCPoint pos) {
        m_mainLayer->addChildAtPosition(
            n,
            Anchor::BottomLeft,
            ccp(pos.x, pos.y)
        );

        track(n);
    }

    void addMenuItem(CCNode* n, CCPoint pos) {
        m_buttonMenu->addChildAtPosition(
            n,
            Anchor::BottomLeft,
            ccp(pos.x, pos.y)
        );

        track(n);
    }

    // Setting box background and label
    void addBoxBase(
        CCPoint c,
        std::string const& label,
        bool enabled = true
    ) {
        auto p = theme::pal();

        addNode(
            theme::roundRect(
                boxW(),
                BOX_H,
                7.f,
                p.box,
                p.strokeThin,
                p.boxBorder
            ),
            c
        );

        auto l = theme::text(
            label,
            0.34f,
            boxW() - 10.f - 78.f
        );

        l->setAnchorPoint(CCPoint(0.f, 0.5f));

        if (!enabled)
            l->setOpacity(130);

        addNode(
            l,
            CCPoint(
                c.x + 10.f,
                c.y + BOX_H / 2.f
            )
        );
    }

    // Informational note
    void addNote(std::string const& s) {
        auto l = theme::text(
            s,
            0.3f,
            m_w - 2 * PAD - 30.f
        );

        l->setColor(theme::noteColor());

        addNode(
            l,
            CCPoint(m_w / 2.f, PAD + 13.f)
        );
    }

    // Checkbox
    void addCheckbox(
        int col,
        int row,
        std::string const& label,
        bool value,
        std::function<void(bool)> onChange,
        bool enabled = true
    ) {
        auto c = cell(col, row);

        addBoxBase(c, label, enabled);

        auto toggler = CCMenuItemExt::createToggler(
            theme::checkBox(true, enabled),
            theme::checkBox(false, enabled),
            [onChange](CCMenuItemToggler* t) {
                onChange(!t->isToggled());
            }
        );

        toggler->toggle(value);
        toggler->setEnabled(enabled);

        addMenuItem(
            toggler,
            CCPoint(
                c.x + boxW() - 24.f,
                c.y + BOX_H / 2.f
            )
        );
    }

    // Choice box
    void addChoice(
        int col,
        int row,
        std::string const& label,
        std::string const& value,
        std::function<std::string()> onNext
    ) {
        auto c = cell(col, row);

        addBoxBase(c, label);

        auto p = theme::pal();

        constexpr float FW = 70.f;
        constexpr float FH = 28.f;

        auto face = CCNode::create();
        face->setContentSize(CCSize(FW, FH));

        face->addChild(
            theme::roundRect(
                FW,
                FH,
                6.f,
                p.field,
                p.strokeThin,
                p.fieldBorder
            )
        );

        auto lbl = theme::text(
            value,
            0.36f,
            FW - 8.f,
            true
        );

        lbl->setPosition(CCPoint(FW / 2.f, FH / 2.f));
        face->addChild(lbl);

        Ref<CCLabelBMFont> lblRef = lbl;
        float baseScale = lbl->getScale();

        auto btn = CCMenuItemExt::createSpriteExtra(
            face,
            [onNext, lblRef, baseScale](CCMenuItemSpriteExtra*) {
                auto s = onNext();

                lblRef->setString(s.c_str());
                lblRef->setScale(baseScale);

                lblRef->limitLabelWidth(
                    FW - 8.f,
                    baseScale,
                    0.1f
                );
            }
        );

        addMenuItem(
            btn,
            CCPoint(
                c.x + boxW() - 6.f - FW / 2.f,
                c.y + BOX_H / 2.f
            )
        );
    }

    // Number input
    void addNumber(
        int col,
        int row,
        std::string const& label,
        std::string const& value,
        std::function<void(std::string const&, TextInput*)> onChange
    ) {
        auto c = cell(col, row);

        addBoxBase(c, label);

        auto input = TextInput::create(
            78.f,
            "0",
            "bigFont.fnt"
        );

        input->setCommonFilter(CommonFilter::Float);
        input->setMaxCharCount(8);
        input->setScale(0.85f);
        input->setString(value);

        TextInput* raw = input;

        input->setCallback(
            [onChange, raw](std::string const& s) {
                onChange(s, raw);
            }
        );

        addNode(
            input,
            CCPoint(
                c.x + boxW() - 6.f - 33.f,
                c.y + BOX_H / 2.f
            )
        );
    }

    // Action button
    void addAction(
        int col,
        int row,
        std::string const& label,
        std::function<void()> onTap
    ) {
        auto c = cell(col, row);
        auto p = theme::pal();

        auto face = CCNode::create();
        face->setContentSize(CCSize(boxW(), BOX_H));

        face->addChild(
            theme::roundRect(
                boxW(),
                BOX_H,
                7.f,
                p.box,
                p.strokeThin,
                p.boxBorder
            )
        );

        auto l = theme::text(
            label,
            0.34f,
            boxW() - 16.f
        );

        l->setPosition(
            CCPoint(
                boxW() / 2.f,
                BOX_H / 2.f
            )
        );

        face->addChild(l);

        auto btn = CCMenuItemExt::createSpriteExtra(
            face,
            [onTap](CCMenuItemSpriteExtra*) {
                onTap();
            }
        );

        addMenuItem(
            btn,
            CCPoint(
                c.x + boxW() / 2.f,
                c.y + BOX_H / 2.f
            )
        );
    }
};
