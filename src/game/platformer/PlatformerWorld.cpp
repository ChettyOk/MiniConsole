#include "game/platformer/PlatformerWorld.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>

namespace mc {

namespace {

int cellFrom(float x) { return static_cast<int>(std::floor(x / static_cast<float>(PlatformerWorld::TileSize))); }

} // namespace

PlatformerWorld::PlatformerWorld() {
    loadMap();
    reset();
}

void PlatformerWorld::loadMap() {
    levels_.clear();
    loadMapFile("levels/platformer_level1.txt");
    loadMapFile("levels/platformer_level2.txt");
    loadMapFile("levels/platformer_level3.txt");
    if (levels_.empty()) {
        loadFallbackLevels();
    }
}

void PlatformerWorld::reset() {
    applyDifficultyTuning();
    currentLevel_ = 0;
    if (levels_.empty()) {
        loadFallbackLevels();
    }
    loadLevel(currentLevel_);
    playerPos_ = spawnPos_;
    playerVel_ = {};
    moveInput_ = 0.f;
    jumpCutRequested_ = false;
    onGround_ = false;
    hazardShots_.clear();
    attackSpawnTimer_ = attackInterval_ * 0.55f;
    attackPatternIndex_ = 0;
    coyoteLeft_ = 0.f;
    jumpBufferLeft_ = 0.f;
    score_ = 0;
    lives_ = (difficulty_ == GameDifficulty::Hard) ? 2 : 3;
    victory_ = false;
}

void PlatformerWorld::applyDifficultyTuning() {
    if (difficulty_ == GameDifficulty::Hard) {
        gravity_ = 1320.f;
        fallGravityMultiplier_ = 1.95f;
        maxFallSpeed_ = 860.f;
        moveSpeed_ = 205.f;
        moveAccel_ = 1500.f;
        idleFriction_ = 0.84f;
        jumpSpeed_ = 525.f;
        coyoteTime_ = 0.075f;
        jumpBufferTime_ = 0.085f;
        attackInterval_ = 1.55f;
    } else {
        gravity_ = 1200.f;
        fallGravityMultiplier_ = 1.7f;
        maxFallSpeed_ = 800.f;
        moveSpeed_ = 210.f;
        moveAccel_ = 1600.f;
        idleFriction_ = 0.82f;
        jumpSpeed_ = 540.f;
        coyoteTime_ = 0.1f;
        jumpBufferTime_ = 0.12f;
        attackInterval_ = 99.f;
    }
}

void PlatformerWorld::loadMapFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return;
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        if (line.size() < static_cast<std::size_t>(Cols)) {
            line += std::string(static_cast<std::size_t>(Cols) - line.size(), '.');
        } else if (line.size() > static_cast<std::size_t>(Cols)) {
            line.resize(static_cast<std::size_t>(Cols));
        }
        lines.push_back(line);
    }
    if (lines.size() < static_cast<std::size_t>(Rows)) {
        lines.resize(static_cast<std::size_t>(Rows), std::string(static_cast<std::size_t>(Cols), '.'));
    } else if (lines.size() > static_cast<std::size_t>(Rows)) {
        lines.resize(static_cast<std::size_t>(Rows));
    }
    if (!lines.empty()) {
        levels_.push_back(std::move(lines));
    }
}

void PlatformerWorld::loadFallbackLevels() {
    levels_ = {
        {
            "...............",
            "...............",
            "...............",
            "...............",
            "...C...........",
            "######.........",
            "...............",
            "........C......",
            "......#####....",
            "...............",
            "..........C....",
            "....#####......",
            "...............",
            "...........###.",
            "...............",
            "..C............",
            "######.........",
            ".............G.",
            "S..............",
            "###############",
        },
        {
            "...............",
            ".............G.",
            ".......###.....",
            ".....C.........",
            "....###....X...",
            "..........###..",
            "..###........#.",
            "......X........",
            "....#####...C..",
            ".X..............",
            "...###....###...",
            ".........X......",
            ".....###........",
            "..C.........###.",
            "........X.......",
            ".####...........",
            ".......####.....",
            ".....X..........",
            "S...............",
            "###############.",
        },
        {
            ".........G.....",
            "...###.....###.",
            ".......X.......",
            "..###.....###..",
            "...............",
            ".C...###.......",
            ".......X...###.",
            "..###.......C..",
            "......###......",
            ".X...........X.",
            "...###...###...",
            "...............",
            ".###.......###.",
            ".....X.........",
            "..C......C.....",
            "....###........",
            "........###....",
            ".X...........X.",
            "S..............",
            "###############",
        },
    };
}

