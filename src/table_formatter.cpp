// table_formatter.cpp — вывод статистики: таблицы с рамками в консоль и CSV для Excel.
//
// Модель и подсчёт статистики сюда не входят: модуль получает готовые FinalStats (statistics.h).
//   * консоль — таблицы вида | Вероятность | День 1 | День 2 | ... | с рамками;
//   * CSV     — файл «по дням», который открывается в Excel и сразу годится для графика.

#include "table_formatter.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <ostream>
#include <sstream>

#include "config.h"

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif

namespace {

    typedef std::vector<std::string> Row;
    typedef std::vector<Row>         Grid;   // таблица: grid[0] — заголовок, дальше строки данных

    enum class Align { Left, Right, Center };

    const std::size_t kNone = static_cast<std::size_t>(-1);
    const std::string kEmpty;

    // В FinalStats массив на 30 дней; SIMULATION_DAYS не должен его превышать.
    const int kStatsDays = static_cast<int>(sizeof(FinalStats::avg_informed_per_day) /
        sizeof(FinalStats::avg_informed_per_day[0]));
    static_assert(SIMULATION_DAYS <= 30, "FinalStats::avg_informed_per_day holds only 30 days");

    // ============================================================================
    // Форматирование чисел и подписей
    // ============================================================================

    std::string toStr(long long v) {
        std::ostringstream ss;
        ss.imbue(std::locale::classic());
        ss << v;
        return ss.str();
    }

    std::string fmt(double v, int decimals) {
        std::ostringstream ss;
        ss.imbue(std::locale::classic());
        ss << std::fixed << std::setprecision(decimals) << v;
        return ss.str();
    }

    // 0.1 -> "10%", 0.125 -> "12.5%"
    std::string probabilityLabel(double p) {
        std::ostringstream ss;
        ss.imbue(std::locale::classic());
        ss << std::setprecision(4) << p * 100.0 << '%';
        return ss.str();
    }

    // Число для CSV: нужное число знаков и (если задано в config.h) десятичная запятая.
    std::string csvNumber(double v) {
        std::string s = fmt(v, CSV_DECIMALS);
        if (CSV_DECIMAL_COMMA) std::replace(s.begin(), s.end(), '.', ',');
        return s;
    }

    // Заголовки итоговой таблицы строятся из констант, чтобы при смене config.h подписи не врали.
    std::string avgFinalTitle() { return "Среднее на " + toStr(SIMULATION_DAYS) + "-й день"; }
    std::string reachedTitle() { return "Запусков ≥" + toStr(REACH_TARGET); }
    std::string avgDayTitle() { return "Средний день ≥" + toStr(REACH_TARGET); }

    double percentReached(const FinalStats& s) { return 100.0 * s.runs_reached_100 / RUNS_COUNT; }
    bool   hasReachDay(const FinalStats& s) { return s.runs_reached_100 > 0 && s.avg_day_reached_100 >= 0.0; }

    // ============================================================================
    // Ширина текста в UTF-8
    // ============================================================================
    // std::setw считает БАЙТЫ, а русская буква в UTF-8 занимает 2 байта — столбцы «поехали бы».
    // Поэтому ширину считаем сами: символов = байты минус «продолжающие» (10xxxxxx).
    std::size_t textWidth(const std::string& s) {
        std::size_t n = 0;
        for (std::size_t i = 0; i < s.size(); ++i)
            if ((static_cast<unsigned char>(s[i]) & 0xC0) != 0x80) ++n;
        return n;
    }

    std::string pad(const std::string& s, std::size_t width, Align align) {
        const std::size_t w = textWidth(s);
        if (w >= width) return s;
        const std::size_t gap = width - w;
        switch (align) {
        case Align::Left:  return s + std::string(gap, ' ');
        case Align::Right: return std::string(gap, ' ') + s;
        case Align::Center: {
            const std::size_t left = gap / 2;
            return std::string(left, ' ') + s + std::string(gap - left, ' ');
        }
        }
        return s;
    }

    // ============================================================================
    // Рамки
    // ============================================================================
    struct Border {
        const char* h, * v;          // горизонталь, вертикаль
        const char* tl, * tm, * tr;   // верхняя линия: левый угол, тройник, правый угол
        const char* ml, * mm, * mr;   // линия под заголовком
        const char* bl, * bm, * br;   // нижняя линия
    };

