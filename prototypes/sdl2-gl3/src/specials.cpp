// specials.cpp - Arena specials + Ghost energy/abilities
#include "specials.h"
#include "game.h"
#include "score.h"
#include "bots.h"
#include "powerups.h"
#include "ghost_rules.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

// ============================================================
// ARENA SPECIALS
// ============================================================

void activateNuclearBlast(Game& game) {
    if (game.loadout != Loadout::ARENA) return;
    if (game.nuclearBlastCooldown > 0.0f) return;

    for (auto& bot : game.bots) {
        if (bot.alive) {
            bot.alive = false;
            game.score += 50;
        }
    }
    game.nuclearBlastCooldown = game.nuclearBlastMaxCooldown;
}

void activateTimeSlow(Game& game) {
    if (game.loadout != Loadout::ARENA) return;
    if (game.timeSlowCooldown > 0.0f) return;

    game.timeSlowTimer = 3.0f;
    game.timeSlowCooldown = game.timeSlowMaxCooldown;
}

void activateShield(Game& game) {
    if (game.loadout != Loadout::ARENA) return;
    if (game.shieldCooldown > 0.0f) return;

    game.shieldTimer = 5.0f;
    game.hasShield = true;
    game.shieldCooldown = game.shieldMaxCooldown;
}

// ============================================================
// GHOST ENERGY
// ============================================================

void addGhostEnergy(Game& game, float amount) {
    game.ghostEnergy = GhostRules::addEnergy(game.ghostEnergy, amount);
}

bool spendGhostEnergy(Game& game, float amount) {
    if (!GhostRules::canActivate(game.ghostEnergy, amount, 0.0f)) return false;
    game.ghostEnergy = GhostRules::spendEnergy(game.ghostEnergy, amount);
    return true;
}

void breakCloak(Game& game) {
    if (game.cloakTimer <= 0.0f) return;
    game.cloakTimer = 0.0f;
    game.ghostAmbushActive = true;
    game.ghostAmbushTimer = GhostRules::AMBUSH_WINDOW;
    game.lastKnownPlayerX = game.player.pos.x;
    game.lastKnownPlayerZ = game.player.pos.z;
    if (game.nukePaintTimer > 0.0f) cancelNukePaint(game);
}

void grantKillCloak(Game& game) {
    if (game.loadout != Loadout::GHOST) return;
    if (game.cloakTimer > 0.0f) return;
    game.cloakTimer = GhostRules::CLOAK_KILL_DURATION;
    game.ghostAmbushActive = true;
    game.ghostAmbushTimer = GhostRules::AMBUSH_WINDOW;
}

void notifyPlayerHit(Game& game, float damage) {
    game.player.health -= damage;
    game.tookDamageThisWave = true;
    game.shakeAmount = 3.0f;
    if (game.nukePaintTimer > 0.0f) cancelNukePaint(game);
    if (game.cloakTimer > 0.0f) breakCloak(game);
    if (game.renderer_) {
        game.renderer_->setHitFlash(0.6f);
        game.renderer_->setChromaticAberration(2.0f);
    }
}

