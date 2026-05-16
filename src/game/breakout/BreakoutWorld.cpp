#include "game/breakout/BreakoutWorld.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>

namespace mc {

namespace {

float clampf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }

bool circleIntersectsAabb(Vec2 c, float r, float ax, float ay, float aw, float ah) {
    const float closestX = clampf(c.x, ax, ax + aw);
    const float closestY = clampf(c.y, ay, ay + ah);
    const float dx = c.x - closestX;
    const float dy = c.y - closestY;
    return (dx * dx + dy * dy) < r * r;
}

bool isBrickChar(char ch) {
    return ch == '#' || ch == 'X' || ch == '1' || ch == 'B';
}

} // namespace

BreakoutWorld::BreakoutWorld() {
    loadLayouts();
    buildBricksFromCurrentLevel();
    resetRound(true);
}

void BreakoutWorld::loadLayouts() {
    layouts_.clear();
    constexpr std::array<const char*, 3> paths = {
        "levels/breakout_level1.txt",
        "levels/breakout_level2.txt",
        "levels/breakout_level3.txt",
    };
    for (const char* path : paths) {
        std::ifstream in(path);
        if (!in.is_open()) {
            continue;
        }
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        if (!lines.empty()) {
            layouts_.push_back(lines);
        }
    }

    if (!layouts_.empty()) {
        return;
    }
    layouts_ = {
        {
            "##########",
            "##########",
            "##########",
            "##########",
            "##########",
        },
        {
            "##.####.##",
            "##########",
            ".########.",
            "##########",
            "##.####.##",
        },
        {
            "#.#.##.#.#",
            ".########.",
            "##########",
            ".########.",
            "#.#.##.#.#",
        },
    };
}

void BreakoutWorld::buildBricksFromCurrentLevel() {
    bricks_.clear();
    if (layouts_.empty()) {
        return;
    }
    if (currentLevel_ < 0 || currentLevel_ >= static_cast<int>(layouts_.size())) {
        currentLevel_ = 0;
    }

    const auto& layout = layouts_[static_cast<std::size_t>(currentLevel_)];
    const int rows = static_cast<int>(layout.size());
    int cols = 0;
    for (const auto& line : layout) {
        cols = std::max(cols, static_cast<int>(line.size()));
    }
    if (rows <= 0 || cols <= 0) {
        return;
    }

    constexpr float marginX = 24.f;
    constexpr float marginY = 80.f;
    constexpr float gap = 6.f;
    const float brickW = (Width - marginX * 2.f - gap * (cols - 1)) / static_cast<float>(cols);
    const float brickH = 22.f;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            const bool hasBrick = col < static_cast<int>(layout[static_cast<std::size_t>(row)].size()) &&
                                  isBrickChar(layout[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)]);
            if (!hasBrick) {
                continue;
            }
            Brick b;
            b.w = brickW;
            b.h = brickH;
            b.x = marginX + col * (brickW + gap);
            b.y = marginY + row * (brickH + gap);
            b.row = row;
            b.alive = true;
            bricks_.push_back(b);
        }
    }
    bricksCleared_ = bricks_.empty();
}

void BreakoutWorld::tryLaunchBall() { launchBallIfIdle(); }

void BreakoutWorld::resetRound(bool fullReset) {
    if (fullReset) {
        lives_ = 3;
        score_ = 0;
        bricksCleared_ = false;
        currentLevel_ = 0;
        buildBricksFromCurrentLevel();
    }
    powerUps_.clear();
    widePaddleTimeLeft_ = 0.f;
    bigBallTimeLeft_ = 0.f;
    paddleHalfW_ = paddleHalfWBase_;
    ballRadius_ = ballRadiusBase_;
    paddleCenter_.x = Width * 0.5f;
    paddleCenter_.y = Height - 40.f;
    ballStuckToPaddle_ = true;
    ballVel_ = {160.f, -220.f};
    ballCenter_.x = paddleCenter_.x;
    ballCenter_.y = paddleCenter_.y - paddleHalfH_ - ballRadius_ - 1.f;
}