    const Border kAscii = { "-", "|", "+", "+", "+", "+", "+", "+", "+", "+", "+" };
    const Border kUnicode = { "─", "│", "┌", "┬", "┐", "├", "┼", "┤", "└", "┴", "┘" };

    const Border& border() { return UNICODE_BORDERS ? kUnicode : kAscii; }

    std::string hLine(const std::vector<std::size_t>& widths,
        const char* left, const char* mid, const char* right) {
        std::string s = left;
        for (std::size_t i = 0; i < widths.size(); ++i) {
            for (std::size_t k = 0; k < widths[i] + 2; ++k) s += border().h;   // +2 — пробелы вокруг текста
            s += (i + 1 < widths.size()) ? mid : right;
        }
        return s;
    }

    // Рисует таблицу с рамками. grid[0] — заголовок (центрируется), aligns — выравнивание данных.
    // Столбцы, начиная с uniformFrom, делаются одной ширины («День 1» и «День 10» не рвут сетку).
    void printGrid(std::ostream& out, const Grid& grid, const std::vector<Align>& aligns,
        std::size_t uniformFrom = kNone) {
        if (grid.empty()) return;
        const std::size_t cols = grid[0].size();

        std::vector<std::size_t> widths(cols, 0);
        for (std::size_t r = 0; r < grid.size(); ++r)
            for (std::size_t c = 0; c < cols && c < grid[r].size(); ++c)
                widths[c] = std::max(widths[c], textWidth(grid[r][c]));

        if (uniformFrom < cols) {
            std::size_t widest = 0;
            for (std::size_t c = uniformFrom; c < cols; ++c) widest = std::max(widest, widths[c]);
            for (std::size_t c = uniformFrom; c < cols; ++c) widths[c] = widest;
        }

        const Border& b = border();
        out << hLine(widths, b.tl, b.tm, b.tr) << '\n';
        for (std::size_t r = 0; r < grid.size(); ++r) {
            out << b.v;
            for (std::size_t c = 0; c < cols; ++c) {
                const std::string& cell = (c < grid[r].size()) ? grid[r][c] : kEmpty;
                const Align a = (r == 0) ? Align::Center
                    : (c < aligns.size() ? aligns[c] : Align::Left);
                out << ' ' << pad(cell, widths[c], a) << ' ' << b.v;
            }
            out << '\n';
            if (r == 0) out << hLine(widths, b.ml, b.mm, b.mr) << '\n';
        }
        out << hLine(widths, b.bl, b.bm, b.br) << '\n';
    }

    // ============================================================================
    // CSV
    // ============================================================================
    std::string csvCell(const std::string& s) {
        if (s.find(CSV_DELIMITER) == std::string::npos &&
            s.find('"') == std::string::npos && s.find('\n') == std::string::npos)
            return s;
        std::string q = "\"";
        for (std::size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '"') q += '"';
            q += s[i];
        }
        return q + "\"";
    }

    bool writeGridCsv(const std::string& path, const Grid& grid) {
        // binary: строки заканчиваем «\r\n» сами, одинаково на любой ОС
        std::ofstream f(path.c_str(), std::ios::binary);
        if (!f) return false;
        if (CSV_WRITE_BOM) f << "\xEF\xBB\xBF";   // UTF-8 BOM
        for (std::size_t r = 0; r < grid.size(); ++r) {
            for (std::size_t c = 0; c < grid[r].size(); ++c) {
                if (c > 0) f << CSV_DELIMITER;
                f << csvCell(grid[r][c]);
            }
            f << "\r\n";
        }
        f.flush();
        return f.good();
    }

#ifdef _WIN32
    UINT g_oldOutputCp = 0;
    void restoreConsole() {
        if (g_oldOutputCp != 0) SetConsoleOutputCP(g_oldOutputCp);
    }
#endif

}  // namespace

// ============================================================================
// Публичные функции
// ============================================================================

void setup_console() {
#ifdef _WIN32
    g_oldOutputCp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);        // кириллица и рамки без «кракозябр»
    std::atexit(restoreConsole);        // при выходе возвращаем прежнюю кодовую страницу