void PlatformerWorld::loadLevel(int levelIndex) {
    if (levels_.empty()) {
        return;
    }
    currentLevel_ = std::max(0, std::min(levelIndex, static_cast<int>(levels_.size()) - 1));
    baseMap_ = levels_[static_cast<std::size_t>(currentLevel_)];
    tiles_ = baseMap_;
    spawnPos_ = {48.f, 32.f};
    for (int y = 0; y < Rows; ++y) {
        for (int x = 0; x < Cols; ++x) {
            char& t = tiles_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            if (t == 'S') {
                spawnPos_ = {x * static_cast<float>(TileSize) + 4.f, y * static_cast<float>(TileSize) + 2.f};
                t = '.';
            }
        }
    }
    applyDifficultyLayoutMutations();
    hazardShots_.clear();
    attackSpawnTimer_ = attackInterval_ * 0.55f;
    attackPatternIndex_ = 0;
}

void PlatformerWorld::applyDifficultyLayoutMutations() {
    if (difficulty_ != GameDifficulty::Hard) {
        return;
    }
    auto setTile = [&](int x, int y, char tile) {
        if (x < 0 || x >= Cols || y < 0 || y >= Rows) {
            return;
        }
        char& t = tiles_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
        if (t == 'G') {
            return;
        }
        t = tile;
    };

    if (currentLevel_ == 0) {
        setTile(5, 15, 'X');
        setTile(9, 12, 'X');
        setTile(6, 10, '#');
        setTile(8, 8, '#');
    } else if (currentLevel_ == 1) {
        setTile(7, 15, 'X');
        setTile(10, 12, 'X');
        setTile(6, 7, 'X');
        setTile(11, 9, '#');
    } else if (currentLevel_ == 2) {
        setTile(4, 16, 'X');
        setTile(10, 14, 'X');
        setTile(8, 10, 'X');
        setTile(12, 6, '#');
    }
}

void PlatformerWorld::queueJump() { jumpBufferLeft_ = jumpBufferTime_; }

void PlatformerWorld::releaseJump() { jumpCutRequested_ = true; }

bool PlatformerWorld::isSolidCell(int cx, int cy) const {
    if (cx < 0 || cx >= Cols) {
        return true;
    }
    if (cy < 0) {
        return false;
    }
    if (cy >= Rows) {
        return true;
    }
    return tiles_[static_cast<std::size_t>(cy)][static_cast<std::size_t>(cx)] == '#';
}

bool PlatformerWorld::isHazardCell(int cx, int cy) const {
    if (cx < 0 || cx >= Cols || cy < 0 || cy >= Rows) {
        return false;
    }
    return tiles_[static_cast<std::size_t>(cy)][static_cast<std::size_t>(cx)] == 'X';
}

bool PlatformerWorld::overlapsSolid(float x, float y, float w, float h) const {
    const int left = cellFrom(x);
    const int right = cellFrom(x + w - 1.f);
    const int top = cellFrom(y);
    const int bottom = cellFrom(y + h - 1.f);
    for (int cy = top; cy <= bottom; ++cy) {
        for (int cx = left; cx <= right; ++cx) {
            if (isSolidCell(cx, cy)) {
                return true;
            }
        }
    }
    return false;
}

bool PlatformerWorld::overlapsHazard(float x, float y, float w, float h) const {
    const int left = cellFrom(x);
    const int right = cellFrom(x + w - 1.f);
    const int top = cellFrom(y);
    const int bottom = cellFrom(y + h - 1.f);
    for (int cy = top; cy <= bottom; ++cy) {
        for (int cx = left; cx <= right; ++cx) {
            if (isHazardCell(cx, cy)) {
                return true;
            }
        }
    }
    return false;
}

