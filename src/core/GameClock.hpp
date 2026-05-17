#pragma once

#include <SFML/System/Time.hpp>

namespace mc {

// Fixed-step simulation with leftover time carried in an accumulator. Gameplay
// systems should only advance in multiples of `fixedDt` to keep physics stable
// across machines with different refresh rates.
class GameClock {
public:
    explicit GameClock(sf::Time fixedStep) : fixedDt_(fixedStep) {}

    void setFixedStep(sf::Time step) { fixedDt_ = step; }
    sf::Time fixedStep() const { return fixedDt_; }

    // Returns how many fixed updates to run this frame (may be 0, 1, or more).
    int consumeRealDelta(sf::Time realDelta) {
        accumulator_ += realDelta;
        int steps = 0;
        while (accumulator_ >= fixedDt_) {
            accumulator_ -= fixedDt_;
            ++steps;
            // Spiral of death guard: drop excess time instead of freezing.
            if (steps >= maxStepsPerFrame_) {
                accumulator_ = sf::Time{};
                break;
            }
        }
        return steps;
    }

private:
    sf::Time fixedDt_;
    sf::Time accumulator_{sf::Time{}};
    static constexpr int maxStepsPerFrame_ = 6;
};

} // namespace mc
