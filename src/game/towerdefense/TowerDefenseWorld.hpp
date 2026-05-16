#pragma once

#include "core/GameDifficulty.hpp"
#include "core/Vec2.hpp"

#include <memory>
#include <string>
#include <vector>

namespace mc {

class TowerDefenseWorld {
public:
    static constexpr int Cols = 15;
    static constexpr int Rows = 20;
    static constexpr int TileSize = 32;
    static constexpr float Width = Cols * TileSize;
    static constexpr float Height = Rows * TileSize;

    struct Cell {
        int x = 0;
        int y = 0;
    };

    enum class EnemyType { Grunt, Fast, Tank };
    enum class TowerType { Cannon, Frost, Ember };
    enum class StatusEffectType { Slow, Burn };

    struct StatusEffect {
        StatusEffectType type = StatusEffectType::Slow;
        float magnitude = 0.f;
        float duration = 0.f;
        float tickPeriod = 0.25f;
        float tickAccumulator = 0.f;
    };

    struct Enemy {
        Vec2 pos{};
        Vec2 vel{};
        float hp = 60.f;
        float maxHp = 60.f;
        float speed = 45.f;
        float radius = 10.f;
        int pathIndex = 0;
        EnemyType type = EnemyType::Grunt;
        std::vector<StatusEffect> effects;
        bool alive = true;
    };

    struct Projectile {
        Vec2 pos{};
        Vec2 vel{};
        float damage = 0.f;
        float radius = 4.f;
        TowerType sourceType = TowerType::Cannon;
        bool applyEffect = false;
        StatusEffect effect{};
        bool alive = true;
    };

    class ITargetStrategy {
    public:
        virtual ~ITargetStrategy() = default;
        virtual Enemy* selectTarget(std::vector<Enemy*>& candidates) const = 0;
        virtual const char* name() const = 0;
    };

    struct Tower {
        Cell cell{};
        TowerType type = TowerType::Cannon;
        int level = 1;
        float cooldown = 0.f;
        int strategyIndex = 0;
        std::unique_ptr<ITargetStrategy> strategy;
    };

    struct WaveDefinition {
        EnemyType type = EnemyType::Grunt;
        int count = 0;
        float interval = 0.8f;
    };

    TowerDefenseWorld();

    void reset();
    void setDifficulty(GameDifficulty d) { difficulty_ = d; }
    GameDifficulty difficulty() const { return difficulty_; }
    void fixedUpdate(float dt);

    bool placeTower(Cell cell, TowerType type);
    bool upgradeTower(Cell cell);
    bool cycleTowerTargeting(Cell cell);
    const Tower* towerAt(Cell cell) const;

    int score() const { return score_; }
    int gold() const { return gold_; }
    int lives() const { return lives_; }
    int wave() const { return wave_; }
    int maxWaves() const { return static_cast<int>(waves_.size()); }
    int buildCost() const { return buildCost_; }
    int upgradeCost() const { return upgradeCost_; }
    bool gameOver() const { return lives_ <= 0; }
    bool victory() const { return victory_; }

    const std::vector<Cell>& path() const { return path_; }
    const std::vector<Cell>& debugOpen() const { return debugOpen_; }
    const std::vector<Cell>& debugClosed() const { return debugClosed_; }
    const std::vector<Enemy>& enemies() const { return enemies_; }
    const std::vector<Tower>& towers() const { return towers_; }
    const std::vector<Projectile>& projectiles() const { return projectiles_; }
    Cell startCell() const { return start_; }
    Cell goalCell() const { return goal_; }

    static const char* towerTypeName(TowerType t);
    static const char* enemyTypeName(EnemyType t);

private:
    static bool sameCell(Cell a, Cell b) { return a.x == b.x && a.y == b.y; }
    bool inBounds(Cell c) const;
    bool isBlocked(Cell c) const;
    int towerIndexAt(Cell c) const;
    Vec2 cellCenter(Cell c) const;

    void applyDifficultyTuning();
    void loadWaves();
    static EnemyType parseEnemyType(const std::string& token);
    static std::unique_ptr<ITargetStrategy> makeStrategy(int mode);
    bool recomputePath();
    std::vector<Cell> smoothPath(const std::vector<Cell>& in) const;
    static float heuristic(Cell a, Cell b);
    int nearestPathIndex(Vec2 pos) const;
    void updateWaves(float dt);
    void updateEnemies(float dt);
    void updateTowers(float dt);
    void updateProjectiles(float dt);
    float tickStatusEffects(Enemy& e, float dt);
    void spawnEnemyFromWave(const WaveDefinition& def);

    Cell start_{0, 9};
    Cell goal_{14, 9};
    std::vector<Cell> path_;
    std::vector<Cell> debugOpen_;
    std::vector<Cell> debugClosed_;
    std::vector<Tower> towers_;
    std::vector<Enemy> enemies_;
    std::vector<Projectile> projectiles_;
    std::vector<WaveDefinition> waves_;

    int score_ = 0;
    int gold_ = 140;
    int lives_ = 20;
    int wave_ = 0;
    bool waveInProgress_ = false;
    int spawnRemaining_ = 0;
    float spawnTimer_ = 0.f;
    float spawnInterval_ = 0.75f;
    WaveDefinition currentWave_{};
    int buildCost_ = 50;
    int upgradeCost_ = 40;
    GameDifficulty difficulty_{GameDifficulty::Normal};
    bool victory_ = false;
};

} // namespace mc
