#pragma once

#include <vector>

// Количество дней в симуляции
const int NUM_DAYS = 30;

// Результат одного запуска симуляции
struct SimulationResult {
    int informed_per_day[NUM_DAYS];// сколько человек знало новость в каждый день
    bool reached_100; // дошла ли новость до 100 человек
    int day_reached_100; // на какой день дошла 
};

// по всем запускам
struct FinalStats {
    double avg_informed_per_day[NUM_DAYS]; //среднее по всем запускам для каждого дня
    int runs_reached_100; // в скольких запусках дошло до 100
    double percent_reached_100; // доля успешных запусков проценты
    double avg_day_reached_100; // средний день достижения 
};

FinalStats calculate_statistics(const std::vector<SimulationResult>& all_runs);