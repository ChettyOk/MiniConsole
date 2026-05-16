#include "game/towerdefense/TowerDefenseWorld.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <limits>
#include <queue>
#include <sstream>

namespace mc {

namespace {

float clampf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }

struct FirstEnemyStrategy final : TowerDefenseWorld::ITargetStrategy {
    TowerDefenseWorld::Enemy* selectTarget(std::vector<TowerDefenseWorld::Enemy*>& candidates) const override {
        if (candidates.empty()) {
            return nullptr;
        }
        auto* best = candidates.front();
        for (auto* e : candidates) {
            if (e->pathIndex > best->pathIndex) {
                best = e;
            }
        }
        return best;
    }
    const char* name() const override { return "First"; }
};

struct StrongestEnemyStrategy final : TowerDefenseWorld::ITargetStrategy {
    TowerDefenseWorld::Enemy* selectTarget(std::vector<TowerDefenseWorld::Enemy*>& candidates) const override {
        if (candidates.empty()) {
            return nullptr;
        }
        auto* best = candidates.front();
        for (auto* e : candidates) {
            if (e->hp > best->hp) {
                best = e;
            }
        }
        return best;
    }
    const char* name() const override { return "Strongest"; }
};

struct ClosestEnemyStrategy final : TowerDefenseWorld::ITargetStrategy {
    TowerDefenseWorld::Enemy* selectTarget(std::vector<TowerDefenseWorld::Enemy*>& candidates) const override {
        if (candidates.empty()) {
            return nullptr;
        }
        auto* best = candidates.front();
        for (auto* e : candidates) {
            if (e->pathIndex > best->pathIndex) {
                best = e;
            }
        }
        return best;
    }
    const char* name() const override { return "Closest"; }
};

} // namespace

TowerDefenseWorld::TowerDefenseWorld() {
    loadWaves();
    reset();
}

void TowerDefenseWorld::reset() {
    applyDifficultyTuning();
    towers_.clear();
    enemies_.clear();
    projectiles_.clear();
    score_ = 0;
    wave_ = 0;
    waveInProgress_ = false;
    spawnRemaining_ = 0;
    spawnTimer_ = 0.f;
    currentWave_ = {};
    victory_ = false;
    recomputePath();
}

void TowerDefenseWorld::applyDifficultyTuning() {
    if (difficulty_ == GameDifficulty::Hard) {
        gold_ = 115;
        lives_ = 14;
        buildCost_ = 56;
        upgradeCost_ = 46;
    } else {
        gold_ = 140;
        lives_ = 20;
        buildCost_ = 50;
        upgradeCost_ = 40;
    }
}

void TowerDefenseWorld::loadWaves() {
    waves_.clear();
    std::ifstream in("levels/td_waves.txt");
    if (in.is_open()) {
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            std::istringstream is(line);
            std::string typeToken;
            WaveDefinition w;
            if (!(is >> typeToken >> w.count >> w.interval)) {
                continue;
            }
            w.type = parseEnemyType(typeToken);
            if (w.count > 0 && w.interval > 0.f) {
                waves_.push_back(w);
            }
        }
    }
    if (!waves_.empty()) {
        return;
    }
    waves_ = {
        {EnemyType::Grunt, 8, 0.8f},
        {EnemyType::Fast, 10, 0.7f},
        {EnemyType::Tank, 6, 1.0f},
        {EnemyType::Grunt, 14, 0.55f},
        {EnemyType::Fast, 16, 0.5f},
        {EnemyType::Tank, 10, 0.8f},
    };
}

TowerDefenseWorld::EnemyType TowerDefenseWorld::parseEnemyType(const std::string& token) {
    if (token == "fast") {
        return EnemyType::Fast;
    }
    if (token == "tank") {
        return EnemyType::Tank;
    }
    return EnemyType::Grunt;
}

std::unique_ptr<TowerDefenseWorld::ITargetStrategy> TowerDefenseWorld::makeStrategy(int mode) {
    switch (mode % 3) {
    case 1:
        return std::make_unique<StrongestEnemyStrategy>();
    case 2:
        return std::make_unique<ClosestEnemyStrategy>();
    default:
        return std::make_unique<FirstEnemyStrategy>();
    }
}

