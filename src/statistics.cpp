#include "statistics.h"
FinalStats calculate_statistics(const std::vector<SimulationResult>& all_runs) {
    FinalStats stats{};
    stats.runs_count = static_cast<int>(all_runs.size());
    if (all_runs.empty()) return stats;
    double day_sum = 0;
    for (const auto& run : all_runs) {
        int first_day = -1;
        for (int day = 0; day < SIMULATION_DAYS; ++day) {
            stats.avg_informed_per_day[day] += run.informed_per_day[day];
            if (first_day < 0 && run.informed_per_day[day] >= REACH_TARGET) first_day = day + 1;
        }
        if (first_day > 0) { ++stats.runs_reached_100; day_sum += first_day; }
    }
    for (double& average : stats.avg_informed_per_day) average /= all_runs.size();
    stats.percent_reached_100 = 100.0 * stats.runs_reached_100 / all_runs.size();
    if (stats.runs_reached_100) stats.avg_day_reached_100 = day_sum / stats.runs_reached_100;
    return stats;
}
