// game.cpp - Game logic implementation with wave survival gameplay
#include "game.h"
#include "weapons.h"
#include "bots.h"
#include "score.h"
#include "powerups.h"
#include "specials.h"
#include "hud.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

Game::Game() {
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        keys[i] = false;
    }
}

Game::~Game() {
    shutdown();
}

bool Game::init(SDL_Window* window) {
    window_ = window;

    renderer_ = new Renderer();
    if (!renderer_->init(window, 1280, 720)) {
        fprintf(stderr, "Failed to initialize renderer\n");
        return false;
    }

    if (!text_.init()) {
        fprintf(stderr, "Failed to initialize text renderer\n");
        return false;
    }

    // Initialize particle system
    particleSystem = new ParticleSystem();
    particleSystem->init();

    spatialHash = new SpatialHash();
    g_spatialHash = spatialHash;

    // Initialize overclock system
    overclock = new OverclockManager();

    // Initialize echo system
    echoSystem = new EchoSystem();
    echoSystem->init();

    SDL_SetRelativeMouseMode(SDL_TRUE);
    loadHighScore(*this);
    setupArena();

    player.pos = Vec3(0, playerHeight, 0);
    player.health = maxHealth;
    player.alive = true;
    player.yaw = 0;
    player.pitch = 0;

    wave = 0;
    waveComplete = true;
    waveBreak = 0;
    updateMenuItems();

    return true;
}

void Game::shutdown() {
    if (!renderer_ && !particleSystem && !spatialHash && !overclock && !echoSystem) return;
    if (spatialHash) {
        delete spatialHash;
        spatialHash = nullptr;
        g_spatialHash = nullptr;
    }
    if (particleSystem) {
        delete particleSystem;
        particleSystem = nullptr;
    }
    if (overclock) {
        delete overclock;
        overclock = nullptr;
    }
    if (echoSystem) {
        delete echoSystem;
        echoSystem = nullptr;
    }
    
    if (renderer_) {
        renderer_->shutdown();
        delete renderer_;
        renderer_ = nullptr;
    }
}

void Game::setupArena() {
    bots.clear();
    projectiles.clear();
}

void Game::run() {
    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTime) / 1000.0f;
        lastTime = now;

        if (dt > 0.1f) dt = 0.1f;

        handleInput(dt);
        update(dt);
        render();

        Uint32 frameTime = SDL_GetTicks() - now;
        if (frameTime < 16) {
            SDL_Delay(16 - frameTime);
        }

        gameTime += dt;
    }
}

