#pragma once

#include "core/Vec2.hpp"

#include <SFML/System/Vector2.hpp>

#include <string>
#include <vector>

namespace mc {

class PacmanWorld {
public:
    static constexpr int Cols = 15;
    static constexpr int Rows = 15;
    static constexpr int TileSize = 32;
    static constexpr float Width = Cols * TileSize;
    static constexpr float Height = Rows * TileSize;

    enum class GhostMode { Scatter, Chase, Frightened, Eaten };
    enum class GhostType { Blinky, Pinky, Inky, Clyde };
    enum class Direction { Up, Down, Left, Right, None };

    struct Ghost {
        sf::Vector2i tile{};
        sf::Vector2i targetTile{};
        sf::Vector2i scatterCorner{};
        Direction dir = Direction::None;
        GhostMode mode = GhostMode::Scatter;
        GhostType type = GhostType::Blinky;
        float speed = 100.f;
        float frightenedTimer = 0.f;
        Vec2 pos{};
    };

    struct Pac {
        sf::Vector2i tile{};
        Direction dir = Direction::Left;
        Direction desired = Direction::Left;
        Vec2 pos{};
    };

    PacmanWorld();

    void reset();
    void setDesiredDirection(Direction dir) { pac_.desired = dir; }
    void fixedUpdate(float dt);

    int score() const { return score_; }
    int lives() const { return lives_; }
    bool gameOver() const { return lives_ <= 0; }
    bool victory() const { return pelletsRemaining_ <= 0; }
    GhostMode globalMode() const { return globalMode_; }

    const std::vector<std::string>& grid() const { return grid_; }
    const Pac& pacman() const { return pac_; }
    const std::vector<Ghost>& ghosts() const { return ghosts_; }
    static const char* modeName(GhostMode m);

private:
    static sf::Vector2i dirVector(Direction d);
    static Direction opposite(Direction d);
    bool inBounds(sf::Vector2i t) const;
    bool isWall(sf::Vector2i t) const;
    bool isTileCenter(const Vec2& pos) const;
    Vec2 tileCenter(sf::Vector2i tile) const;
    sf::Vector2i tileFromPos(const Vec2& p) const;
    float tileDistance(sf::Vector2i a, sf::Vector2i b) const;

    void updateModeTimer(float dt);
    void switchGlobalMode();
    void updatePacman(float dt);
    void consumePelletAtPacTile();
    void triggerFrightened();
    void updateGhosts(float dt);
    void updateSingleGhost(Ghost& g, const Ghost& blinky, float dt);
    Direction chooseGhostDirection(const Ghost& g, bool frightenedRandom) const;
    sf::Vector2i computeTarget(const Ghost& g, const Ghost& blinky) const;
    void handleCollisions();
    void resetRoundPositions();

    std::vector<std::string> baseGrid_;
    std::vector<std::string> grid_;
    Pac pac_{};
    std::vector<Ghost> ghosts_;
    sf::Vector2i houseTile_{7, 7};
    int pelletsRemaining_ = 0;
    int score_ = 0;
    int lives_ = 3;
    GhostMode globalMode_ = GhostMode::Scatter;
    float modeTimer_ = 7.f;
    int modeStep_ = 0;
};

} // namespace mc
