#include "io_handler.h"
#include <iostream>
#include <fstream>
#include <iomanip>

void append_stats_to_csv(double probability, const FinalStats& stats, const std::string& filename) {
    //открытие файла результатов в режиме добавления новых строк в конец
    std::ofstream out(filename, std::ios::app);
    //проверка корректности открытия потока вывода
    if (!out.is_open()) {
        std::cerr << "[Ошибка] Не удалось открыть файл: " << filename << std::endl;
        return;
    }

    //смещение указателя в конец файла для проверки его размера
    out.seekp(0, std::ios::end);
    //если файл пустой то записываем заголовок таблицы с разделителями
    if (out.tellp() == 0) {
        out << "Probability;Metric;";
        for (int d = 1; d <= 30; ++d) {
            out << "Day_" << d << ";";
        }
        out << "Runs_Reached_100;Avg_Day_Reached_100\n";
    }

    //запись текущей вероятности и типа метрики в строку файла
    out << (probability * 100) << "%;Avg_Users;";
    //последовательный вывод средних значений информированных по дням из массива
    for (double val : stats.avg_informed_per_day) {
        out << std::fixed << std::setprecision(2) << val << ";";
    }

    //запись количества успешных запусков где достигнута отметка сто
    out << stats.runs_reached_100 << ";";
    //защита от деления на ноль если лимит не был достигнут
    if (stats.avg_day_reached_100 < 0) {
        out << "N/A\n";
    }
    else {
        out << std::fixed << std::setprecision(2) << stats.avg_day_reached_100 << "\n";
    }

    //закрытие потока и сохранение изменений на диске
    out.close();
}

void print_summary(double probability, const FinalStats& stats) {
    //вывод декоративной рамки для оформления консольного отчета
    std::cout << "==================================================\n";
    std::cout << "РЕЗУЛЬТАТЫ СИМУЛЯЦИИ ДЛЯ ВЕРОЯТНОСТИ: " << (probability * 100) << "%\n";
    std::cout << "==================================================\n";
    //отображение общего числа успешных симуляций из тысячи запусков
    std::cout << "Новость дошла до 100 человек в " << stats.runs_reached_100 << " из 1000 прогонов.\n";
    //расчет и вывод процентного соотношения успешных запусков
    std::cout << "Процент успешных запусков: " << std::fixed << std::setprecision(1) << (stats.runs_reached_100 / 10.0) << "%\n";
    std::cout << "Средний день достижения отметки 100 человек: ";
    //обработка случая отсутствия успешных запусков для консоли
    if (stats.avg_day_reached_100 < 0) {
        std::cout << "не достигнуто\n";
    }
    else {
        std::cout << std::fixed << std::setprecision(2) << stats.avg_day_reached_100 << "-й день\n";
    }
    //вывод финального среднего охвата аудитории на тридцатый день из массива
    std::cout << "Среднее число знающих новость на 30-й день: "
        << std::fixed << std::setprecision(1) << stats.avg_informed_per_day[29]
        << " человек.\n\n";
}