void Game::handleInput(float dt) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            if (state == GameState::PLAYING || state == GameState::PAUSED) {
                SavegameManager::save(*this);
            }
            running = false;
        } else if (event.type == SDL_KEYDOWN) {
            keys[event.key.keysym.scancode] = true;

            if (state == GameState::MENU) {
                handleMenuInput(event);
            } else if (state == GameState::PAUSED) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::PLAYING;
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                } else if (event.key.keysym.sym == SDLK_q) {
                    state = GameState::MENU;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    updateMenuItems();
                }
            } else if (state == GameState::PLAYING) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::PAUSED;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    SavegameManager::save(*this);
                }
                if (showUpgradeMenu) {
                    handleUpgradeInput(*this, event);
                } else if (event.key.keysym.sym == SDLK_SPACE && waveComplete) {
                    nextWave();
                }
                if (loadout == Loadout::GHOST) {
                    if (event.key.keysym.sym == SDLK_1 || event.key.keysym.sym == SDLK_4) {
                        currentWeapon = WeaponType::GHOST_SNIPER;
                    }
                    if (event.key.keysym.sym == SDLK_g) {
                        activateScannerSweep(*this);
                    }
                    if (event.key.keysym.sym == SDLK_h) {
                        activateEMPBlast(*this);
                    }
                    if (event.key.keysym.sym == SDLK_i || event.key.keysym.sym == SDLK_n) {
                        activateTacNuke(*this);
                    }
                    if (event.key.keysym.sym == SDLK_j) {
                        activateCloak(*this);
                    }
                } else {
                    if (event.key.keysym.sym == SDLK_1) {
                        currentWeapon = WeaponType::RAILGUN;
                    }
                    if (event.key.keysym.sym == SDLK_2) {
                        currentWeapon = WeaponType::LIGHTNING_GUN;
                    }
                    if (event.key.keysym.sym == SDLK_3) {
                        currentWeapon = WeaponType::PLASMA_RIFLE;
                    }
                    if (event.key.keysym.sym == SDLK_e) {
                        activateNuclearBlast(*this);
                    }
                    if (event.key.keysym.sym == SDLK_r) {
                        activateTimeSlow(*this);
                    }
                    if (event.key.keysym.sym == SDLK_f) {
                        activateShield(*this);
                    }
                    if (event.key.keysym.sym == SDLK_q) {
                        currentWeapon = WeaponType::LIGHTNING_GUN;
                    }
                }
            } else if (state == GameState::OPTIONS) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::MENU;
                }
                if (event.key.keysym.sym == SDLK_LEFT) {
                    if (menuSelection == 0) {
                        mouseSensitivity -= 0.0005f;
                        if (mouseSensitivity < 0.0005f) mouseSensitivity = 0.0005f;
                    }
                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    if (menuSelection == 0) {
                        mouseSensitivity += 0.0005f;
                        if (mouseSensitivity > 0.005f) mouseSensitivity = 0.005f;
                    }
                }
                if (event.key.keysym.sym == SDLK_UP) {
                    menuSelection--;
                    if (menuSelection < 0) menuSelection = 2;
                }
                if (event.key.keysym.sym == SDLK_DOWN) {
                    menuSelection++;
                    if (menuSelection > 2) menuSelection = 0;
                }
            } else if (state == GameState::GAME_OVER) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    startNewRun(loadout);
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::MENU;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    updateMenuItems();
                }
            }

        } else if (event.type == SDL_KEYUP) {
            keys[event.key.keysym.scancode] = false;
        } else if (event.type == SDL_MOUSEMOTION) {
            mouseX = event.motion.xrel;
            mouseY = event.motion.yrel;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                shootRequested = true;
            }
            if (event.button.button == SDL_BUTTON_RIGHT) {
                if (loadout == Loadout::GHOST) adsHeld = true;
                else shootLightning = true;
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                shootRequested = false;
            }
            if (event.button.button == SDL_BUTTON_RIGHT) {
                shootLightning = false;
                adsHeld = false;
            }
        }
    }
}

void Game::updateMenuItems() {
    menuItems.clear();
    if (SavegameManager::exists()) {
        menuItems.push_back("Continue");
        menuItems.push_back("New Arena");
        menuItems.push_back("New Ghost");
        menuItems.push_back("Sensitivity");
        menuItems.push_back("Quit");
    } else {
        menuItems.push_back("Start Arena");
        menuItems.push_back("Start Ghost");
        menuItems.push_back("Sensitivity");
        menuItems.push_back("Quit");
    }
}

void Game::startNewRun(Loadout kit) {
    resetGame();
    loadout = kit;
    adsHeld = false;
    if (kit == Loadout::GHOST) {
        currentWeapon = WeaponType::GHOST_SNIPER;
        ghostEnergy = GhostRules::ENERGY_START;
    } else {
        currentWeapon = WeaponType::RAILGUN;
        ghostEnergy = 0.0f;
    }
    state = GameState::PLAYING;
    SDL_SetRelativeMouseMode(SDL_TRUE);
    nextWave();
}

