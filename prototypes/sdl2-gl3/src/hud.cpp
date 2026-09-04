// hud.cpp - HUD rendering functions implementation
#include "hud.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

void renderMinimap(Game& game) {
    // Minimap oben rechts (150x150 Pixel Bereich)
    float mapSize = 0.25f;
    float mapX = 0.95f - mapSize;
    float mapY = 0.95f - mapSize;
    float scale = mapSize / (game.arenaSize * 2.0f);

    // Hintergrund
    Vertex bg[] = {
        Vertex(Vec3(mapX, mapY, -0.1f), Vec3(0.05f, 0.05f, 0.1f)),
        Vertex(Vec3(mapX + mapSize, mapY, -0.1f), Vec3(0.05f, 0.05f, 0.1f)),
        Vertex(Vec3(mapX + mapSize, mapY + mapSize, -0.1f), Vec3(0.05f, 0.05f, 0.1f)),
        Vertex(Vec3(mapX, mapY + mapSize, -0.1f), Vec3(0.05f, 0.05f, 0.1f)),
    };
    game.renderer_->drawQuad(bg);

    // Spieler (weiß)
    float px = mapX + (game.player.pos.x + game.arenaSize) * scale;
    float py = mapY + (game.player.pos.z + game.arenaSize) * scale;
    float ps = 0.015f;
    Vertex playerIcon[] = {
        Vertex(Vec3(px - ps, py - ps, -0.05f), Vec3(1.0f, 1.0f, 1.0f)),
        Vertex(Vec3(px + ps, py - ps, -0.05f), Vec3(1.0f, 1.0f, 1.0f)),
        Vertex(Vec3(px + ps, py + ps, -0.05f), Vec3(1.0f, 1.0f, 1.0f)),
        Vertex(Vec3(px - ps, py + ps, -0.05f), Vec3(1.0f, 1.0f, 1.0f)),
    };
    game.renderer_->drawQuad(playerIcon);

    // Gegner (rot/orange)
    for (auto& bot : game.bots) {
        if (!bot.alive) continue;
        float bx = mapX + (bot.pos.x + game.arenaSize) * scale;
        float by = mapY + (bot.pos.z + game.arenaSize) * scale;
        float bs = 0.01f;
        Vec3 botColor = (bot.botType == 4) ? Vec3(1.0f, 0.8f, 0.0f) : Vec3(1.0f, 0.3f, 0.0f);
        Vertex botIcon[] = {
            Vertex(Vec3(bx - bs, by - bs, -0.05f), botColor),
            Vertex(Vec3(bx + bs, by - bs, -0.05f), botColor),
            Vertex(Vec3(bx + bs, by + bs, -0.05f), botColor),
            Vertex(Vec3(bx - bs, by + bs, -0.05f), botColor),
        };
        game.renderer_->drawQuad(botIcon);
    }

    // Power-Ups (grün/gelb)
    for (auto& pu : game.powerUps) {
        float pux = mapX + (pu.pos.x + game.arenaSize) * scale;
        float puy = mapY + (pu.pos.z + game.arenaSize) * scale;
        float pus = 0.008f;
        Vec3 puColor;
        switch (pu.type) {
            case 0: puColor = Vec3(0.0f, 1.0f, 0.3f); break;
            case 1: puColor = Vec3(1.0f, 0.8f, 0.0f); break;
            case 2: puColor = Vec3(1.0f, 0.3f, 0.0f); break;
            default: puColor = Vec3(1.0f, 1.0f, 1.0f); break;
        }
        Vertex puIcon[] = {
            Vertex(Vec3(pux - pus, puy - pus, -0.05f), puColor),
            Vertex(Vec3(pux + pus, puy - pus, -0.05f), puColor),
            Vertex(Vec3(pux + pus, puy + pus, -0.05f), puColor),
            Vertex(Vec3(pux - pus, puy + pus, -0.05f), puColor),
        };
        game.renderer_->drawQuad(puIcon);
    }
}

