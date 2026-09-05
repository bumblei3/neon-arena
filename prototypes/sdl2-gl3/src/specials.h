#ifndef SPECIALS_H
#define SPECIALS_H

class Game;
struct Entity;
enum class WeaponType;

// --- Arena specials ---
void activateNuclearBlast(Game& game);
void activateTimeSlow(Game& game);
void activateShield(Game& game);

// --- Ghost specials ---
void addGhostEnergy(Game& game, float amount);
bool spendGhostEnergy(Game& game, float amount);
void activateScannerSweep(Game& game);
void activateEMPBlast(Game& game);
void activateTacNuke(Game& game);
void cancelNukePaint(Game& game);
void detonateTacNuke(Game& game);
void activateCloak(Game& game);
void breakCloak(Game& game);
void grantKillCloak(Game& game);
void notifyPlayerHit(Game& game, float damage);
void registerBotKill(Game& game, Entity& bot, WeaponType weapon, bool ambush, const char* feedOverride = nullptr);

// --- Per-frame update for specials / cooldowns ---
void updateSpecials(float dt, Game& game);

// --- Rendering ---
void renderSpecialEffects(Game& game);
void renderSpecialsHUD(Game& game);

#endif // SPECIALS_H