void Game::handleMenuInput(SDL_Event& event) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
        menuSelection--;
        if (menuSelection < 0) menuSelection = (int)menuItems.size() - 1;
    } else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
        menuSelection++;
        if (menuSelection >= (int)menuItems.size()) menuSelection = 0;
    } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
        bool hasSave = SavegameManager::exists();
        int continueIdx = hasSave ? 0 : -1;
        int arenaIdx = hasSave ? 1 : 0;
        int ghostIdx = hasSave ? 2 : 1;
        int optionsIdx = hasSave ? 3 : 2;
        int quitIdx = hasSave ? 4 : 3;

        if (menuSelection == continueIdx && continueIdx >= 0) {
            if (SavegameManager::load(*this)) {
                loadout = (currentWeapon == WeaponType::GHOST_SNIPER)
                    ? Loadout::GHOST : Loadout::ARENA;
                adsHeld = false;
                state = GameState::PLAYING;
                SDL_SetRelativeMouseMode(SDL_TRUE);
            }
        } else if (menuSelection == arenaIdx) {
            if (hasSave) SavegameManager::remove();
            startNewRun(Loadout::ARENA);
        } else if (menuSelection == ghostIdx) {
            if (hasSave) SavegameManager::remove();
            startNewRun(Loadout::GHOST);
        } else if (menuSelection == optionsIdx) {
            state = GameState::OPTIONS;
        } else if (menuSelection == quitIdx) {
            running = false;
        }
    }
}

void Game::resetGame() {
    gameOver = false;
    wave = 0;
    score = 0;
    kills = 0;
    killStreak = 0;
    player.health = maxHealth;
    player.alive = true;
    player.pos = Vec3(0, playerHeight, 0);
    waveComplete = true;
    waveBreak = 0;
    projectiles.clear();
    particles.clear();
    resetUpgrades(*this);
    if (overclock) overclock->reset();
    if (echoSystem) echoSystem->reset();
    // Reset ghost mode
    loadout = Loadout::ARENA;
    currentWeapon = WeaponType::RAILGUN;
    adsHeld = false;
    ghostEnergy = 0.0f;
    ghostCooldown = 0.0f;
    ghostKills = 0;
    ghostComboCount = 0;
    ghostComboTimer = 0.0f;
    ghostAmbushActive = false;
    ghostAmbushTimer = 0.0f;
    cloakTimer = 0.0f;
    cloakCooldown = 0.0f;
    scannerCooldown = 0.0f;
    empCooldown = 0.0f;
    nukeCooldown = 0.0f;
    scannerTimer = 0.0f;
    empTimer = 0.0f;
    nukePaintTimer = 0.0f;
    nukeInboundTimer = 0.0f;
    nukeFlashTimer = 0.0f;
    detectorSwarmTimer = 0.0f;
    comboBonusDamage = 1.0f;
    lastKnownPlayerX = 0.0f;
    lastKnownPlayerZ = 0.0f;
    for (auto& b : bots) {
        b.ghostMarked = 0;
        b.ghostMarkTimer = 0.0f;
    }
    // Reset bug effects
    railgunFeedbackChance = 0.0f;
    plasmaOverheatPenalty = 0.0f;
    lightningBacklashChance = 0.0f;
    shieldCrashChance = 0.0f;
    splitterVirusLevel = 0;
    splitterFriendlyFire = false;
    scoreMultiplierFloat = 1.0f;
    scoreDecayRate = 0.0f;
    phaseGlitchChance = 0.0f;
    phaseShiftKills = 0;
    phaseShiftTimer = 0.0f;
    if (g_music) g_music->playScene(MusicScene::MENU);
}

