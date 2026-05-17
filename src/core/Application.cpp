#include "core/Application.hpp"

#include "states/MenuState.hpp"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include <chrono>
#include <optional>
#include <thread>

namespace mc {

Application::Application()
    : window_(sf::VideoMode({960u, 720u}), "MiniConsole"),
      fixedClock_(sf::seconds(1.f / 60.f)),
      scores_("highscores.txt") {
    window_.setVerticalSyncEnabled(true);
    window_.setKeyRepeatEnabled(false);
    scores_.load();
    state_ = std::make_unique<MenuState>();
    state_->onEnter(*this);
}

void Application::quit() { running_ = false; }

void Application::requestState(std::unique_ptr<GameState> nextState) {
    pending_ = std::move(nextState);
}

void Application::applyPendingState() {
    if (!pending_) {
        return;
    }
    if (state_) {
        state_->onExit(*this);
    }
    state_ = std::move(*pending_);
    pending_.reset();
    if (state_) {
        state_->onEnter(*this);
    }
}

bool Application::runFrame() {
    while (const std::optional<sf::Event> event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            running_ = false;
        }
        if (state_) {
            state_->handleInput(*event);
        }
    }

    applyPendingState();
    if (!state_) {
        return false;
    }

    const sf::Time realDelta = frameClock_.restart();
    const int steps = fixedClock_.consumeRealDelta(realDelta);
    const sf::Time step = fixedClock_.fixedStep();
    for (int i = 0; i < steps; ++i) {
        state_->update(step);
        applyPendingState();
        if (!state_) {
            return false;
        }
    }

    if (!running_ || !state_) {
        return false;
    }

    window_.clear(sf::Color(18, 18, 24));
    state_->render(window_);
    window_.display();

    // Gentle throttle if vsync is off on some drivers.
#ifndef __EMSCRIPTEN__
    const sf::Time frameTime = frameClock_.getElapsedTime();
    if (frameTime < sf::milliseconds(1)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
#endif
    return running_ && window_.isOpen();
}

void Application::run() {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(
        [](void* userData) {
            auto* app = static_cast<Application*>(userData);
            if (!app->runFrame()) {
                emscripten_cancel_main_loop();
            }
        },
        this, 0, true);
#else
    while (running_ && window_.isOpen()) {
        if (!runFrame()) {
            break;
        }
    }
#endif
}

} // namespace mc
