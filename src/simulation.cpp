#include "simulation.h"
#include "config.h"
#include "random_utils.h"
#include <algorithm> // для std::max, если понадобится

SimulationResult run_single_simulation(double spread_probability) {
    SimulationResult result;
    std::vector<User> users(TOTAL_USERS);

    // Инициализация пользователей
    for (int i = 0; i < TOTAL_USERS; ++i) {
        users[i].id = i;
    }

    // День 1: случайно выбираем 5 человек
    int informed_count = 0;
    while (informed_count < INITIAL_INFORMED) {
        int rand_id = get_random_int(TOTAL_USERS);
        if (!users[rand_id].is_informed) {
            users[rand_id].is_informed = true;
            users[rand_id].days_spreading_left = SPREAD_DAYS;
            informed_count++;
        }
    }

    int current_informed_total = INITIAL_INFORMED;

    // Цикл по дням (0-й индекс - это День 1)
    for (int day = 0; day < SIMULATION_DAYS; ++day) {
        result.informed_per_day[day] = current_informed_total;

        // Проверка на достижение 100 (сработает 1 раз)
        if (current_informed_total >= 100 && !result.reached_100) {
            result.reached_100 = true;
            result.day_reached_100 = day + 1; // +1, так как индексы с 0
        }

        // Оптимизация: если все 200 знают, дальше можно не считать
        if (current_informed_total == TOTAL_USERS) {
            // Заполняем оставшиеся дни тем же значением (200)
            for (int remaining_day = day + 1; remaining_day < SIMULATION_DAYS; ++remaining_day) {
                result.informed_per_day[remaining_day] = TOTAL_USERS;
            }
            break; 
        }

        std::vector<int> newly_informed_today;

        // Все, кто знает и еще может рассылать, делают попытки
        for (int i = 0; i < TOTAL_USERS; ++i) {
            if (users[i].is_informed && users[i].days_spreading_left > 0) {
                
                for (int attempt = 0; attempt < ATTEMPTS_PER_DAY; ++attempt) {
                    // Выбираем случайного человека из 199 (исключая себя)
                    int target_id;
                    do {
                        target_id = get_random_int(TOTAL_USERS);
                    } while (target_id == i); // Повторяем выбор, если попали в себя

                    // Проверяем вероятность и узнал ли он уже
                    if (!users[target_id].is_informed && check_probability(spread_probability)) {
                        // Добавляем во временный список (узнает только завтра)
                        // Используем find, чтобы не добавить дважды за один день, если обе попытки в него попали
                        if (std::find(newly_informed_today.begin(), newly_informed_today.end(), target_id) == newly_informed_today.end()) {
                            newly_informed_today.push_back(target_id);
                        }
                    }
                }
            }
        }

        // В конце дня обновляем статусы тех, кто узнал сегодня, и уменьшаем таймеры
        for (int i = 0; i < TOTAL_USERS; ++i) {
            if (users[i].is_informed && users[i].days_spreading_left > 0) {
                users[i].days_spreading_left--; // Отработал день
            }
        }

        for (int id : newly_informed_today) {
            users[id].is_informed = true;
            users[id].days_spreading_left = SPREAD_DAYS; // Начнет рассылать со следующего дня
            current_informed_total++;
        }
    }

    return result;
}
