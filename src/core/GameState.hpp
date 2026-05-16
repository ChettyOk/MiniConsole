#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

namespace mc {

class Application;

// Pure gameplay orchestration lives in per-game "World" types; states wire input
// and delegate update/render to worlds and views. Rendering uses SFML only here
// at the boundary so entity logic can stay free of draw calls.
class GameState {
public:
    virtual ~GameState() = default;

    virtual void onEnter(Application& app) { (void)app; }
    virtual void onExit(Application& app) { (void)app; }

    virtual void handleInput(const sf::Event& event) = 0;
    virtual void update(sf::Time fixedDt) = 0;
    virtual void render(sf::RenderTarget& target) = 0;
};

} // namespace mc