void Game::update(float dt) {
    if (state == GameState::MENU) {
        if (g_music) g_music->playScene(MusicScene::MENU);
        updateMenu(dt);
        return;
    }

    if (state == GameState::PAUSED) {
        return;
    }

    if (state == GameState::GAME_OVER) {
        return;
    }

    if (gameOver) {
        state = GameState::GAME_OVER;
        if (g_music) g_music->playScene(MusicScene::GAME_OVER);
        if (echoSystem) {
            echoSystem->stopRecording();
            echoSystem->play();
        }
        SavegameManager::remove();
        return;
    }

    if (waveComplete) {
        waveBreak += dt;
        if (waveBreak >= nextWaveDelay) {
            nextWave();
        }
        return;
    }

    // Update coop (player 2 input, revive system)
    if (coopActive) {
        CoopManager::update(dt);
        CoopManager::updatePlayer2(*this, dt);
    }

    updatePlayer(dt);
    updateBots(*this, dt);
    updateProjectiles(dt);
    if (particleSystem) particleSystem->update(dt);

    // Update overclock
    if (overclock) {
        // Score decay
        if (scoreDecayRate > 0.0f) {
            score -= (int)(scoreDecayRate * dt * 60.0f); // per second basis
        }
        // Phase shift timer
        if (phaseShiftTimer > 0.0f) phaseShiftTimer -= dt;

        // Update echo system
        if (echoSystem) {
            echoSystem->update(dt);

            // Record player state for echo
            if (echoSystem->isRecording()) {
                echoSystem->recordFrame(
                    player.pos.x, player.pos.y, player.pos.z,
                    player.vx, player.vy, player.vz,
                    player.yaw, player.pitch, gameTime
                );
            }

            // Check player boost (if player is near echo ghost)
            echoSystem->checkPlayerBoost(player.pos.x, player.pos.y, player.pos.z, playerSpeed);

            // Check bot collisions with echo
            for (int i = 0; i < (int)bots.size(); i++) {
                if (bots[i].alive) {
                    echoSystem->checkBotCollision(
                        bots[i].pos.x, bots[i].pos.y, bots[i].pos.z, i
                    );
                }
            }

            // Apply boost decay
            if (echoSystem->isBoostActive()) {
                // Boost is active - speed is already multiplied
            } else if (playerSpeed > 10.0f && !echoSystem->isBoostActive()) {
                // Boost expired - reset speed
                playerSpeed = 10.0f;
            }
        }
    }
    updateWeapons(dt, *this);
    updatePowerUps(dt, *this);
    updateScore(dt, *this);
    updateSpecials(dt, *this);
    updateKillFeed(dt, *this);
    updateDamageNumbers(dt, *this);

    // Ghost energy regen (kit only)
    if (loadout == Loadout::GHOST) {
        ghostEnergy = GhostRules::addEnergy(ghostEnergy, GhostRules::ENERGY_REGEN * dt);
        if (cloakTimer <= 0.0f) {
            lastKnownPlayerX = player.pos.x;
            lastKnownPlayerZ = player.pos.z;
        }
    }

    // Rebuild spatial hash for collision detection
    if (spatialHash) {
        spatialHash->clear();
        for (int i = 0; i < (int)bots.size(); i++) {
            if (bots[i].alive) {
                spatialHash->insert(i, bots[i].pos.x, bots[i].pos.z);
            }
        }
    }
    
    checkCollisions();
    
    // Count alive bots for audio
    int aliveBots = 0;
    bool bossActive = false;
    for (const auto& bot : bots) {
        if (bot.alive) {
            aliveBots++;
            if (bot.botType == 4) bossActive = true;
        }
    }
    
    // Audio polish: dynamic layers, reverb, occlusion
    if (g_audio) {
        AudioPolish::update(dt, aliveBots, wave,
            bossActive,
            arenaSize > 60.0f);
        AudioPolish::setListenerPosition(player.pos.x, player.pos.z);
    }
    
    // Update camera shake
    if (shakeAmount > 0.0f) {
        shakeOffset.x = (rand() % 100 / 100.0f - 0.5f) * shakeAmount;
        shakeOffset.z = (rand() % 100 / 100.0f - 0.5f) * shakeAmount;
        shakeAmount -= shakeDecay * dt;
        if (shakeAmount < 0.0f) {
            shakeAmount = 0.0f;
            shakeOffset = Vec3(0,0,0);
        }
    }

    // Decay HUD timers
    if (hitFeedbackTimer > 0.0f) { hitFeedbackTimer -= dt; if (hitFeedbackTimer < 0.0f) hitFeedbackTimer = 0.0f; }
    if (waveAnnounceTimer > 0.0f) { waveAnnounceTimer -= dt; if (waveAnnounceTimer < 0.0f) waveAnnounceTimer = 0.0f; }

    // Achievement popup queue processing
    if (achievementPopupTimer <= 0.0f && !pendingAchievements.empty()) {
        AchievementSystem::ID id = pendingAchievements.front();
        pendingAchievements.erase(pendingAchievements.begin());
        const auto& ach = AchievementSystem::getAchievement(id);
        achievementPopupText = ach.name;
        achievementPopupDesc = ach.description;
        achievementPopupColor = Vec3(0.0f, 1.0f, 0.8f);
        achievementPopupTimer = 3.0f;
        if (g_audio) g_audio->playAchievement();
    }

    // Poll newly unlocked achievements
    AchievementSystem::ID newlyUnlocked[8];
    int newCount = AchievementSystem::consumeNewlyUnlocked(newlyUnlocked, 8);
    for (int i = 0; i < newCount; i++) {
        pendingAchievements.push_back(newlyUnlocked[i]);
    }
    
    // Decay post-processing effects
    if (renderer_) {
        // Hit flash decay
        float hitFlash = renderer_->hitFlashIntensity;
        if (hitFlash > 0.0f) {
            hitFlash -= dt * 3.0f;
            if (hitFlash < 0.0f) hitFlash = 0.0f;
            renderer_->setHitFlash(hitFlash);
        }
        
        // Chromatic aberration decay
        float ca = renderer_->chromaticAberrationAmount;
        if (ca > 0.0f) {
            ca -= dt * 4.0f;
            if (ca < 0.0f) ca = 0.0f;
            renderer_->setChromaticAberration(ca);
        }
        
        // Game over vignette
        if (gameOver) {
            float gov = renderer_->gameOverVignette;
            gov += dt * 0.5f;
            if (gov > 1.0f) gov = 1.0f;
            renderer_->setGameOverVignette(gov);
        }
    }

    // Check wave complete
    bool anyAlive = false;
    for (auto& bot : bots) {
        if (bot.alive) {
            anyAlive = true;
            break;
        }
    }
    if (!anyAlive && bots.size() > 0) {
        waveComplete = true;
        waveBreak = 0;
        killStreak = 0;
        if (echoSystem) echoSystem->stopRecording();
        score += wave * 100;
        upgradePoints += 1 + wave / 3;
        showUpgradeMenu = true;
        // Achievement: wave check
        AchievementSystem::checkWaveAchievements(achievementProgress, wave, gameTime, tookDamageThisWave);
        if (wave % 5 == 0) {
            upgradePoints += 3;
            printf("BOSS KILLED! Wave %d cleared! Score: %d\n", wave, score);
        } else {
            printf("Wave %d cleared! Score: %d\n", wave, score);
        }
    }

    // Check player death
    if (player.health <= 0) {
        if (coopActive) {
            // In coop: game over only if both players dead
            if (player2.health <= 0) {
                gameOver = true;
                saveHighScore(*this);
                printf("Game Over! Waves survived: %d, Kills: %d/%d\n", wave - 1, kills, player2Kills);
            } else {
                // Player 1 down, player 2 can revive
                player.alive = false;
                if (!CoopManager::getRevive().playerDown) {
                    CoopManager::getRevive().playerDown = true;
                    printf("Player 1 DOWN! Player 2 has 5 seconds to revive!\n");
                }
            }
        } else {
            gameOver = true;
            saveHighScore(*this);
            printf("Game Over! Waves survived: %d, Kills: %d\n", wave - 1, kills);
        }
    }
    
    // Check player 2 death
    if (coopActive && player2.health <= 0) {
        if (player.health <= 0) {
            gameOver = true;
            saveHighScore(*this);
            printf("Game Over! Waves survived: %d, Kills: %d/%d\n", wave - 1, kills, player2Kills);
        } else {
            player2.alive = false;
            if (!CoopManager::getRevive().playerDown) {
                CoopManager::getRevive().playerDown = true;
                printf("Player 2 DOWN! Player 1 has 5 seconds to revive!\n");
            }
        }
    }
    
    // Revive check: if revive timer expires, player stays down
    if (coopActive && CoopManager::getRevive().playerDown && !CoopManager::getRevive().reviving) {
        // Check if other player is close enough to revive
        float dist = distance(player.pos, player2.pos);
        if (dist < CoopManager::getRevive().reviveRange) {
            CoopManager::startRevive();
        }
    }
    
    // Complete revive
    if (coopActive && CoopManager::getRevive().reviving && CoopManager::getRevive().reviveTimer <= 0.0f) {
        if (!player.alive) {
            CoopManager::completeRevive(*this);
            printf("Player 1 REVIVED!\n");
        } else if (!player2.alive) {
            player2.alive = true;
            player2.health = maxHealth * 0.5f;
            player2.pos = player.pos + Vec3(2.0f, 0, 2.0f);
            CoopManager::getRevive().playerDown = false;
            printf("Player 2 REVIVED!\n");
        }
    }
}

