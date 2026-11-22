#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include "settings.hpp"

using namespace geode::prelude;

// =====================================================================
// NullCore monochrome theme + reusable drawing helpers.
// =====================================================================
namespace nc::theme {
    inline bool light() {
        return Mod::get()->getSettingValue<std::string>("theme") != "Dark";
    }

    inline ccColor4F rgb(int r, int g, int b) {
        return { r / 255.f, g / 255.f, b / 255.f, 1.f };
    }

    struct Palette {
        ccColor4F frame, frameBorder;
        ccColor4F panel, panelBorder;
        ccColor4F sidebar, sidebarBorder;
        ccColor4F selected, selectedBorder;
        ccColor4F card, cardBorder;
        ccColor4F field, fieldBorder;
        ccColor4F accent;
        ccColor3B primaryText, secondaryText;
        float stroke, thin;
    };

    inline Palette pal() {
        if (light()) {
            return {
                rgb(250, 250, 250), rgb(125, 125, 125),
                rgb(255, 255, 255), rgb(205, 205, 205),
                rgb(244, 244, 244), rgb(205, 205, 205),
                rgb(225, 225, 225), rgb(120, 120, 120),
                rgb(255, 255, 255), rgb(205, 205, 205),
                rgb(235, 235, 235), rgb(130, 130, 130),
                rgb(0, 0, 0),
                { 12, 12, 12 }, { 105, 105, 105 },
                1.1f, 0.7f
            };
        }

        return {
            rgb(16, 16, 16), rgb(95, 95, 95),
            rgb(23, 23, 23), rgb(75, 75, 75),
            rgb(20, 20, 20), rgb(70, 70, 70),
            rgb(52, 52, 52), rgb(145, 145, 145),
            rgb(31, 31, 31), rgb(75, 75, 75),
            rgb(39, 39, 39), rgb(105, 105, 105),
            rgb(255, 255, 255),
            { 245, 245, 245 }, { 165, 165, 165 },
            1.1f, 0.7f
        };
    }

    inline ccColor3B noteColor() {
        return pal().secondaryText;
    }

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

    inline float fontScale() {
        static std::string cachedName;
        static float cachedScale = 1.f;
        auto name = Mod::get()->getSettingValue<std::string>("font");
        if (name == cachedName) return cachedScale;

        float scale = 1.f;
        if (name != "Pusab") {
            auto ref = CCLabelBMFont::create("Hg", "bigFont.fnt");
            auto custom = CCLabelBMFont::create("Hg", fontFile().c_str());
            if (ref && custom) {
                float rh = ref->getContentSize().height;
                float ch = custom->getContentSize().height;
                if (rh > 1.f && ch > 1.f) scale = rh / ch;
            }
            if (name == "PressStart2P") scale *= 0.85f;
            if (name == "Chopin Script") scale *= 1.05f;
        }

        cachedName = name;
        cachedScale = scale;
        return scale;
    }

    inline CCLabelBMFont* text(
        std::string const& value,
        float scale,
        float maxWidth = 0.f,
        bool onField = false
    ) {
        auto label = CCLabelBMFont::create(value.c_str(), fontFile().c_str());
        if (!label) return nullptr;

        float actualScale = scale * fontScale();
        label->setScale(actualScale);
        if (maxWidth > 0.f)
            label->limitLabelWidth(maxWidth, actualScale, 0.1f);

        if (onField) label->setColor({ 255, 255, 255 });
        else if (!isPusab()) label->setColor(pal().primaryText);
        return label;
    }

