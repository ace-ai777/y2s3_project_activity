#pragma once
#include <vector>
#include "simulation.h"
struct FinalStats {
    double avg_informed_per_day[SIMULATION_DAYS]{};
    int runs_count = 0;
    int runs_reached_100 = 0;
    double percent_reached_100 = 0;
    double avg_day_reached_100 = -1;
};
FinalStats calculate_statistics(const std::vector<SimulationResult>& all_runs);