void Game::updatePlayer(float dt) {
    handleMouse();

    float moveX = 0, moveZ = 0;
    if (keys[SDL_SCANCODE_W]) moveZ -= 1;
    if (keys[SDL_SCANCODE_S]) moveZ += 1;
    if (keys[SDL_SCANCODE_A]) moveX -= 1;
    if (keys[SDL_SCANCODE_D]) moveX += 1;

    float speedMult = 1.0f;
    if (cloakTimer > 0.0f) speedMult = cloakSpeedBoost;

    if (moveX != 0 || moveZ != 0) {
        Vec3 moveDir = normalize(Vec3(moveX, 0, moveZ));
        player.pos.x += moveDir.x * playerSpeed * dt * speedMult;
        player.pos.z += moveDir.z * playerSpeed * dt * speedMult;
    }

    player.pos.y = playerHeight;

    if (player.pos.x < -arenaSize) player.pos.x = -arenaSize;
    if (player.pos.x > arenaSize) player.pos.x = arenaSize;
    if (player.pos.z < -arenaSize) player.pos.z = -arenaSize;
    if (player.pos.z > arenaSize) player.pos.z = arenaSize;

    if (shootRequested && currentWeapon == WeaponType::RAILGUN) {
        fireRailgun(*this);
    }
    if (shootLightning && currentWeapon == WeaponType::LIGHTNING_GUN) {
        fireLightning(*this);
    }
    if (shootRequested && currentWeapon == WeaponType::PLASMA_RIFLE) {
        firePlasma(*this);
    }
    if (shootRequested && currentWeapon == WeaponType::GHOST_SNIPER && loadout == Loadout::GHOST) {
        fireGhost(*this);
    }

    // Player 2 shooting (coop)
    if (coopActive && player2Shoot && player2ShootCooldown <= 0.0f) {
        // Player 2 fires railgun in facing direction
        Vec3 dir(
            sinf(player2.yaw),
            0.0f,
            -cosf(player2.yaw)
        );
        Vec3 muzzlePos = player2.pos + Vec3(0, 1.0f, 0);
        projectiles.push_back(Projectile(muzzlePos, dir, true, WeaponType::RAILGUN, 15.0f));
        player2ShootCooldown = 0.3f;
    }
    player2ShootCooldown -= dt;
    if (player2ShootCooldown < 0.0f) player2ShootCooldown = 0.0f;
}