    inline CCDrawNode* roundRect(
        float width, float height, float radius,
        ccColor4F fill, float border, ccColor4F borderColor
    ) {
        auto draw = CCDrawNode::create();
        std::vector<CCPoint> points;
        constexpr int segments = 6;
        constexpr float halfPi = 1.5707963f;

        auto arc = [&](float cx, float cy, float start) {
            for (int i = 0; i <= segments; ++i) {
                float a = start + halfPi * static_cast<float>(i) / segments;
                points.emplace_back(
                    cx + radius * std::cos(a),
                    cy + radius * std::sin(a)
                );
            }
        };

        arc(width - radius, radius, -halfPi);
        arc(width - radius, height - radius, 0.f);
        arc(radius, height - radius, halfPi);
        arc(radius, radius, 2.f * halfPi);
        draw->drawPolygon(
            points.data(),
            static_cast<unsigned int>(points.size()),
            fill, border, borderColor
        );
        return draw;
    }

    inline CCNode* closeX(float size) {
        auto node = CCNode::create();
        node->setContentSize({ size, size });
        auto draw = CCDrawNode::create();

        CCPoint a(size * .25f, size * .25f);
        CCPoint b(size * .75f, size * .75f);
        CCPoint c(size * .25f, size * .75f);
        CCPoint d(size * .75f, size * .25f);

        auto p = pal();
        draw->drawSegment(a, b, size * .15f, p.frameBorder);
        draw->drawSegment(c, d, size * .15f, p.frameBorder);
        draw->drawSegment(a, b, size * .075f, rgb(light() ? 0 : 255, light() ? 0 : 255, light() ? 0 : 255));
        draw->drawSegment(c, d, size * .075f, rgb(light() ? 0 : 255, light() ? 0 : 255, light() ? 0 : 255));
        node->addChild(draw);
        return node;
    }

    inline CCNode* checkBox(bool on, bool enabled = true) {
        constexpr float size = 22.f;
        auto p = pal();
        auto node = CCNode::create();
        node->setContentSize({ size, size });

        auto fill = enabled ? p.field : rgb(100, 100, 100);
        node->addChild(roundRect(size, size, 5.f, fill, p.thin, p.fieldBorder));

        if (on) {
            auto tick = CCDrawNode::create();
            auto color = light() ? rgb(255, 255, 255) : rgb(0, 0, 0);
            tick->drawSegment({ size * .2f, size * .52f }, { size * .42f, size * .3f }, 1.8f, color);
            tick->drawSegment({ size * .42f, size * .3f }, { size * .82f, size * .76f }, 1.8f, color);
            node->addChild(tick);
        }
        return node;
    }

    inline CCNode* switchTrack(bool on, bool enabled = true) {
        constexpr float width = 34.f;
        constexpr float height = 19.f;
        auto p = pal();
        auto node = CCNode::create();
        node->setContentSize({ width, height });

        auto track = roundRect(
            width, height, height / 2.f,
            on ? p.accent : p.field,
            p.thin,
            enabled ? p.fieldBorder : p.panelBorder
        );
        node->addChild(track);

        auto knob = CCDrawNode::create();
        float x = on ? width - 9.5f : 9.5f;
        auto knobColor = on
            ? (light() ? rgb(255, 255, 255) : rgb(0, 0, 0))
            : (light() ? rgb(100, 100, 100) : rgb(190, 190, 190));
        knob->drawDot({ x, height / 2.f }, 6.5f, knobColor);
        node->addChild(knob);
        if (!enabled) node->setOpacity(115);
        return node;
    }
}

namespace theme = nc::theme;

// =====================================================================
// NCWindow — sidebar navigation, compact header and feature controls.
// Public helper methods intentionally retain their old signatures so
// popups.hpp and settings_popup.hpp keep their existing callbacks.
// =====================================================================
class NCWindow : public geode::Popup {
protected:
    struct TabDef {
        std::string name;
        std::string icon;
    };

    static constexpr float PAD = 10.f;
    static constexpr float SIDEBAR_W = 96.f;
    static constexpr float GAP = 8.f;
    static constexpr float HEADER_H = 34.f;
    static constexpr float CARD_H = 34.f;
    static constexpr float ROW_GAP = 41.f;

    float m_w = 0.f;
    float m_h = 0.f;
    int m_tab = 0;

    std::vector<TabDef> m_tabs;
    std::vector<Ref<CCNode>> m_barNodes;
    std::vector<Ref<CCNode>> m_tabNodes;

