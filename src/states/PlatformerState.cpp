#include "states/PlatformerState.hpp"

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
#include <string>

namespace mc {

void PlatformerState::onEnter(Application& app) {
    app_ = &app;
    world_.setDifficulty(app.difficulty());
    world_.reset();
    left_ = false;
    right_ = false;
    paused_ = false;
    pauseSelection_ = 0;
    scoreSubmitted_ = false;
    endFlashTime_ = 0.f;
    overlayFontReady_ = tryLoadSystemFont(overlayFont_);
    refreshHud();
}

void PlatformerState::handlePauseInput(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Key::P || key == sf::Keyboard::Key::Escape) {
        paused_ = false;
        return;
    }
    if (key == sf::Keyboard::Key::Up || key == sf::Keyboard::Key::W) {
        pauseSelection_ = (pauseSelection_ + 2) % 3;
        return;
    }
    if (key == sf::Keyboard::Key::Down || key == sf::Keyboard::Key::S) {
        pauseSelection_ = (pauseSelection_ + 1) % 3;
        return;
    }
    if (key != sf::Keyboard::Key::Enter && key != sf::Keyboard::Key::Space) {
        return;
    }

    if (pauseSelection_ == 0) {
        paused_ = false;
    } else if (pauseSelection_ == 1) {
        world_.setDifficulty(app_->difficulty());
        world_.reset();
        left_ = right_ = false;
        scoreSubmitted_ = false;
        endFlashTime_ = 0.f;
        paused_ = false;
    } else if (pauseSelection_ == 2 && app_) {
        app_->requestState(std::make_unique<MenuState>());
    }
}

void PlatformerState::handleInput(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (paused_) {
            handlePauseInput(key->code);
            return;
        }
        if (key->code == sf::Keyboard::Key::P) {
            paused_ = true;
            pauseSelection_ = 0;
            return;
        }
        if (key->code == sf::Keyboard::Key::Escape && app_) {
            app_->requestState(std::make_unique<MenuState>());
            return;
        }
        if (key->code == sf::Keyboard::Key::R) {
            world_.setDifficulty(app_->difficulty());
            world_.reset();
            scoreSubmitted_ = false;
            endFlashTime_ = 0.f;
        }
        if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A) {
            left_ = true;
        }
        if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D) {
            right_ = true;
        }
        if (key->code == sf::Keyboard::Key::Space || key->code == sf::Keyboard::Key::Up ||
            key->code == sf::Keyboard::Key::W) {
            world_.queueJump();
        }
    } else if (const auto* key = event.getIf<sf::Event::KeyReleased>()) {
        if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A) {
            left_ = false;
        }
        if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D) {
            right_ = false;
        }
        if (key->code == sf::Keyboard::Key::Space || key->code == sf::Keyboard::Key::Up ||
            key->code == sf::Keyboard::Key::W) {
            world_.releaseJump();
        }
    }
}

void PlatformerState::update(sf::Time fixedDt) {
    if (paused_) {
        refreshHud();
        return;
    }
    float dir = 0.f;
    if (left_) {
        dir -= 1.f;
    }
    if (right_) {
        dir += 1.f;
    }
    world_.setMoveInput(dir);
    world_.fixedUpdate(fixedDt.asSeconds());

    if (world_.gameOver() || world_.victory()) {
        endFlashTime_ += fixedDt.asSeconds();
        if (!scoreSubmitted_ && app_) {
            const std::string name =
                std::string("Platformer-") + difficultyName(app_->difficulty());
            app_->highScores().submit(name, world_.score());
            scoreSubmitted_ = true;
        }
    }
    refreshHud();
}

void PlatformerState::refreshHud() {
    std::ostringstream os;
    const int top = app_ ? app_->highScores().topScore() : 0;
    os << "Platformer[" << difficultyName(world_.difficulty()) << "] L" << world_.levelIndex() << "/"
       << world_.levelCount() << " S:" << world_.score() << " T:" << top << " HP:" << world_.lives();
    if (world_.victory()) {
        os << " | WIN R";
    } else if (world_.gameOver()) {
        os << " | LOSE R";
    } else if (paused_) {
        os << " | PAUSED";
    } else {
        os << " | A/D move Space jump P";
        if (world_.difficulty() == GameDifficulty::Hard) {
            os << " | Avoid red orbs";
        }
    }
    view_.setHudLine(os.str());
}

void PlatformerState::render(sf::RenderTarget& target) {
    view_.draw(target, world_);

    if (!overlayFontReady_ || !(world_.gameOver() || world_.victory())) {
        if (overlayFontReady_ && paused_) {
            sf::RectangleShape veil;
            veil.setSize({static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)});
            veil.setFillColor(sf::Color(0, 0, 0, 150));
            target.draw(veil);

            sf::Text title = makeText(overlayFont_, "PAUSED", 54u);
            title.setFillColor(sf::Color(235, 235, 245));
            title.setPosition({static_cast<float>(target.getSize().x) * 0.5f - 110.f, 180.f});
            target.draw(title);

            const char* opts[3] = {"Resume", "Restart", "Back to Menu"};
            for (int i = 0; i < 3; ++i) {
                sf::Text t = makeText(overlayFont_, std::string(i == pauseSelection_ ? "> " : "  ") + opts[i], 30u);
                t.setFillColor(i == pauseSelection_ ? sf::Color(120, 210, 255) : sf::Color(190, 195, 210));
                t.setPosition({static_cast<float>(target.getSize().x) * 0.5f - 130.f, 280.f + i * 46.f});
                target.draw(t);
            }
        }
        return;
    }
    if (static_cast<int>(endFlashTime_ * 5.f) % 2 != 0) {
        return;
    }
    sf::RectangleShape veil;
    veil.setSize({static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)});
    veil.setFillColor(sf::Color(0, 0, 0, 120));
    target.draw(veil);

    const char* label = world_.victory() ? "VICTORY!" : "GAME OVER";
    sf::Text text = makeText(overlayFont_, label, 64u);
    text.setFillColor(world_.victory() ? sf::Color(120, 255, 160) : sf::Color(255, 130, 130));
    const sf::FloatRect b = text.getLocalBounds();
    text.setOrigin({b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f});
    text.setPosition({static_cast<float>(target.getSize().x) * 0.5f, static_cast<float>(target.getSize().y) * 0.5f});
    target.draw(text);
}

} // namespace mc
