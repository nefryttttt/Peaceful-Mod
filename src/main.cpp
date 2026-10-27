#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <random>
#include <chrono>

using namespace geode::prelude;

bool g_chaosEnabled = true;

class $modify(ChaosPlayLayer, PlayLayer) {
    struct Fields {
        float timer = 0.0f;
        float nextEventTime = 2.0f;
        
        CCLayerColor* blackScreen = nullptr;
        float blackScreenTimer = 0.0f;
        
        bool isFlipped = false;
        float flipTimer = 0.0f;
        
        float timeWarpTimer = 0.0f;

        bool isRotating = false;
        float rotationTimer = 0.0f;

        bool isHardFrozen = false;
        std::chrono::steady_clock::time_point freezeStartTime;
        float freezeDurationSeconds = 3.0f;

        CCLabelBMFont* exclamationLabel = nullptr;
        float exclamationTimer = 0.0f;
        float exclamationMaxTime = 0.7f;

        bool lastShiftState = false;
        int shiftPressCount = 0;
        float shiftResetTimer = 0.0f;
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

        m_fields->timer = 0.0f;
        m_fields->nextEventTime = 2.0f + static_cast<float>(rand() % 20) / 10.0f;

        return true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!m_player1 || m_hasCompletedLevel) return;
    
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
                    this->setScaleX(1.0f);
                    this->setRotation(0.0f);
                    CCScheduler::get()->setTimeScale(1.0f);
                    m_fields->isHardFrozen = false;
                    
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

        if (m_fields->blackScreen && m_fields->blackScreen->getOpacity() > 0 && !m_fields->isHardFrozen) {
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

        m_fields->timer += dt;

        if (m_fields->timer >= m_fields->nextEventTime) {
            m_fields->timer = 0.0f;
            m_fields->nextEventTime = 2.0f + static_cast<float>(rand() % 20) / 10.0f;

            if ((rand() % 100) < 7 && !m_fields->isHardFrozen) {
                m_fields->isHardFrozen = true;
                m_fields->freezeStartTime = std::chrono::steady_clock::now();
                m_fields->freezeDurationSeconds = 3.0f + static_cast<float>(rand() % 15) / 10.0f;
                
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
                return;
            }

            if ((rand() % 100) < 40) {
                m_player1->pushButton(PlayerButton::Jump);
                m_player1->releaseButton(PlayerButton::Jump);
            }

            if ((rand() % 100) < 12) {
                if (m_fields->blackScreen) {
                    m_fields->blackScreen->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
                    m_fields->blackScreen->setColor({0, 0, 0});
                    m_fields->blackScreen->setOpacity(255);
                    m_fields->blackScreenTimer = 0.8f + static_cast<float>(rand() % 10) / 10.0f;
                }
            }

            if ((rand() % 100) < 50) {
                auto fmod = FMODAudioEngine::get();
                if (fmod) {
                    fmod->playEffectAsync("secretKey.ogg");
                }
                if (m_fields->exclamationLabel && m_player1) {
                    m_fields->exclamationLabel->setPosition(m_player1->getPosition() + ccp(0.0f, 40.0f));
                    m_fields->exclamationLabel->setOpacity(255);
                    m_fields->exclamationMaxTime = 0.7f;
                    m_fields->exclamationTimer = 0.7f;
                }
            }

            if ((rand() % 100) < 20 && !m_fields->isFlipped) {
                this->setScaleX(-1.0f);
                m_fields->isFlipped = true;
                m_fields->flipTimer = 1.5f + static_cast<float>(rand() % 15) / 10.0f;
            }

            if ((rand() % 100) < 30 && m_fields->timeWarpTimer <= 0.0f) {
                float warpSpeeds[] = { 0.5f, 0.75f, 1.25f, 1.5f, 2.0f };
                float chosenSpeed = warpSpeeds[rand() % 5];
                
                CCScheduler::get()->setTimeScale(chosenSpeed);
                
                auto fmod = FMODAudioEngine::get();
                if (fmod && fmod->m_backgroundMusicChannel) {
                    fmod->m_backgroundMusicChannel->setPitch(chosenSpeed);
                }

                m_fields->timeWarpTimer = 1.5f + static_cast<float>(rand() % 15) / 10.0f;
            }

            if ((rand() % 100) < 15 && !m_fields->isRotating) {
                float angles[] = { 90.0f, 180.0f, 270.0f, 360.0f, -90.0f, -180.0f };
                float chosenAngle = angles[rand() % 6];
                this->setRotation(chosenAngle);
                m_fields->isRotating = true;
                m_fields->rotationTimer = 1.5f + static_cast<float>(rand() % 15) / 10.0f;
            }
        }
    }
};