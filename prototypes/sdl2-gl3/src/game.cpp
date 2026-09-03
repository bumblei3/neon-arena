// game.cpp - Game logic implementation with wave survival gameplay
#include "game.h"
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

    setupArena();

    // Initial player state
    player.pos = Vec3(0, playerHeight, 0);
    player.health = maxHealth;
    player.alive = true;
    player.yaw = 0;
    player.pitch = 0;

    // Start wave 1
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

void Game::spawnWave() {
    wave++;
    int botCount = wave + 1;  // Wave N spawns N+1 bots
    bots.clear();

    for (int i = 0; i < botCount; i++) {
        Entity bot;
        // Spawn bots at random positions around the arena edge
        float angle = (float)i / botCount * 6.28318f;
        float radius = arenaSize * 0.7f;
        bot.pos = Vec3(
            cosf(angle) * radius,
            0.5f,
            sinf(angle) * radius
        );
        bot.yaw = 0;
        bot.pitch = 0;
        bot.health = 100.0f + wave * 10;  // Bots get tougher
        bot.alive = true;
        bot.type = 1;
        bots.push_back(bot);
    }

    waveComplete = false;
    printf("Wave %d: %d bots spawned\n", wave, botCount);
}

void Game::run() {
    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTime) / 1000.0f;
        lastTime = now;

        // Cap dt
        if (dt > 0.1f) dt = 0.1f;

        handleInput(dt);
        update(dt);
        render();

        // FPS limit
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
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
            if (event.key.keysym.sym == SDLK_SPACE && waveComplete) {
                nextWave();
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
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                shootRequested = false;
            }
        }
    }
}

void Game::update(float dt) {
    if (gameOver) {
        // Wait for restart
        if (keys[SDL_SCANCODE_SPACE]) {
            // Reset game
            gameOver = false;
            wave = 0;
            score = 0;
            kills = 0;
            player.health = maxHealth;
            player.alive = true;
            waveComplete = true;
            waveBreak = 0;
        }
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
    updateBots(dt);
    updateProjectiles(dt);
    updateParticles(dt);
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
        printf("Wave %d cleared! Score: %d\n", wave, score);
    }

    // Check player death
    if (player.health <= 0) {
        gameOver = true;
        printf("Game Over! Waves survived: %d, Kills: %d\n", wave - 1, kills);
    }
}

void Game::updatePlayer(float dt) {
    // Mouse look
    handleMouse();

    // Movement
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

    // Keep player on ground
    player.pos.y = playerHeight;

    // Arena bounds
    if (player.pos.x < -arenaSize) player.pos.x = -arenaSize;
    if (player.pos.x > arenaSize) player.pos.x = arenaSize;
    if (player.pos.z < -arenaSize) player.pos.z = -arenaSize;
    if (player.pos.z > arenaSize) player.pos.z = arenaSize;

    // Shoot
    if (shootRequested) {
        shootRequested = false;
        // Fire railgun shot
        Vec3 forward(
            sinf(player.yaw) * cosf(player.pitch),
            -sinf(player.pitch),
            -cosf(player.yaw) * cosf(player.pitch)
        );
        Vec3 muzzlePos = player.pos + forward * 0.5f;
        projectiles.push_back(Projectile(muzzlePos, forward, true));
        playSnd(g_sndShoot);
    }
}

void Game::updateBots(float dt) {
    for (auto& bot : bots) {
        if (!bot.alive) continue;

        // Move towards player
        Vec3 toPlayer = Vec3(
            player.pos.x - bot.pos.x,
            0,
            player.pos.z - bot.pos.z
        );

        float dist = toPlayer.length();
        if (dist > 3.0f) {
            toPlayer = toPlayer.normalized();
            bot.pos.x += toPlayer.x * 3.0f * dt;
            bot.pos.z += toPlayer.z * 3.0f * dt;
        }

        // Rotate towards player
        bot.yaw = atan2f(toPlayer.x, -toPlayer.z);

        // Hover animation (sinusoidal Y offset)
        float hoverOffset = sinf(gameTime * 2.0f + bot.pos.x * 0.1f + bot.pos.z * 0.1f) * 0.3f;
        bot.pos.y = hoverOffset;

        // Keep bots in bounds
        if (bot.pos.x < -arenaSize + 2) bot.pos.x = -arenaSize + 2;
        if (bot.pos.x > arenaSize - 2) bot.pos.x = arenaSize - 2;
        if (bot.pos.z < -arenaSize + 2) bot.pos.z = -arenaSize + 2;
        if (bot.pos.z > arenaSize - 2) bot.pos.z = arenaSize - 2;

        // Bot attacks player when close
        if (dist < 5.0f) {
            player.health -= 5.0f * dt;  // Damage over time when close
        }
    }
}

