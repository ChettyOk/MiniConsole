#pragma once

#include "core/Vec2.hpp"

#include <array>
#include <string>
#include <vector>

namespace mc {

// Gameplay-only model for a Breakout-style game. No SFML types: rendering is
// handled by `BreakoutView` so this class stays unit-test friendly and free of
// draw concerns.
class BreakoutWorld {
public:
    static constexpr float Width = 480.f;
    static constexpr float Height = 640.f;

    struct Brick {
        float x = 0.f;
        float y = 0.f;
        float w = 0.f;
        float h = 0.f;
        int row = 0;
        bool alive = true;
    };

    enum class PowerUpType { BigBall, WidePaddle };

    struct PowerUp {
        PowerUpType type = PowerUpType::BigBall;
        Vec2 pos{};
        Vec2 vel{};
        bool alive = true;
    };

    BreakoutWorld();

    void resetRound(bool fullReset);
    void tryLaunchBall();
    void setPaddleDir(float dir) { paddleDir_ = dir; }

    void fixedUpdate(float dt);

    int lives() const { return lives_; }
    int score() const { return score_; }
    bool gameOver() const { return lives_ <= 0; }
    bool cleared() const { return bricksCleared_; }
    int levelIndex() const { return currentLevel_ + 1; }
    int levelCount() const { return static_cast<int>(layouts_.size()); }

    const Vec2& paddleCenter() const { return paddleCenter_; }
    Vec2 paddleHalfExtents() const { return {paddleHalfW_, paddleHalfH_}; }

    const Vec2& ballCenter() const { return ballCenter_; }
    float ballRadius() const { return ballRadius_; }
    const Vec2& ballVelocity() const { return ballVel_; }

    const std::vector<Brick>& bricks() const { return bricks_; }
    const std::vector<PowerUp>& powerUps() const { return powerUps_; }
    float widePaddleTimeLeft() const { return widePaddleTimeLeft_; }
    float bigBallTimeLeft() const { return bigBallTimeLeft_; }

private:
    void loadLayouts();
    void buildBricksFromCurrentLevel();
    void launchBallIfIdle();
    void integrateBall(float dt);
    void updatePowerUps(float dt);
    void maybeSpawnPowerUp(const Brick& brick);
    void advanceLevelOrWin();
    bool resolveCircleBrick(const Brick& brick);
    bool resolveCirclePaddle();
    void resolveCircleWalls();

    std::vector<Brick> bricks_;
    std::vector<std::vector<std::string>> layouts_;
    int currentLevel_ = 0;
    std::vector<PowerUp> powerUps_;

    Vec2 paddleCenter_{Width * 0.5f, Height - 40.f};
    float paddleHalfW_ = 70.f;
    float paddleHalfH_ = 10.f;
    float paddleSpeed_ = 320.f;
    float paddleDir_ = 0.f;
    float paddleHalfWBase_ = 70.f;
    float paddleHalfWBoost_ = 105.f;
    float widePaddleTimeLeft_ = 0.f;
    float widePaddleDuration_ = 8.f;

    Vec2 ballCenter_{};
    Vec2 ballVel_{};
    float ballRadius_ = 7.f;
    float ballRadiusBase_ = 7.f;
    float ballRadiusBoost_ = 12.f;
    float bigBallTimeLeft_ = 0.f;
    float bigBallDuration_ = 7.f;
    bool ballStuckToPaddle_{true};

    int lives_ = 3;
    int score_ = 0;
    bool bricksCleared_{false};
};

} // namespace mc
