#include "io_handler.h"
#include "config.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <locale>
namespace {
using Row = std::vector<std::string>;
using Grid = std::vector<Row>;
std::string toStr(long long value) { return std::to_string(value); }
std::string fmt(double value, int decimals) {
    std::ostringstream out; out.imbue(std::locale::classic());
    out << std::fixed << std::setprecision(decimals) << value; return out.str();
}
std::string probabilityLabel(double p) { return fmt(p * 100, 0) + "%"; }
std::string avgFinalTitle() { return "Среднее на " + toStr(SIMULATION_DAYS) + "-й день"; }
std::string reachedTitle() { return "Запусков >=" + toStr(REACH_TARGET); }
bool hasReachDay(const FinalStats& s) { return s.runs_reached_100 > 0; }
double percentReached(const FinalStats& s) { return s.percent_reached_100; }
    // Число для CSV: нужное число знаков и (если задано в config.h) десятичная запятая.
    std::string csvNumber(double v) {
        std::string s = fmt(v, CSV_DECIMALS);
        if (CSV_DECIMAL_COMMA) std::replace(s.begin(), s.end(), '.', ',');
        return s;
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
        f.close();
        return !f.fail();
    }

}
bool write_daily_csv(const std::string& path,
    const std::vector<double>& probabilities,
    const std::vector<FinalStats>& stats) {
    if (probabilities.size() != stats.size()) return false;
    const std::size_t n = probabilities.size();

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
    if (probabilities.size() != stats.size()) return false;
    const std::size_t n = probabilities.size();

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
                        toStr(s.runs_count) });
    }
    return writeGridCsv(path, grid);
}
bool write_runs_csv(const std::string& path, const std::vector<double>& probabilities,
                    const std::vector<std::vector<SimulationResult>>& runs) {
    if (probabilities.size() != runs.size()) return false;
    Grid grid;
    Row header{"Вероятность", "Прогон", "Первый день достижения 100"};
    for (int d = 1; d <= SIMULATION_DAYS; ++d) header.push_back("День " + toStr(d));
    grid.push_back(header);
    for (std::size_t i = 0; i < runs.size(); ++i) {
        for (std::size_t j = 0; j < runs[i].size(); ++j) {
            const auto& run = runs[i][j];
            Row row{probabilityLabel(probabilities[i]), toStr(j + 1),
                    run.reached_100 ? toStr(run.day_reached_100) : NO_DATA};
            for (int value : run.informed_per_day) row.push_back(toStr(value));
            grid.push_back(row);
        }
    }
    return writeGridCsv(path, grid);
}
