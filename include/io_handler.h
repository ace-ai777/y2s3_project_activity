#pragma once
#include <string>
#include <vector>
#include "statistics.h"
bool write_daily_csv(const std::string&, const std::vector<double>&, const std::vector<FinalStats>&);
bool write_summary_csv(const std::string&, const std::vector<double>&, const std::vector<FinalStats>&);
bool write_runs_csv(const std::string&, const std::vector<double>&,
                    const std::vector<std::vector<SimulationResult>>&);
