#include "config.h"
#include "simulation.h"
#include "statistics.h"
#include "io_handler.h"
#include "random_utils.h"
#include "table_formatter.h"
#include <iostream>
#include <exception>
int main() {
    try {
        setup_console();
        init_random();
        const std::vector<double> probabilities(PROBABILITIES, PROBABILITIES + NUM_PROBABILITIES);
        std::vector<FinalStats> stats;
        std::vector<std::vector<SimulationResult>> runs;
        for (double probability : probabilities) {
            runs.emplace_back();
            runs.back().reserve(RUNS_COUNT);
            for (int i = 0; i < RUNS_COUNT; ++i)
                runs.back().push_back(run_single_simulation(probability));
            stats.push_back(calculate_statistics(runs.back()));
            print_pretty_table(probability, stats.back());
        }
        if (!write_daily_csv(CSV_DAILY_FILE, probabilities, stats) ||
            !write_summary_csv(CSV_SUMMARY_FILE, probabilities, stats) ||
            !write_runs_csv(CSV_RUNS_FILE, probabilities, runs)) {
            std::cerr << "Ошибка записи CSV в текущий рабочий каталог.\n";
            return 1;
        }
        std::cout << "CSV сохранены в текущем рабочем каталоге: " << CSV_DAILY_FILE
                  << ", " << CSV_SUMMARY_FILE << ", " << CSV_RUNS_FILE << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
