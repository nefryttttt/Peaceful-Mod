#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <random>
#include <chrono>

using namespace geode::prelude;

// Изначально мод выключен
bool g_chaosEnabled = false;
int g_chanceZoom = 20;
int g_chanceSound = 50;
int g_chanceJump = 25;
int g_chanceFlash = 10;
int g_chanceFlip = 30;
int g_chanceWarp = 35;
int g_chanceRotate = 25;
int g_chanceFreeze = 5;

class ChaosSettingsPopup : public Popup {
protected:
    TextInput* m_zoomInput = nullptr;
    TextInput* m_soundInput = nullptr;
    TextInput* m_jumpInput = nullptr;
    TextInput* m_flashInput = nullptr;
    TextInput* m_warpInput = nullptr;

    int m_tempZoom = g_chanceZoom;
    int m_tempSound = g_chanceSound;
    int m_tempJump = g_chanceJump;
    int m_tempFlash = g_chanceFlash;
    int m_tempWarp = g_chanceWarp;

public:
    static ChaosSettingsPopup* create() {
        auto ret = new ChaosSettingsPopup();
        if (ret && ret->init(380.f, 260.f)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool init(float w, float h) {
        if (!Popup::init(w, h)) return false;

        auto& ml = *m_mainLayer;

        auto title = CCLabelBMFont::create("Chaos Settings", "bigFont.fnt");
        title->setScale(0.48f);
        title->setPosition({m_size.width / 2.0f, m_size.height - 25.0f});
        ml.addChild(title);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        ml.addChild(menu);

        auto createRow = [&](const char* labelText, int& valueRef, float yPos, int tagMin, int tagPlus, TextInput*& outInput) {
            auto label = CCLabelBMFont::create(labelText, "goldFont.fnt");
            label->setScale(0.35f);
            label->setAnchorPoint({0.0f, 0.5f});
            label->setPosition({20.f, yPos});
            ml.addChild(label);

            float centerX = m_size.width - 150.f;

            auto minusSpr = ButtonSprite::create("-", 26, 26, 0.4f, true, "bigFont.fnt", "GJ_button_02.png");
            minusSpr->setScale(0.5f);
            auto minusBtn = CCMenuItemSpriteExtra::create(minusSpr, this, menu_selector(ChaosSettingsPopup::onMinus));
            minusBtn->setTag(tagMin);
            minusBtn->setPosition({centerX - 65.f, yPos});
            menu->addChild(minusBtn);

            outInput = TextInput::create(45.f, "0", "bigFont.fnt");
            outInput->setString(fmt::format("{}", valueRef));
            outInput->setCommonFilter(CommonFilter::Uint);
            outInput->setMaxCharCount(3);
            outInput->setScale(0.5f);
            outInput->setPosition({centerX - 20.f, yPos});

            outInput->setCallback([&valueRef](std::string const& text) {
                if (!text.empty()) {
                    int parsed = geode::utils::numFromString<int>(text).unwrapOr(0);
                    valueRef = std::clamp(parsed, 0, 100);
                } else {
                    valueRef = 0;
                }
            });
            ml.addChild(outInput);

            auto percentSign = CCLabelBMFont::create("%", "bigFont.fnt");
            percentSign->setScale(0.4f);
            percentSign->setAnchorPoint({0.0f, 0.5f});
            percentSign->setPosition({centerX + 8.f, yPos});
            ml.addChild(percentSign);

            auto plusSpr = ButtonSprite::create("+", 26, 26, 0.4f, true, "bigFont.fnt", "GJ_button_01.png");
            plusSpr->setScale(0.5f);
            auto plusBtn = CCMenuItemSpriteExtra::create(plusSpr, this, menu_selector(ChaosSettingsPopup::onPlus));
            plusBtn->setTag(tagPlus);
            plusBtn->setPosition({centerX + 55.f, yPos});
            menu->addChild(plusBtn);
        };

        float startY = m_size.height - 65.0f;
        float stepY = 32.0f;

        createRow("Zoom Chance:", m_tempZoom, startY, 1, 2, m_zoomInput);
        createRow("Sound & ! Chance:", m_tempSound, startY - stepY, 3, 4, m_soundInput);
        createRow("Jump Chance:", m_tempJump, startY - (stepY * 2.0f), 5, 6, m_jumpInput);
        createRow("Flash/Dark Screen:", m_tempFlash, startY - (stepY * 3.0f), 7, 8, m_flashInput);
        createRow("Flip / Rotate / Warp:", m_tempWarp, startY - (stepY * 4.0f), 9, 10, m_warpInput);

        auto saveBtnSpr = ButtonSprite::create("Save", 90, 30, 0.6f, true, "bigFont.fnt", "GJ_button_01.png");
        saveBtnSpr->setScale(0.6f);
        auto saveBtn = CCMenuItemSpriteExtra::create(saveBtnSpr, this, menu_selector(ChaosSettingsPopup::onSave));
        saveBtn->setPosition({m_size.width / 2.0f, 22.f});
        menu->addChild(saveBtn);

        return true;
    }

    void updateInputs() {
        if (m_zoomInput) m_zoomInput->setString(fmt::format("{}", m_tempZoom));
        if (m_soundInput) m_soundInput->setString(fmt::format("{}", m_tempSound));
        if (m_jumpInput) m_jumpInput->setString(fmt::format("{}", m_tempJump));
        if (m_flashInput) m_flashInput->setString(fmt::format("{}", m_tempFlash));
        if (m_warpInput) m_warpInput->setString(fmt::format("{}", m_tempWarp));
    }

    void onMinus(CCObject* sender) {
        int tag = sender->getTag();
        if (tag == 1) m_tempZoom = std::max(0, m_tempZoom - 5);
        else if (tag == 3) m_tempSound = std::max(0, m_tempSound - 5);
        else if (tag == 5) m_tempJump = std::max(0, m_tempJump - 5);
        else if (tag == 7) m_tempFlash = std::max(0, m_tempFlash - 5);
        else if (tag == 9) {
            m_tempZoom = std::max(0, m_tempZoom - 5);
            m_tempWarp = std::max(0, m_tempWarp - 5);
        }
        updateInputs();
    }

    void onPlus(CCObject* sender) {
        int tag = sender->getTag();
        if (tag == 2) m_tempZoom = std::min(100, m_tempZoom + 5);
        else if (tag == 4) m_tempSound = std::min(100, m_tempSound + 5);
        else if (tag == 6) m_tempJump = std::min(100, m_tempJump + 5);
        else if (tag == 8) m_tempFlash = std::min(100, m_tempFlash + 5);
        else if (tag == 10) {
            m_tempZoom = std::min(100, m_tempZoom + 5);
            m_tempWarp = std::min(100, m_tempWarp + 5);
        }
        updateInputs();
    }

    void onSave(CCObject*) {
        g_chanceZoom = m_tempZoom;
        g_chanceSound = m_tempSound;
        g_chanceJump = m_tempJump;
        g_chanceFlash = m_tempFlash;
        g_chanceWarp = m_tempWarp;
        g_chanceRotate = m_tempWarp;
        g_chanceFlip = m_tempWarp;

        Notification::create("Settings Saved!", NotificationIcon::Success, 0.6f)->show();
        this->onClose(nullptr);
    }
};

class $modify(ChaosMenuLayer, MenuLayer) {
    struct Fields {
        bool lastPState = false;
        float pDoublePressTimer = 0.0f;
        int pPressCount = 0;
    };

    bool init() {
        if (!MenuLayer::init()) return false;
        g_chaosEnabled = false;
        CCScheduler::get()->setTimeScale(1.0f);
        this->schedule(schedule_selector(ChaosMenuLayer::checkKeys));
        return true;
    }

    void checkKeys(float dt) {
        bool currentPState = (GetAsyncKeyState('P') & 0x8000) != 0;
        if (currentPState && !m_fields->lastPState) {
            m_fields->pPressCount++;
            m_fields->pDoublePressTimer = 0.4f;

            if (m_fields->pPressCount >= 2) {
                m_fields->pPressCount = 0;
                ChaosSettingsPopup::create()->show();
            }
        }
        m_fields->lastPState = currentPState;

        if (m_fields->pDoublePressTimer > 0.0f) {
            m_fields->pDoublePressTimer -= dt;
            if (m_fields->pDoublePressTimer <= 0.0f) {
                m_fields->pPressCount = 0;
            }
        }
    }
};

class $modify(ChaosPlayLayer, PlayLayer) {
    struct Fields {
        CCLayerColor* blackScreen = nullptr;
        float blackScreenTimer = 0.0f;

        float jumpTimer = 0.0f;
        float flashTimer = 0.0f;
        float soundTimer = 0.0f;

        bool isFlipped = false;
        float flipTimer = 0.0f;
        float flipCooldown = 3.0f;

        float timeWarpTimer = 0.0f;
        float timeWarpCooldown = 4.0f;

        bool isRotating = false;
        float rotationTimer = 0.0f;
        float rotationCooldown = 4.0f;

        bool isZooming = false;
        float zoomTimer = 0.0f;
        float zoomCooldown = 3.0f;

        bool isHardFrozen = false;
        std::chrono::steady_clock::time_point freezeStartTime;
        float freezeDurationSeconds = 3.0f;
        float freezeCooldown = 8.0f;

        CCLabelBMFont* exclamationLabel = nullptr;
        float exclamationTimer = 0.0f;
        float exclamationMaxTime = 0.7f;

        bool lastShiftState = false;
        int shiftPressCount = 0;
        float shiftResetTimer = 0.0f;

        bool lastPState = false;
        float pDoublePressTimer = 0.0f;
        int pPressCount = 0;

        int realDeaths = 0;
        bool chaosActive = false;
        float enterGraceTimer = 0.0f;
    };

    void ensureOverlays() {
        auto winSize = CCDirector::get()->getWinSize();

        if (!m_fields->blackScreen) {
            m_fields->blackScreen = CCLayerColor::create(ccc4(0, 0, 0, 255), winSize.width * 2.0f, winSize.height * 2.0f);
            m_fields->blackScreen->setPosition({-winSize.width * 0.5f, -winSize.height * 0.5f});
            m_fields->blackScreen->setOpacity(0);
            m_fields->blackScreen->setZOrder(99999);
            this->addChild(m_fields->blackScreen);
        }

        if (!m_fields->exclamationLabel) {
            m_fields->exclamationLabel = CCLabelBMFont::create("!", "goldFont.fnt");
            m_fields->exclamationLabel->setOpacity(0);
            m_fields->exclamationLabel->setScale(1.2f);
            m_fields->exclamationLabel->setZOrder(9998);

            if (m_objectLayer) {
                m_objectLayer->addChild(m_fields->exclamationLabel);
            } else {
                this->addChild(m_fields->exclamationLabel);
            }
        }
    }

    void showBlackScreen(float duration) {
        ensureOverlays();
        if (!m_fields->blackScreen) return;
        m_fields->blackScreen->setColor({0, 0, 0});
        m_fields->blackScreen->setOpacity(255);
        m_fields->blackScreenTimer = duration;
    }

    void hideBlackScreen() {
        if (!m_fields->blackScreen) return;
        m_fields->blackScreen->setOpacity(0);
        m_fields->blackScreenTimer = 0.0f;
    }

    void resetChaosEffects() {
        hideBlackScreen();
        if (m_fields->exclamationLabel) {
            m_fields->exclamationLabel->setOpacity(0);
        }
        this->setScale(1.0f);
        this->setScaleX(1.0f);
        this->setRotation(0.0f);

        m_fields->isHardFrozen = false;
        m_fields->isZooming = false;
        m_fields->isFlipped = false;
        m_fields->isRotating = false;
        m_fields->blackScreenTimer = 0.0f;
        m_fields->exclamationTimer = 0.0f;
        m_fields->timeWarpTimer = 0.0f;
        m_fields->flipTimer = 0.0f;
        m_fields->rotationTimer = 0.0f;
        m_fields->zoomTimer = 0.0f;

        CCScheduler::get()->setTimeScale(1.0f);
    }

    void setChaosState(bool enabled, bool showOffToast) {
        bool wasEnabled = m_fields->chaosActive && g_chaosEnabled;
        m_fields->chaosActive = enabled;
        g_chaosEnabled = enabled;

        if (!enabled) {
            resetChaosEffects();
            if (showOffToast && wasEnabled) {
                Notification::create("Chaos Mode: OFF", NotificationIcon::Success, 0.6f)->show();
            }
        } else {
            m_fields->jumpTimer = 0.8f;
            m_fields->flashTimer = 1.5f;
            m_fields->soundTimer = 1.0f;
            m_fields->flipCooldown = 2.5f;
            m_fields->timeWarpCooldown = 3.5f;
            m_fields->rotationCooldown = 3.5f;
            m_fields->zoomCooldown = 2.5f;
            m_fields->freezeCooldown = 9.0f;
        }
    }

    void onEnter() override {
        PlayLayer::onEnter();
        m_fields->realDeaths = 0;
        m_fields->enterGraceTimer = 1.0f;
        setChaosState(false, false);
        ensureOverlays();
    }

    void onExit() override {
        PlayLayer::onExit();
        setChaosState(false, false);
        m_fields->blackScreen = nullptr;
        m_fields->exclamationLabel = nullptr;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) override {
        PlayLayer::destroyPlayer(player, object);

        if (m_fields->enterGraceTimer > 0.0f || !m_player1 || m_hasCompletedLevel) {
            return;
        }

        m_fields->realDeaths++;
        if (m_fields->realDeaths >= 2 && !m_fields->chaosActive) {
            setChaosState(true, false);
        }
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (!m_player1 || m_hasCompletedLevel) return;

        float realDt = (dt > 0.0f) ? dt : (1.0f / 60.0f);
        float currentScale = CCScheduler::get()->getTimeScale();
        if (currentScale > 0.0001f && currentScale != 1.0f) {
            realDt = dt / currentScale;
        }

        if (m_fields->enterGraceTimer > 0.0f) {
            m_fields->enterGraceTimer -= realDt;
            if (m_fields->enterGraceTimer < 0.0f) m_fields->enterGraceTimer = 0.0f;
        }

        bool currentPState = (GetAsyncKeyState('P') & 0x8000) != 0;
        if (currentPState && !m_fields->lastPState) {
            m_fields->pPressCount++;
            m_fields->pDoublePressTimer = 0.4f;
            if (m_fields->pPressCount >= 2) {
                m_fields->pPressCount = 0;
                this->pauseGame(false);
                ChaosSettingsPopup::create()->show();
            }
        }
        m_fields->lastPState = currentPState;
        if (m_fields->pDoublePressTimer > 0.0f) {
            m_fields->pDoublePressTimer -= realDt;
            if (m_fields->pDoublePressTimer <= 0.0f) m_fields->pPressCount = 0;
        }

        bool currentShiftState = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        if (currentShiftState && !m_fields->lastShiftState) {
            m_fields->shiftPressCount++;
            m_fields->shiftResetTimer = 0.6f;
            if (m_fields->shiftPressCount >= 3) {
                m_fields->shiftPressCount = 0;
                setChaosState(!m_fields->chaosActive, true);
            }
        }
        m_fields->lastShiftState = currentShiftState;
        if (m_fields->shiftResetTimer > 0.0f) {
            m_fields->shiftResetTimer -= realDt;
            if (m_fields->shiftResetTimer <= 0.0f) m_fields->shiftPressCount = 0;
        }

        if (!g_chaosEnabled || !m_fields->chaosActive) {
            if (m_fields->exclamationLabel && m_fields->exclamationLabel->getOpacity() > 0) {
                m_fields->exclamationLabel->setOpacity(0);
            }
            if (m_fields->blackScreen && m_fields->blackScreen->getOpacity() > 0) {
                hideBlackScreen();
            }
            return;
        }

        if (m_fields->exclamationTimer > 0.0f) {
            m_fields->exclamationTimer -= realDt;
            if (m_player1 && m_fields->exclamationLabel) {
                m_fields->exclamationLabel->setPosition(m_player1->getPosition() + ccp(0.0f, 40.0f));
                float progress = m_fields->exclamationTimer / m_fields->exclamationMaxTime;
                m_fields->exclamationLabel->setOpacity(static_cast<GLubyte>(std::clamp(progress * 255.0f, 0.0f, 255.0f)));
            }
        }

        if (m_fields->isHardFrozen) {
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<float> elapsed = now - m_fields->freezeStartTime;
            if (elapsed.count() >= m_fields->freezeDurationSeconds) {
                m_fields->isHardFrozen = false;
                hideBlackScreen();
                CCScheduler::get()->setTimeScale(1.0f);
            }
            return;
        }

        if (m_fields->blackScreen && m_fields->blackScreen->getOpacity() > 0) {
            m_fields->blackScreenTimer -= realDt;
            if (m_fields->blackScreenTimer <= 0.0f) {
                hideBlackScreen();
            }
        }

        if (m_fields->isFlipped) {
            m_fields->flipTimer -= realDt;
            if (m_fields->flipTimer <= 0.0f) {
                this->setScaleX(1.0f);
                m_fields->isFlipped = false;
            }
        }

        if (m_fields->timeWarpTimer > 0.0f) {
            m_fields->timeWarpTimer -= realDt;
            if (m_fields->timeWarpTimer <= 0.0f) {
                CCScheduler::get()->setTimeScale(1.0f);
            }
        }

        if (m_fields->isRotating) {
            m_fields->rotationTimer -= realDt;
            if (m_fields->rotationTimer <= 0.0f) {
                this->setRotation(0.0f);
                m_fields->isRotating = false;
            }
        }

        if (m_fields->isZooming) {
            m_fields->zoomTimer -= realDt;
            if (m_fields->zoomTimer <= 0.0f) {
                this->setScale(1.0f);
                m_fields->isZooming = false;
            }
        }

        // --- СОБЫТИЯ ХАОСА ---

        m_fields->freezeCooldown -= realDt;
        if (m_fields->freezeCooldown <= 0.0f && (rand() % 100) < g_chanceFreeze) {
            m_fields->isHardFrozen = true;
            m_fields->freezeStartTime = std::chrono::steady_clock::now();
            m_fields->freezeDurationSeconds = 2.0f + static_cast<float>(rand() % 15) / 10.0f;

            CCScheduler::get()->setTimeScale(0.02f);
            showBlackScreen(m_fields->freezeDurationSeconds);

            m_fields->freezeCooldown = 10.0f + static_cast<float>(rand() % 10);
            return;
        }

        m_fields->jumpTimer -= realDt;
        if (m_fields->jumpTimer <= 0.0f) {
            m_fields->jumpTimer = 0.6f + static_cast<float>(rand() % 20) / 10.0f;
            if ((rand() % 100) < g_chanceJump) {
                m_player1->pushButton(PlayerButton::Jump);
                m_player1->releaseButton(PlayerButton::Jump);
            }
        }

        m_fields->flashTimer -= realDt;
        if (m_fields->flashTimer <= 0.0f) {
            m_fields->flashTimer = 1.2f + static_cast<float>(rand() % 30) / 10.0f;
            if ((rand() % 100) < g_chanceFlash) {
                float duration = 0.5f + static_cast<float>(rand() % 10) / 10.0f;
                showBlackScreen(duration);
            }
        }

        m_fields->soundTimer -= realDt;
        if (m_fields->soundTimer <= 0.0f) {
            m_fields->soundTimer = 0.8f + static_cast<float>(rand() % 20) / 10.0f;
            if ((rand() % 100) < g_chanceSound) {
                auto fmod = FMODAudioEngine::get();
                if (fmod) fmod->playEffectAsync("secretKey.ogg");
                if (m_fields->exclamationLabel && m_player1) {
                    m_fields->exclamationLabel->setPosition(m_player1->getPosition() + ccp(0.0f, 40.0f));
                    m_fields->exclamationLabel->setOpacity(255);
                    m_fields->exclamationMaxTime = 0.7f;
                    m_fields->exclamationTimer = 0.7f;
                }
            }
        }

        m_fields->flipCooldown -= realDt;
        if (m_fields->flipCooldown <= 0.0f && !m_fields->isFlipped) {
            m_fields->flipCooldown = 2.0f + static_cast<float>(rand() % 30) / 10.0f;
            if ((rand() % 100) < g_chanceFlip) {
                this->setScaleX(-1.0f);
                m_fields->isFlipped = true;
                m_fields->flipTimer = 1.5f + static_cast<float>(rand() % 15) / 10.0f;
            }
        }

        m_fields->timeWarpCooldown -= realDt;
        if (m_fields->timeWarpCooldown <= 0.0f && m_fields->timeWarpTimer <= 0.0f) {
            m_fields->timeWarpCooldown = 3.0f + static_cast<float>(rand() % 40) / 10.0f;
            if ((rand() % 100) < g_chanceWarp) {
                float warpSpeeds[] = { 0.5f, 0.75f, 1.25f, 1.5f, 2.0f };
                float chosenSpeed = warpSpeeds[rand() % 5];

                CCScheduler::get()->setTimeScale(chosenSpeed);
                m_fields->timeWarpTimer = 1.5f + static_cast<float>(rand() % 15) / 10.0f;
            }
        }

        m_fields->rotationCooldown -= realDt;
        if (m_fields->rotationCooldown <= 0.0f && !m_fields->isRotating) {
            m_fields->rotationCooldown = 3.0f + static_cast<float>(rand() % 40) / 10.0f;
            if ((rand() % 100) < g_chanceRotate) {
                float angles[] = { 90.0f, 180.0f, 270.0f, 360.0f, -90.0f, -180.0f };
                float chosenAngle = angles[rand() % 6];
                this->setRotation(chosenAngle);
                m_fields->isRotating = true;
                m_fields->rotationTimer = 1.5f + static_cast<float>(rand() % 15) / 10.0f;
            }
        }

        m_fields->zoomCooldown -= realDt;
        if (m_fields->zoomCooldown <= 0.0f && !m_fields->isZooming) {
            m_fields->zoomCooldown = 2.0f + static_cast<float>(rand() % 25) / 10.0f;
            if ((rand() % 100) < g_chanceZoom) {
                float zoomLevels[] = { 0.6f, 0.7f, 1.35f, 1.5f };
                float chosenZoom = zoomLevels[rand() % 4];

                this->setScale(chosenZoom);
                m_fields->isZooming = true;
                m_fields->zoomTimer = 1.2f + static_cast<float>(rand() % 15) / 10.0f;
            }
        }
    }
};