void PlatformerWorld::collectCoinAtPlayer() {
    const float cx = playerPos_.x + playerW_ * 0.5f;
    const float cy = playerPos_.y + playerH_ * 0.5f;
    const int tx = std::max(0, std::min(Cols - 1, cellFrom(cx)));
    const int ty = std::max(0, std::min(Rows - 1, cellFrom(cy)));
    char& tile = tiles_[static_cast<std::size_t>(ty)][static_cast<std::size_t>(tx)];
    if (tile == 'C') {
        tile = '.';
        score_ += 10;
    }
}

bool PlatformerWorld::touchesGoal() const {
    const int left = cellFrom(playerPos_.x);
    const int right = cellFrom(playerPos_.x + playerW_ - 1.f);
    const int top = cellFrom(playerPos_.y);
    const int bottom = cellFrom(playerPos_.y + playerH_ - 1.f);
    for (int cy = top; cy <= bottom; ++cy) {
        if (cy < 0 || cy >= Rows) {
            continue;
        }
        for (int cx = left; cx <= right; ++cx) {
            if (cx < 0 || cx >= Cols) {
                continue;
            }
            if (tiles_[static_cast<std::size_t>(cy)][static_cast<std::size_t>(cx)] == 'G') {
                return true;
            }
        }
    }
    return false;
}

bool PlatformerWorld::touchesHazard() const {
    return overlapsHazard(playerPos_.x, playerPos_.y, playerW_, playerH_);
}

bool PlatformerWorld::touchesHazardShots() const {
    const float left = playerPos_.x;
    const float right = playerPos_.x + playerW_;
    const float top = playerPos_.y;
    const float bottom = playerPos_.y + playerH_;
    for (const auto& shot : hazardShots_) {
        if (!shot.alive) {
            continue;
        }
        const float clampedX = std::max(left, std::min(right, shot.pos.x));
        const float clampedY = std::max(top, std::min(bottom, shot.pos.y));
        const float dx = shot.pos.x - clampedX;
        const float dy = shot.pos.y - clampedY;
        if (dx * dx + dy * dy <= shot.radius * shot.radius) {
            return true;
        }
    }
    return false;
}

void PlatformerWorld::respawnOrLoseLife() {
    --lives_;
    if (lives_ <= 0) {
        return;
    }
    playerPos_ = spawnPos_;
    playerVel_ = {};
    onGround_ = false;
    hazardShots_.clear();
    attackSpawnTimer_ = attackInterval_ * 0.8f;
    coyoteLeft_ = 0.f;
    jumpBufferLeft_ = 0.f;
}

void PlatformerWorld::advanceLevelOrWin() {
    if (currentLevel_ + 1 >= static_cast<int>(levels_.size())) {
        victory_ = true;
        return;
    }
    ++currentLevel_;
    loadLevel(currentLevel_);
    playerPos_ = spawnPos_;
    playerVel_ = {};
    moveInput_ = 0.f;
    jumpCutRequested_ = false;
    onGround_ = false;
    coyoteLeft_ = 0.f;
    jumpBufferLeft_ = 0.f;
    hazardShots_.clear();
    attackSpawnTimer_ = attackInterval_ * 0.55f;
    attackPatternIndex_ = 0;
    score_ += 60;
}

void PlatformerWorld::spawnHardAttack() {
    if (difficulty_ != GameDifficulty::Hard) {
        return;
    }
    static constexpr std::array<int, 6> lanes = {4, 7, 10, 12, 14, 16};
    const int lane = lanes[static_cast<std::size_t>(attackPatternIndex_ % static_cast<int>(lanes.size()))];
    const bool fromLeft = (attackPatternIndex_ % 2) == 0;
    ++attackPatternIndex_;

    HazardShot shot;
    shot.radius = 8.f;
    shot.pos = fromLeft ? Vec2{-12.f, lane * static_cast<float>(TileSize) + TileSize * 0.5f}
                        : Vec2{Width + 12.f, lane * static_cast<float>(TileSize) + TileSize * 0.5f};
    shot.vel = fromLeft ? Vec2{230.f, 0.f} : Vec2{-230.f, 0.f};
    shot.alive = true;
    hazardShots_.push_back(shot);
}

