#include "random_utils.h"
#include <random>

// TODO (Маша): Настрой std::mt19937 и std::random_device.
// Реализуй функции.

namespace {
    //инициализация генератора псевдослучайных чисел вихрь мерсенна
    std::mt19937 g_gen;
}

void init_random() {
    //настройка сид-генератора через аппаратное случайное число
    std::random_device rd;
    g_gen.seed(rd());
}

bool check_probability(double probability) {
    //создание равномерного распределения вещественных чисел от нуля до единицы
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    //возврат истины если сгенерированное число попало в диапазон вероятности
    return dis(g_gen) < probability;
}

int get_random_int(int max_val) {
    //проверка корректности переданного максимального значения индекса
    if (max_val <= 0) return 0;
    //генерация случайного целого числа для выбора пользователя
    std::uniform_int_distribution<int> dis(0, max_val);
    return dis(g_gen);
}

