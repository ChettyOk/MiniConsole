#pragma once

#include "core/GameDifficulty.hpp"
#include "core/Vec2.hpp"

#include <string>
#include <vector>

namespace mc {

class PlatformerWorld {
public:
    static constexpr float Width = 480.f;
    static constexpr float Height = 640.f;
    static constexpr int TileSize = 32;
    static constexpr int Cols = 15;
    static constexpr int Rows = 20;

    PlatformerWorld();

    void reset();
    void setDifficulty(GameDifficulty d) { difficulty_ = d; }
    GameDifficulty difficulty() const { return difficulty_; }
    void setMoveInput(float dir) { moveInput_ = dir; }
    void queueJump();
    void releaseJump();
    void fixedUpdate(float dt);

    int score() const { return score_; }
    int lives() const { return lives_; }
    bool gameOver() const { return lives_ <= 0; }
    bool victory() const { return victory_; }
    int levelIndex() const { return currentLevel_ + 1; }
    int levelCount() const { return static_cast<int>(levels_.size()); }

    struct HazardShot {
        Vec2 pos{};
        Vec2 vel{};
        float radius = 8.f;
        bool alive = true;
    };

    const std::vector<std::string>& tiles() const { return tiles_; }
    Vec2 playerPos() const { return playerPos_; }
    Vec2 playerSize() const { return {playerW_, playerH_}; }
    const std::vector<HazardShot>& hazardShots() const { return hazardShots_; }

private:
    void loadMap();
    void applyDifficultyTuning();
    void loadMapFile(const std::string& path);
    void loadFallbackLevels();
    void loadLevel(int levelIndex);
    void applyDifficultyLayoutMutations();
    bool isSolidCell(int cx, int cy) const;
    bool isHazardCell(int cx, int cy) const;
    bool overlapsSolid(float x, float y, float w, float h) const;
    bool overlapsHazard(float x, float y, float w, float h) const;
    void collectCoinAtPlayer();
    bool touchesGoal() const;
    bool touchesHazard() const;
    bool touchesHazardShots() const;
    void respawnOrLoseLife();
    void advanceLevelOrWin();
    void updateHardModeAttacks(float dt);
    void spawnHardAttack();

    std::vector<std::vector<std::string>> levels_;
    std::vector<std::string> baseMap_;
    std::vector<std::string> tiles_;
    int currentLevel_ = 0;
    Vec2 spawnPos_{48.f, 32.f};

    Vec2 playerPos_{48.f, 32.f};
    Vec2 playerVel_{};
    float playerW_ = 24.f;
    float playerH_ = 28.f;
    float moveInput_ = 0.f;
    bool jumpCutRequested_ = false;
    bool onGround_ = false;
    std::vector<HazardShot> hazardShots_;
    float attackSpawnTimer_ = 0.f;
    int attackPatternIndex_ = 0;

    float gravity_ = 1200.f;
    float fallGravityMultiplier_ = 1.7f;
    float maxFallSpeed_ = 800.f;
    float moveSpeed_ = 210.f;
    float moveAccel_ = 1600.f;
    float idleFriction_ = 0.82f;
    float jumpSpeed_ = 540.f;
    float coyoteLeft_ = 0.f;
    float coyoteTime_ = 0.1f;
    float jumpBufferLeft_ = 0.f;
    float jumpBufferTime_ = 0.12f;
    float attackInterval_ = 1.8f;

    GameDifficulty difficulty_{GameDifficulty::Normal};
    int score_ = 0;
    int lives_ = 3;
    bool victory_ = false;
};

} // namespace mc