const char* TowerDefenseWorld::towerTypeName(TowerType t) {
    switch (t) {
    case TowerType::Frost:
        return "Frost";
    case TowerType::Ember:
        return "Ember";
    default:
        return "Cannon";
    }
}

const char* TowerDefenseWorld::enemyTypeName(EnemyType t) {
    switch (t) {
    case EnemyType::Fast:
        return "Fast";
    case EnemyType::Tank:
        return "Tank";
    default:
        return "Grunt";
    }
}

bool TowerDefenseWorld::inBounds(Cell c) const { return c.x >= 0 && c.x < Cols && c.y >= 0 && c.y < Rows; }

int TowerDefenseWorld::towerIndexAt(Cell c) const {
    for (int i = 0; i < static_cast<int>(towers_.size()); ++i) {
        if (sameCell(towers_[static_cast<std::size_t>(i)].cell, c)) {
            return i;
        }
    }
    return -1;
}

const TowerDefenseWorld::Tower* TowerDefenseWorld::towerAt(Cell c) const {
    const int i = towerIndexAt(c);
    return i >= 0 ? &towers_[static_cast<std::size_t>(i)] : nullptr;
}

bool TowerDefenseWorld::isBlocked(Cell c) const { return towerIndexAt(c) >= 0; }

Vec2 TowerDefenseWorld::cellCenter(Cell c) const {
    return {c.x * TileSize + TileSize * 0.5f, c.y * TileSize + TileSize * 0.5f};
}

bool TowerDefenseWorld::recomputePath() {
    struct Node {
        int x = 0;
        int y = 0;
        float g = 0.f;
        float h = 0.f;
        float f = 0.f;
        int parent = -1;
    };
    struct OpenItem {
        float f = 0.f;
        int nodeIdx = -1;
        bool operator>(const OpenItem& other) const { return f > other.f; }
    };

    auto idx = [](Cell c) { return c.y * Cols + c.x; };
    debugOpen_.clear();
    debugClosed_.clear();

    std::array<float, Cols * Rows> bestG;
    std::array<bool, Cols * Rows> closed;
    bestG.fill(std::numeric_limits<float>::infinity());
    closed.fill(false);

    std::vector<Node> nodePool;
    nodePool.reserve(Cols * Rows * 2);
    std::priority_queue<OpenItem, std::vector<OpenItem>, std::greater<OpenItem>> open;

    Node startNode;
    startNode.x = start_.x;
    startNode.y = start_.y;
    startNode.g = 0.f;
    startNode.h = heuristic(start_, goal_);
    startNode.f = startNode.h;
    nodePool.push_back(startNode);
    open.push(OpenItem{startNode.f, 0});
    bestG[static_cast<std::size_t>(idx(start_))] = 0.f;

    constexpr std::array<Cell, 4> dirs = {{{0, -1}, {0, 1}, {-1, 0}, {1, 0}}};
    int goalNodeIdx = -1;

    while (!open.empty()) {
        const OpenItem item = open.top();
        open.pop();
        const Node current = nodePool[static_cast<std::size_t>(item.nodeIdx)];
        const Cell cc{current.x, current.y};
        const int cid = idx(cc);
        if (closed[static_cast<std::size_t>(cid)]) {
            continue;
        }
        closed[static_cast<std::size_t>(cid)] = true;
        debugClosed_.push_back(cc);

        if (sameCell(cc, goal_)) {
            goalNodeIdx = item.nodeIdx;
            break;
        }

        for (const Cell d : dirs) {
            const Cell n{cc.x + d.x, cc.y + d.y};
            if (!inBounds(n) || isBlocked(n)) {
                continue;
            }
            const int nid = idx(n);
            if (closed[static_cast<std::size_t>(nid)]) {
                continue;
            }
            const float newG = current.g + 1.f;
            if (newG >= bestG[static_cast<std::size_t>(nid)]) {
                continue;
            }
            bestG[static_cast<std::size_t>(nid)] = newG;

            Node neighbor;
            neighbor.x = n.x;
            neighbor.y = n.y;
            neighbor.g = newG;
            neighbor.h = heuristic(n, goal_);
            neighbor.f = neighbor.g + neighbor.h;
            neighbor.parent = item.nodeIdx;
            const int ni = static_cast<int>(nodePool.size());
            nodePool.push_back(neighbor);
            open.push(OpenItem{neighbor.f, ni});
            debugOpen_.push_back(n);
        }
    }

    if (goalNodeIdx < 0) {
        path_.clear();
        return false;
    }

    std::vector<Cell> out;
    for (int at = goalNodeIdx; at >= 0; at = nodePool[static_cast<std::size_t>(at)].parent) {
        const Node& n = nodePool[static_cast<std::size_t>(at)];
        out.push_back(Cell{n.x, n.y});
    }
    std::reverse(out.begin(), out.end());
    path_ = smoothPath(out);

    for (auto& e : enemies_) {
        e.pathIndex = nearestPathIndex(e.pos);
    }
    return true;
}

