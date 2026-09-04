#ifndef SPECIALS_H
#define SPECIALS_H

class Game;

// --- Special ability activation ---
void activateNuclearBlast(Game& game);
void activateTimeSlow(Game& game);
void activateShield(Game& game);

// --- Per-frame update for specials / cooldowns ---
void updateSpecials(float dt, Game& game);

// --- Rendering ---
void renderSpecialEffects(Game& game);
void renderSpecialsHUD(Game& game);

#endif // SPECIALS_H
