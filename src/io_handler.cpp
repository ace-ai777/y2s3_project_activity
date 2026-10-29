#include "io_handler.h"
#include <iostream>

void append_stats_to_csv(double probability, const FinalStats& stats, const std::string& filename) {
    // TODO (Маша): Открой файл через std::ofstream в режиме добавления (std::ios::app).
    // Запиши строку с вероятностью, средними значениями по дням и т.д.
}

void print_summary(double probability, const FinalStats& stats) {
    // TODO (Маша): Выведи в консоль текстом: "Для 10%: достигло 100 в X прогонах, средний день Y".
}
