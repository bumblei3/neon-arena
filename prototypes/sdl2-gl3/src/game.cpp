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

    return true;
}

void Game::shutdown() {
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
            running = false;
        } else if (event.type == SDL_KEYDOWN) {
            keys[event.key.keysym.scancode] = true;

            if (state == GameState::MENU) {
                handleMenuInput(event);
            } else if (state == GameState::PAUSED) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::PLAYING;
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                }
            } else if (state == GameState::PLAYING) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::PAUSED;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                }
                if (showUpgradeMenu) {
                    handleUpgradeInput(*this, event);
                } else if (event.key.keysym.sym == SDLK_SPACE && waveComplete) {
                    nextWave();
                }
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
                    if (currentWeapon == WeaponType::RAILGUN) {
                        currentWeapon = WeaponType::LIGHTNING_GUN;
                    } else if (currentWeapon == WeaponType::LIGHTNING_GUN) {
                        currentWeapon = WeaponType::PLASMA_RIFLE;
                    } else {
                        currentWeapon = WeaponType::RAILGUN;
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
                    resetGame();
                    state = GameState::PLAYING;
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    state = GameState::MENU;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
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
                shootLightning = true;
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                shootRequested = false;
            }
            if (event.button.button == SDL_BUTTON_RIGHT) {
                shootLightning = false;
            }
        }
    }
}

void Game::handleMenuInput(SDL_Event& event) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
        menuSelection--;
        if (menuSelection < 0) menuSelection = (int)menuItems.size() - 1;
    } else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
        menuSelection++;
        if (menuSelection >= (int)menuItems.size()) menuSelection = 0;
    } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
        if (menuSelection == 0) {
            resetGame();
            state = GameState::PLAYING;
            SDL_SetRelativeMouseMode(SDL_TRUE);
        } else if (menuSelection == 1) {
            state = GameState::OPTIONS;
        } else if (menuSelection == 2) {
            running = false;
        }
    }
}

void Game::resetGame() {
    gameOver = false;
    wave = 0;
    score = 0;
    kills = 0;
    player.health = maxHealth;
    player.alive = true;
    player.pos = Vec3(0, 0, 0);
    waveComplete = true;
    waveBreak = 0;
    projectiles.clear();
    particles.clear();
    resetUpgrades(*this);
}

void Game::update(float dt) {
    if (state == GameState::MENU) {
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
        return;
    }

    if (waveComplete) {
        waveBreak += dt;
        if (waveBreak >= nextWaveDelay) {
            nextWave();
        }
        return;
    }

    updatePlayer(dt);
    updateBots(*this, dt);
    updateProjectiles(dt);
    updateParticles(dt);
    updateWeapons(dt, *this);
    updatePowerUps(dt, *this);
    updateScore(dt, *this);
    updateSpecials(dt, *this);
    updateKillFeed(dt, *this);
    updateDamageNumbers(dt, *this);
    checkCollisions();

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
        score += wave * 100;
        upgradePoints += 1 + wave / 3;
        showUpgradeMenu = true;
        if (wave % 5 == 0) {
            upgradePoints += 3;
            printf("BOSS KILLED! Wave %d cleared! Score: %d\n", wave, score);
        } else {
            printf("Wave %d cleared! Score: %d\n", wave, score);
        }
    }

    // Check player death
    if (player.health <= 0) {
        gameOver = true;
        saveHighScore(*this);
        printf("Game Over! Waves survived: %d, Kills: %d\n", wave - 1, kills);
    }
}

void Game::updatePlayer(float dt) {
    handleMouse();

    float moveX = 0, moveZ = 0;
    if (keys[SDL_SCANCODE_W]) moveZ -= 1;
    if (keys[SDL_SCANCODE_S]) moveZ += 1;
    if (keys[SDL_SCANCODE_A]) moveX -= 1;
    if (keys[SDL_SCANCODE_D]) moveX += 1;

    if (moveX != 0 || moveZ != 0) {
        Vec3 moveDir = normalize(Vec3(moveX, 0, moveZ));
        player.pos.x += moveDir.x * playerSpeed * dt;
        player.pos.z += moveDir.z * playerSpeed * dt;
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

void Game::updateParticles(float dt) {
    for (auto& p : particles) {
        p.pos = p.pos + p.vel * dt;
        p.vel.y -= 9.8f * dt;
        p.life -= dt;
    }
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.life <= 0.0f; }),
        particles.end());
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
                        bot.alive = false;
                        kills++;
                        addScore(*this, 10);
                        if (g_audio) g_audio->playKill();
                        if (bot.botType == 4) {
                            addKillFeed(*this, "BOSS KILLED!", Vec3(1.0f, 0.8f, 0.0f));
                        } else {
                            addKillFeed(*this, "KILL", Vec3(0.0f, 1.0f, 0.5f));
                        }
                        addDamageNumber(*this, bot.pos, 50);
                        spawnExplosion(*this, bot.pos, Vec3(0.0f, 0.8f, 1.0f), 20);
                        int dropChance = rand() % 100;
                        if (dropChance < 30) {
                            int type = rand() % 3;
                            spawnPowerUp(*this, bot.pos, type);
                        }

                        // Splitter: spawn mini-bots on death
                        if (bot.splitters > 0) {
                            for (int s = 0; s < bot.splitters; s++) {
                                Entity mini;
                                float sAngle = (float)s / bot.splitters * 6.28318f;
                                mini.pos = bot.pos + Vec3(cosf(sAngle) * 2.0f, 0.0f, sinf(sAngle) * 2.0f);
                                mini.yaw = 0;
                                mini.pitch = 0;
                                mini.alive = true;
                                mini.type = 1;
                                mini.botType = 0;
                                mini.health = 30.0f + wave * 3;
                                mini.moveSpeed = bot.moveSpeed * 1.3f;
                                mini.splitters = bot.splitters - 1;
                                bots.push_back(mini);
                            }
                            printf("Splitter! %d mini-bots spawned\n", bot.splitters);
                        }
                    }
                    hit = true;
                    break;
                }
            }
        } else {
            if (distance(it->pos, player.pos) < 1.0f) {
                player.health -= it->damage;
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
    if (mouseX != 0 || mouseY != 0) {
        player.yaw += mouseX * mouseSensitivity;
        player.pitch -= mouseY * mouseSensitivity;
        player.pitch = fmaxf(-1.5f, fminf(1.5f, player.pitch));
    }
    mouseX = 0;
    mouseY = 0;
}

void Game::nextWave() {
    waveComplete = false;
    waveBreak = 0;
    spawnWave(*this);
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
    Vec3 eye = player.pos;
    Vec3 center = eye + forward;
    Vec3 up(0, 1, 0);

    Mat4 view = Mat4::lookAt(eye, center, up);
    float aspect = (float)renderer_->getWidth() / renderer_->getHeight();
    Mat4 proj = Mat4::perspective(1.1f, aspect, 0.1f, 200.0f);

    renderer_->setView(view);
    renderer_->setProjection(proj);
    renderer_->setViewPos(eye);

    renderer_->beginFrame();
    renderer_->clear(0.02f, 0.02f, 0.05f, 1.0f);

    renderArena();
    renderBots(*this);
    renderProjectiles();
    renderParticles();
    renderLightning(*this);
    renderPowerUps(*this);

    renderer_->endFrame();

    renderHUD(*this);

    SDL_GL_SwapWindow(window_);
}

void Game::renderParticles() {
    if (particles.empty()) return;
    renderer_->drawParticles(particles.data(), static_cast<int>(particles.size()));
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