void Game::updateProjectiles(float dt) {
    for (auto& proj : projectiles) {
        proj.pos = proj.pos + proj.dir * proj.speed * dt;
        proj.life -= dt;
    }

    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const Projectile& p) { return p.life <= 0; }),
        projectiles.end()
    );
}

void Game::checkCollisions() {
    // Check power-up collection
    for (int i = (int)powerUps.size() - 1; i >= 0; i--) {
        if (distance(player.pos, powerUps[i].pos) < 2.0f) {
            collectPowerUp(*this, i);
        }
    }
    // Check projectile hits
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        bool hit = false;

        if (it->fromPlayer) {
            for (auto& bot : bots) {
                if (!bot.alive) continue;
                if (distance(it->pos, bot.pos) < 1.5f) {
                    bot.health -= it->damage;
                    if (bot.health <= 0) {
                        bool ambush = cloakTimer > 0.0f || ghostAmbushActive;
                        registerBotKill(*this, bot, it->weapon, ambush);
                    }
                    hit = true;
                    break;
                }
            }
        } else {
            if (distance(it->pos, player.pos) < 1.0f) {
                notifyPlayerHit(*this, it->damage);
                hit = true;
            }
        }

        if (hit) {
            it = projectiles.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::handleMouse() {
    if (fabsf(mouseX) > 80.0f) mouseX = 0.0f;
    if (fabsf(mouseY) > 80.0f) mouseY = 0.0f;
    if (mouseX != 0 || mouseY != 0) {
        player.yaw += mouseX * mouseSensitivity;
        player.pitch -= mouseY * mouseSensitivity;
        player.pitch = fmaxf(-1.2f, fminf(1.2f, player.pitch));
    }
    mouseX = 0;
    mouseY = 0;
}

void Game::nextWave() {
    waveComplete = false;
    waveBreak = 0;
    tookDamageThisWave = false;
    spawnWave(*this);
    waveAnnounceTimer = 2.0f;
    if (echoSystem) echoSystem->startRecording();

    // Detect wave fusion
    WaveConfig config = generateWaveConfig(wave);
    currentFusion = detectFusion(config.modifiers);
    const FusionEffect* fusion = getFusionEffect(currentFusion);
    if (currentFusion != WaveFusion::NONE) {
        printf("⚡ FUSION: %s — %s\n", fusion->name, fusion->description);
        fusionDisplayTimer = 3.0f;
        if (fusion->arenaShrinkRate > 0) {
            printf("⚠️ ARENA SHRINKING!\n");
        }
    }

    if (g_music && wave % 5 != 0) g_music->playScene(MusicScene::GAMEPLAY);
}

void Game::render() {
    if (state == GameState::MENU) {
        renderMenu();
        return;
    }

    if (state == GameState::OPTIONS) {
        renderOptions();
        return;
    }

    if (showUpgradeMenu && waveComplete) {
        renderer_->beginFrame();
        renderer_->clear(0.02f, 0.02f, 0.05f, 1.0f);

        Vec3 forward(
            sinf(player.yaw) * cosf(player.pitch),
            -sinf(player.pitch),
            -cosf(player.yaw) * cosf(player.pitch)
        );
        Vec3 eye = player.pos;
        Vec3 center = eye + forward;
        Vec3 up(0, 1, 0);

        Mat4 view = Mat4::lookAt(eye, center, up);
        float aspect = (float)renderer_->getWidth() / renderer_->getHeight();
        Mat4 proj = Mat4::perspective(1.1f, aspect, 0.1f, 200.0f);

        renderer_->setView(view);
        renderer_->setProjection(proj);
        renderer_->setViewPos(eye);

        renderArena();
        renderBots(*this);
        renderProjectiles();
        renderParticles();
        renderLightning(*this);

        renderer_->endFrame();
        renderer_->beginOverlay();

        renderUpgradeMenu(*this);
        return;
    }

    if (state == GameState::PAUSED) {
        renderPauseMenu();
        return;
    }

    if (state == GameState::GAME_OVER) {
        renderGameOver();
        return;
    }

    Vec3 forward(
        sinf(player.yaw) * cosf(player.pitch),
        -sinf(player.pitch),
        -cosf(player.yaw) * cosf(player.pitch)
    );
    
    Vec3 eye, center;
    float fov;
    
    if (coopActive) {
        // Shared screen camera: center between both players
        CoopManager::calculateSharedCamera(eye, center, fov,
            player.pos, player2.pos, arenaSize,
            (float)renderer_->getWidth() / renderer_->getHeight());
        // Adjust for shake
        eye = eye + Vec3(shakeOffset.x, shakeOffset.y, shakeOffset.z);
    } else {
        eye = player.pos + shakeOffset;
        center = eye + forward;
        fov = (loadout == Loadout::GHOST && adsHeld)
            ? GhostRules::ADS_FOV
            : GhostRules::HIP_FOV;
    }
    
    Vec3 up(0, 1, 0);
    Mat4 view = Mat4::lookAt(eye, center, up);
    float aspect = (float)renderer_->getWidth() / renderer_->getHeight();
    Mat4 proj = Mat4::perspective(fov, aspect, 0.1f, 200.0f);
    
    renderer_->setView(view);
    renderer_->setProjection(proj);
    renderer_->setViewPos(eye);
    
    renderer_->setBloomIntensity(coopActive ? 1.2f : 0.8f);

    renderer_->beginFrame();
    renderer_->clear(0.02f, 0.02f, 0.05f, 1.0f);

    renderArena();
    renderBots(*this);
    renderProjectiles();
    renderParticles();
    renderLightning(*this);
    renderPowerUps(*this);
    renderSpecialEffects(*this);

    renderer_->endFrame();
    renderer_->beginOverlay();

    renderHUD(*this);

    // Achievement popup
    if (achievementPopupTimer > 0.0f) {
        float popupY = 0.3f;
        float alpha = fminf(1.0f, achievementPopupTimer / 0.5f);
        if (achievementPopupTimer > 2.5f) alpha = fminf(1.0f, (3.0f - achievementPopupTimer) / 0.5f);
        Vec3 popupColor = achievementPopupColor * alpha;
        text_.drawTextCentered("ACHIEVEMENT UNLOCKED", popupY, 0.8f, popupColor);
        text_.drawTextCentered(achievementPopupText, popupY - 0.06f, 1.0f, popupColor);
        text_.drawTextCentered(achievementPopupDesc, popupY - 0.12f, 0.7f, popupColor * 0.7f);
        achievementPopupTimer -= 0.016f;
    }

    SDL_GL_SwapWindow(window_);
}

void Game::renderParticles() {
    if (!particleSystem) return;
    int count;
    const float* data = particleSystem->getRenderBuffer(count);
    if (count > 0) {
        renderer_->drawParticlesECS(data, count);
    }
}

void Game::renderArena() {
    renderer_->drawGround(gameTime);

    Vec3 wallColor(0.0f, 0.8f, 1.0f);
    float h = 3.0f;
    Vertex w1[] = {
        Vertex(Vec3(-arenaSize, 0, -arenaSize), wallColor),
        Vertex(Vec3(arenaSize, 0, -arenaSize), wallColor),
        Vertex(Vec3(arenaSize, h, -arenaSize), wallColor),
        Vertex(Vec3(-arenaSize, h, -arenaSize), wallColor)
    };
    renderer_->drawLineLoop(w1, 4, wallColor);

    Vertex w2[] = {
        Vertex(Vec3(arenaSize, 0, -arenaSize), wallColor),
        Vertex(Vec3(arenaSize, 0, arenaSize), wallColor),
        Vertex(Vec3(arenaSize, h, arenaSize), wallColor),
        Vertex(Vec3(arenaSize, h, -arenaSize), wallColor)
    };
    renderer_->drawLineLoop(w2, 4, wallColor);

    Vertex w3[] = {
        Vertex(Vec3(arenaSize, 0, arenaSize), wallColor),
        Vertex(Vec3(-arenaSize, 0, arenaSize), wallColor),
        Vertex(Vec3(-arenaSize, h, arenaSize), wallColor),
        Vertex(Vec3(arenaSize, h, arenaSize), wallColor)
    };
    renderer_->drawLineLoop(w3, 4, wallColor);

    Vertex w4[] = {
        Vertex(Vec3(-arenaSize, 0, arenaSize), wallColor),
        Vertex(Vec3(-arenaSize, 0, -arenaSize), wallColor),
        Vertex(Vec3(-arenaSize, h, -arenaSize), wallColor),
        Vertex(Vec3(-arenaSize, h, arenaSize), wallColor)
    };
    renderer_->drawLineLoop(w4, 4, wallColor);
}

void Game::renderProjectiles() {
    for (auto& proj : projectiles) {
        Vec3 projColor = proj.fromPlayer ? Vec3(0.0f, 1.0f, 1.0f) : Vec3(1.0f, 0.3f, 0.0f);
        float s = 0.5f;

        Vertex line[] = {
            Vertex(proj.pos - proj.dir * s * 2.0f, projColor * 0.3f),
            Vertex(proj.pos, projColor),
        };
        renderer_->drawLineLoop(line, 2, projColor);
    }
}

Vec3 Game::normalize(Vec3 v) {
    float len = v.length();
    return len > 0 ? Vec3(v.x / len, v.y / len, v.z / len) : Vec3();
}

float Game::distance(Vec3 a, Vec3 b) {
    return (a - b).length();
}
