// weapons.cpp - Weapon system implementation
#include "weapons.h"
#include "game.h"
#include "specials.h"
#include "ghost_rules.h"
#include <cmath>
#include <algorithm>

void switchWeapon(Game& game, WeaponType w) {
    game.currentWeapon = w;
}

void fireRailgun(Game& game) {
    if (game.railgunCooldown > 0.0f) return;
    game.railgunCooldown = game.railgunFireRate;

    float damage = 50.0f + (game.railgunLevel - 1) * 15.0f;

    Vec3 forward(
        sinf(game.player.yaw) * cosf(game.player.pitch),
        -sinf(game.player.pitch),
        -cosf(game.player.yaw) * cosf(game.player.pitch)
    );
    Vec3 muzzlePos = game.player.pos + forward * 0.5f;
    game.projectiles.push_back(Projectile(muzzlePos, forward, true, WeaponType::RAILGUN, damage));
    if (g_audio) g_audio->playShoot();
    if (g_audio) g_audio->playShoot();
}

void firePlasma(Game& game) {
    if (game.plasmaCooldown > 0.0f) return;
    game.plasmaCooldown = game.plasmaFireRate;

    Vec3 forward(
        sinf(game.player.yaw) * cosf(game.player.pitch),
        -sinf(game.player.pitch),
        -cosf(game.player.yaw) * cosf(game.player.pitch)
    );
    Vec3 muzzlePos = game.player.pos + forward * 0.5f;
    game.projectiles.push_back(Projectile(muzzlePos, forward, true, WeaponType::PLASMA_RIFLE, game.plasmaDamage));
    if (g_audio) g_audio->playShoot();
}

void fireGhost(Game& game) {
    if (game.ghostCooldown > 0.0f) return;

    bool ambush = game.cloakTimer > 0.0f || game.ghostAmbushActive;
    float damage = GhostRules::sniperDamage(ambush);

    Vec3 forward(
        sinf(game.player.yaw) * cosf(game.player.pitch),
        -sinf(game.player.pitch),
        -cosf(game.player.yaw) * cosf(game.player.pitch)
    );
    Vec3 dir = forward.normalized();
    Vec3 muzzlePos = game.player.pos + dir * 0.5f;

    Entity* hit = nullptr;
    float bestT = GhostRules::SNIPER_RANGE;
    for (auto& bot : game.bots) {
        if (!bot.alive) continue;
        float radius = GhostRules::sniperHitRadius(bot.ghostMarked != 0);
        float t = 0.0f;
        if (GhostRules::rayHitsSphere(
                muzzlePos.x, muzzlePos.y, muzzlePos.z,
                dir.x, dir.y, dir.z,
                bot.pos.x, bot.pos.y, bot.pos.z,
                radius, GhostRules::SNIPER_RANGE, &t)) {
            if (t < bestT) {
                bestT = t;
                hit = &bot;
            }
        }
    }

    // Breaking cloak on the shot still applies ambush to this shot.
    if (game.cloakTimer > 0.0f) breakCloak(game);

    Vec3 impact = muzzlePos + dir * (hit ? bestT : GhostRules::SNIPER_RANGE);
    game.lightningArcs.push_back(LightningArc(muzzlePos, impact, Vec3(0.2f, 0.6f, 1.0f), 0.35f, 6));
    if (g_audio) g_audio->playShoot();

    if (!hit) {
        game.ghostCooldown = GhostRules::sniperLockout(false, false);
        return;
    }

    hit->health -= damage;
    bool killed = hit->health <= 0.0f;
    game.ghostCooldown = GhostRules::sniperLockout(true, killed);
    if (killed) {
        registerBotKill(game, *hit, WeaponType::GHOST_SNIPER, ambush);
    }
}

void findLightningTargets(Vec3 pos, Game& game, std::vector<Entity*>& targets) {
    std::vector<std::pair<float, Entity*>> sortedBots;
    for (auto& bot : game.bots) {
        if (!bot.alive) continue;
        float d = (pos - bot.pos).length();
        if (d < game.lightningRange) {
            sortedBots.push_back({d, &bot});
        }
    }
    std::sort(sortedBots.begin(), sortedBots.end());
    for (auto& pair : sortedBots) {
        targets.push_back(pair.second);
    }
}

void fireLightning(Game& game) {
    if (game.lightningCooldown > 0.0f) return;
    game.lightningCooldown = game.lightningFireRate;
    if (g_audio) g_audio->playExplosion();
    if (g_audio) g_audio->playLightning();

    Vec3 forward(
        sinf(game.player.yaw) * cosf(game.player.pitch),
        -sinf(game.player.pitch),
        -cosf(game.player.yaw) * cosf(game.player.pitch)
    );

    std::vector<Entity*> targets;
    findLightningTargets(game.player.pos, game, targets);

    if (!targets.empty()) {
        Vec3 lightningColor(0.5f, 0.7f, 1.0f);
        for (int i = 0; i < (int)targets.size() && i < game.lightningChainCount; i++) {
            Vec3 start = game.player.pos + forward * 0.5f;
            Vec3 end = targets[i]->pos;
            game.lightningArcs.push_back(LightningArc(start, end, lightningColor, 0.15f, 10));
            targets[i]->health -= game.lightningDamage + (game.lightningLevel - 1) * 5.0f;
            if (targets[i]->health <= 0 && targets[i]->alive) {
                targets[i]->alive = false;
                game.kills++;
                game.score += 10;
                spawnExplosion(game, targets[i]->pos, Vec3(0.0f, 0.8f, 1.0f), 20);
            }
        }
    } else {
        Vec3 end = game.player.pos + forward * game.lightningRange;
        game.lightningArcs.push_back(LightningArc(game.player.pos + forward * 0.5f, end, Vec3(0.3f, 0.5f, 1.0f), 0.1f, 6));
    }
}

void updateWeapons(float dt, Game& game) {
    if (game.railgunCooldown > 0.0f) game.railgunCooldown -= dt;
    if (game.lightningCooldown > 0.0f) game.lightningCooldown -= dt;
    if (game.plasmaCooldown > 0.0f) game.plasmaCooldown -= dt;
    if (game.ghostCooldown > 0.0f) game.ghostCooldown -= dt;

    for (auto& arc : game.lightningArcs) {
        arc.life -= dt;
    }
    game.lightningArcs.erase(
        std::remove_if(game.lightningArcs.begin(), game.lightningArcs.end(),
            [](const LightningArc& a) { return a.life <= 0.0f; }),
        game.lightningArcs.end());
}

void renderLightning(Game& game) {
    for (const auto& arc : game.lightningArcs) {
        float alpha = arc.life / arc.maxLife;
        Vec3 color = arc.color * alpha;

        Vec3 points[12];
        points[0] = arc.start;
        points[arc.segments + 1] = arc.end;

        for (int i = 1; i <= arc.segments; i++) {
            float t = (float)i / (arc.segments + 1);
            Vec3 base = arc.start + (arc.end - arc.start) * t;
            float offset = 0.3f * (1.0f - t * 0.5f);
            points[i] = Vec3(
                base.x + ((rand() % 100) / 100.0f - 0.5f) * offset,
                base.y + ((rand() % 100) / 100.0f - 0.5f) * offset,
                base.z + ((rand() % 100) / 100.0f - 0.5f) * offset
            );
        }

        for (int i = 0; i <= arc.segments; i++) {
            Vertex line[] = {
                Vertex(points[i], color),
                Vertex(points[i + 1], color)
            };
            game.renderer_->drawLineLoop(line, 2, color);
        }
    }
}