void registerBotKill(Game& game, Entity& bot, WeaponType weapon, bool ambush, const char* feedOverride) {
    bot.alive = false;
    game.kills++;
    game.killStreak++;
    addScore(game, 10);
    if (g_audio) g_audio->playKill();

    if (feedOverride) {
        addKillFeed(game, feedOverride, Vec3(1.0f, 0.35f, 0.1f));
    } else if (bot.botType == 4) {
        addKillFeed(game, "BOSS KILLED!", Vec3(1.0f, 0.8f, 0.0f));
    } else if (weapon == WeaponType::GHOST_SNIPER && ambush) {
        addKillFeed(game, "GHOST ASSASSINATE", Vec3(0.3f, 0.9f, 1.0f));
    } else if (weapon == WeaponType::GHOST_SNIPER) {
        addKillFeed(game, "GHOST KILL", Vec3(0.2f, 0.6f, 1.0f));
    } else {
        addKillFeed(game, "KILL", Vec3(0.0f, 1.0f, 0.5f));
    }

    addDamageNumber(game, bot.pos, 50);
    spawnExplosion(game, bot.pos, Vec3(0.0f, 0.8f, 1.0f), 20);
    game.shakeAmount = 2.0f + bot.botType * 1.0f;

    int dropChance = rand() % 100;
    if (dropChance < 30) {
        spawnPowerUp(game, bot.pos, rand() % 3);
    }

    const char* weaponName = "Railgun";
    if (weapon == WeaponType::LIGHTNING_GUN) weaponName = "Lightning Gun";
    else if (weapon == WeaponType::PLASMA_RIFLE) weaponName = "Plasma Rifle";
    else if (weapon == WeaponType::GHOST_SNIPER) weaponName = "Ghost Sniper";
    AchievementSystem::checkCombatAchievements(game.achievementProgress, weaponName, false, game.comboCount, game.killStreak);
    AchievementSystem::checkSkillAchievements(game.achievementProgress,
        game.echoSystem ? game.echoSystem->getTriggerCount() : 0,
        game.overclock ? game.overclock->getUseCount() : 0,
        (int)game.currentFusion);

    if (game.loadout == Loadout::GHOST) {
        addGhostEnergy(game, GhostRules::energyFromKill(bot.botType == 5));
        game.ghostComboCount++;
        game.ghostComboTimer = game.COMBO_WINDOW;
        if (game.ghostComboCount <= game.COMBO_MAX_LEVEL) {
            game.comboBonusDamage = 1.0f + (game.ghostComboCount - 1) * 0.25f;
        }
        if (weapon == WeaponType::GHOST_SNIPER) {
            game.ghostKills++;
            grantKillCloak(game);
        }
    }

    if (bot.splitters > 0) {
        for (int s = 0; s < bot.splitters; s++) {
            Entity mini;
            float sAngle = (float)s / bot.splitters * 6.28318f;
            mini.pos = bot.pos + Vec3(cosf(sAngle) * 2.0f, 0.0f, sinf(sAngle) * 2.0f);
            mini.alive = true;
            mini.type = 1;
            mini.botType = 0;
            mini.health = 30.0f + game.wave * 3;
            mini.moveSpeed = bot.moveSpeed * 1.3f;
            mini.splitters = bot.splitters - 1;
            game.bots.push_back(mini);
        }
    }
}

// ============================================================
// GHOST SPECIALS
// ============================================================

void activateScannerSweep(Game& game) {
    if (game.loadout != Loadout::GHOST) return;
    if (!GhostRules::canActivate(game.ghostEnergy, game.SCANNER_COST, game.scannerCooldown)) return;
    if (!spendGhostEnergy(game, game.SCANNER_COST)) return;

    game.scannerTimer = GhostRules::SCANNER_VISUAL;
    game.scannerCooldown = game.scannerMaxCooldown;
    for (auto& bot : game.bots) {
        if (!bot.alive) continue;
        float dist = (game.player.pos - bot.pos).length();
        if (dist > GhostRules::SCANNER_RADIUS) continue;
        bot.ghostMarked = 1;
        bot.ghostMarkTimer = (bot.botType == 5)
            ? GhostRules::SCANNER_MARK_STEALTH
            : GhostRules::SCANNER_MARK;
    }
    if (g_audio) g_audio->playWaveComplete();
}

void activateEMPBlast(Game& game) {
    if (game.loadout != Loadout::GHOST) return;
    if (!GhostRules::canActivate(game.ghostEnergy, game.EMP_COST, game.empCooldown)) return;
    if (!spendGhostEnergy(game, game.EMP_COST)) return;

    game.empTimer = GhostRules::EMP_VISUAL;
    game.empCooldown = game.empMaxCooldown;
    game.empStunTimer = GhostRules::EMP_STUN;

    for (auto& bot : game.bots) {
        if (!bot.alive) continue;
        float dist = (game.player.pos - bot.pos).length();
        if (dist >= GhostRules::EMP_RADIUS) continue;
        BotAI::setState(bot.aiState, BotAI::State::STUNNED);
        bot.aiState.stunDuration = GhostRules::EMP_STUN;
        bot.aiState.stateTimer = 0.0f;
    }

    for (auto& proj : game.projectiles) {
        if (proj.fromPlayer) continue;
        if ((proj.pos - game.player.pos).length() < GhostRules::EMP_RADIUS) {
            proj.life = 0.0f;
        }
    }
    if (g_audio) g_audio->playExplosion();
}