void Game::updateProjectiles(float dt) {
    for (auto& proj : projectiles) {
        proj.pos = proj.pos + proj.dir * proj.speed * dt;
        proj.life -= dt;
    }

    // Remove dead projectiles
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const Projectile& p) { return p.life <= 0; }),
        projectiles.end()
    );
}

void Game::updateParticles(float dt) {
    for (auto& p : particles) {
        p.pos = p.pos + p.vel * dt;
        p.vel.y -= 9.8f * dt; // gravity
        p.life -= dt;
    }
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.life <= 0.0f; }),
        particles.end());
}

void Game::spawnExplosion(Vec3 pos, Vec3 color, int count) {
    for (int i = 0; i < count; i++) {
        Vec3 vel(
            (rand() % 100 - 50) / 50.0f * 5.0f,
            (rand() % 100) / 100.0f * 8.0f,
            (rand() % 100 - 50) / 50.0f * 5.0f
        );
        float life = 0.5f + (rand() % 100) / 200.0f;
        float size = 0.1f + (rand() % 100) / 500.0f;
        particles.push_back(Particle(pos, vel, color, life, size));
    }
}

void Game::checkCollisions() {
    // Check projectile hits
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        bool hit = false;

        if (it->fromPlayer) {
            // Check bot hits
            for (auto& bot : bots) {
                if (!bot.alive) continue;
                if (distance(it->pos, bot.pos) < 1.5f) {
                    bot.health -= 50;  // Railgun damage
                    if (bot.health <= 0) {
                        bot.alive = false;
                        kills++;
                        score += 10;
                        // Spawn explosion at bot position
                        spawnExplosion(bot.pos, Vec3(0.0f, 0.8f, 1.0f), 20);
                    }
                    hit = true;
                    break;
                }
            }
        } else {
            // Check player hit
            if (distance(it->pos, player.pos) < 1.0f) {
                player.health -= 20;
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
    spawnWave();
}

void Game::render() {
    // Set up camera
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
    renderBots();
    renderProjectiles();
    renderParticles();

    renderer_->endFrame();

    // Render HUD on top
    renderHUD();

    SDL_GL_SwapWindow(window_);
}

void Game::renderParticles() {
    if (particles.empty()) return;
    renderer_->drawParticles(particles.data(), static_cast<int>(particles.size()));
}

void Game::renderArena() {
    // Draw plasma ground
    renderer_->drawGround(gameTime);

    // Draw arena walls (neon cyan)
    Vec3 wallColor(0.0f, 0.8f, 1.0f);
    float h = 3.0f;
    // Four walls
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

void Game::renderBots() {
    for (auto& bot : bots) {
        if (!bot.alive) continue;

        // Draw bot as a glowing diamond/octahedron shape
        // Pulsating size based on game time
        float pulse = 1.0f + sinf(gameTime * 4.0f + bot.pos.x * 0.5f) * 0.15f;
        float s = 0.8f * pulse;  // Size with pulse

        // Color based on health (green-cyan when healthy, red when damaged)
        float healthPct = bot.health / (100.0f + wave * 10);
        Vec3 botColor(
            (1.0f - healthPct) * 0.8f,
            healthPct * 1.0f,
            healthPct * 0.5f
        );

        // Rotation offset based on game time
        float rotOffset = gameTime * 1.5f + bot.pos.x * 0.3f;
        float cosR = cosf(rotOffset);
        float sinR = sinf(rotOffset);

        // Top pyramid (rotated)
        Vertex top[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y + s * 2, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x - s * cosR, bot.pos.y, bot.pos.z - s * sinR), botColor),
            Vertex(Vec3(bot.pos.x + s * sinR, bot.pos.y, bot.pos.z - s * cosR), botColor),
        };
        renderer_->drawLineLoop(top, 3, botColor);

        Vertex top2[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y + s * 2, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x + s * sinR, bot.pos.y, bot.pos.z - s * cosR), botColor),
            Vertex(Vec3(bot.pos.x + s * cosR, bot.pos.y, bot.pos.z + s * sinR), botColor),
        };
        renderer_->drawLineLoop(top2, 3, botColor);

        Vertex top3[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y + s * 2, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x + s * cosR, bot.pos.y, bot.pos.z + s * sinR), botColor),
            Vertex(Vec3(bot.pos.x - s * sinR, bot.pos.y, bot.pos.z + s * cosR), botColor),
        };
        renderer_->drawLineLoop(top3, 3, botColor);

        Vertex top4[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y + s * 2, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x - s * sinR, bot.pos.y, bot.pos.z + s * cosR), botColor),
            Vertex(Vec3(bot.pos.x - s * cosR, bot.pos.y, bot.pos.z - s * sinR), botColor),
        };
        renderer_->drawLineLoop(top4, 3, botColor);

        // Bottom pyramid (rotated opposite direction)
        float rotOffset2 = -rotOffset * 0.7f;
        float cosR2 = cosf(rotOffset2);
        float sinR2 = sinf(rotOffset2);

        Vertex bot1[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y - s, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x - s * cosR2, bot.pos.y, bot.pos.z - s * sinR2), botColor),
            Vertex(Vec3(bot.pos.x + s * sinR2, bot.pos.y, bot.pos.z - s * cosR2), botColor),
        };
        renderer_->drawLineLoop(bot1, 3, botColor);

        Vertex bot2[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y - s, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x + s * sinR2, bot.pos.y, bot.pos.z - s * cosR2), botColor),
            Vertex(Vec3(bot.pos.x + s * cosR2, bot.pos.y, bot.pos.z + s * sinR2), botColor),
        };
        renderer_->drawLineLoop(bot2, 3, botColor);

        Vertex bot3[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y - s, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x + s * cosR2, bot.pos.y, bot.pos.z + s * sinR2), botColor),
            Vertex(Vec3(bot.pos.x - s * sinR2, bot.pos.y, bot.pos.z + s * cosR2), botColor),
        };
        renderer_->drawLineLoop(bot3, 3, botColor);

        Vertex bot4[] = {
            Vertex(Vec3(bot.pos.x, bot.pos.y - s, bot.pos.z), botColor),
            Vertex(Vec3(bot.pos.x - s * cosR2, bot.pos.y, bot.pos.z - s * sinR2), botColor),
            Vertex(Vec3(bot.pos.x - s * sinR2, bot.pos.y, bot.pos.z + s * cosR2), botColor),
        };
        renderer_->drawLineLoop(bot4, 3, botColor);

        // Health bar above bot
        Vec3 hpColor(1.0f - healthPct, healthPct, 0.0f);
        float hbWidth = 1.5f;
        float hbY = bot.pos.y + 2.5f;

        Vertex hp[] = {
            Vertex(Vec3(bot.pos.x - hbWidth * 0.5f * healthPct, hbY, bot.pos.z), hpColor),
            Vertex(Vec3(bot.pos.x + hbWidth * 0.5f * healthPct, hbY, bot.pos.z), hpColor),
        };
        renderer_->drawLineLoop(hp, 2, hpColor);
    }
}

