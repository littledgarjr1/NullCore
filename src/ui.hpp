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
                rgb(250, 250, 250), rgb(0, 0, 0),
                rgb(255, 255, 255), rgb(0, 0, 0),
                rgb(244, 244, 244), rgb(0, 0, 0),
                rgb(225, 225, 225), rgb(0, 0, 0),
                rgb(255, 255, 255), rgb(205, 205, 205),
                rgb(235, 235, 235), rgb(0, 0, 0),
                rgb(0, 0, 0),
                { 12, 12, 12 }, { 105, 105, 105 },
                1.1f, 0.7f
            };
        }

        return {
            rgb(16, 16, 16), rgb(255, 255, 255),
            rgb(23, 23, 23), rgb(255, 255, 255),
            rgb(20, 20, 20), rgb(255, 255, 255),
            rgb(52, 52, 52), rgb(255, 255, 255),
            rgb(31, 31, 31), rgb(255, 255, 255),
            rgb(39, 39, 39), rgb(255, 255, 255),
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
        constexpr int segments = 28;
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

        auto outline = light() ? rgb(0, 0, 0) : rgb(255, 255, 255);
        auto fill = on ? p.accent : p.field;
        node->addChild(roundRect(size, size, 6.5f, fill, 1.f, outline));

        if (on) {
            auto tick = CCDrawNode::create();
            auto color = light() ? rgb(255, 255, 255) : rgb(0, 0, 0);
            tick->drawSegment({ size * .20f, size * .51f }, { size * .41f, size * .31f }, 2.2f, color);
            tick->drawSegment({ size * .41f, size * .31f }, { size * .81f, size * .73f }, 2.2f, color);
            node->addChild(tick);
        }

        return node;
    }

    inline CCNode* switchTrack(bool on, bool enabled = true) {
        // Modern pill toggle: crisp monochrome outline, smooth track and a
        // high-contrast circular thumb. The two states remain compatible
        // with CCMenuItemToggler and its existing callbacks.
        constexpr float width = 38.f;
        constexpr float height = 21.f;
        constexpr float inset = 2.5f;
        constexpr float thumbRadius = 7.5f;
        auto p = pal();
        auto node = CCNode::create();
        node->setContentSize({ width, height });

        auto outline = light() ? rgb(0, 0, 0) : rgb(255, 255, 255);
        auto track = roundRect(
            width, height, height / 2.f,
            on ? p.accent : p.field,
            enabled ? 1.f : .8f,
            outline
        );
        node->addChild(track);

        auto thumb = CCDrawNode::create();
        float x = on ? width - inset - thumbRadius : inset + thumbRadius;
        auto thumbColor = on
            ? (light() ? rgb(255, 255, 255) : rgb(0, 0, 0))
            : rgb(p.primaryText.r, p.primaryText.g, p.primaryText.b);
        thumb->drawDot({ x, height / 2.f }, thumbRadius, thumbColor);
        node->addChild(thumb);

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

    // Base dimensions are designed around a 380x370 reference window.
    // Every layout measurement is multiplied by m_scale.
    static constexpr float BASE_W = 380.f;
    static constexpr float BASE_H = 370.f;
    static constexpr float PAD = 10.f;
    static constexpr float SIDEBAR_W = 96.f;
    static constexpr float GAP = 8.f;
    static constexpr float HEADER_H = 34.f;
    static constexpr float CARD_H = 34.f;
    static constexpr float ROW_GAP = 41.f;

    float m_w = 0.f;
    float m_h = 0.f;
    float m_scale = 1.f;
    int m_tab = 0;

    float scaled(float value) const { return value * m_scale; }
    float pad() const { return scaled(PAD); }
    float sidebarW() const { return scaled(SIDEBAR_W); }
    float gap() const { return scaled(GAP); }
    float headerH() const { return scaled(HEADER_H); }
    float cardH() const { return scaled(CARD_H); }
    float rowGap() const { return scaled(ROW_GAP); }

    std::vector<TabDef> m_tabs;
    std::vector<Ref<CCNode>> m_barNodes;
    std::vector<Ref<CCNode>> m_tabNodes;

    virtual void buildTab(int index) = 0;

    float contentX() const { return pad() + sidebarW() + gap(); }
    float contentW() const { return m_w - contentX() - pad(); }
    float contentCenterX() const { return contentX() + contentW() * .5f; }

    bool initWindow(float, float, std::vector<TabDef> tabs, int startTab = 0) {
        // The popup always uses 70% of the active game window in both axes.
        // The supplied legacy dimensions remain in the signature for compatibility.
        auto win = CCDirector::sharedDirector()->getWinSize();
        float w = win.width * .70f;
        float h = win.height * .70f;
        if (w <= 0.f || h <= 0.f || tabs.empty()) return false;

        m_w = w;
        m_h = h;
        m_scale = std::clamp(std::min(w / BASE_W, h / BASE_H), .35f, 1.8f);
        if (!Popup::init(w, h)) return false;
        m_tabs = std::move(tabs);
        m_tab = std::clamp(startTab, 0, static_cast<int>(m_tabs.size()) - 1);

        m_bgSprite->setVisible(false);
        m_closeBtn->setVisible(false);

        auto p = theme::pal();

        m_mainLayer->addChildAtPosition(
            theme::roundRect(w, h, scaled(13.f), p.frame, p.stroke * m_scale, p.frameBorder),
            Anchor::BottomLeft, { 0.f, 0.f }
        );

        // Sidebar rail.
        m_mainLayer->addChildAtPosition(
            theme::roundRect(sidebarW(), h - 2.f * pad(), scaled(9.f), p.sidebar, p.thin * m_scale, p.sidebarBorder),
            Anchor::BottomLeft, { pad(), pad() }
        );

        // Main content surface.
        m_mainLayer->addChildAtPosition(
            theme::roundRect(contentW(), h - 2.f * pad(), scaled(9.f), p.panel, p.thin * m_scale, p.panelBorder),
            Anchor::BottomLeft, { contentX(), pad() }
        );

        // Brand mark + product label in the sidebar.
        auto mark = theme::roundRect(scaled(25.f), scaled(25.f), scaled(7.f), p.accent, 0.f, p.accent);
        m_mainLayer->addChildAtPosition(mark, Anchor::BottomLeft, { pad() + scaled(11.f), h - pad() - scaled(38.f) });

        auto brand = theme::text("NC", scaled(.48f), scaled(22.f));
        if (brand) {
            brand->setColor(theme::light() ? ccColor3B{255,255,255} : ccColor3B{0,0,0});
            m_mainLayer->addChildAtPosition(brand, Anchor::BottomLeft, { pad() + scaled(23.5f), h - pad() - scaled(25.5f) });
        }

        auto wordmark = theme::text("NULLCORE", scaled(.34f), scaled(58.f));
        if (wordmark) {
            wordmark->setAnchorPoint({ 0.f, .5f });
            m_mainLayer->addChildAtPosition(wordmark, Anchor::BottomLeft, { pad() + scaled(43.f), h - pad() - scaled(25.5f) });
        }

        // Thin divider below sidebar brand.
        auto divider = CCDrawNode::create();
        divider->drawSegment(
            { pad() + scaled(9.f), h - pad() - scaled(55.f) },
            { pad() + sidebarW() - scaled(9.f), h - pad() - scaled(55.f) },
            .6f * m_scale, p.sidebarBorder
        );
        m_mainLayer->addChild(divider);

        // Close control.
        auto close = CCMenuItemExt::createSpriteExtra(
            theme::closeX(scaled(22.f)),
            [this](CCMenuItemSpriteExtra*) { this->onClose(nullptr); }
        );
        m_buttonMenu->addChildAtPosition(
            close, Anchor::TopRight, { -scaled(3.f), -scaled(3.f) }
        );

        buildBar();
        buildHeader();
        buildTab(m_tab);
        return true;
    }

    void buildHeader() {
        auto p = theme::pal();

        auto title = theme::text(m_tabs[m_tab].name, scaled(.58f), contentW() - scaled(70.f));
        if (title) {
            title->setAnchorPoint({ 0.f, .5f });
            addNode(title, { contentX() + scaled(12.f), m_h - pad() - scaled(25.f) });
        }

        auto status = theme::text("READY", scaled(.27f), scaled(42.f));
        if (status) {
            status->setColor(p.secondaryText);
            status->setAnchorPoint({ 1.f, .5f });
            addNode(status, { m_w - pad() - scaled(12.f), m_h - pad() - scaled(25.f) });
        }

        auto line = CCDrawNode::create();
        line->drawSegment(
            { contentX() + scaled(10.f), m_h - pad() - headerH() },
            { m_w - pad() - scaled(10.f), m_h - pad() - headerH() },
            .65f * m_scale, p.panelBorder
        );
        addNode(line, { 0.f, 0.f });
    }

    void buildBar() {
        auto p = theme::pal();
        float itemW = sidebarW() - scaled(14.f);
        float itemH = scaled(34.f);
        float startY = m_h - pad() - scaled(77.f);

        for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
            bool selected = i == m_tab;
            auto face = CCNode::create();
            face->setContentSize({ itemW, itemH });

            if (selected) {
                face->addChild(theme::roundRect(
                    itemW, itemH, scaled(6.f),
                    p.selected, p.thin * m_scale, p.selectedBorder
                ));
                auto indicator = CCDrawNode::create();
                indicator->drawSegment(
                    { scaled(3.f), scaled(8.f) }, { scaled(3.f), itemH - scaled(8.f) },
                    scaled(2.f), p.accent
                );
                face->addChild(indicator);
            }

            auto icon = CCSprite::create(
                Mod::get()->expandSpriteName(m_tabs[i].icon).c_str()
            );
            float textX = scaled(12.f);
            if (icon) {
                icon->setScale(scaled(16.f) / icon->getContentSize().height);
                icon->setPosition({ scaled(19.f), itemH / 2.f });
                face->addChild(icon);
                textX = scaled(34.f);
            }

            auto label = theme::text(m_tabs[i].name, scaled(.31f), itemW - textX - scaled(5.f));
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
                { pad() + scaled(7.f) + itemW / 2.f, startY - i * scaled(39.f) }
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
        return contentW() - 2.f * gap();
    }

    CCPoint cell(int col, int row) const {
        (void)col; // Kept for compatibility with existing popup calls.
        float x = contentX() + gap();
        float yTop = m_h - pad() - headerH() - scaled(12.f) - row * rowGap();
        return { x, yTop - cardH() };
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
            theme::roundRect(boxW(), cardH(), scaled(6.f), p.card, p.thin * m_scale, p.cardBorder),
            pos
        );

        auto text = theme::text(label, scaled(.30f), boxW() - scaled(98.f));
        if (text) {
            text->setAnchorPoint({ 0.f, .5f });
            if (!enabled) text->setOpacity(115);
            addNode(text, { pos.x + scaled(8.f), pos.y + cardH() / 2.f });
        }
    }

    void addNote(std::string const& value) {
        auto label = theme::text(value, scaled(.27f), contentW() - scaled(20.f));
        if (!label) return;
        label->setColor(theme::noteColor());
        addNode(label, { contentCenterX(), pad() + scaled(13.f) });
    }

    void addCheckbox(
        int col, int row, std::string const& label, bool value,
        std::function<void(bool)> onChange, bool enabled = true
    ) {
        auto pos = cell(col, row);
        addBoxBase(pos, label, enabled);

        auto toggle = CCMenuItemExt::createToggler(
            theme::checkBox(true, enabled),
            theme::checkBox(false, enabled),
            [onChange](CCMenuItemToggler* item) {
                onChange(!item->isToggled());
            }
        );
        toggle->toggle(value);
        toggle->setEnabled(enabled);
        toggle->setScale(m_scale);

        addMenuItem(toggle, {
            pos.x + boxW() - scaled(23.f),
            pos.y + cardH() / 2.f
        });
    }

    void addCheckboxWithGear(
        int col, int row, std::string const& label, bool value,
        std::function<void(bool)> onChange, std::function<void()> onSettings,
        bool enabled = true
    ) {
        auto pos = cell(col, row);
        addBoxBase(pos, label, enabled);
        auto toggle = CCMenuItemExt::createToggler(
            theme::checkBox(true, enabled), theme::checkBox(false, enabled),
            [onChange](CCMenuItemToggler* item) { onChange(!item->isToggled()); }
        );
        toggle->toggle(value);
        toggle->setEnabled(enabled);
        toggle->setScale(m_scale);
        addMenuItem(toggle, { pos.x + boxW() - scaled(23.f), pos.y + cardH() / 2.f });
        auto gear = CCSprite::create("nullcore-settings.png"_spr);
        if (gear) {
            gear->setScale(scaled(15.f) / gear->getContentSize().height);
            auto button = CCMenuItemExt::createSpriteExtra(gear,
                [onSettings](CCMenuItemSpriteExtra*) { onSettings(); });
            addMenuItem(button, { pos.x + boxW() - scaled(51.f), pos.y + cardH() / 2.f });
        }
    }
    void addChoice(
        int col, int row, std::string const& label,
        std::string const& value, std::function<std::string()> onNext
    ) {
        auto pos = cell(col, row);
        addBoxBase(pos, label);

        auto p = theme::pal();
        float fieldW = scaled(66.f);
        float fieldH = scaled(25.f);

        auto face = CCNode::create();
        face->setContentSize({ fieldW, fieldH });
        face->addChild(theme::roundRect(
            fieldW, fieldH, scaled(5.f), p.field, p.thin * m_scale, p.fieldBorder
        ));

        auto labelNode = theme::text(value, scaled(.29f), fieldW - scaled(7.f), true);
        if (labelNode) {
            labelNode->setPosition({ fieldW / 2.f, fieldH / 2.f });
            face->addChild(labelNode);
        }

        Ref<CCLabelBMFont> labelRef = labelNode;
        float baseScale = labelNode ? labelNode->getScale() : 1.f;
        float labelMaxWidth = scaled(59.f);
        auto button = CCMenuItemExt::createSpriteExtra(
            face,
            [onNext, labelRef, baseScale, labelMaxWidth](CCMenuItemSpriteExtra*) {
                auto next = onNext();
                if (labelRef) {
                    labelRef->setString(next.c_str());
                    labelRef->setScale(baseScale);
                    labelRef->limitLabelWidth(labelMaxWidth, baseScale, .1f);
                }
            }
        );

        addMenuItem(button, {
            pos.x + boxW() - fieldW / 2.f - scaled(5.f),
            pos.y + cardH() / 2.f
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
        input->setScale(.78f * m_scale);
        input->setString(value);

        TextInput* raw = input;
        input->setCallback([onChange, raw](std::string const& text) {
            onChange(text, raw);
        });

        addNode(input, {
            pos.x + boxW() - scaled(34.f),
            pos.y + cardH() / 2.f
        });
    }

    void addAction(
        int col, int row, std::string const& label,
        std::function<void()> onTap
    ) {
        auto pos = cell(col, row);
        auto p = theme::pal();

        auto face = CCNode::create();
        face->setContentSize({ boxW(), cardH() });
        face->addChild(theme::roundRect(
            boxW(), cardH(), scaled(6.f), p.card, p.thin * m_scale, p.cardBorder
        ));

        auto text = theme::text(label, scaled(.31f), boxW() - scaled(14.f));
        if (text) {
            text->setPosition({ boxW() / 2.f, cardH() / 2.f });
            face->addChild(text);
        }

        auto button = CCMenuItemExt::createSpriteExtra(
            face, [onTap](CCMenuItemSpriteExtra*) { onTap(); }
        );
        addMenuItem(button, {
            pos.x + boxW() / 2.f,
            pos.y + cardH() / 2.f
        });
    }
};
