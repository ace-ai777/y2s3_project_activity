#pragma once
#include "config.h"
struct SimulationResult {
    int informed_per_day[SIMULATION_DAYS]{};
    bool reached_100 = false;
    int day_reached_100 = -1;
};
SimulationResult run_single_simulation(double spread_probability);
// Deterministic attempt injection for isolated model tests.
using TransmissionAttempt = bool (*)(int day, int sender, int& recipient, double probability);
SimulationResult run_single_simulation(double spread_probability, TransmissionAttempt attempt);
