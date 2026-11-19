#include "simulation.h"
#include "user.h"
#include "random_utils.h"
#include <algorithm>
#include <vector>
#include <cmath>
#include <stdexcept>
namespace {
bool random_attempt(int, int sender, int& recipient, double probability) {
    recipient = get_random_recipient(sender);
    return check_probability(probability);
}
}
SimulationResult run_single_simulation(double probability) {
    return run_single_simulation(probability, random_attempt);
}
SimulationResult run_single_simulation(double probability, TransmissionAttempt attempt) {
    if (!attempt || !std::isfinite(probability) || probability < 0 || probability > 1)
        throw std::invalid_argument("Invalid simulation parameters");
    SimulationResult result{};
    std::vector<User> users(TOTAL_USERS);
    for (int i = 0; i < TOTAL_USERS; ++i) {
        users[i].id = i;
        if (i < INITIAL_INFORMED) {
            users[i].is_informed = true;
            users[i].days_spreading_left = SPREAD_DAYS;
        }
    }
    int informed = INITIAL_INFORMED;
    for (int day = 0; day < SIMULATION_DAYS; ++day) {
        std::vector<int> active;
        for (const auto& user : users)
            if (user.is_informed && user.days_spreading_left > 0) active.push_back(user.id);
        for (int sender : active) {
            for (int n = 0; n < ATTEMPTS_PER_DAY && informed < TOTAL_USERS; ++n) {
                int recipient = -1;
                const bool success = attempt(day + 1, sender, recipient, probability);
                if (recipient < 0 || recipient >= TOTAL_USERS || recipient == sender)
                    throw std::logic_error("Invalid recipient returned by attempt");
                if (success && !users[recipient].is_informed) {
                    users[recipient].is_informed = true;
                    users[recipient].days_spreading_left = SPREAD_DAYS;
                    ++informed;
                }
            }
            --users[sender].days_spreading_left;
            if (informed == TOTAL_USERS) break;
        }
        result.informed_per_day[day] = informed;
        if (!result.reached_100 && informed >= REACH_TARGET) {
            result.reached_100 = true;
            result.day_reached_100 = day + 1;
        }
        if (informed == TOTAL_USERS || active.empty()) {
            std::fill(result.informed_per_day + day + 1,
                      result.informed_per_day + SIMULATION_DAYS, informed);
            break;
        }
    }
    return result;
}
