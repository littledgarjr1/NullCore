#pragma once
#include <Geode/Geode.hpp>
#include "popups.hpp"

using namespace geode::prelude;

namespace nc {
    // A small button that floats above everything. Drag it anywhere,
    // tap it to open the NullCore menu. Its position is remembered.
    // Size and opacity come from NullCore Settings.
    class FloatingButton : public CCLayer {
    protected:
        inline static FloatingButton* s_instance = nullptr;

        float m_size = 34.f;
        CCSprite* m_icon = nullptr;
        CCLabelBMFont* m_fallback = nullptr;
        CCPoint m_touchStart;
        CCPoint m_posStart;
        bool m_held = false;
        bool m_moved = false;

        static float sizeFromSetting() {
            auto s = Mod::get()->getSettingValue<std::string>("button-size");
            if (s == "Large") return 52.f;
            if (s == "Medium") return 42.f;
            return 34.f; // Small
        }

        static CCPoint defaultPos() {
            auto win = CCDirector::sharedDirector()->getWinSize();
            return CCPoint(30.f, win.height * 0.7f);
        }

        CCPoint clampPos(CCPoint p) {
            auto win = CCDirector::sharedDirector()->getWinSize();
            float h = m_size / 2.f;
            p.x = std::clamp(p.x, h, win.width - h);
            p.y = std::clamp(p.y, h, win.height - h);
            return p;
        }

        void applySize(float s) {
            m_size = s;
            this->setContentSize(CCSize(s, s));
            if (m_icon) {
                m_icon->setScale(s / m_icon->getContentSize().height);
                m_icon->setPosition(CCPoint(s / 2.f, s / 2.f));
            }
            if (m_fallback) m_fallback->setPosition(CCPoint(s / 2.f, s / 2.f));
            this->setPosition(clampPos(this->getPosition()));
        }

        bool init() override {
            if (!CCLayer::init()) return false;
            s_instance = this;

            this->ignoreAnchorPointForPosition(false);
            this->setAnchorPoint(CCPoint(0.5f, 0.5f));
            this->setTouchMode(kCCTouchesOneByOne);
            this->setTouchEnabled(true);

            // The icon already has its own rounded border, so it is the whole button.
            m_icon = CCSprite::create("nullcore-mainicon.png"_spr);
            if (m_icon) {
                this->addChild(m_icon);
            }
            else {
                m_fallback = CCLabelBMFont::create("N", "bigFont.fnt");
                m_fallback->setScale(0.6f);
                this->addChild(m_fallback);
            }

            float x = Mod::get()->getSavedValue<float>("btn-x", defaultPos().x);
            float y = Mod::get()->getSavedValue<float>("btn-y", defaultPos().y);
            this->setPosition(CCPoint(x, y));
            this->applySize(sizeFromSetting());
            this->setZOrder(1000);

            this->scheduleUpdate();
            return true;
        }

        // Applies the show / size / opacity settings.
        void update(float) override {
            this->setVisible(Mod::get()->getSettingValue<bool>("show-button"));

            float s = sizeFromSetting();
            if (s != m_size) this->applySize(s);

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
            if (local.x < -6.f || local.y < -6.f || local.x > m_size + 6.f || local.y > m_size + 6.f) return false;
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
        ~FloatingButton() {
            if (s_instance == this) s_instance = nullptr;
        }

        static FloatingButton* create() {
            auto ret = new FloatingButton();
            if (ret->init()) { ret->autorelease(); return ret; }
            delete ret;
            return nullptr;
        }

        // Puts the button back at its default spot.
        static void resetPosition() {
            auto p = defaultPos();
            Mod::get()->setSavedValue<float>("btn-x", p.x);
            Mod::get()->setSavedValue<float>("btn-y", p.y);
            if (s_instance) s_instance->setPosition(s_instance->clampPos(p));
        }
    };
}