void PlatformerWorld::updateHardModeAttacks(float dt) {
    if (difficulty_ != GameDifficulty::Hard) {
        return;
    }
    attackSpawnTimer_ -= dt;
    if (attackSpawnTimer_ <= 0.f) {
        attackSpawnTimer_ = attackInterval_;
        spawnHardAttack();
    }

    for (auto& shot : hazardShots_) {
        if (!shot.alive) {
            continue;
        }
        shot.pos.x += shot.vel.x * dt;
        shot.pos.y += shot.vel.y * dt;
        if (shot.pos.x < -24.f || shot.pos.x > Width + 24.f || shot.pos.y < -24.f || shot.pos.y > Height + 24.f) {
            shot.alive = false;
        }
    }
    hazardShots_.erase(std::remove_if(hazardShots_.begin(), hazardShots_.end(), [](const HazardShot& s) { return !s.alive; }),
                       hazardShots_.end());
}

void PlatformerWorld::fixedUpdate(float dt) {
    if (gameOver() || victory()) {
        return;
    }

    jumpBufferLeft_ = std::max(0.f, jumpBufferLeft_ - dt);
    coyoteLeft_ = onGround_ ? coyoteTime_ : std::max(0.f, coyoteLeft_ - dt);

    if (std::abs(moveInput_) > 0.01f) {
        playerVel_.x += moveInput_ * moveAccel_ * dt;
        playerVel_.x = std::max(-moveSpeed_, std::min(moveSpeed_, playerVel_.x));
    } else {
        const float friction = std::pow(idleFriction_, dt * 60.f);
        playerVel_.x *= friction;
        if (std::abs(playerVel_.x) < 3.f) {
            playerVel_.x = 0.f;
        }
    }

    const float g = (playerVel_.y > 0.f) ? gravity_ * fallGravityMultiplier_ : gravity_;
    playerVel_.y += g * dt;
    playerVel_.y = std::min(playerVel_.y, maxFallSpeed_);

    if (jumpBufferLeft_ > 0.f && (onGround_ || coyoteLeft_ > 0.f)) {
        playerVel_.y = -jumpSpeed_;
        onGround_ = false;
        coyoteLeft_ = 0.f;
        jumpBufferLeft_ = 0.f;
    }
    if (jumpCutRequested_ && playerVel_.y < 0.f) {
        playerVel_.y *= 0.4f;
    }
    jumpCutRequested_ = false;

    float nextX = playerPos_.x + playerVel_.x * dt;
    if (!overlapsSolid(nextX, playerPos_.y, playerW_, playerH_)) {
        playerPos_.x = nextX;
    } else if (playerVel_.x > 0.f) {
        const int rightCell = cellFrom(nextX + playerW_ - 1.f);
        playerPos_.x = rightCell * static_cast<float>(TileSize) - playerW_;
    } else if (playerVel_.x < 0.f) {
        const int leftCell = cellFrom(nextX);
        playerPos_.x = (leftCell + 1) * static_cast<float>(TileSize);
    }

    onGround_ = false;
    float nextY = playerPos_.y + playerVel_.y * dt;
    if (!overlapsSolid(playerPos_.x, nextY, playerW_, playerH_)) {
        playerPos_.y = nextY;
    } else if (playerVel_.y > 0.f) {
        const int bottomCell = cellFrom(nextY + playerH_ - 1.f);
        playerPos_.y = bottomCell * static_cast<float>(TileSize) - playerH_;
        playerVel_.y = 0.f;
        onGround_ = true;
    } else if (playerVel_.y < 0.f) {
        const int topCell = cellFrom(nextY);
        playerPos_.y = (topCell + 1) * static_cast<float>(TileSize);
        playerVel_.y = 0.f;
    }

    collectCoinAtPlayer();
    updateHardModeAttacks(dt);
    if (touchesHazard()) {
        respawnOrLoseLife();
        return;
    }
    if (touchesHazardShots()) {
        respawnOrLoseLife();
        return;
    }
    if (touchesGoal()) {
        score_ += 100;
        advanceLevelOrWin();
        return;
    }
    if (playerPos_.y > Height + 80.f) {
        respawnOrLoseLife();
    }
}

} // namespace mc