void renderHUD(Game& game) {
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
    game.renderer_->drawLineLoop(chH, 2, chColor);

    // Vertical line
    Vertex chV[] = {
        Vertex(Vec3(cx, cy - chSize, 0), chColor),
        Vertex(Vec3(cx, cy + chSize, 0), chColor),
    };
    game.renderer_->drawLineLoop(chV, 2, chColor);

    // Health bar (bottom left)
    float hbWidth = 0.4f;
    float hbHeight = 0.03f;
    float hbX = -0.8f;
    float hbY = -0.85f;
    float healthPct = game.player.health / game.maxHealth;
    Vec3 hpColor(1.0f - healthPct, healthPct, 0.0f);

    // Background
    Vertex bg[] = {
        Vertex(Vec3(hbX, hbY, 0), Vec3(0.2f, 0.2f, 0.2f)),
        Vertex(Vec3(hbX + hbWidth, hbY, 0), Vec3(0.2f, 0.2f, 0.2f)),
        Vertex(Vec3(hbX + hbWidth, hbY + hbHeight, 0), Vec3(0.2f, 0.2f, 0.2f)),
        Vertex(Vec3(hbX, hbY + hbHeight, 0), Vec3(0.2f, 0.2f, 0.2f)),
    };
    game.renderer_->drawLineLoop(bg, 4, Vec3(0.2f, 0.2f, 0.2f));

    // Health fill
    Vertex fill[] = {
        Vertex(Vec3(hbX, hbY, 0), hpColor),
        Vertex(Vec3(hbX + hbWidth * healthPct, hbY, 0), hpColor),
        Vertex(Vec3(hbX + hbWidth * healthPct, hbY + hbHeight, 0), hpColor),
        Vertex(Vec3(hbX, hbY + hbHeight, 0), hpColor),
    };
    game.renderer_->drawLineLoop(fill, 4, hpColor);

    // Score, Wave, Kills (using text)
    game.text_.drawText("SCORE: " + std::to_string(game.score), -0.95f, 0.9f, 1.0f, Vec3(1.0f, 0.8f, 0.0f));
    game.text_.drawText("WAVE: " + std::to_string(game.wave), -0.95f, 0.83f, 1.0f, Vec3(0.0f, 0.8f, 1.0f));
    game.text_.drawText("KILLS: " + std::to_string(game.kills), -0.95f, 0.76f, 1.0f, Vec3(0.8f, 0.3f, 0.3f));
    game.text_.drawText("HIGH: " + std::to_string(game.highScore), -0.95f, 0.69f, 0.8f, Vec3(0.6f, 0.6f, 0.6f));

    // Multiplier display
    if (game.scoreMultiplier > 1) {
        Vec3 multColor(1.0f, 0.5f, 0.0f);
        std::string multStr = "x" + std::to_string(game.scoreMultiplier) + " COMBO!";
        game.text_.drawText(multStr, -0.95f, 0.62f, 1.2f, multColor);
    }

    // Damage boost indicator
    if (game.damageBoostTimer > 0.0f) {
        game.text_.drawText("DMG BOOST: " + std::to_string((int)game.damageBoostTimer) + "s", 0.5f, 0.9f, 0.8f, Vec3(1.0f, 0.3f, 0.0f));
    }

    // Weapon indicator
    Vec3 weaponColor;
    const char* weaponName;
    switch (game.currentWeapon) {
        case WeaponType::RAILGUN:
            weaponColor = Vec3(0.0f, 1.0f, 1.0f);
            weaponName = "RAILGUN";
            break;
        case WeaponType::LIGHTNING_GUN:
            weaponColor = Vec3(0.5f, 0.7f, 1.0f);
            weaponName = "LIGHTNING";
            break;
        case WeaponType::PLASMA_RIFLE:
            weaponColor = Vec3(1.0f, 0.5f, 0.0f);
            weaponName = "PLASMA";
            break;
        default:
            weaponColor = Vec3(1.0f, 1.0f, 1.0f);
            weaponName = "UNKNOWN";
            break;
    }
    game.text_.drawText(weaponName, 0.6f, -0.9f, 1.0f, weaponColor);

    // Weapon switch hint
    game.text_.drawText("1: RAIL  2: LIGHT  3: PLASMA  Q: SWITCH", 0.5f, -0.95f, 0.7f, Vec3(0.4f, 0.4f, 0.5f));

    // Minimap (oben rechts)
    renderMinimap(game);

    // Specials Cooldown-Anzeige
    renderSpecialsHUD(game);

    // Kill Feed
    renderKillFeed(game);

    // Damage Numbers
    renderDamageNumbers(game);

    // Boss warning
    bool hasBoss = false;
    for (const auto& bot : game.bots) {
        if (bot.alive && bot.botType == 4) {
            hasBoss = true;
            break;
        }
    }
    if (hasBoss) {
        game.text_.drawTextCentered("!!! BOSS !!!", 0.95f, 1.5f, Vec3(1.0f, 0.2f, 0.0f));
    }

    // Wave complete message area
    if (game.waveComplete && !game.gameOver) {
        Vec3 msgColor(0.0f, 1.0f, 0.5f);
        game.text_.drawTextCentered("WAVE CLEARED!", 0.05f, 1.5f, msgColor);
        game.text_.drawTextCentered("SPACE: NEXT WAVE", -0.05f, 0.8f, Vec3(0.5f, 0.5f, 0.5f));
    }

    // Game over message area
    if (game.gameOver) {
        Vec3 goColor(1.0f, 0.2f, 0.2f);
        game.text_.drawTextCentered("GAME OVER", 0.05f, 1.8f, goColor);
        game.text_.drawTextCentered("SPACE: RETRY", -0.05f, 0.8f, Vec3(0.5f, 0.5f, 0.5f));
    }
}

