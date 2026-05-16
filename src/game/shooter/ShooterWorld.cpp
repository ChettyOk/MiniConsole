#include "game/shooter/ShooterWorld.hpp"

#include <algorithm>
#include <cmath>

namespace mc {

namespace {

float clampf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }

bool circlesOverlap(Vec2 a, float ra, Vec2 b, float rb) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float r = ra + rb;
    return dx * dx + dy * dy <= r * r;
}

} // namespace

ShooterWorld::ShooterWorld() { reset(); }

void ShooterWorld::reset() {
    playerPos_ = {Width * 0.5f, Height - 70.f};
    moveInput_ = {};
    fireHeld_ = false;
    fireCooldownLeft_ = 0.f;
    spawnTimer_ = 0.35f;
    lives_ = 3;
    score_ = 0;
    invulnLeft_ = 0.f;
    enemies_.clear();
    for (auto& b : bullets_) {
        b.alive = false;
    }
}

void ShooterWorld::setMoveInput(float x, float y) {
    moveInput_ = {x, y};
    const float len = moveInput_.length();
    if (len > 1e-4f) {
        moveInput_ = moveInput_ * (1.f / len);
    } else {
        moveInput_ = {};
    }
}

void ShooterWorld::trySpawnEnemy(float dt) {
    if (gameOver()) {
        return;
    }
    spawnTimer_ -= dt;
    if (spawnTimer_ > 0.f) {
        return;
    }
    spawnTimer_ = spawnInterval_;

    const std::size_t alive =
        static_cast<std::size_t>(std::count_if(enemies_.begin(), enemies_.end(), [](const Enemy& e) { return e.alive; }));
    if (alive >= static_cast<std::size_t>(maxAliveEnemies_)) {
        return;
    }

    Enemy e;
    e.pos = {40.f + static_cast<float>(std::fmod(static_cast<double>(enemies_.size()) * 47.11, static_cast<double>(Width - 80))),
             48.f};
    e.speed = 65.f + static_cast<float>(enemies_.size() % 5) * 8.f;
    e.radius = 13.f + static_cast<float>(enemies_.size() % 3);
    e.alive = true;
    enemies_.push_back(e);
}

void ShooterWorld::tryFire(float dt) {
    if (gameOver() || !fireHeld_) {
        return;
    }
    fireCooldownLeft_ -= dt;
    if (fireCooldownLeft_ > 0.f) {
        return;
    }
    fireCooldownLeft_ = fireInterval_;

    for (auto& b : bullets_) {
        if (!b.alive) {
            b.alive = true;
            b.pos = playerPos_ + Vec2{0.f, -playerRadius_ - 4.f};
            b.vel = {0.f, -520.f};
            return;
        }
    }
}

void ShooterWorld::integrateBullets(float dt) {
    for (auto& b : bullets_) {
        if (!b.alive) {
            continue;
        }
        b.pos += b.vel * dt;
        if (b.pos.y < -20.f || b.pos.x < -20.f || b.pos.x > Width + 20.f) {
            b.alive = false;
        }
    }
}

void ShooterWorld::integrateEnemies(float dt, const Vec2& playerPos) {
    for (auto& e : enemies_) {
        if (!e.alive) {
            continue;
        }
        Vec2 to = playerPos - e.pos;
        const float len = to.length();
        if (len > 1e-4f) {
            to = to * (1.f / len);
        } else {
            to = {0.f, 1.f};
        }
        e.pos += to * (e.speed * dt);
        e.pos.x = clampf(e.pos.x, e.radius, Width - e.radius);
        e.pos.y = clampf(e.pos.y, e.radius, Height - e.radius);
    }
}

void ShooterWorld::resolveBulletEnemyHits() {
    for (auto& b : bullets_) {
        if (!b.alive) {
            continue;
        }
        for (auto& e : enemies_) {
            if (!e.alive) {
                continue;
            }
            if (circlesOverlap(b.pos, 4.f, e.pos, e.radius)) {
                b.alive = false;
                e.alive = false;
                score_ += 25;
                break;
            }
        }
    }
}

void ShooterWorld::resolveEnemyPlayerHits() {
    if (invulnLeft_ > 0.f || gameOver()) {
        return;
    }
    for (const auto& e : enemies_) {
        if (!e.alive) {
            continue;
        }
        if (circlesOverlap(playerPos_, playerRadius_, e.pos, e.radius)) {
            --lives_;
            invulnLeft_ = invulnDuration_;
            // Clear nearby threats so one frame does not drain multiple lives.
            for (auto& e2 : enemies_) {
                if (e2.alive && circlesOverlap(playerPos_, playerRadius_ * 3.f, e2.pos, e2.radius)) {
                    e2.alive = false;
                }
            }
            return;
        }
    }
}

void ShooterWorld::fixedUpdate(float dt) {
    if (gameOver()) {
        return;
    }

    invulnLeft_ = std::max(0.f, invulnLeft_ - dt);

    const float px = playerPos_.x + moveInput_.x * playerSpeed_ * dt;
    const float py = playerPos_.y + moveInput_.y * playerSpeed_ * dt;
    playerPos_.x = clampf(px, playerRadius_, Width - playerRadius_);
    playerPos_.y = clampf(py, playerRadius_, Height - playerRadius_);

    trySpawnEnemy(dt);
    tryFire(dt);
    integrateBullets(dt);
    integrateEnemies(dt, playerPos_);
    resolveBulletEnemyHits();
    resolveEnemyPlayerHits();

    enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), [](const Enemy& e) { return !e.alive; }),
                   enemies_.end());
}

} // namespace mc
