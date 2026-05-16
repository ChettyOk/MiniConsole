#include "states/MinesweeperState.hpp"

#include "core/Application.hpp"
#include "core/SystemFont.hpp"
#include "states/MenuState.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>

namespace mc {

const char* MinesweeperState::difficultyName(MinesweeperWorld::Difficulty d) {
    if (d == MinesweeperWorld::Difficulty::Beginner) {
        return "Beginner";
    }
    if (d == MinesweeperWorld::Difficulty::Intermediate) {
        return "Intermediate";
    }
    return "Expert";
}

void MinesweeperState::onEnter(Application& app) {
    app_ = &app;
    scoreSubmitted_ = false;
    endFlashTime_ = 0.f;
    hoverX_ = hoverY_ = -1;
    overlayFontReady_ = tryLoadSystemFont(overlayFont_);
    world_.setDifficulty(preset_);
    refreshHud();
}

void MinesweeperState::recomputeBoardGeometry(const sf::RenderTarget& target) {
    if (world_.difficulty() == MinesweeperWorld::Difficulty::Beginner) {
        cellSize_ = 40.f;
    } else if (world_.difficulty() == MinesweeperWorld::Difficulty::Intermediate) {
        cellSize_ = 30.f;
    } else {
        cellSize_ = 24.f;
    }
    const float availW = std::max(220.f, static_cast<float>(target.getSize().x) - 40.f);
    const float availH = std::max(220.f, static_cast<float>(target.getSize().y) - 150.f);
    const float maxCellW = availW / static_cast<float>(world_.cols());
    const float maxCellH = availH / static_cast<float>(world_.rows());
    cellSize_ = std::max(12.f, std::min(cellSize_, std::min(maxCellW, maxCellH)));

    const float boardW = world_.cols() * cellSize_;
    const float boardH = world_.rows() * cellSize_;
    boardOriginX_ = std::max(10.f, (static_cast<float>(target.getSize().x) - boardW) * 0.5f);
    boardOriginY_ = std::max(56.f, (static_cast<float>(target.getSize().y) - boardH) * 0.56f);
}

bool MinesweeperState::mouseToCell(sf::Vector2i pixel, int& outX, int& outY) const {
    const float fx = static_cast<float>(pixel.x) - boardOriginX_;
    const float fy = static_cast<float>(pixel.y) - boardOriginY_;
    if (fx < 0.f || fy < 0.f) {
        return false;
    }
    const int x = static_cast<int>(fx / cellSize_);
    const int y = static_cast<int>(fy / cellSize_);
    if (x < 0 || x >= world_.cols() || y < 0 || y >= world_.rows()) {
        return false;
    }
    outX = x;
    outY = y;
    return true;
}

void MinesweeperState::handleInput(const sf::Event& event) {
    if (const auto* move = event.getIf<sf::Event::MouseMoved>()) {
        int cx = -1;
        int cy = -1;
        if (mouseToCell(move->position, cx, cy)) {
            hoverX_ = cx;
            hoverY_ = cy;
        } else {
            hoverX_ = hoverY_ = -1;
        }
    }

    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        int cx = -1;
        int cy = -1;
        if (!mouseToCell(mouse->position, cx, cy)) {
            return;
        }
        if (mouse->button == sf::Mouse::Button::Left) {
            world_.reveal(cx, cy);
        } else if (mouse->button == sf::Mouse::Button::Right) {
            world_.toggleFlag(cx, cy);
        } else if (mouse->button == sf::Mouse::Button::Middle) {
            world_.chordReveal(cx, cy);
        }
        return;
    }

    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key) {
        return;
    }
    if (key->code == sf::Keyboard::Key::Escape && app_) {
        app_->requestState(std::make_unique<MenuState>());
        return;
    }
    if (key->code == sf::Keyboard::Key::R) {
        world_.reset();
        scoreSubmitted_ = false;
        endFlashTime_ = 0.f;
        return;
    }
    if (key->code == sf::Keyboard::Key::Num1) {
        world_.setDifficulty(MinesweeperWorld::Difficulty::Beginner);
        scoreSubmitted_ = false;
        endFlashTime_ = 0.f;
    } else if (key->code == sf::Keyboard::Key::Num2) {
        world_.setDifficulty(MinesweeperWorld::Difficulty::Intermediate);
        scoreSubmitted_ = false;
        endFlashTime_ = 0.f;
    } else if (key->code == sf::Keyboard::Key::Num3) {
        world_.setDifficulty(MinesweeperWorld::Difficulty::Expert);
        scoreSubmitted_ = false;
        endFlashTime_ = 0.f;
    }
}

void MinesweeperState::update(sf::Time fixedDt) {
    world_.fixedUpdate(fixedDt.asSeconds());
    if (world_.status() != MinesweeperWorld::Status::Playing) {
        endFlashTime_ += fixedDt.asSeconds();
        if (!scoreSubmitted_ && app_ && world_.status() == MinesweeperWorld::Status::Win) {
            const int base = 5000;
            const int mineBonus = world_.mineCount() * 12;
            const int timePenalty = static_cast<int>(world_.elapsedSeconds() * 4.f);
            const int score = std::max(100, base + mineBonus - timePenalty);
            app_->highScores().submit("Minesweeper", score);
            scoreSubmitted_ = true;
        }
    }
    refreshHud();
}

void MinesweeperState::refreshHud() {
    std::ostringstream os;
    os << "Minesweeper[" << difficultyName(world_.difficulty()) << "] M:" << world_.mineCount() << " F:"
       << world_.flaggedCount() << " Safe:" << world_.remainingSafeCells() << " T:"
       << static_cast<int>(world_.elapsedSeconds()) << "s";
    const int best = world_.bestTime(world_.difficulty());
    if (best > 0) {
        os << " B:" << best << "s";
    }
    if (world_.status() == MinesweeperWorld::Status::Win) {
        os << " | WIN R";
    } else if (world_.status() == MinesweeperWorld::Status::Lose) {
        os << " | LOSE R";
    } else {
        os << " | LMB open RMB flag MMB chord 1/2/3 Esc";
    }
    view_.setHudLine(os.str());
}

void MinesweeperState::render(sf::RenderTarget& target) {
    recomputeBoardGeometry(target);
    view_.draw(target, world_, boardOriginX_, boardOriginY_, cellSize_, true, hoverX_, hoverY_);
    if (!overlayFontReady_ || world_.status() == MinesweeperWorld::Status::Playing) {
        return;
    }
    if (static_cast<int>(endFlashTime_ * 5.f) % 2 != 0) {
        return;
    }
    sf::RectangleShape veil({static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)});
    veil.setFillColor(sf::Color(0, 0, 0, 110));
    target.draw(veil);

    const char* label = world_.status() == MinesweeperWorld::Status::Win ? "VICTORY!" : "GAME OVER";
    sf::Text text(overlayFont_, label, 60u);
    text.setFillColor(world_.status() == MinesweeperWorld::Status::Win ? sf::Color(120, 255, 160)
                                                                        : sf::Color(255, 130, 130));
    const sf::FloatRect b = text.getLocalBounds();
    text.setOrigin({b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f});
    text.setPosition({static_cast<float>(target.getSize().x) * 0.5f, 78.f});
    target.draw(text);
}

} // namespace mc