std::vector<TowerDefenseWorld::Cell> TowerDefenseWorld::smoothPath(const std::vector<Cell>& in) const {
    if (in.size() <= 2) {
        return in;
    }
    std::vector<Cell> out;
    out.push_back(in.front());
    for (std::size_t i = 1; i + 1 < in.size(); ++i) {
        const Cell a = in[i - 1];
        const Cell b = in[i];
        const Cell c = in[i + 1];
        if ((b.x - a.x) == (c.x - b.x) && (b.y - a.y) == (c.y - b.y)) {
            continue;
        }
        out.push_back(b);
    }
    out.push_back(in.back());
    return out;
}

float TowerDefenseWorld::heuristic(Cell a, Cell b) {
    return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

int TowerDefenseWorld::nearestPathIndex(Vec2 pos) const {
    if (path_.empty()) {
        return 0;
    }
    int best = 0;
    float bestD2 = 1e30f;
    for (int i = 0; i < static_cast<int>(path_.size()); ++i) {
        const Vec2 c = cellCenter(path_[static_cast<std::size_t>(i)]);
        const float dx = c.x - pos.x;
        const float dy = c.y - pos.y;
        const float d2 = dx * dx + dy * dy;
        if (d2 < bestD2) {
            bestD2 = d2;
            best = i;
        }
    }
    return best;
}

bool TowerDefenseWorld::placeTower(Cell cell, TowerType type) {
    if (gameOver() || victory()) {
        return false;
    }
    if (!inBounds(cell) || sameCell(cell, start_) || sameCell(cell, goal_) || towerIndexAt(cell) >= 0) {
        return false;
    }
    if (gold_ < buildCost_) {
        return false;
    }

    Tower tower;
    tower.cell = cell;
    tower.type = type;
    tower.level = 1;
    tower.cooldown = 0.f;
    tower.strategyIndex = 0;
    tower.strategy = makeStrategy(0);
    towers_.push_back(std::move(tower));
    if (!recomputePath()) {
        towers_.pop_back();
        return false;
    }
    gold_ -= buildCost_;
    return true;
}

bool TowerDefenseWorld::upgradeTower(Cell cell) {
    const int idx = towerIndexAt(cell);
    if (idx < 0 || gold_ < upgradeCost_) {
        return false;
    }
    Tower& t = towers_[static_cast<std::size_t>(idx)];
    if (t.level >= 3) {
        return false;
    }
    ++t.level;
    gold_ -= upgradeCost_;
    return true;
}

bool TowerDefenseWorld::cycleTowerTargeting(Cell cell) {
    const int idx = towerIndexAt(cell);
    if (idx < 0) {
        return false;
    }
    Tower& t = towers_[static_cast<std::size_t>(idx)];
    t.strategyIndex = (t.strategyIndex + 1) % 3;
    t.strategy = makeStrategy(t.strategyIndex);
    return true;
}

void TowerDefenseWorld::spawnEnemyFromWave(const WaveDefinition& def) {
    Enemy e;
    e.pos = cellCenter(start_);
    e.pathIndex = 0;
    e.type = def.type;
    e.alive = true;

    float baseHp = 55.f;
    float baseSpeed = 44.f;
    if (def.type == EnemyType::Fast) {
        baseHp = 38.f;
        baseSpeed = 70.f;
    } else if (def.type == EnemyType::Tank) {
        baseHp = 100.f;
        baseSpeed = 30.f;
    }

    const float waveScale = std::pow(1.15f, static_cast<float>(wave_ - 1));
    const float hardHpScale = (difficulty_ == GameDifficulty::Hard) ? 1.2f : 1.f;
    const float hardSpeedScale = (difficulty_ == GameDifficulty::Hard) ? 1.1f : 1.f;
    e.maxHp = baseHp * waveScale * hardHpScale;
    e.hp = e.maxHp;
    e.speed = baseSpeed * hardSpeedScale;
    enemies_.push_back(e);
}

void TowerDefenseWorld::updateWaves(float dt) {
    if (!waveInProgress_ && enemies_.empty()) {
        if (wave_ >= static_cast<int>(waves_.size())) {
            victory_ = true;
            return;
        }
        currentWave_ = waves_[static_cast<std::size_t>(wave_)];
        ++wave_;
        waveInProgress_ = true;
        spawnRemaining_ = currentWave_.count;
        spawnInterval_ = currentWave_.interval;
        if (difficulty_ == GameDifficulty::Hard) {
            spawnInterval_ *= 0.9f;
        }
        spawnTimer_ = 0.2f;
    }
    if (!waveInProgress_) {
        return;
    }

    spawnTimer_ -= dt;
    if (spawnTimer_ <= 0.f && spawnRemaining_ > 0) {
        spawnTimer_ = spawnInterval_;
        --spawnRemaining_;
        spawnEnemyFromWave(currentWave_);
    }
    if (spawnRemaining_ <= 0 && enemies_.empty()) {
        waveInProgress_ = false;
    }
}

float TowerDefenseWorld::tickStatusEffects(Enemy& e, float dt) {
    float speedMult = 1.f;
    for (auto& fx : e.effects) {
        fx.duration -= dt;
        if (fx.type == StatusEffectType::Slow) {
            speedMult *= clampf(1.f - fx.magnitude, 0.2f, 1.f);
        } else if (fx.type == StatusEffectType::Burn) {
            fx.tickAccumulator += dt;
            while (fx.tickAccumulator >= fx.tickPeriod) {
                fx.tickAccumulator -= fx.tickPeriod;
                e.hp -= fx.magnitude;
            }
        }
    }
    e.effects.erase(std::remove_if(e.effects.begin(), e.effects.end(), [](const StatusEffect& fx) { return fx.duration <= 0.f; }),
                    e.effects.end());
    return speedMult;
}

void TowerDefenseWorld::updateEnemies(float dt) {
    if (path_.size() < 2) {
        return;
    }
    for (auto& e : enemies_) {
        if (!e.alive) {
            continue;
        }

        if (e.hp <= 0.f) {
            e.alive = false;
            score_ += 12;
            gold_ += (difficulty_ == GameDifficulty::Hard) ? 10 : 12;
            continue;
        }

        if (e.pathIndex >= static_cast<int>(path_.size()) - 1) {
            e.alive = false;
            --lives_;
            continue;
        }

        const float speedMult = tickStatusEffects(e, dt);
        Vec2 target = cellCenter(path_[static_cast<std::size_t>(e.pathIndex + 1)]);
        Vec2 delta{target.x - e.pos.x, target.y - e.pos.y};
        const float len = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (len < 1e-4f) {
            ++e.pathIndex;
            continue;
        }
        const float step = e.speed * speedMult * dt;
        e.vel = {delta.x / len * (e.speed * speedMult), delta.y / len * (e.speed * speedMult)};
        if (step >= len) {
            e.pos = target;
            ++e.pathIndex;
        } else {
            e.pos.x += delta.x / len * step;
            e.pos.y += delta.y / len * step;
        }
    }
}

void TowerDefenseWorld::updateTowers(float dt) {
    for (auto& t : towers_) {
        t.cooldown -= dt;
        if (t.cooldown > 0.f) {
            continue;
        }

        const Vec2 origin = cellCenter(t.cell);
        const float range = (t.type == TowerType::Cannon ? 90.f : (t.type == TowerType::Frost ? 82.f : 88.f)) +
                            t.level * 14.f;
        const float rangeSq = range * range;
        std::vector<Enemy*> candidates;
        for (auto& e : enemies_) {
            if (!e.alive) {
                continue;
            }
            const float dx = e.pos.x - origin.x;
            const float dy = e.pos.y - origin.y;
            if (dx * dx + dy * dy <= rangeSq) {
                candidates.push_back(&e);
            }
        }
        if (candidates.empty()) {
            t.cooldown = 0.1f;
            continue;
        }
        Enemy* target = t.strategy ? t.strategy->selectTarget(candidates) : nullptr;
        if (!target) {
            continue;
        }

        const float projSpeed = (t.type == TowerType::Frost ? 230.f : 290.f) + t.level * 16.f;
        const float dx = target->pos.x - origin.x;
        const float dy = target->pos.y - origin.y;
        const float dist = std::sqrt(dx * dx + dy * dy);
        const float travel = dist / std::max(1.f, projSpeed);
        const Vec2 intercept = target->pos + target->vel * travel;
        Vec2 dir{intercept.x - origin.x, intercept.y - origin.y};
        const float dlen = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (dlen < 1e-4f) {
            dir = {1.f, 0.f};
        } else {
            dir = {dir.x / dlen, dir.y / dlen};
        }

        Projectile p;
        p.pos = origin;
        p.vel = dir * projSpeed;
        p.radius = 4.f;
        p.sourceType = t.type;
        p.alive = true;
        if (t.type == TowerType::Cannon) {
            p.damage = 21.f + t.level * 12.f;
            p.applyEffect = false;
        } else if (t.type == TowerType::Frost) {
            p.damage = 13.f + t.level * 7.f;
            p.applyEffect = true;
            p.effect.type = StatusEffectType::Slow;
            p.effect.magnitude = 0.18f + 0.05f * t.level;
            p.effect.duration = 1.2f + 0.2f * t.level;
            p.effect.tickPeriod = 0.25f;
        } else {
            p.damage = 11.f + t.level * 6.f;
            p.applyEffect = true;
            p.effect.type = StatusEffectType::Burn;
            p.effect.magnitude = 3.5f + 1.2f * t.level;
            p.effect.duration = 2.2f + 0.4f * t.level;
            p.effect.tickPeriod = 0.25f;
        }
        projectiles_.push_back(p);

        const float cooldownBase = (difficulty_ == GameDifficulty::Hard) ? 0.92f : 0.85f;
        t.cooldown = clampf(cooldownBase - t.level * 0.12f, 0.22f, cooldownBase);
    }
}

void TowerDefenseWorld::updateProjectiles(float dt) {
    for (auto& p : projectiles_) {
        if (!p.alive) {
            continue;
        }
        p.pos += p.vel * dt;
        if (p.pos.x < -20.f || p.pos.x > Width + 20.f || p.pos.y < -20.f || p.pos.y > Height + 20.f) {
            p.alive = false;
            continue;
        }

        for (auto& e : enemies_) {
            if (!e.alive) {
                continue;
            }
            const float dx = e.pos.x - p.pos.x;
            const float dy = e.pos.y - p.pos.y;
            const float rr = e.radius + p.radius;
            if (dx * dx + dy * dy > rr * rr) {
                continue;
            }
            e.hp -= p.damage;
            if (p.applyEffect) {
                e.effects.push_back(p.effect);
            }
            p.alive = false;
            break;
        }
    }

    projectiles_.erase(std::remove_if(projectiles_.begin(), projectiles_.end(), [](const Projectile& p) { return !p.alive; }),
                       projectiles_.end());
}

void TowerDefenseWorld::fixedUpdate(float dt) {
    if (gameOver() || victory()) {
        return;
    }
    updateWaves(dt);
    updateEnemies(dt);
    updateTowers(dt);
    updateProjectiles(dt);

    enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), [](const Enemy& e) { return !e.alive; }),
                   enemies_.end());
}

} // namespace mc