void BreakoutWorld::launchBallIfIdle() {
    if (!ballStuckToPaddle_) {
        return;
    }
    ballStuckToPaddle_ = false;
    if (ballVel_.lengthSq() < 1.f) {
        ballVel_ = {160.f, -220.f};
    }
}

void BreakoutWorld::fixedUpdate(float dt) {
    if (gameOver() || cleared()) {
        return;
    }

    if (widePaddleTimeLeft_ > 0.f) {
        widePaddleTimeLeft_ = std::max(0.f, widePaddleTimeLeft_ - dt);
    }
    if (bigBallTimeLeft_ > 0.f) {
        bigBallTimeLeft_ = std::max(0.f, bigBallTimeLeft_ - dt);
    }
    paddleHalfW_ = (widePaddleTimeLeft_ > 0.f) ? paddleHalfWBoost_ : paddleHalfWBase_;
    ballRadius_ = (bigBallTimeLeft_ > 0.f) ? ballRadiusBoost_ : ballRadiusBase_;

    paddleCenter_.x += paddleDir_ * paddleSpeed_ * dt;
    const float minX = paddleHalfW_;
    const float maxX = Width - paddleHalfW_;
    paddleCenter_.x = clampf(paddleCenter_.x, minX, maxX);

    if (ballStuckToPaddle_) {
        ballCenter_.x = paddleCenter_.x;
        ballCenter_.y = paddleCenter_.y - paddleHalfH_ - ballRadius_ - 1.f;
        return;
    }

    updatePowerUps(dt);
    integrateBall(dt);
}

void BreakoutWorld::integrateBall(float dt) {
    // Sub-step lightly to reduce tunneling at high speeds.
    constexpr int substeps = 2;
    const float h = dt / static_cast<float>(substeps);
    for (int i = 0; i < substeps; ++i) {
        ballCenter_ += ballVel_ * h;
        resolveCircleWalls();

        if (resolveCirclePaddle()) {
            continue;
        }

        for (auto& brick : bricks_) {
            if (!brick.alive) {
                continue;
            }
            if (resolveCircleBrick(brick)) {
                brick.alive = false;
                score_ += 10;
                maybeSpawnPowerUp(brick);
                break;
            }
        }

        if (ballCenter_.y - ballRadius_ > Height) {
            --lives_;
            if (!gameOver()) {
                ballStuckToPaddle_ = true;
                ballVel_ = {160.f, -220.f};
                ballCenter_.x = paddleCenter_.x;
                ballCenter_.y = paddleCenter_.y - paddleHalfH_ - ballRadius_ - 1.f;
            }
            return;
        }
    }

    bricksCleared_ = std::all_of(bricks_.begin(), bricks_.end(), [](const Brick& b) { return !b.alive; });
    if (bricksCleared_) {
        advanceLevelOrWin();
    }
}

void BreakoutWorld::resolveCircleWalls() {
    if (ballCenter_.x - ballRadius_ < 0.f) {
        ballCenter_.x = ballRadius_;
        ballVel_.x *= -1.f;
    } else if (ballCenter_.x + ballRadius_ > Width) {
        ballCenter_.x = Width - ballRadius_;
        ballVel_.x *= -1.f;
    }
    if (ballCenter_.y - ballRadius_ < 0.f) {
        ballCenter_.y = ballRadius_;
        ballVel_.y *= -1.f;
    }
}

bool BreakoutWorld::resolveCirclePaddle() {
    const float px = paddleCenter_.x - paddleHalfW_;
    const float py = paddleCenter_.y - paddleHalfH_;
    const float pw = paddleHalfW_ * 2.f;
    const float ph = paddleHalfH_ * 2.f;
    if (!circleIntersectsAabb(ballCenter_, ballRadius_, px, py, pw, ph)) {
        return false;
    }

    // Push to top of paddle if coming from above (typical case).
    ballCenter_.y = py - ballRadius_ - 0.5f;
    const float hit = (ballCenter_.x - paddleCenter_.x) / paddleHalfW_;
    const float clamped = clampf(hit, -1.f, 1.f);
    const float angle = clamped * 0.85f; // radians-ish weighting
    const float speed = std::max(220.f, ballVel_.length());
    ballVel_.x = speed * std::sin(angle * 1.2f);
    ballVel_.y = -std::abs(std::cos(angle)) * speed;
    if (ballVel_.y > -40.f) {
        ballVel_.y = -220.f;
    }
    return true;
}

