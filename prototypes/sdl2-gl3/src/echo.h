#pragma once
// echo.h - Echo-System für NeonArena: Echoes
// Ring-Buffer für Replay, Ghost-Entity, Merge/Boost-Mechanik
#include <vector>
#include <cstdint>
#include <cmath>

// Forward declare
class Game;

// Echo Frame - aufgezeichneter Spieler-Zustand
struct EchoFrame {
    float posX, posY, posZ;      // Position
    float velX, velY, velZ;      // Geschwindigkeit
    float yaw, pitch;            // Blickrichtung
    float speed;                 // |velocity| (cached)
    float gameTime;              // Zeitstempel
    bool active;                 // Ist dieses Frame gültig?

    EchoFrame() : posX(0), posY(0), posZ(0), velX(0), velY(0), velZ(0),
                  yaw(0), pitch(0), speed(0), gameTime(0), active(false) {}
};

// Echo-Ghost (visuelle Darstellung)
struct EchoGhost {
    float posX, posY, posZ;
    float yaw, pitch;
    float alpha;                 // Transparenz (1.0 = sichtbar, 0.0 = unsichtbar)
    bool active;

    EchoGhost() : posX(0), posY(0), posZ(0), yaw(0), pitch(0), alpha(1.0f), active(false) {}
};

// Echo-Effekte
enum class EchoEffectType {
    NONE,
    PLAYER_BOOST,      // Spieler hat Echo berührt → Speed-Boost
    BOT_STUN,          // Bot hat Echo berührt → 1s Stun
    MERGE_CONSUME      // Echo wurde konsumiert
};

struct EchoEffect {
    EchoEffectType type;
    float posX, posY, posZ;
    float timer;
    float duration;

    EchoEffect() : type(EchoEffectType::NONE), posX(0), posY(0), posZ(0), timer(0), duration(0.5f) {}
    EchoEffect(EchoEffectType t, float x, float y, float z)
        : type(t), posX(x), posY(y), posZ(z), timer(0), duration(0.5f) {}
};

// Hauptklasse für das Echo-System
class EchoSystem {
public:
    EchoSystem();
    ~EchoSystem();

    // Konfiguration
    static constexpr int MAX_ECHO_FRAMES = 900;      // 15 Sekunden bei 60fps
    static constexpr float ECHO_LIFETIME = 15.0f;    // Sekunden bis Echo zerfällt
    static constexpr float ECHO_RADIUS = 32.0f;      // Interaktions-Radius
    static constexpr float BOOST_MULTIPLIER = 2.0f;  // Speed-Boost Faktor
    static constexpr float BOOST_DURATION = 2.0f;    // Boost-Dauer in Sekunden
    static constexpr float BOT_STUN_DURATION = 1.0f; // Bot-Stun-Dauer

    // Init/Shutdown
    void init();
    void shutdown();

    // Aufzeichnung
    void recordFrame(float x, float y, float z, float vx, float vy, float vz, float yaw, float pitch, float gameTime);
    void startRecording();
    void stopRecording();
    bool isRecording() const { return recording; }

    // Playback
    void update(float dt);
    void play();
    void pause();
    bool isPlaying() const { return playing; }

    // Interaktion
    bool checkPlayerBoost(float playerX, float playerY, float playerZ, float& playerSpeed);  // Echo berührt?
    void checkBotCollision(float botX, float botY, float botZ, int botIndex);  // Bot berührt Echo?

    // Aktueller Zustand
    int getActiveFrameCount() const { return frameCount; }
    float getLifetimeRemaining() const { return lifetimeRemaining; }
    bool isActive() const { return active; }
    bool isBoostActive() const { return boostTimer > 0.0f; }
    float getBoostRemaining() const { return boostTimer; }
    float getBoostMultiplier() const { return BOOST_MULTIPLIER; }

    // Ghost-Zugriff
    const EchoGhost& getGhost() const { return ghost; }

    // Effekte
    const std::vector<EchoEffect>& getEffects() const { return effects; }

    // Reset (neuer Loop)
    void reset();

private:
    EchoFrame frames[MAX_ECHO_FRAMES];
    int writeIndex;          // Nächster Schreib-Slot
    int frameCount;          // Anzahl belegter Frames
    int playIndex;           // Aktueller Playback-Index

    bool recording;
    bool playing;
    bool active;
    float lifetimeRemaining; // Verbleibende Zeit bis Echo zerfällt
    float boostTimer;        // Verbleibende Boost-Zeit

    EchoGhost ghost;
    std::vector<EchoEffect> effects;

    // Helper
    EchoFrame* getNextFrame();
    void updateGhost();
    void updateEffects(float dt);
};
