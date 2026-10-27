#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <random>
#include <chrono>

using namespace geode::prelude;

// Реальные глобальные настройки хаоса, которые используются в игре
bool g_chaosEnabled = true;
int g_chanceZoom = 20;
int g_chanceSound = 50;
int g_chanceJump = 25;
int g_chanceFlash = 10;
int g_chanceFlip = 30;
int g_chanceWarp = 35;
int g_chanceRotate = 25;
int g_chanceFreeze = 5;

// Класс меню настроек хаоса (Popup) с кнопкой Save
// Класс меню настроек хаоса (Popup) с компактыми размерами и красивым расположением
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
        // Уменьшаем размер окна (было 440x320, делаем 380x260), чтобы крестик встал на место
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

            // Центр строки для элементов управления (сдвинут правее для названий)
            float centerX = m_size.width - 150.f;

            // Кнопка минус (ближе к цифре)
            auto minusSpr = ButtonSprite::create("-", 26, 26, 0.4f, true, "bigFont.fnt", "GJ_button_02.png");
            minusSpr->setScale(0.5f);
            auto minusBtn = CCMenuItemSpriteExtra::create(minusSpr, this, menu_selector(ChaosSettingsPopup::onMinus));
            minusBtn->setTag(tagMin);
            minusBtn->setPosition({centerX - 65.f, yPos});
            menu->addChild(minusBtn);

            // Поле ввода цифры
            outInput = TextInput::create(45.f, "0", "bigFont.fnt");
            outInput->setString(fmt::format("{}", valueRef));
            outInput->setCommonFilter(CommonFilter::Uint);
            outInput->setMaxCharCount(3);
            outInput->setScale(0.5f);
            outInput->setPosition({centerX - 20.f, yPos});
            
            outInput->setCallback([&valueRef](std::string const& text) {
                if (!text.empty()) {
                    int parsed = std::stoi(text);
                    valueRef = std::clamp(parsed, 0, 100);
                } else {
                    valueRef = 0;
                }
            });
            ml.addChild(outInput);

            // Знак % вплотную к цифре
            auto percentSign = CCLabelBMFont::create("%", "bigFont.fnt");
            percentSign->setScale(0.4f);
            percentSign->setAnchorPoint({0.0f, 0.5f});
            percentSign->setPosition({centerX + 8.f, yPos});
            ml.addChild(percentSign);

            // Кнопка плюс (ближе к % )
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

        // Аккуратная кнопка Save по центру внизу
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
        // Переносим временные значения в глобальные настройки хаоса
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

// Открытие по двойному нажатию P в Главном меню
class $modify(ChaosMenuLayer, MenuLayer) {
    struct Fields {
        bool lastPState = false;
        float pDoublePressTimer = 0.0f;
        int pPressCount = 0;
    };

    bool init() {
        if (!MenuLayer::init()) return false;
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

// Логика хаоса и обработка в игре
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
    };