void Game::renderProjectiles() {
    for (auto& proj : projectiles) {
        Vec3 projColor = proj.fromPlayer ? Vec3(0.0f, 1.0f, 1.0f) : Vec3(1.0f, 0.3f, 0.0f);
        float s = 0.5f;

        // Draw projectile as a glowing trail
        Vertex line[] = {
            Vertex(proj.pos - proj.dir * s * 2.0f, projColor * 0.3f),
            Vertex(proj.pos, projColor),
        };
        renderer_->drawLineLoop(line, 2, projColor);
    }
}

void Game::renderHUD() {
    // Simple HUD using OpenGL lines
    // Crosshair
    float cx = 0.0f, cy = 0.0f;
    float chSize = 0.03f;
    Vec3 chColor(0.0f, 1.0f, 0.8f);

    // Horizontal line
    Vertex chH[] = {
        Vertex(Vec3(cx - chSize, cy, 0), chColor),
        Vertex(Vec3(cx + chSize, cy, 0), chColor),
    };
    renderer_->drawLineLoop(chH, 2, chColor);

    // Vertical line
    Vertex chV[] = {
        Vertex(Vec3(cx, cy - chSize, 0), chColor),
        Vertex(Vec3(cx, cy + chSize, 0), chColor),
    };
    renderer_->drawLineLoop(chV, 2, chColor);

    // Health bar (bottom left)
    float hbWidth = 0.4f;
    float hbHeight = 0.03f;
    float hbX = -0.8f;
    float hbY = -0.85f;
    float healthPct = player.health / maxHealth;
    Vec3 hpColor(1.0f - healthPct, healthPct, 0.0f);

    // Background
    Vertex bg[] = {
        Vertex(Vec3(hbX, hbY, 0), Vec3(0.2f, 0.2f, 0.2f)),
        Vertex(Vec3(hbX + hbWidth, hbY, 0), Vec3(0.2f, 0.2f, 0.2f)),
        Vertex(Vec3(hbX + hbWidth, hbY + hbHeight, 0), Vec3(0.2f, 0.2f, 0.2f)),
        Vertex(Vec3(hbX, hbY + hbHeight, 0), Vec3(0.2f, 0.2f, 0.2f)),
    };
    renderer_->drawLineLoop(bg, 4, Vec3(0.2f, 0.2f, 0.2f));

    // Health fill
    Vertex fill[] = {
        Vertex(Vec3(hbX, hbY, 0), hpColor),
        Vertex(Vec3(hbX + hbWidth * healthPct, hbY, 0), hpColor),
        Vertex(Vec3(hbX + hbWidth * healthPct, hbY + hbHeight, 0), hpColor),
        Vertex(Vec3(hbX, hbY + hbHeight, 0), hpColor),
    };
    renderer_->drawLineLoop(fill, 4, hpColor);

    // Wave indicator (top center)
    // Using simple lines to draw "W" for wave
    Vec3 waveColor(0.0f, 0.8f, 1.0f);
    float wSize = 0.04f;
    float wX = -0.08f;
    float wY = 0.85f;

    // Simple wave number representation (just a marker for now)
    Vertex waveMark[] = {
        Vertex(Vec3(wX - wSize, wY - wSize, 0), waveColor),
        Vertex(Vec3(wX + wSize, wY - wSize, 0), waveColor),
        Vertex(Vec3(wX + wSize, wY + wSize, 0), waveColor),
        Vertex(Vec3(wX - wSize, wY + wSize, 0), waveColor),
    };
    renderer_->drawLineLoop(waveMark, 4, waveColor);

    // Score indicator (top right)
    Vec3 scoreColor(1.0f, 0.8f, 0.0f);
    float sX = 0.7f;
    float sY = 0.85f;
    Vertex scoreMark[] = {
        Vertex(Vec3(sX - wSize, sY - wSize, 0), scoreColor),
        Vertex(Vec3(sX + wSize, sY - wSize, 0), scoreColor),
        Vertex(Vec3(sX + wSize, sY + wSize, 0), scoreColor),
        Vertex(Vec3(sX - wSize, sY + wSize, 0), scoreColor),
    };
    renderer_->drawLineLoop(scoreMark, 4, scoreColor);

    // Wave complete message area
    if (waveComplete && !gameOver) {
        Vec3 msgColor(0.0f, 1.0f, 0.5f);
        float mY = 0.0f;
        Vertex msg[] = {
            Vertex(Vec3(-0.3f, mY - 0.05f, 0), msgColor),
            Vertex(Vec3(0.3f, mY - 0.05f, 0), msgColor),
            Vertex(Vec3(0.3f, mY + 0.05f, 0), msgColor),
            Vertex(Vec3(-0.3f, mY + 0.05f, 0), msgColor),
        };
        renderer_->drawLineLoop(msg, 4, msgColor);
    }

    // Game over message area
    if (gameOver) {
        Vec3 goColor(1.0f, 0.2f, 0.2f);
        float mY = 0.0f;
        Vertex go[] = {
            Vertex(Vec3(-0.4f, mY - 0.08f, 0), goColor),
            Vertex(Vec3(0.4f, mY - 0.08f, 0), goColor),
            Vertex(Vec3(0.4f, mY + 0.08f, 0), goColor),
            Vertex(Vec3(-0.4f, mY + 0.08f, 0), goColor),
        };
        renderer_->drawLineLoop(go, 4, goColor);
    }
}

Vec3 Game::normalize(Vec3 v) {
    float len = v.length();
    return len > 0 ? Vec3(v.x / len, v.y / len, v.z / len) : Vec3();
}

float Game::distance(Vec3 a, Vec3 b) {
    return (a - b).length();
}