void cancelNukePaint(Game& game) {
    game.nukePaintTimer = 0.0f;
}

void activateTacNuke(Game& game) {
    if (game.loadout != Loadout::GHOST) return;
    if (game.nukePaintTimer > 0.0f || game.nukeInboundTimer > 0.0f) return;
    if (!GhostRules::canActivate(game.ghostEnergy, game.NUKE_COST, game.nukeCooldown)) return;
    if (!spendGhostEnergy(game, game.NUKE_COST)) return;

    Vec3 forward(
        sinf(game.player.yaw) * cosf(game.player.pitch),
        -sinf(game.player.pitch),
        -cosf(game.player.yaw) * cosf(game.player.pitch)
    );
    Vec3 dir = forward.normalized();
    GhostRules::groundAim(
        game.player.pos.x, game.player.pos.y, game.player.pos.z,
        dir.x, dir.y, dir.z,
        GhostRules::NUKE_MAX_AIM, game.arenaSize,
        &game.nukeX, &game.nukeZ);

    game.nukePaintTimer = GhostRules::NUKE_PAINT;
    game.nukePaintOriginX = game.player.pos.x;
    game.nukePaintOriginZ = game.player.pos.z;
    if (g_audio) g_audio->playWaveComplete();
}

void detonateTacNuke(Game& game) {
    Vec3 epicenter(game.nukeX, 0.5f, game.nukeZ);
    spawnExplosion(game, epicenter, Vec3(1.0f, 0.25f, 0.05f), 80);
    game.shakeAmount = 8.0f;
    game.nukeFlashTimer = 0.6f;
    if (game.renderer_) {
        game.renderer_->setHitFlash(1.0f);
        game.renderer_->setChromaticAberration(4.0f);
    }
    if (g_audio) g_audio->playExplosion();

    if (GhostRules::inNukeRadius(game.player.pos.x, game.player.pos.z, game.nukeX, game.nukeZ)) {
        notifyPlayerHit(game, GhostRules::NUKE_SELF_DAMAGE);
    }

    for (int i = 0; i < (int)game.bots.size(); i++) {
        Entity& bot = game.bots[i];
        if (!bot.alive) continue;
        if (!GhostRules::inNukeRadius(bot.pos.x, bot.pos.z, game.nukeX, game.nukeZ)) continue;
        bool isBoss = (bot.botType == 4 || bot.isBoss);
        bot.health -= GhostRules::nukeDamageFor(isBoss);
        if (bot.health <= 0.0f) {
            registerBotKill(game, bot, WeaponType::RAILGUN, false, isBoss ? "BOSS NUKED" : "NUKED");
        } else {
            addDamageNumber(game, bot.pos, (int)GhostRules::NUKE_BOSS_DAMAGE);
        }
    }

    for (auto& bot : game.bots) {
        if (bot.aiState.state == BotAI::State::EVADE) {
            BotAI::setState(bot.aiState, BotAI::State::HUNT);
        }
    }
}

void activateCloak(Game& game) {
    if (game.loadout != Loadout::GHOST) return;
    if (!GhostRules::canActivate(game.ghostEnergy, game.CLOAK_COST, game.cloakCooldown)) return;
    if (!spendGhostEnergy(game, game.CLOAK_COST)) return;

    game.cloakTimer = GhostRules::CLOAK_DURATION;
    game.cloakCooldown = game.cloakMaxCooldown;
    game.lastKnownPlayerX = game.player.pos.x;
    game.lastKnownPlayerZ = game.player.pos.z;
    if (g_audio) g_audio->playHit();
}

// ============================================================
// UPDATE
// ============================================================