    bool init(GJGameLevel* level, bool useTransactions, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useTransactions, dontCreateObjects)) return false;

        m_fields->blackScreen = CCLayerColor::create(ccc4(0, 0, 0, 255));
        m_fields->blackScreen->setOpacity(0);
        m_fields->blackScreen->setZOrder(99999);
        this->addChild(m_fields->blackScreen);

        m_fields->exclamationLabel = CCLabelBMFont::create("!", "goldFont.fnt");
        m_fields->exclamationLabel->setOpacity(0);
        m_fields->exclamationLabel->setScale(1.2f);
        m_fields->exclamationLabel->setZOrder(9998);
        
        if (m_objectLayer) {
            m_objectLayer->addChild(m_fields->exclamationLabel);
        } else {
            this->addChild(m_fields->exclamationLabel);
        }

        return true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (!m_player1 || m_hasCompletedLevel) return;

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
            m_fields->pDoublePressTimer -= dt;
            if (m_fields->pDoublePressTimer <= 0.0f) {
                m_fields->pPressCount = 0;
            }
        }

        bool currentShiftState = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        if (currentShiftState && !m_fields->lastShiftState) {
            m_fields->shiftPressCount++;
            m_fields->shiftResetTimer = 0.6f;

            if (m_fields->shiftPressCount >= 3) {
                m_fields->shiftPressCount = 0;
                g_chaosEnabled = !g_chaosEnabled;

                if (g_chaosEnabled) {
                    Notification::create("Chaos Mode: ON", NotificationIcon::Warning, 0.6f)->show();
                } else {
                    if (m_fields->blackScreen) {
                        m_fields->blackScreen->setOpacity(0);
                        m_fields->blackScreen->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
                    }
                    if (m_fields->exclamationLabel) {
                        m_fields->exclamationLabel->setOpacity(0);
                    }
                    this->setScale(1.0f);
                    this->setScaleX(1.0f);
                    this->setRotation(0.0f);
                    CCScheduler::get()->setTimeScale(1.0f);
                    m_fields->isHardFrozen = false;
                    m_fields->isZooming = false;
                    m_fields->isFlipped = false;
                    m_fields->isRotating = false;
                    
                    auto fmod = FMODAudioEngine::get();
                    if (fmod && fmod->m_backgroundMusicChannel) {
                        fmod->m_backgroundMusicChannel->setPaused(false);
                        fmod->m_backgroundMusicChannel->setPitch(1.0f);
                    }

                    Notification::create("Chaos Mode: OFF", NotificationIcon::Success, 0.6f)->show();
                }
            }
        }
        m_fields->lastShiftState = currentShiftState;

        if (m_fields->shiftResetTimer > 0.0f) {
            m_fields->shiftResetTimer -= dt;
            if (m_fields->shiftResetTimer <= 0.0f) {
                m_fields->shiftPressCount = 0;
            }
        }

        if (m_fields->exclamationTimer > 0.0f) {
            m_fields->exclamationTimer -= dt;
            if (m_player1 && m_fields->exclamationLabel) {
                m_fields->exclamationLabel->setPosition(m_player1->getPosition() + ccp(0.0f, 40.0f));
                float progress = m_fields->exclamationTimer / m_fields->exclamationMaxTime;
                m_fields->exclamationLabel->setOpacity(static_cast<GLubyte>(std::clamp(progress * 255.0f, 0.0f, 255.0f)));
            }
        }

        if (!g_chaosEnabled) return;

        if (m_fields->isHardFrozen) {
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<float> elapsed = now - m_fields->freezeStartTime;
            if (elapsed.count() >= m_fields->freezeDurationSeconds) {
                m_fields->isHardFrozen = false;
                CCScheduler::get()->setTimeScale(1.0f);
                
                if (m_fields->blackScreen) {
                    m_fields->blackScreen->setOpacity(0);
                    m_fields->blackScreen->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
                }

                auto fmod = FMODAudioEngine::get();
                if (fmod && fmod->m_backgroundMusicChannel) {
                    fmod->m_backgroundMusicChannel->setPaused(false);
                    fmod->m_backgroundMusicChannel->setPitch(1.0f);
                }
            }
            return;
        }

        if (m_fields->blackScreen && m_fields->blackScreen->getOpacity() > 0) {
            m_fields->blackScreenTimer -= dt;
            if (m_fields->blackScreenTimer <= 0.0f) {
                m_fields->blackScreen->setOpacity(0);
            }
        }

        if (m_fields->isFlipped) {
            m_fields->flipTimer -= dt;
            if (m_fields->flipTimer <= 0.0f) {
                this->setScaleX(1.0f);
                m_fields->isFlipped = false;
            }
        }

        if (m_fields->timeWarpTimer > 0.0f) {
            m_fields->timeWarpTimer -= dt;
            if (m_fields->timeWarpTimer <= 0.0f) {
                CCScheduler::get()->setTimeScale(1.0f);
                auto fmod = FMODAudioEngine::get();
                if (fmod && fmod->m_backgroundMusicChannel) {
                    fmod->m_backgroundMusicChannel->setPitch(1.0f);
                }
            }
        }

        if (m_fields->isRotating) {
            m_fields->rotationTimer -= dt;
            if (m_fields->rotationTimer <= 0.0f) {
                this->setRotation(0.0f);
                m_fields->isRotating = false;
            }
        }

        if (m_fields->isZooming) {
            m_fields->zoomTimer -= dt;
            if (m_fields->zoomTimer <= 0.0f) {
                this->setScale(1.0f);
                m_fields->isZooming = false;
            }
        }

        // События
        m_fields->freezeCooldown -= dt;
        if (m_fields->freezeCooldown <= 0.0f && (rand() % 100) < g_chanceFreeze) {
            m_fields->isHardFrozen = true;
            m_fields->freezeStartTime = std::chrono::steady_clock::now();
            m_fields->freezeDurationSeconds = 2.0f + static_cast<float>(rand() % 15) / 10.0f;
            CCScheduler::get()->setTimeScale(0.02f);
            
            if (m_fields->blackScreen) {
                m_fields->blackScreen->setBlendFunc({GL_DST_COLOR, GL_ZERO});
                m_fields->blackScreen->setColor({140, 140, 140});
                m_fields->blackScreen->setOpacity(255);
            }

            auto fmod = FMODAudioEngine::get();
            if (fmod && fmod->m_backgroundMusicChannel) {
                fmod->m_backgroundMusicChannel->setPaused(true);
            }
            m_fields->freezeCooldown = 10.0f + static_cast<float>(rand() % 10);
            return;
        }

        m_fields->jumpTimer -= dt;
        if (m_fields->jumpTimer <= 0.0f) {
            m_fields->jumpTimer = 0.6f + static_cast<float>(rand() % 20) / 10.0f;
            if ((rand() % 100) < g_chanceJump) {
                m_player1->pushButton(PlayerButton::Jump);
                m_player1->releaseButton(PlayerButton::Jump);
            }
        }

        m_fields->flashTimer -= dt;
        if (m_fields->flashTimer <= 0.0f) {
            m_fields->flashTimer = 1.2f + static_cast<float>(rand() % 30) / 10.0f;
            if ((rand() % 100) < g_chanceFlash) {
                if (m_fields->blackScreen) {
                    m_fields->blackScreen->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
                    m_fields->blackScreen->setColor({0, 0, 0});
                    m_fields->blackScreen->setOpacity(255);
                    m_fields->blackScreenTimer = 0.5f + static_cast<float>(rand() % 10) / 10.0f;
                }
            }
        }

        m_fields->soundTimer -= dt;
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

        m_fields->flipCooldown -= dt;
        if (m_fields->flipCooldown <= 0.0f && !m_fields->isFlipped) {
            m_fields->flipCooldown = 2.0f + static_cast<float>(rand() % 30) / 10.0f;
            if ((rand() % 100) < g_chanceFlip) {
                this->setScaleX(-1.0f);
                m_fields->isFlipped = true;
                m_fields->flipTimer = 1.5f + static_cast<float>(rand() % 15) / 10.0f;
            }
        }

        m_fields->timeWarpCooldown -= dt;
        if (m_fields->timeWarpCooldown <= 0.0f && m_fields->timeWarpTimer <= 0.0f) {
            m_fields->timeWarpCooldown = 3.0f + static_cast<float>(rand() % 40) / 10.0f;
            if ((rand() % 100) < g_chanceWarp) {
                float warpSpeeds[] = { 0.5f, 0.75f, 1.25f, 1.5f, 2.0f };
                float chosenSpeed = warpSpeeds[rand() % 5];
                
                CCScheduler::get()->setTimeScale(chosenSpeed);
                auto fmod = FMODAudioEngine::get();
                if (fmod && fmod->m_backgroundMusicChannel) {
                    fmod->m_backgroundMusicChannel->setPitch(chosenSpeed);
                }

                m_fields->timeWarpTimer = 1.5f + static_cast<float>(rand() % 15) / 10.0f;
            }
        }

        m_fields->rotationCooldown -= dt;
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

        m_fields->zoomCooldown -= dt;
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