#pragma once
#include <vector>
#include "user.h"

struct SimulationResult {
    int informed_per_day[30]; 
    bool reached_100 = false; 
    int day_reached_100 = -1; 
};

SimulationResult run_single_simulation(double spread_probability);