void updateSpecials(float dt, Game& game) {
    if (game.nuclearBlastCooldown > 0.0f)
        game.nuclearBlastCooldown -= dt;
    if (game.timeSlowCooldown > 0.0f)
        game.timeSlowCooldown -= dt;
    if (game.shieldCooldown > 0.0f)
        game.shieldCooldown -= dt;

    if (game.scannerCooldown > 0.0f)
        game.scannerCooldown -= dt;
    if (game.empCooldown > 0.0f)
        game.empCooldown -= dt;
    if (game.nukeCooldown > 0.0f)
        game.nukeCooldown -= dt;
    if (game.cloakCooldown > 0.0f)
        game.cloakCooldown -= dt;
    if (game.nukeFlashTimer > 0.0f)
        game.nukeFlashTimer -= dt;
    if (game.detectorSwarmTimer > 0.0f)
        game.detectorSwarmTimer -= dt;

    if (game.timeSlowTimer > 0.0f)
        game.timeSlowTimer -= dt;

    if (game.shieldTimer > 0.0f) {
        game.shieldTimer -= dt;
        if (game.shieldTimer <= 0.0f)
            game.hasShield = false;
    }

    if (game.scannerTimer > 0.0f)
        game.scannerTimer -= dt;

    for (auto& bot : game.bots) {
        if (bot.ghostMarked) {
            bot.ghostMarkTimer -= dt;
            if (bot.ghostMarkTimer <= 0.0f) {
                bot.ghostMarked = 0;
            }
        }
    }

    if (game.empTimer > 0.0f)
        game.empTimer -= dt;
    if (game.empStunTimer > 0.0f)
        game.empStunTimer -= dt;

    if (game.nukePaintTimer > 0.0f) {
        if (!game.player.alive || game.player.health <= 0.0f) {
            cancelNukePaint(game);
        } else {
            float mdx = game.player.pos.x - game.nukePaintOriginX;
            float mdz = game.player.pos.z - game.nukePaintOriginZ;
            if (mdx * mdx + mdz * mdz > GhostRules::NUKE_MOVE_CANCEL * GhostRules::NUKE_MOVE_CANCEL) {
                cancelNukePaint(game);
            } else {
                game.nukePaintTimer -= dt;
                if (game.nukePaintTimer <= 0.0f) {
                    game.nukePaintTimer = 0.0f;
                    game.nukeInboundTimer = GhostRules::NUKE_INBOUND;
                    game.nukeCooldown = game.nukeMaxCooldown;
                    for (auto& bot : game.bots) {
                        if (!bot.alive) continue;
                        if (bot.aiState.state == BotAI::State::STUNNED) continue;
                        BotAI::setState(bot.aiState, BotAI::State::EVADE);
                        bot.aiState.evadeX = game.nukeX;
                        bot.aiState.evadeZ = game.nukeZ;
                    }
                    if (g_audio) g_audio->playExplosion();
                }
            }
        }
    }

    if (game.nukeInboundTimer > 0.0f) {
        game.nukeInboundTimer -= dt;
        if (game.nukeInboundTimer <= 0.0f) {
            game.nukeInboundTimer = 0.0f;
            detonateTacNuke(game);
        }
    }

    if (game.cloakTimer > 0.0f) {
        game.cloakTimer -= dt;
        if (game.cloakTimer <= 0.0f) {
            game.cloakTimer = 0.0f;
            game.ghostAmbushActive = true;
            game.ghostAmbushTimer = GhostRules::AMBUSH_WINDOW;
            game.lastKnownPlayerX = game.player.pos.x;
            game.lastKnownPlayerZ = game.player.pos.z;
        }
    } else if (game.ghostAmbushActive) {
        game.ghostAmbushTimer -= dt;
        if (game.ghostAmbushTimer <= 0.0f) {
            game.ghostAmbushActive = false;
            game.ghostAmbushTimer = 0.0f;
        }
    }

    if (game.ghostComboTimer > 0.0f)
        game.ghostComboTimer -= dt;
    else if (game.ghostComboCount > 0) {
        game.ghostComboCount = 0;
        game.comboBonusDamage = 1.0f;
    }
}

// ============================================================
// RENDER
// ============================================================

