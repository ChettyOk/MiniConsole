#include "states/PacmanState.hpp"

#include "core/Application.hpp"
#include "core/SystemFont.hpp"
#include "states/MenuState.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <memory>
#include <sstream>

namespace mc {

namespace {

void fitTextToWidth(sf::Text& text, float maxWidth, unsigned int maxSize, unsigned int minSize = 12u) {
    text.setCharacterSize(maxSize);
    while (text.getCharacterSize() > minSize && text.getLocalBounds().size.x > maxWidth) {
        text.setCharacterSize(text.getCharacterSize() - 1u);
    }
}

} // namespace

void PacmanState::onEnter(Application& app) {
    app_ = &app;
    world_.reset();
    inAttractMode_ = true;
    attractTime_ = 0.f;
    attractBlinkTime_ = 0.f;
    scoreSubmitted_ = false;
    endFlashTime_ = 0.f;
    overlayFontReady_ = tryLoadSystemFont(overlayFont_);
    refreshHud();
}

void PacmanState::handleInput(const sf::Event& event) {
    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key) {
        return;
    }

    if (key->code == sf::Keyboard::Key::Escape && app_) {
        app_->requestState(std::make_unique<MenuState>());
        return;
    }
    if (inAttractMode_) {
        if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
            inAttractMode_ = false;
            world_.reset();
            scoreSubmitted_ = false;
            endFlashTime_ = 0.f;
        }
        return;
    }
    if (key->code == sf::Keyboard::Key::R) {
        world_.reset();
        scoreSubmitted_ = false;
        endFlashTime_ = 0.f;
        return;
    }
    if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up) {
        world_.setDesiredDirection(PacmanWorld::Direction::Up);
    } else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down) {
        world_.setDesiredDirection(PacmanWorld::Direction::Down);
    } else if (key->code == sf::Keyboard::Key::A || key->code == sf::Keyboard::Key::Left) {
        world_.setDesiredDirection(PacmanWorld::Direction::Left);
    } else if (key->code == sf::Keyboard::Key::D || key->code == sf::Keyboard::Key::Right) {
        world_.setDesiredDirection(PacmanWorld::Direction::Right);
    }
}

void PacmanState::updateAttractMode(float dt) {
    attractTime_ += dt;
    attractBlinkTime_ += dt;
    const int phase = static_cast<int>(attractTime_) % 8;
    if (phase < 2) {
        world_.setDesiredDirection(PacmanWorld::Direction::Left);
    } else if (phase < 4) {
        world_.setDesiredDirection(PacmanWorld::Direction::Up);
    } else if (phase < 6) {
        world_.setDesiredDirection(PacmanWorld::Direction::Right);
    } else {
        world_.setDesiredDirection(PacmanWorld::Direction::Down);
    }
    world_.fixedUpdate(dt);
    if (world_.gameOver() || world_.victory()) {
        world_.reset();
    }
}

void PacmanState::update(sf::Time fixedDt) {
    if (inAttractMode_) {
        updateAttractMode(fixedDt.asSeconds());
        refreshHud();
        return;
    }

    world_.fixedUpdate(fixedDt.asSeconds());
    if (world_.gameOver() || world_.victory()) {
        endFlashTime_ += fixedDt.asSeconds();
        if (!scoreSubmitted_ && app_) {
            app_->highScores().submit("Pacman", world_.score());
            scoreSubmitted_ = true;
        }
    }
    refreshHud();
}

void PacmanState::refreshHud() {
    std::ostringstream os;
    const int top = app_ ? app_->highScores().topScore() : 0;
    os << "Pac-Man S:" << world_.score() << " T:" << top << " HP:" << world_.lives() << " "
       << PacmanWorld::modeName(world_.globalMode());
    if (inAttractMode_) {
        os << " | Demo Enter/Space";
    } else if (world_.victory()) {
        os << " | WIN R";
    } else if (world_.gameOver()) {
        os << " | LOSE R";
    } else {
        os << " | Move WASD Esc";
    }
    view_.setHudLine(os.str());
}

void PacmanState::render(sf::RenderTarget& target) {
    view_.draw(target, world_);
    if (overlayFontReady_ && inAttractMode_) {
        sf::RectangleShape veil({static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)});
        veil.setFillColor(sf::Color(0, 0, 0, 110));
        target.draw(veil);

        sf::Text title(overlayFont_, "PAC-MAN", 68u);
        title.setFillColor(sf::Color(255, 230, 80));
        fitTextToWidth(title, static_cast<float>(target.getSize().x) - 40.f, 68u, 36u);
        const sf::FloatRect tb = title.getLocalBounds();
        title.setOrigin({tb.position.x + tb.size.x * 0.5f, tb.position.y + tb.size.y * 0.5f});
        title.setPosition({static_cast<float>(target.getSize().x) * 0.5f, 150.f});
        target.draw(title);

        sf::Text subtitle(overlayFont_, "Ghost AI demo running", 28u);
        subtitle.setFillColor(sf::Color(170, 185, 210));
        fitTextToWidth(subtitle, static_cast<float>(target.getSize().x) - 60.f, 28u, 16u);
        const sf::FloatRect sb = subtitle.getLocalBounds();
        subtitle.setOrigin({sb.position.x + sb.size.x * 0.5f, sb.position.y + sb.size.y * 0.5f});
        subtitle.setPosition({static_cast<float>(target.getSize().x) * 0.5f, 216.f});
        target.draw(subtitle);

        if (static_cast<int>(attractBlinkTime_ * 2.f) % 2 == 0) {
            sf::Text prompt(overlayFont_, "Press Enter/Space to Start", 30u);
            prompt.setFillColor(sf::Color(120, 220, 255));
            fitTextToWidth(prompt, static_cast<float>(target.getSize().x) - 80.f, 30u, 16u);
            const sf::FloatRect pb = prompt.getLocalBounds();
            prompt.setOrigin({pb.position.x + pb.size.x * 0.5f, pb.position.y + pb.size.y * 0.5f});
            prompt.setPosition({static_cast<float>(target.getSize().x) * 0.5f, static_cast<float>(target.getSize().y) - 120.f});
            target.draw(prompt);
        }
        return;
    }
    if (!overlayFontReady_ || !(world_.gameOver() || world_.victory())) {
        return;
    }
    if (static_cast<int>(endFlashTime_ * 5.f) % 2 != 0) {
        return;
    }
    sf::RectangleShape veil({static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y)});
    veil.setFillColor(sf::Color(0, 0, 0, 120));
    target.draw(veil);

    const char* label = world_.victory() ? "VICTORY!" : "GAME OVER";
    sf::Text text(overlayFont_, label, 64u);
    text.setFillColor(world_.victory() ? sf::Color(120, 255, 160) : sf::Color(255, 130, 130));
    const sf::FloatRect b = text.getLocalBounds();
    text.setOrigin({b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f});
    text.setPosition({static_cast<float>(target.getSize().x) * 0.5f, static_cast<float>(target.getSize().y) * 0.5f});
    target.draw(text);
}

} // namespace mc
