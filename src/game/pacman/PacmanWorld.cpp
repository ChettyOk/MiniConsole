#include "game/pacman/PacmanWorld.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <random>

namespace mc {

namespace {

constexpr std::array<float, 7> kModeDurations = {7.f, 20.f, 7.f, 20.f, 5.f, 20.f, 5.f};

} // namespace

PacmanWorld::PacmanWorld()
    : baseGrid_({
          "###############",
          "#o...........o#",
          "#.###.###.###.#",
          "#.............#",
          "#.###.#.#.###.#",
          "#.....#.#.....#",
          "###.#.#.#.#.###",
          "#...#.....#...#",
          "#.#.##...##.#.#",
          "#...#.....#...#",
          "###.#.#.#.#.###",
          "#.....#.#.....#",
          "#.###.###.###.#",
          "#o...........o#",
          "###############",
      }) {
    reset();
}

const char* PacmanWorld::modeName(GhostMode m) {
    switch (m) {
    case GhostMode::Scatter:
        return "Scatter";
    case GhostMode::Chase:
        return "Chase";
    case GhostMode::Frightened:
        return "Frightened";
    case GhostMode::Eaten:
        return "Eaten";
    default:
        return "Unknown";
    }
}

sf::Vector2i PacmanWorld::dirVector(Direction d) {
    switch (d) {
    case Direction::Up:
        return {0, -1};
    case Direction::Down:
        return {0, 1};
    case Direction::Left:
        return {-1, 0};
    case Direction::Right:
        return {1, 0};
    default:
        return {0, 0};
    }
}

PacmanWorld::Direction PacmanWorld::opposite(Direction d) {
    switch (d) {
    case Direction::Up:
        return Direction::Down;
    case Direction::Down:
        return Direction::Up;
    case Direction::Left:
        return Direction::Right;
    case Direction::Right:
        return Direction::Left;
    default:
        return Direction::None;
    }
}

bool PacmanWorld::inBounds(sf::Vector2i t) const { return t.x >= 0 && t.x < Cols && t.y >= 0 && t.y < Rows; }

bool PacmanWorld::isWall(sf::Vector2i t) const {
    if (!inBounds(t)) {
        return true;
    }
    return grid_[static_cast<std::size_t>(t.y)][static_cast<std::size_t>(t.x)] == '#';
}

bool PacmanWorld::isTileCenter(const Vec2& pos) const {
    const float tx = (pos.x - TileSize * 0.5f) / static_cast<float>(TileSize);
    const float ty = (pos.y - TileSize * 0.5f) / static_cast<float>(TileSize);
    const float dx = std::abs(tx - std::round(tx));
    const float dy = std::abs(ty - std::round(ty));
    return dx < 0.12f && dy < 0.12f;
}

Vec2 PacmanWorld::tileCenter(sf::Vector2i tile) const {
    return {tile.x * TileSize + TileSize * 0.5f, tile.y * TileSize + TileSize * 0.5f};
}

sf::Vector2i PacmanWorld::tileFromPos(const Vec2& p) const {
    const int x = static_cast<int>(std::floor(p.x / static_cast<float>(TileSize)));
    const int y = static_cast<int>(std::floor(p.y / static_cast<float>(TileSize)));
    return {std::max(0, std::min(Cols - 1, x)), std::max(0, std::min(Rows - 1, y))};
}

float PacmanWorld::tileDistance(sf::Vector2i a, sf::Vector2i b) const {
    const float dx = static_cast<float>(a.x - b.x);
    const float dy = static_cast<float>(a.y - b.y);
    return std::sqrt(dx * dx + dy * dy);
}

void PacmanWorld::resetRoundPositions() {
    pac_.tile = {7, 11};
    pac_.dir = Direction::Left;
    pac_.desired = Direction::Left;
    pac_.pos = tileCenter(pac_.tile);

    ghosts_.clear();
    ghosts_.push_back(Ghost{{7, 7}, {}, {Cols - 2, 1}, Direction::Left, globalMode_, GhostType::Blinky, 82.f, 0.f,
                            tileCenter({7, 7})});
    ghosts_.push_back(Ghost{{6, 7}, {}, {1, 1}, Direction::Right, globalMode_, GhostType::Pinky, 80.f, 0.f,
                            tileCenter({6, 7})});
    ghosts_.push_back(Ghost{{8, 7}, {}, {Cols - 2, Rows - 2}, Direction::Left, globalMode_, GhostType::Inky, 79.f,
                            0.f, tileCenter({8, 7})});
    ghosts_.push_back(Ghost{{7, 8}, {}, {1, Rows - 2}, Direction::Up, globalMode_, GhostType::Clyde, 78.f, 0.f,
                            tileCenter({7, 8})});
}

void PacmanWorld::reset() {
    grid_ = baseGrid_;
    score_ = 0;
    lives_ = 3;
    pelletsRemaining_ = 0;
    for (const auto& row : grid_) {
        for (char c : row) {
            if (c == '.' || c == 'o') {
                ++pelletsRemaining_;
            }
        }
    }
    globalMode_ = GhostMode::Scatter;
    modeStep_ = 0;
    modeTimer_ = kModeDurations.front();
    resetRoundPositions();

    if (grid_[static_cast<std::size_t>(pac_.tile.y)][static_cast<std::size_t>(pac_.tile.x)] == '.') {
        grid_[static_cast<std::size_t>(pac_.tile.y)][static_cast<std::size_t>(pac_.tile.x)] = ' ';
        --pelletsRemaining_;
    }
}

void PacmanWorld::switchGlobalMode() {
    if (globalMode_ == GhostMode::Scatter) {
        globalMode_ = GhostMode::Chase;
    } else {
        globalMode_ = GhostMode::Scatter;
    }
    for (auto& g : ghosts_) {
        if (g.mode != GhostMode::Frightened && g.mode != GhostMode::Eaten) {
            g.mode = globalMode_;
            g.dir = opposite(g.dir);
        }
    }
}

void PacmanWorld::updateModeTimer(float dt) {
    const bool anyFrightened =
        std::any_of(ghosts_.begin(), ghosts_.end(), [](const Ghost& g) { return g.mode == GhostMode::Frightened; });
    if (anyFrightened) {
        return;
    }
    modeTimer_ -= dt;
    if (modeTimer_ > 0.f) {
        return;
    }
    switchGlobalMode();
    if (modeStep_ < static_cast<int>(kModeDurations.size()) - 1) {
        ++modeStep_;
    }
    modeTimer_ = kModeDurations[static_cast<std::size_t>(modeStep_)];
    if (modeStep_ >= static_cast<int>(kModeDurations.size()) - 1) {
        modeTimer_ = 9999.f;
    }
}

void PacmanWorld::consumePelletAtPacTile() {
    const char tile = grid_[static_cast<std::size_t>(pac_.tile.y)][static_cast<std::size_t>(pac_.tile.x)];
    if (tile == '.') {
        grid_[static_cast<std::size_t>(pac_.tile.y)][static_cast<std::size_t>(pac_.tile.x)] = ' ';
        score_ += 10;
        --pelletsRemaining_;
    } else if (tile == 'o') {
        grid_[static_cast<std::size_t>(pac_.tile.y)][static_cast<std::size_t>(pac_.tile.x)] = ' ';
        score_ += 50;
        --pelletsRemaining_;
        triggerFrightened();
    }
}

void PacmanWorld::triggerFrightened() {
    for (auto& g : ghosts_) {
        if (g.mode == GhostMode::Eaten) {
            continue;
        }
        g.mode = GhostMode::Frightened;
        g.frightenedTimer = 6.f;
        g.dir = opposite(g.dir);
    }
}

void PacmanWorld::updatePacman(float dt) {
    const float speed = 98.f;
    if (isTileCenter(pac_.pos)) {
        pac_.tile = tileFromPos(pac_.pos);
        const Direction desired = pac_.desired;
        if (desired != Direction::None) {
            const sf::Vector2i d = dirVector(desired);
            if (!isWall({pac_.tile.x + d.x, pac_.tile.y + d.y})) {
                pac_.dir = desired;
            }
        }
        const sf::Vector2i fwd = dirVector(pac_.dir);
        if (pac_.dir != Direction::None && isWall({pac_.tile.x + fwd.x, pac_.tile.y + fwd.y})) {
            pac_.dir = Direction::None;
        }
    }
    const sf::Vector2i dv = dirVector(pac_.dir);
    pac_.pos.x += dv.x * speed * dt;
    pac_.pos.y += dv.y * speed * dt;
    pac_.tile = tileFromPos(pac_.pos);
    consumePelletAtPacTile();
}

sf::Vector2i PacmanWorld::computeTarget(const Ghost& g, const Ghost& blinky) const {
    if (g.mode == GhostMode::Scatter) {
        return g.scatterCorner;
    }
    if (g.mode == GhostMode::Eaten) {
        return houseTile_;
    }
    if (g.mode == GhostMode::Frightened) {
        return g.tile;
    }
    switch (g.type) {
    case GhostType::Blinky:
        return pac_.tile;
    case GhostType::Pinky: {
        const sf::Vector2i d = dirVector(pac_.dir);
        return {pac_.tile.x + d.x * 4, pac_.tile.y + d.y * 4};
    }
    case GhostType::Inky: {
        const sf::Vector2i d = dirVector(pac_.dir);
        const sf::Vector2i pivot{pac_.tile.x + d.x * 2, pac_.tile.y + d.y * 2};
        const sf::Vector2i diff{pivot.x - blinky.tile.x, pivot.y - blinky.tile.y};
        return {pivot.x + diff.x, pivot.y + diff.y};
    }
    case GhostType::Clyde: {
        const float dist = tileDistance(g.tile, pac_.tile);
        return dist > 8.f ? pac_.tile : g.scatterCorner;
    }
    default:
        return pac_.tile;
    }
}

PacmanWorld::Direction PacmanWorld::chooseGhostDirection(const Ghost& g, bool frightenedRandom) const {
    std::vector<Direction> valid;
    for (Direction d : {Direction::Up, Direction::Left, Direction::Down, Direction::Right}) {
        if (d == opposite(g.dir)) {
            continue;
        }
        const sf::Vector2i dv = dirVector(d);
        const sf::Vector2i nt{g.tile.x + dv.x, g.tile.y + dv.y};
        if (!isWall(nt)) {
            valid.push_back(d);
        }
    }
    if (valid.empty()) {
        return opposite(g.dir);
    }
    if (frightenedRandom) {
        static std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> pick(0, static_cast<int>(valid.size()) - 1);
        return valid[static_cast<std::size_t>(pick(rng))];
    }

    float bestDist = std::numeric_limits<float>::max();
    Direction best = valid.front();
    for (Direction d : valid) {
        const sf::Vector2i dv = dirVector(d);
        const sf::Vector2i nt{g.tile.x + dv.x, g.tile.y + dv.y};
        const float dist = tileDistance(nt, g.targetTile);
        if (dist < bestDist) {
            bestDist = dist;
            best = d;
        }
    }
    return best;
}

void PacmanWorld::updateSingleGhost(Ghost& g, const Ghost& blinky, float dt) {
    if (g.mode == GhostMode::Frightened) {
        g.frightenedTimer -= dt;
        if (g.frightenedTimer <= 0.f) {
            g.mode = globalMode_;
        }
    }
    if (g.mode == GhostMode::Eaten && g.tile == houseTile_) {
        g.mode = globalMode_;
    }

    if (g.mode == GhostMode::Frightened) {
        g.speed = 60.f;
    } else if (g.mode == GhostMode::Eaten) {
        g.speed = 150.f;
    } else {
        g.speed = (g.type == GhostType::Blinky) ? 84.f : 80.f;
    }

    if (isTileCenter(g.pos)) {
        g.tile = tileFromPos(g.pos);
        g.targetTile = computeTarget(g, blinky);
        int options = 0;
        for (Direction d : {Direction::Up, Direction::Left, Direction::Down, Direction::Right}) {
            const sf::Vector2i dv = dirVector(d);
            if (!isWall({g.tile.x + dv.x, g.tile.y + dv.y})) {
                ++options;
            }
        }
        const sf::Vector2i fwd = dirVector(g.dir);
        const bool blockedForward = isWall({g.tile.x + fwd.x, g.tile.y + fwd.y});
        if (options >= 3 || blockedForward || g.dir == Direction::None) {
            g.dir = chooseGhostDirection(g, g.mode == GhostMode::Frightened);
        }
    }

    const sf::Vector2i dv = dirVector(g.dir);
    g.pos.x += dv.x * g.speed * dt;
    g.pos.y += dv.y * g.speed * dt;
    g.tile = tileFromPos(g.pos);
}

void PacmanWorld::updateGhosts(float dt) {
    if (ghosts_.empty()) {
        return;
    }
    const Ghost blinky = ghosts_.front();
    for (auto& g : ghosts_) {
        updateSingleGhost(g, blinky, dt);
    }
}

void PacmanWorld::handleCollisions() {
    for (auto& g : ghosts_) {
        const float dx = g.pos.x - pac_.pos.x;
        const float dy = g.pos.y - pac_.pos.y;
        if (dx * dx + dy * dy > 14.f * 14.f) {
            continue;
        }
        if (g.mode == GhostMode::Frightened) {
            score_ += 200;
            g.mode = GhostMode::Eaten;
            g.pos = tileCenter(houseTile_);
            g.tile = houseTile_;
            g.dir = Direction::Up;
            continue;
        }
        if (g.mode == GhostMode::Eaten) {
            continue;
        }
        --lives_;
        if (lives_ <= 0) {
            return;
        }
        resetRoundPositions();
        return;
    }
}

void PacmanWorld::fixedUpdate(float dt) {
    if (gameOver() || victory()) {
        return;
    }
    updateModeTimer(dt);
    updatePacman(dt);
    updateGhosts(dt);
    handleCollisions();
}

} // namespace mc