void renderGhostEnergyBar(Game& game) {
    if (game.loadout != Loadout::GHOST) return;

    float barW = 180.0f;
    float barH = 14.0f;
    float x = 12.0f;
    float y = 72.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + barW, y);
    glVertex2f(x + barW, y - barH);
    glVertex2f(x, y - barH);
    glEnd();

    float ratio = game.ghostEnergy / GhostRules::ENERGY_MAX;
    float fillW = barW * ratio;
    if (fillW > 0) {
        float r = 0.1f + (1.0f - ratio) * 0.5f;
        float g = 0.4f + ratio * 0.6f;
        float b = 0.9f;
        glColor4f(r, g, b, 0.9f);
        glBegin(GL_QUADS);
        glVertex2f(x + 1, y - 1);
        glVertex2f(x + 1 + fillW - 2, y - 1);
        glVertex2f(x + 1 + fillW - 2, y - barH + 1);
        glVertex2f(x + 1, y - barH + 1);
        glEnd();
    }

    // Cost ticks at Scanner/EMP/Cloak/Storm
    const float costs[] = {
        GhostRules::SCANNER_COST, GhostRules::EMP_COST,
        GhostRules::CLOAK_COST, GhostRules::NUKE_COST
    };
    glColor4f(1.0f, 1.0f, 1.0f, 0.45f);
    glBegin(GL_LINES);
    for (float cost : costs) {
        float tx = x + barW * (cost / GhostRules::ENERGY_MAX);
        glVertex2f(tx, y);
        glVertex2f(tx, y - barH);
    }
    glEnd();

    char buf[40];
    snprintf(buf, sizeof(buf), "ENERGY %.0f/%.0f", game.ghostEnergy, GhostRules::ENERGY_MAX);
    game.text_.drawText(buf, x + barW + 8, y - 8, 0.10f, Vec3(0.0f, 1.0f, 0.8f));

    if (game.nukePaintTimer > 0.0f) {
        char nbuf[40];
        snprintf(nbuf, sizeof(nbuf), "DESIGNATING %.1f", game.nukePaintTimer);
        game.text_.drawText(nbuf, x, y + 16, 0.12f, Vec3(1.0f, 0.3f, 0.1f));
    } else if (game.nukeInboundTimer > 0.0f) {
        char nbuf[40];
        snprintf(nbuf, sizeof(nbuf), "NUKE INBOUND %d", (int)ceilf(game.nukeInboundTimer));
        game.text_.drawText(nbuf, x, y + 16, 0.14f, Vec3(1.0f, 0.15f, 0.05f));
    } else if (game.cloakTimer > 0.0f) {
        char cloakBuf[32];
        snprintf(cloakBuf, sizeof(cloakBuf), "CLOAKED %.1fs", game.cloakTimer);
        game.text_.drawText(cloakBuf, x, y + 16, 0.12f, Vec3(0.4f, 0.9f, 1.0f));
    } else if (game.ghostAmbushActive) {
        game.text_.drawText("AMBUSH", x, y + 16, 0.12f, Vec3(1.0f, 0.5f, 0.2f));
    } else if (game.detectorSwarmTimer > 0.0f) {
        game.text_.drawText("DETECTED", x, y + 16, 0.12f, Vec3(1.0f, 0.2f, 0.2f));
    }

    glDisable(GL_BLEND);
}

