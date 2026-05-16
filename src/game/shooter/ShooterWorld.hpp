#pragma once

#include "core/Vec2.hpp"

#include <array>
#include <vector>

namespace mc {

// Gameplay-only: pooled bullets, lightweight enemies, simple seek steering.
// Rendering lives in ShooterView.
class ShooterWorld {
public:
    static constexpr float Width = 480.f;
    static constexpr float Height = 640.f;
    static constexpr std::size_t MaxBullets = 96;

    struct BulletSlot {
        bool alive = false;
        Vec2 pos{};
        Vec2 vel{};
    };

    struct Enemy {
        Vec2 pos{};
        float speed = 70.f;
        float radius = 14.f;
        bool alive = true;
    };

    ShooterWorld();

    void reset();
    void setMoveInput(float x, float y);
    void setFireHeld(bool held) { fireHeld_ = held; }

    void fixedUpdate(float dt);

    int lives() const { return lives_; }
    int score() const { return score_; }
    bool gameOver() const { return lives_ <= 0; }

    const Vec2& playerPos() const { return playerPos_; }
    float playerRadius() const { return playerRadius_; }

    const std::array<BulletSlot, MaxBullets>& bullets() const { return bullets_; }
    const std::vector<Enemy>& enemies() const { return enemies_; }

    float invulnerabilityLeft() const { return invulnLeft_; }

private:
    void trySpawnEnemy(float dt);
    void tryFire(float dt);
    void integrateBullets(float dt);
    void integrateEnemies(float dt, const Vec2& playerPos);
    void resolveBulletEnemyHits();
    void resolveEnemyPlayerHits();

    Vec2 playerPos_{Width * 0.5f, Height - 70.f};
    float playerRadius_ = 14.f;
    float playerSpeed_ = 260.f;
    Vec2 moveInput_{};

    std::array<BulletSlot, MaxBullets> bullets_{};
    std::vector<Enemy> enemies_;

    float fireCooldownLeft_ = 0.f;
    float fireInterval_ = 0.14f;
    bool fireHeld_ = false;

    float spawnTimer_ = 0.f;
    float spawnInterval_ = 1.1f;
    int maxAliveEnemies_ = 14;

    int lives_ = 3;
    int score_ = 0;
    float invulnLeft_ = 0.f;
    float invulnDuration_ = 1.25f;
};

} // namespace mc
