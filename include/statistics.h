#pragma once
#include <vector>
#include "simulation.h"

struct FinalStats {
    double avg_informed_per_day[30]; 
    int runs_reached_100 = 0;        
    double avg_day_reached_100 = -1; // -1 означает "не достигнуто"
};

FinalStats calculate_statistics(const std::vector<SimulationResult>& all_runs);