#endif
}

void print_pretty_table(double probability, const FinalStats& stats) {
    std::ostream& out = std::cout;
    const std::string label = probabilityLabel(probability);

    out << "\nВероятность: " << label << '\n';   // заголовок вида «Вероятность: 10%»
    out << "\nСреднее число осведомлённых пользователей по дням (среднее по "
        << RUNS_COUNT << " запускам)\n";

    // --- по дням, блоками ---
    const std::size_t days = static_cast<std::size_t>(SIMULATION_DAYS);
    const std::size_t perBlock = static_cast<std::size_t>(DAYS_PER_BLOCK);
    for (std::size_t from = 0; from < days; from += perBlock) {
        const std::size_t to = std::min(days, from + perBlock);   // дни [from, to)

        Row header, row;
        header.push_back("Вероятность");
        row.push_back(label);
        for (std::size_t d = from; d < to; ++d) {
            header.push_back("День " + toStr(static_cast<long long>(d) + 1));
            row.push_back(fmt(stats.avg_informed_per_day[d], CONSOLE_DECIMALS));
        }
        Grid grid;
        grid.push_back(header);
        grid.push_back(row);

        std::vector<Align> aligns(header.size(), Align::Right);
        aligns[0] = Align::Left;

        out << "\nДни " << (from + 1) << "–" << to << ":\n";
        printGrid(out, grid, aligns, 1);
    }

    // --- итоги ---
    Grid summary;
    summary.push_back({ "Вероятность", avgFinalTitle(), reachedTitle(), "Доля запусков, %", avgDayTitle() });
    summary.push_back({ label,
                       fmt(stats.avg_informed_per_day[SIMULATION_DAYS - 1], CONSOLE_DECIMALS),
                       toStr(stats.runs_reached_100),
                       fmt(percentReached(stats), 1),
                       hasReachDay(stats) ? fmt(stats.avg_day_reached_100, CONSOLE_DECIMALS)
                                          : std::string(NO_DATA) });

    std::vector<Align> aligns(5, Align::Right);
    aligns[0] = Align::Left;

    out << "\nИтоги\n";
    printGrid(out, summary, aligns);
    out << "Запусков на вероятность: " << RUNS_COUNT << ". «" << reachedTitle()
        << "» — сколько запусков дошли хотя бы до " << REACH_TARGET << " человек; «"
        << avgDayTitle() << "» усредняется только по ним.\n";
}

bool write_daily_csv(const std::string& path,
    const std::vector<double>& probabilities,
    const std::vector<FinalStats>& stats) {
    const std::size_t n = std::min(probabilities.size(), stats.size());

    Grid grid;
    Row header;
    header.push_back("День");
    for (std::size_t i = 0; i < n; ++i)
        header.push_back("Вероятность " + probabilityLabel(probabilities[i]));
    grid.push_back(header);

    for (int d = 0; d < SIMULATION_DAYS; ++d) {
        Row row;
        row.push_back(toStr(d + 1));
        for (std::size_t i = 0; i < n; ++i)
            row.push_back(csvNumber(stats[i].avg_informed_per_day[d]));
        grid.push_back(row);
    }
    return writeGridCsv(path, grid);
}

bool write_summary_csv(const std::string& path,
    const std::vector<double>& probabilities,
    const std::vector<FinalStats>& stats) {
    const std::size_t n = std::min(probabilities.size(), stats.size());

    Grid grid;
    grid.push_back({ "Вероятность", avgFinalTitle(), reachedTitle(),
                    "Доля запусков ≥" + toStr(REACH_TARGET) + ", %",
                    "Средний день достижения " + toStr(REACH_TARGET),
                    "Всего запусков" });
    for (std::size_t i = 0; i < n; ++i) {
        const FinalStats& s = stats[i];
        grid.push_back({ probabilityLabel(probabilities[i]),
                        csvNumber(s.avg_informed_per_day[SIMULATION_DAYS - 1]),
                        toStr(s.runs_reached_100),
                        csvNumber(percentReached(s)),
                        hasReachDay(s) ? csvNumber(s.avg_day_reached_100) : std::string(NO_DATA),
                        toStr(RUNS_COUNT) });
    }
    return writeGridCsv(path, grid);
}