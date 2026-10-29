#include <iostream>
#include <vector>
#include "config.h"
#include "simulation.h"
#include "statistics.h"
#include "io_handler.h"
#include "random_utils.h"
#include "table_formatter.h"

int main() {
    init_random();
    std::string csv_filename = "results.csv";
    
    // Массив вероятностей (10%, 30%, 50%)
    double probabilities[] = {0.1, 0.3, 0.5};

    for (double prob : probabilities) {
        std::cout << "Running simulation for probability: " << prob * 100 << "%\n";
        std::vector<SimulationResult> all_runs;
        
        // 1000 прогонов
        for (int i = 0; i < RUNS_COUNT; ++i) {
            all_runs.push_back(run_single_simulation(prob));
        }

        // Считаем статистику (Влад)
        FinalStats stats = calculate_statistics(all_runs);
        
        // Выводим в консоль (Катя) и сохраняем (Маша)
        print_pretty_table(prob, stats);
        print_summary(prob, stats); 
        append_stats_to_csv(prob, stats, csv_filename);
    }

    // КОНТРОЛЬНЫЙ ЗАПУСК (0%)
    std::cout << "\n--- CONTROL RUN (0%) ---\n";
    SimulationResult control_run = run_single_simulation(0.0);
    std::cout << "Informed on Day 1: " << control_run.informed_per_day[0] << "\n";
    std::cout << "Informed on Day 30: " << control_run.informed_per_day[29] << "\n";

    return 0;
}
