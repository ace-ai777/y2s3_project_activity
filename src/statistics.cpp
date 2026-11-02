
#include "statistics.h"
using namespace std;
FinalStats calculate_statistics(const vector<SimulationResult>& all_runs) {
    FinalStats stats;

    // обнуляем массив
    for (int day = 0; day < NUM_DAYS; day++) {
        stats.avg_informed_per_day[day] = 0.0;
    }
    stats.runs_reached_100 = 0;
    stats.percent_reached_100 = 0.0;
    stats.avg_day_reached_100 = -1;

    // сумма информированных по дням
    for (const SimulationResult& run : all_runs) {
        for (int day = 0; day < NUM_DAYS; day++) {
            stats.avg_informed_per_day[day] += run.informed_per_day[day];
        }
    }
    // сделать:

    //поделить каждую сумму на количество запусков (all_runs.size()),
    //не забыть про пустой вектор и про double

    //посчитать, в скольких запусках reached_100 == true -> runs_reached_100
    // посчитать percent_reached_100
    // суммировать day_reached_100 только для успешных запусков
    //поделить на runs_reached_100 (если успешных 0 -> оставить -1)

    return stats;
}