void resetUpgrades(Game& game) {
    game.upgradePoints = 0;
    game.railgunLevel = 1;
    game.lightningLevel = 1;
    game.healthLevel = 1;
    game.speedLevel = 1;
    game.showUpgradeMenu = false;
    game.upgradeSelection = 0;
}

void applyUpgrade(Game& game, int selection) {
    if (game.upgradePoints <= 0) return;

    switch (selection) {
        case 0: // Railgun damage
            if (game.railgunLevel < game.maxUpgradeLevel) {
                game.railgunLevel++;
                game.upgradePoints--;
            }
            break;
        case 1: // Lightning range
            if (game.lightningLevel < game.maxUpgradeLevel) {
                game.lightningLevel++;
                game.lightningRange += 2.0f;
                game.upgradePoints--;
            }
            break;
        case 2: // Max health
            if (game.healthLevel < game.maxUpgradeLevel) {
                game.healthLevel++;
                game.maxHealth += 25.0f;
                game.player.health = game.maxHealth;
                game.upgradePoints--;
            }
            break;
        case 3: // Move speed
            if (game.speedLevel < game.maxUpgradeLevel) {
                game.speedLevel++;
                game.playerSpeed += 1.5f;
                game.upgradePoints--;
            }
            break;
    }
}

void handleUpgradeInput(Game& game, SDL_Event& event) {
    if (event.key.keysym.sym == SDLK_UP) {
        game.upgradeSelection--;
        if (game.upgradeSelection < 0) game.upgradeSelection = 3;
    }
    if (event.key.keysym.sym == SDLK_DOWN) {
        game.upgradeSelection++;
        if (game.upgradeSelection > 3) game.upgradeSelection = 0;
    }
    if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
        if (event.key.keysym.sym == SDLK_RETURN) {
            applyUpgrade(game, game.upgradeSelection);
        } else {
            game.showUpgradeMenu = false;
            game.nextWave();
        }
    }
}

void renderUpgradeMenu(Game& game) {
    // Semi-transparent overlay
    Vertex overlay[] = {
        Vertex(Vec3(-0.7f, -0.7f, 0), Vec3(0, 0, 0.1f)),
        Vertex(Vec3(0.7f, -0.7f, 0), Vec3(0, 0, 0.1f)),
        Vertex(Vec3(0.7f, 0.7f, 0), Vec3(0, 0, 0.1f)),
        Vertex(Vec3(-0.7f, 0.7f, 0), Vec3(0, 0, 0.1f)),
    };
    game.renderer_->drawQuad(overlay);

    // Title
    game.text_.drawTextCentered("UPGRADES", 0.55f, 2.0f, Vec3(0.0f, 0.8f, 1.0f));
    game.text_.drawTextCentered("POINTS: " + std::to_string(game.upgradePoints), 0.45f, 1.2f, Vec3(1.0f, 0.8f, 0.0f));

    // Upgrade options
    const char* upgradeNames[] = {
        "RAILGUN DAMAGE",
        "LIGHTNING RANGE",
        "MAX HEALTH",
        "MOVE SPEED"
    };

    for (int i = 0; i < 4; i++) {
        Vec3 itemColor = (i == game.upgradeSelection) ? Vec3(0.0f, 1.0f, 0.8f) : Vec3(0.5f, 0.5f, 0.5f);
        float yPos = 0.25f - i * 0.15f;

        // Selection arrow
        if (i == game.upgradeSelection) {
            Vertex arrow[] = {
                Vertex(Vec3(-0.4f, yPos - 0.015f, 0), itemColor),
                Vertex(Vec3(-0.37f, yPos, 0), itemColor),
                Vertex(Vec3(-0.4f, yPos + 0.015f, 0), itemColor),
            };
            game.renderer_->drawLineLoop(arrow, 3, itemColor);
        }

        // Name and level
        int level = 1;
        if (i == 0) level = game.railgunLevel;
        if (i == 1) level = game.lightningLevel;
        if (i == 2) level = game.healthLevel;
        if (i == 3) level = game.speedLevel;

        std::string levelStr = "LV " + std::to_string(level);
        if (level >= game.maxUpgradeLevel) levelStr = "MAX";

        game.text_.drawText(upgradeNames[i], -0.33f, yPos - 0.02f, 1.0f, itemColor);
        game.text_.drawText(levelStr, 0.3f, yPos - 0.02f, 1.0f, itemColor);
    }

    // Controls
    game.text_.drawTextCentered("UP/DOWN: SELECT  ENTER: BUY  SPACE: CONTINUE", -0.55f, 0.7f, Vec3(0.3f, 0.3f, 0.4f));

    SDL_GL_SwapWindow(game.window_);
}