    virtual void buildTab(int index) = 0;

    float contentX() const { return PAD + SIDEBAR_W + GAP; }
    float contentW() const { return m_w - contentX() - PAD; }

    bool initWindow(float w, float h, std::vector<TabDef> tabs, int startTab = 0) {
        if (!Popup::init(w, h)) return false;

        m_w = w;
        m_h = h;
        m_tabs = std::move(tabs);
        m_tab = std::clamp(startTab, 0, static_cast<int>(m_tabs.size()) - 1);

        m_bgSprite->setVisible(false);
        m_closeBtn->setVisible(false);

        auto p = theme::pal();

        m_mainLayer->addChildAtPosition(
            theme::roundRect(w, h, 13.f, p.frame, p.stroke, p.frameBorder),
            Anchor::BottomLeft, { 0.f, 0.f }
        );

        // Sidebar rail.
        m_mainLayer->addChildAtPosition(
            theme::roundRect(SIDEBAR_W, h - 2.f * PAD, 9.f, p.sidebar, p.thin, p.sidebarBorder),
            Anchor::BottomLeft, { PAD, PAD }
        );

        // Main content surface.
        m_mainLayer->addChildAtPosition(
            theme::roundRect(contentW(), h - 2.f * PAD, 9.f, p.panel, p.thin, p.panelBorder),
            Anchor::BottomLeft, { contentX(), PAD }
        );

        // Brand mark + product label in the sidebar.
        auto mark = theme::roundRect(25.f, 25.f, 7.f, p.accent, 0.f, p.accent);
        m_mainLayer->addChildAtPosition(mark, Anchor::BottomLeft, { PAD + 11.f, h - PAD - 38.f });

        auto brand = theme::text("NC", .48f, 22.f);
        if (brand) {
            brand->setColor(theme::light() ? ccColor3B{255,255,255} : ccColor3B{0,0,0});
            m_mainLayer->addChildAtPosition(brand, Anchor::BottomLeft, { PAD + 23.5f, h - PAD - 25.5f });
        }

        auto wordmark = theme::text("NULLCORE", .34f, 58.f);
        if (wordmark) {
            wordmark->setAnchorPoint({ 0.f, .5f });
            m_mainLayer->addChildAtPosition(wordmark, Anchor::BottomLeft, { PAD + 43.f, h - PAD - 25.5f });
        }

        // Thin divider below sidebar brand.
        auto divider = CCDrawNode::create();
        divider->drawSegment(
            { PAD + 9.f, h - PAD - 55.f },
            { PAD + SIDEBAR_W - 9.f, h - PAD - 55.f },
            .6f, p.sidebarBorder
        );
        m_mainLayer->addChild(divider);

        // Close control.
        auto close = CCMenuItemExt::createSpriteExtra(
            theme::closeX(22.f),
            [this](CCMenuItemSpriteExtra*) { this->onClose(nullptr); }
        );
        m_buttonMenu->addChildAtPosition(
            close, Anchor::TopRight, { -3.f, -3.f }
        );

        buildBar();
        buildHeader();
        buildTab(m_tab);
        return true;
    }

    void buildHeader() {
        auto p = theme::pal();

        auto title = theme::text(m_tabs[m_tab].name, .58f, contentW() - 70.f);
        if (title) {
            title->setAnchorPoint({ 0.f, .5f });
            addNode(title, { contentX() + 12.f, m_h - PAD - 25.f });
        }

        auto status = theme::text("READY", .27f, 42.f);
        if (status) {
            status->setColor(p.secondaryText);
            status->setAnchorPoint({ 1.f, .5f });
            addNode(status, { m_w - PAD - 12.f, m_h - PAD - 25.f });
        }

        auto line = CCDrawNode::create();
        line->drawSegment(
            { contentX() + 10.f, m_h - PAD - HEADER_H },
            { m_w - PAD - 10.f, m_h - PAD - HEADER_H },
            .65f, p.panelBorder
        );
        addNode(line, { 0.f, 0.f });
    }