bool BreakoutWorld::resolveCircleBrick(const Brick& brick) {
    if (!circleIntersectsAabb(ballCenter_, ballRadius_, brick.x, brick.y, brick.w, brick.h)) {
        return false;
    }

    const float left = brick.x;
    const float right = brick.x + brick.w;
    const float top = brick.y;
    const float bottom = brick.y + brick.h;

    const float overlapLeft = (ballCenter_.x + ballRadius_) - left;
    const float overlapRight = right - (ballCenter_.x - ballRadius_);
    const float overlapTop = (ballCenter_.y + ballRadius_) - top;
    const float overlapBottom = bottom - (ballCenter_.y - ballRadius_);

    const float penL = std::max(0.f, overlapLeft);
    const float penR = std::max(0.f, overlapRight);
    const float penT = std::max(0.f, overlapTop);
    const float penB = std::max(0.f, overlapBottom);

    const float minPen = std::min({penL, penR, penT, penB});
    if (minPen == penL) {
        ballCenter_.x = left - ballRadius_;
        ballVel_.x *= -1.f;
    } else if (minPen == penR) {
        ballCenter_.x = right + ballRadius_;
        ballVel_.x *= -1.f;
    } else if (minPen == penT) {
        ballCenter_.y = top - ballRadius_;
        ballVel_.y *= -1.f;
    } else {
        ballCenter_.y = bottom + ballRadius_;
        ballVel_.y *= -1.f;
    }
    return true;
}

void BreakoutWorld::updatePowerUps(float dt) {
    for (auto& p : powerUps_) {
        if (!p.alive) {
            continue;
        }
        p.pos += p.vel * dt;
        if (p.pos.y > Height + 30.f) {
            p.alive = false;
            continue;
        }

        const float px = paddleCenter_.x - paddleHalfW_;
        const float py = paddleCenter_.y - paddleHalfH_;
        const float pw = paddleHalfW_ * 2.f;
        const float ph = paddleHalfH_ * 2.f;
        if (p.pos.x >= px && p.pos.x <= px + pw && p.pos.y >= py && p.pos.y <= py + ph) {
            p.alive = false;
            if (p.type == PowerUpType::WidePaddle) {
                widePaddleTimeLeft_ = widePaddleDuration_;
            } else {
                bigBallTimeLeft_ = bigBallDuration_;
            }
        }
    }
    powerUps_.erase(std::remove_if(powerUps_.begin(), powerUps_.end(), [](const PowerUp& p) { return !p.alive; }),
                    powerUps_.end());
}

void BreakoutWorld::maybeSpawnPowerUp(const Brick& brick) {
    const int token = static_cast<int>((brick.row * 31 + score_ + lives_ * 13) % 12);
    if (token != 2 && token != 7) {
        return;
    }
    PowerUp p;
    p.type = (token == 2) ? PowerUpType::WidePaddle : PowerUpType::BigBall;
    p.pos = {brick.x + brick.w * 0.5f, brick.y + brick.h * 0.5f};
    p.vel = {0.f, 120.f};
    p.alive = true;
    powerUps_.push_back(p);
}

void BreakoutWorld::advanceLevelOrWin() {
    if (currentLevel_ + 1 >= static_cast<int>(layouts_.size())) {
        bricksCleared_ = true;
        return;
    }
    ++currentLevel_;
    buildBricksFromCurrentLevel();
    powerUps_.clear();
    widePaddleTimeLeft_ = 0.f;
    bigBallTimeLeft_ = 0.f;
    paddleHalfW_ = paddleHalfWBase_;
    ballRadius_ = ballRadiusBase_;
    ballStuckToPaddle_ = true;
    ballVel_ = {160.f, -220.f};
    ballCenter_.x = paddleCenter_.x;
    ballCenter_.y = paddleCenter_.y - paddleHalfH_ - ballRadius_ - 1.f;
}

} // namespace mc