void renderSpecialEffects(Game& game) {
    if (game.hasShield) {
        glColor4f(0.2f, 0.6f, 1.0f, 0.4f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 360; i += 10) {
            float angle = (float)i * 3.14159f / 180.0f;
            float px = game.player.pos.x + cosf(angle) * 30.0f;
            float py = game.player.pos.y + sinf(angle) * 30.0f;
            glVertex2f(px, py);
        }
        glEnd();
    }

    if (game.cloakTimer > 0.0f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float pulse = 0.15f + 0.1f * sinf(game.gameTime * 8.0f);
        glColor4f(0.3f, 0.8f, 1.0f, pulse);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 36; i++) {
            float a = (float)i / 36.0f * 6.28318f;
            float px = game.player.pos.x + cosf(a) * 1.4f;
            float pz = game.player.pos.z + sinf(a) * 1.4f;
            glVertex3f(px, game.player.pos.y, pz);
        }
        glEnd();
        glDisable(GL_BLEND);
    }

    if (game.scannerTimer > 0.0f) {
        float alpha = game.scannerTimer / GhostRules::SCANNER_VISUAL;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.3f, 0.8f, 0.35f * alpha);
        float radius = GhostRules::SCANNER_RADIUS * (1.0f - alpha);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 360; i += 5) {
            float a = (float)i * 3.14159f / 180.0f;
            float px = game.player.pos.x + cosf(a) * radius;
            float pz = game.player.pos.z + sinf(a) * radius;
            glVertex3f(px, game.player.pos.y + 1.0f, pz);
        }
        glEnd();
        glDisable(GL_BLEND);
    }

    if (game.empTimer > 0.0f) {
        float alpha = game.empTimer / GhostRules::EMP_VISUAL;
        glEnable(GL_BLEND);
        glColor4f(0.3f, 0.5f, 1.0f, 0.4f * alpha);
        float radius = GhostRules::EMP_RADIUS * (1.0f - alpha * 0.3f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 360; i += 5) {
            float a = (float)i * 3.14159f / 180.0f;
            float px = game.player.pos.x + cosf(a) * radius;
            float pz = game.player.pos.z + sinf(a) * radius;
            glVertex3f(px, 0.5f, pz);
        }
        glEnd();
        glDisable(GL_BLEND);
    }

    if (game.nukePaintTimer > 0.0f || game.nukeInboundTimer > 0.0f) {
        bool inbound = game.nukeInboundTimer > 0.0f;
        float pulse = inbound
            ? (0.5f + 0.5f * sinf(game.gameTime * 12.0f))
            : (0.35f + 0.25f * sinf(game.gameTime * 6.0f));
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(1.0f, 0.15f, 0.05f, 0.55f * pulse);
        glBegin(GL_LINES);
        glVertex3f(game.nukeX, 40.0f, game.nukeZ);
        glVertex3f(game.nukeX, 0.0f, game.nukeZ);
        glEnd();

        float radius = inbound ? GhostRules::NUKE_RADIUS : 4.0f + (1.0f - game.nukePaintTimer / GhostRules::NUKE_PAINT) * 6.0f;
        glColor4f(1.0f, 0.2f, 0.05f, 0.4f * pulse);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 48; i++) {
            float a = (float)i / 48.0f * 6.28318f;
            glVertex3f(game.nukeX + cosf(a) * radius, 0.2f, game.nukeZ + sinf(a) * radius);
        }
        glEnd();
        glDisable(GL_BLEND);
    }
}

void renderSpecialsHUD(Game& game) {
    renderGhostEnergyBar(game);

    struct Box { const char* key; float cooldown; float maxCooldown; };
    Box boxes[7];
    int numBoxes = 0;

    if (game.loadout == Loadout::GHOST) {
        boxes[numBoxes++] = {"G", game.scannerCooldown, game.scannerMaxCooldown};
        boxes[numBoxes++] = {"H", game.empCooldown, game.empMaxCooldown};
        boxes[numBoxes++] = {"N", game.nukeCooldown, game.nukeMaxCooldown};
        boxes[numBoxes++] = {"J", game.cloakCooldown, game.cloakMaxCooldown};
    } else {
        boxes[numBoxes++] = {"E", game.nuclearBlastCooldown, game.nuclearBlastMaxCooldown};
        boxes[numBoxes++] = {"R", game.timeSlowCooldown, game.timeSlowMaxCooldown};
        boxes[numBoxes++] = {"F", game.shieldCooldown, game.shieldMaxCooldown};
    }

    float startX = 12.0f;
    float y = 98.0f;
    float boxSize = 28.0f;
    float gap = 6.0f;
    float fontSize = 0.10f;

    for (int i = 0; i < numBoxes; i++) {
        float x = startX + i * (boxSize + gap);

        glColor4f(0.1f, 0.1f, 0.1f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + boxSize, y);
        glVertex2f(x + boxSize, y - boxSize);
        glVertex2f(x, y - boxSize);
        glEnd();

        if (boxes[i].cooldown > 0.0f) {
            float ratio = boxes[i].cooldown / boxes[i].maxCooldown;
            glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
            glBegin(GL_QUADS);
            glVertex2f(x, y - boxSize * ratio);
            glVertex2f(x + boxSize, y - boxSize * ratio);
            glVertex2f(x + boxSize, y);
            glVertex2f(x, y);
            glEnd();
        }

        game.text_.drawText(boxes[i].key,
            x + boxSize * 0.5f - fontSize * strlen(boxes[i].key) * 0.5f,
            y - 8, fontSize, Vec3(1.0f, 1.0f, 0.8f));
    }
}
