#include "states/BreakoutState.hpp"

#include "core/Application.hpp"
#include "core/SfmlCompat.hpp"
#include "core/SystemFont.hpp"
#include "states/MenuState.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <memory>
#include <sstream>

namespace mc {

void BreakoutState::onEnter(Application& app) {
    app_ = &app;
    world_.resetRound(true);
    moveLeft_ = moveRight_ = false;
    scoreSubmitted_ = false;
    endFlashTime_ = 0.f;
    overlayFontReady_ = tryLoadSystemFont(overlayFont_);
    refreshHud();
}

void BreakoutState::handleInput(const sf::Event& event) {
    if (const auto* pressed = event.getIf<sf::Event::KeyPressed>()) {
        if (pressed->code == sf::Keyboard::Key::Escape && app_) {
            app_->requestState(std::make_unique<MenuState>());
            return;
        }
        if (pressed->code == sf::Keyboard::Key::Space) {
            world_.tryLaunchBall();
        }
        if (pressed->code == sf::Keyboard::Key::R) {
            world_.resetRound(true);
            scoreSubmitted_ = false;
            endFlashTime_ = 0.f;
        }
        if (pressed->code == sf::Keyboard::Key::Left || pressed->code == sf::Keyboard::Key::A) {
            moveLeft_ = true;
        }
        if (pressed->code == sf::Keyboard::Key::Right || pressed->code == sf::Keyboard::Key::D) {
            moveRight_ = true;
        }
    } else if (const auto* released = event.getIf<sf::Event::KeyReleased>()) {
        if (released->code == sf::Keyboard::Key::Left || released->code == sf::Keyboard::Key::A) {
            moveLeft_ = false;
        }
        if (released->code == sf::Keyboard::Key::Right || released->code == sf::Keyboard::Key::D) {
            moveRight_ = false;
        }
    }
}

void BreakoutState::update(sf::Time fixedDt) {
    float dir = 0.f;
    if (moveLeft_) {
        dir -= 1.f;
    }
    if (moveRight_) {
        dir += 1.f;
    }
    world_.setPaddleDir(dir);
    world_.fixedUpdate(fixedDt.asSeconds());
    if (world_.gameOver() || world_.cleared()) {
        endFlashTime_ += fixedDt.asSeconds();
        if (!scoreSubmitted_ && app_) {
            app_->highScores().submit("Breakout", world_.score());
            scoreSubmitted_ = true;
        }
    }
    refreshHud();
}

void BreakoutState::refreshHud() {
    std::ostringstream os;
    const int top = app_ ? app_->highScores().topScore() : 0;
    os << "Breakout L" << world_.levelIndex() << "/" << world_.levelCount() << " S:" << world_.score() << " T:" << top
       << " HP:" << world_.lives();
    if (world_.widePaddleTimeLeft() > 0.f) {
        os << " WIDE";
    }
    if (world_.bigBallTimeLeft() > 0.f) {
        os << " BIG";
    }
    if (world_.cleared()) {
        os << " | WIN R";
    } else if (world_.gameOver()) {
        os << " | LOSE R";
    } else {
        os << " | Space serve Esc menu";
    }
    view_.setStatusText(os.str());
}

void BreakoutState::render(sf::RenderTarget& target) {
    view_.draw(target, world_);

    if (!overlayFontReady_) {
        return;
    }
    if (!(world_.gameOver() || world_.cleared())) {
        return;
    }
    if (static_cast<int>(endFlashTime_ * 5.f) % 2 != 0) {
        return;
    }

    sf::RectangleShape veil;
    veil.setSize({static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)});
    veil.setFillColor(sf::Color(0, 0, 0, 120));
    target.draw(veil);

    const char* label = world_.cleared() ? "VICTORY!" : "GAME OVER";
    sf::Text text = makeText(overlayFont_, label, 64u);
    text.setFillColor(world_.cleared() ? sf::Color(110, 255, 150) : sf::Color(255, 120, 120));
    const sf::FloatRect b = text.getLocalBounds();
    text.setOrigin({b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f});
    text.setPosition({static_cast<float>(target.getSize().x) * 0.5f, static_cast<float>(target.getSize().y) * 0.5f});
    target.draw(text);
}

} // namespace mc