    void buildBar() {
        auto p = theme::pal();
        constexpr float itemW = SIDEBAR_W - 14.f;
        constexpr float itemH = 34.f;
        float startY = m_h - PAD - 77.f;

        for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
            bool selected = i == m_tab;
            auto face = CCNode::create();
            face->setContentSize({ itemW, itemH });

            if (selected) {
                face->addChild(theme::roundRect(
                    itemW, itemH, 6.f,
                    p.selected, p.thin, p.selectedBorder
                ));
                auto indicator = CCDrawNode::create();
                indicator->drawSegment(
                    { 3.f, 8.f }, { 3.f, itemH - 8.f },
                    2.f, p.accent
                );
                face->addChild(indicator);
            }

            auto icon = CCSprite::create(
                Mod::get()->expandSpriteName(m_tabs[i].icon).c_str()
            );
            float textX = 12.f;
            if (icon) {
                icon->setScale(16.f / icon->getContentSize().height);
                icon->setPosition({ 19.f, itemH / 2.f });
                face->addChild(icon);
                textX = 34.f;
            }

            auto label = theme::text(m_tabs[i].name, .31f, itemW - textX - 5.f);
            if (label) {
                label->setAnchorPoint({ 0.f, .5f });
                label->setPosition({ textX, itemH / 2.f });
                if (!selected) label->setColor(p.secondaryText);
                face->addChild(label);
            }

            auto button = CCMenuItemExt::createSpriteExtra(
                face,
                [this, i](CCMenuItemSpriteExtra*) {
                    if (i == m_tab) return;
                    queueInMainThread([self = Ref<NCWindow>(this), i] {
                        self->selectTab(i);
                    });
                }
            );

            m_buttonMenu->addChildAtPosition(
                button, Anchor::BottomLeft,
                { PAD + 7.f + itemW / 2.f, startY - i * 39.f }
            );
            m_barNodes.push_back(button);
        }
    }

    void clearNodes() {
        for (auto& node : m_barNodes) node->removeFromParent();
        for (auto& node : m_tabNodes) node->removeFromParent();
        m_barNodes.clear();
        m_tabNodes.clear();
    }

    void refresh() {
        clearNodes();
        buildBar();
        buildHeader();
        buildTab(m_tab);
    }

    void refreshLater() {
        queueInMainThread([self = Ref<NCWindow>(this)] { self->refresh(); });
    }

    void selectTab(int index) {
        if (index < 0 || index >= static_cast<int>(m_tabs.size())) return;
        m_tab = index;
        refresh();
    }

    float boxW() const {
        return (contentW() - 3.f * GAP) / 2.f;
    }

    CCPoint cell(int col, int row) const {
        float x = contentX() + GAP + col * (boxW() + GAP);
        float yTop = m_h - PAD - HEADER_H - 12.f - row * ROW_GAP;
        return { x, yTop - CARD_H };
    }

    void track(CCNode* node) {
        if (node) m_tabNodes.push_back(node);
    }

    void addNode(CCNode* node, CCPoint pos) {
        if (!node) return;
        m_mainLayer->addChildAtPosition(node, Anchor::BottomLeft, pos);
        track(node);
    }

    void addMenuItem(CCNode* node, CCPoint pos) {
        if (!node) return;
        m_buttonMenu->addChildAtPosition(node, Anchor::BottomLeft, pos);
        track(node);
    }

    void addBoxBase(CCPoint pos, std::string const& label, bool enabled = true) {
        auto p = theme::pal();
        addNode(
            theme::roundRect(boxW(), CARD_H, 6.f, p.card, p.thin, p.cardBorder),
            pos
        );

        auto text = theme::text(label, .30f, boxW() - 42.f);
        if (text) {
            text->setAnchorPoint({ 0.f, .5f });
            if (!enabled) text->setOpacity(115);
            addNode(text, { pos.x + 8.f, pos.y + CARD_H / 2.f });
        }
    }

    void addNote(std::string const& value) {
        auto label = theme::text(value, .27f, contentW() - 20.f);
        if (!label) return;
        label->setColor(theme::noteColor());
        addNode(label, { contentX() + contentW() / 2.f, PAD + 13.f });
    }

    void addCheckbox(
        int col, int row, std::string const& label, bool value,
        std::function<void(bool)> onChange, bool enabled = true
    ) {
        auto pos = cell(col, row);
        addBoxBase(pos, label, enabled);

        auto toggle = CCMenuItemExt::createToggler(
            theme::switchTrack(true, enabled),
            theme::switchTrack(false, enabled),
            [onChange](CCMenuItemToggler* item) {
                onChange(!item->isToggled());
            }
        );
        toggle->toggle(value);
        toggle->setEnabled(enabled);

        addMenuItem(toggle, {
            pos.x + boxW() - 23.f,
            pos.y + CARD_H / 2.f
        });
    }

    void addChoice(
        int col, int row, std::string const& label,
        std::string const& value, std::function<std::string()> onNext
    ) {
        auto pos = cell(col, row);
        addBoxBase(pos, label);

        auto p = theme::pal();
        constexpr float fieldW = 66.f;
        constexpr float fieldH = 25.f;

        auto face = CCNode::create();
        face->setContentSize({ fieldW, fieldH });
        face->addChild(theme::roundRect(
            fieldW, fieldH, 5.f, p.field, p.thin, p.fieldBorder
        ));

        auto labelNode = theme::text(value, .29f, fieldW - 7.f, true);
        if (labelNode) {
            labelNode->setPosition({ fieldW / 2.f, fieldH / 2.f });
            face->addChild(labelNode);
        }

        Ref<CCLabelBMFont> labelRef = labelNode;
        float baseScale = labelNode ? labelNode->getScale() : 1.f;
        auto button = CCMenuItemExt::createSpriteExtra(
            face,
            [onNext, labelRef, baseScale](CCMenuItemSpriteExtra*) {
                auto next = onNext();
                if (labelRef) {
                    labelRef->setString(next.c_str());
                    labelRef->setScale(baseScale);
                    labelRef->limitLabelWidth(59.f, baseScale, .1f);
                }
            }
        );

        addMenuItem(button, {
            pos.x + boxW() - fieldW / 2.f - 5.f,
            pos.y + CARD_H / 2.f
        });
    }

    void addNumber(
        int col, int row, std::string const& label,
        std::string const& value,
        std::function<void(std::string const&, TextInput*)> onChange
    ) {
        auto pos = cell(col, row);
        addBoxBase(pos, label);

        auto input = TextInput::create(64.f, "0", "bigFont.fnt");
        input->setCommonFilter(CommonFilter::Float);
        input->setMaxCharCount(8);
        input->setScale(.78f);
        input->setString(value);

        TextInput* raw = input;
        input->setCallback([onChange, raw](std::string const& text) {
            onChange(text, raw);
        });

        addNode(input, {
            pos.x + boxW() - 34.f,
            pos.y + CARD_H / 2.f
        });
    }

    void addAction(
        int col, int row, std::string const& label,
        std::function<void()> onTap
    ) {
        auto pos = cell(col, row);
        auto p = theme::pal();

        auto face = CCNode::create();
        face->setContentSize({ boxW(), CARD_H });
        face->addChild(theme::roundRect(
            boxW(), CARD_H, 6.f, p.card, p.thin, p.cardBorder
        ));

        auto text = theme::text(label, .31f, boxW() - 14.f);
        if (text) {
            text->setPosition({ boxW() / 2.f, CARD_H / 2.f });
            face->addChild(text);
        }

        auto button = CCMenuItemExt::createSpriteExtra(
            face, [onTap](CCMenuItemSpriteExtra*) { onTap(); }
        );
        addMenuItem(button, {
            pos.x + boxW() / 2.f,
            pos.y + CARD_H / 2.f
        });
    }
};
