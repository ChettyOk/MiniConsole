#include "states/ShooterState.hpp"

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

void ShooterState::onEnter(Application& app) {
    app_ = &app;
    world_.reset();
    up_ = down_ = left_ = right_ = fire_ = false;
    scoreSubmitted_ = false;
    endFlashTime_ = 0.f;
    overlayFontReady_ = tryLoadSystemFont(overlayFont_);
    refreshHud();
}

void ShooterState::recomputeMoveInput() {
    float x = 0.f;
    float y = 0.f;
    if (left_) {
        x -= 1.f;
    }
    if (right_) {
        x += 1.f;
    }
    if (up_) {
        y -= 1.f;
    }
    if (down_) {
        y += 1.f;
    }
    world_.setMoveInput(x, y);
}

void ShooterState::handleInput(const sf::Event& event) {
    const auto setKey = [&](sf::Keyboard::Key key, bool down) {
        switch (key) {
        case sf::Keyboard::Key::Escape:
            if (down && app_) {
                app_->requestState(std::make_unique<MenuState>());
            }
            break;
        case sf::Keyboard::Key::R:
            if (down) {
                world_.reset();
                scoreSubmitted_ = false;
                endFlashTime_ = 0.f;
            }
            break;
        case sf::Keyboard::Key::W:
        case sf::Keyboard::Key::Up:
            up_ = down;
            break;
        case sf::Keyboard::Key::S:
        case sf::Keyboard::Key::Down:
            down_ = down;
            break;
        case sf::Keyboard::Key::A:
        case sf::Keyboard::Key::Left:
            left_ = down;
            break;
        case sf::Keyboard::Key::D:
        case sf::Keyboard::Key::Right:
            right_ = down;
            break;
        case sf::Keyboard::Key::Space:
        case sf::Keyboard::Key::Z:
            fire_ = down;
            break;
        default:
            break;
        }
    };

    if (const auto* pressed = event.getIf<sf::Event::KeyPressed>()) {
        setKey(pressed->code, true);
    } else if (const auto* released = event.getIf<sf::Event::KeyReleased>()) {
        setKey(released->code, false);
    }

    world_.setFireHeld(fire_);
    recomputeMoveInput();
}

void ShooterState::update(sf::Time fixedDt) {
    world_.setFireHeld(fire_);
    recomputeMoveInput();
    world_.fixedUpdate(fixedDt.asSeconds());
    if (world_.gameOver()) {
        endFlashTime_ += fixedDt.asSeconds();
        if (!scoreSubmitted_ && app_) {
            app_->highScores().submit("Shooter", world_.score());
            scoreSubmitted_ = true;
        }
    }
    refreshHud();
}

void ShooterState::refreshHud() {
    std::ostringstream os;
    const int top = app_ ? app_->highScores().topScore() : 0;
    os << "Shooter S:" << world_.score() << " T:" << top << " HP:" << world_.lives();
    if (world_.gameOver()) {
        os << " | LOSE R Esc";
    } else {
        os << " | Move WASD Fire Space Esc";
    }
    view_.setHudLine(os.str());
}

void ShooterState::render(sf::RenderTarget& target) {
    view_.draw(target, world_);
    if (!overlayFontReady_ || !world_.gameOver()) {
        return;
    }
    if (static_cast<int>(endFlashTime_ * 5.f) % 2 != 0) {
        return;
    }

    sf::RectangleShape veil;
    veil.setSize({static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)});
    veil.setFillColor(sf::Color(0, 0, 0, 120));
    target.draw(veil);

    sf::Text text = makeText(overlayFont_, "GAME OVER", 64u);
    text.setFillColor(sf::Color(255, 120, 120));
    const sf::FloatRect b = text.getLocalBounds();
    text.setOrigin({b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f});
    text.setPosition({static_cast<float>(target.getSize().x) * 0.5f, static_cast<float>(target.getSize().y) * 0.5f});
    target.draw(text);
}

} // namespace mc
