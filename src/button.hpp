#pragma once
#include <Geode/Geode.hpp>
#include "popups.hpp"

using namespace geode::prelude;

namespace nc {
    // A small round button that floats above everything. Drag it anywhere,
    // tap it to open the NullCore menu. Its position is remembered.
    class FloatingButton : public CCLayer {
    protected:
        static constexpr float SIZE = 46.f;
        CCSprite* m_icon = nullptr;
        CCPoint m_touchStart;
        CCPoint m_posStart;
        bool m_held = false;
        bool m_moved = false;

        CCPoint clampPos(CCPoint p) {
            auto win = CCDirector::sharedDirector()->getWinSize();
            float h = SIZE / 2.f;
            p.x = std::clamp(p.x, h, win.width - h);
            p.y = std::clamp(p.y, h, win.height - h);
            return p;
        }

        bool init() override {
            if (!CCLayer::init()) return false;

            this->setContentSize(CCSize(SIZE, SIZE));
            this->ignoreAnchorPointForPosition(false);
            this->setAnchorPoint(CCPoint(0.5f, 0.5f));
            this->setTouchMode(kCCTouchesOneByOne);
            this->setTouchEnabled(true);

            // Grey ring with a black disc behind the icon.
            auto ring = CCDrawNode::create();
            ring->drawDot(CCPoint(SIZE / 2.f, SIZE / 2.f), SIZE / 2.f, theme::grey());
            ring->drawDot(CCPoint(SIZE / 2.f, SIZE / 2.f), SIZE / 2.f - 2.f, theme::black());
            this->addChild(ring);

            m_icon = CCSprite::create("nullcore-icon.png"_spr);
            if (m_icon) {
                m_icon->setScale((SIZE - 10.f) / m_icon->getContentSize().height);
                m_icon->setPosition(CCPoint(SIZE / 2.f, SIZE / 2.f));
                this->addChild(m_icon);
            }
            else {
                auto n = CCLabelBMFont::create("N", "bigFont.fnt");
                n->setScale(0.6f);
                n->setPosition(CCPoint(SIZE / 2.f, SIZE / 2.f));
                this->addChild(n);
            }

            auto win = CCDirector::sharedDirector()->getWinSize();
            float x = Mod::get()->getSavedValue<float>("btn-x", 30.f);
            float y = Mod::get()->getSavedValue<float>("btn-y", win.height * 0.7f);
            this->setPosition(clampPos(CCPoint(x, y)));
            this->setZOrder(1000);

            this->scheduleUpdate();
            return true;
        }

        // Applies the "show button" and "opacity" settings from the mod's settings page.
        void update(float) override {
            this->setVisible(Mod::get()->getSettingValue<bool>("show-button"));
            if (m_icon) {
                float o = static_cast<float>(Mod::get()->getSettingValue<double>("button-opacity"));
                m_icon->setOpacity(static_cast<GLubyte>(std::clamp(o, 0.2f, 1.f) * 255.f));
            }
        }

        void registerWithTouchDispatcher() override {
            CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -1000, true);
        }

        bool ccTouchBegan(CCTouch* touch, CCEvent*) override {
            if (!this->isVisible()) return false;
            auto local = this->convertTouchToNodeSpace(touch);
            if (local.x < -6.f || local.y < -6.f || local.x > SIZE + 6.f || local.y > SIZE + 6.f) return false;
            m_held = true;
            m_moved = false;
            m_touchStart = touch->getLocation();
            m_posStart = this->getPosition();
            this->setScale(0.92f);
            return true;
        }

        void ccTouchMoved(CCTouch* touch, CCEvent*) override {
            if (!m_held) return;
            auto d = touch->getLocation() - m_touchStart;
            if (!m_moved && (d.x * d.x + d.y * d.y) > 100.f) m_moved = true; // 10px dead zone
            if (m_moved) this->setPosition(clampPos(m_posStart + d));
        }

        void ccTouchEnded(CCTouch*, CCEvent*) override {
            if (!m_held) return;
            m_held = false;
            this->setScale(1.f);
            if (m_moved) {
                Mod::get()->setSavedValue<float>("btn-x", this->getPositionX());
                Mod::get()->setSavedValue<float>("btn-y", this->getPositionY());
            }
            else {
                NullCorePopup::toggle();
            }
        }

        void ccTouchCancelled(CCTouch*, CCEvent*) override {
            m_held = false;
            this->setScale(1.f);
        }

    public:
        static FloatingButton* create() {
            auto ret = new FloatingButton();
            if (ret->init()) { ret->autorelease(); return ret; }
            delete ret;
            return nullptr;
        }
    };
}
