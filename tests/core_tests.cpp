#include "simulation.h"
#include "statistics.h"
#include "random_utils.h"
#include "io_handler.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>
namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
int attempts[SIMULATION_DAYS + 1][TOTAL_USERS]{};
int next_recipient = INITIAL_INFORMED;
bool scripted(int day, int sender, int& recipient, double) {
    ++attempts[day][sender];
    recipient = sender == 0 ? 1 : 0;
    if (day == 1 && sender == 0) recipient = INITIAL_INFORMED;
    return true;
}
bool fill_population(int day, int sender, int& recipient, double) {
    ++attempts[day][sender];
    recipient = next_recipient++;
    return true;
}
bool late_chain(int day, int sender, int& recipient, double) {
    ++attempts[day][sender];
    const int active_sender = day == 1 ? 0 : INITIAL_INFORMED + day - 2;
    recipient = sender == 0 ? 1 : 0;
    if (sender == active_sender && attempts[day][sender] == 1)
        recipient = INITIAL_INFORMED + day - 1;
    return true;
}
void check_run(const SimulationResult& r) {
    int previous = INITIAL_INFORMED, first = -1;
    for (int d = 0; d < SIMULATION_DAYS; ++d) {
        require(r.informed_per_day[d] >= previous && r.informed_per_day[d] <= TOTAL_USERS, "Invalid count");
        if (first < 0 && r.informed_per_day[d] >= REACH_TARGET) first = d + 1;
        previous = r.informed_per_day[d];
    }
    require(r.reached_100 == (first > 0) && r.day_reached_100 == first, "Incorrect threshold day");
}
void test(int id) {
    init_random(12345);
    if (id == 1) {
        auto r = run_single_simulation(0);
        for (int n : r.informed_per_day) require(n == INITIAL_INFORMED, "Zero probability");
        check_run(r);
    } else if (id == 2) {
        auto r = run_single_simulation(1, scripted);
        for (int d = 1; d <= SIMULATION_DAYS; ++d)
            for (int u = 0; u < TOTAL_USERS; ++u) {
                const bool active = (u < INITIAL_INFORMED && d <= 3) || (u == INITIAL_INFORMED && d >= 2 && d <= 4);
                require(attempts[d][u] == (active ? ATTEMPTS_PER_DAY : 0), "Sender timing/attempt count");
            }
        for (int n : r.informed_per_day) require(n == INITIAL_INFORMED + 1, "Same-day count or duplicate recipient");
        check_run(r);
    } else if (id == 3) {
        auto r = run_single_simulation(1, fill_population);
        check_run(r);
        require(r.informed_per_day[0] == 15 && r.informed_per_day[1] == 45 && r.informed_per_day[2] == 135, "Daily cascade");
        require(r.day_reached_100 == 3, "First threshold day");
        for (int d = 3; d < SIMULATION_DAYS; ++d) require(r.informed_per_day[d] == TOTAL_USERS, "Completion tail");
        int count = 0;
        for (const auto& day : attempts) for (int n : day) count += n;
        require(count == TOTAL_USERS - INITIAL_INFORMED, "Did not stop on full population");
    } else if (id == 4) {
        for (int sender = 0; sender < TOTAL_USERS; ++sender) {
            bool seen[TOTAL_USERS]{};
            bool duplicate = false;
            for (int i = 0; i < 5000; ++i) {
                int r = get_random_recipient(sender);
                require(r >= 0 && r < TOTAL_USERS && r != sender, "Recipient bounds");
                duplicate = duplicate || seen[r]; seen[r] = true;
            }
            for (int r = 0; r < TOTAL_USERS; ++r) if (r != sender) require(seen[r], "Recipient unreachable");
            require(duplicate, "Repeated recipients forbidden");
        }
    } else if (id == 5) {
        SimulationResult a{}, b{}, c{};
        for (int d = 0; d < SIMULATION_DAYS; ++d) {
            a.informed_per_day[d] = d < 1 ? 5 : 100;
            b.informed_per_day[d] = d < 3 ? 5 : 150;
            c.informed_per_day[d] = 5;
        }
        const auto s = calculate_statistics({a,b,c});
        require(s.runs_count == 3 && s.runs_reached_100 == 2, "Success count");
        require(std::abs(s.percent_reached_100 - 200.0/3) < 1e-10 && s.avg_day_reached_100 == 3, "Threshold statistics");
        require(s.avg_informed_per_day[0] == 5 && s.avg_informed_per_day[3] == 85, "Daily means");
        auto empty = calculate_statistics({});
        require(empty.runs_count == 0 && empty.avg_day_reached_100 == -1 && empty.percent_reached_100 == 0, "Empty statistics");
        auto failed = calculate_statistics({c});
        require(failed.runs_reached_100 == 0 && failed.avg_day_reached_100 == -1, "Failed run day");
    } else if (id == 6) {
        auto a = run_single_simulation(0.3), b = run_single_simulation(0.3);
        init_random(12345);
        auto c = run_single_simulation(0.3), d = run_single_simulation(0.3);
        bool different = false;
        for (int i = 0; i < SIMULATION_DAYS; ++i) {
            require(a.informed_per_day[i] == c.informed_per_day[i] && b.informed_per_day[i] == d.informed_per_day[i], "Seed reproducibility");
            different = different || a.informed_per_day[i] != b.informed_per_day[i];
        }
        require(different, "Runs reseeded");
    } else if (id == 7) {
        for (double p : {0.0,0.1,0.3,0.5,1.0})
            for (int n = 0; n < 100; ++n) check_run(run_single_simulation(p));
    } else if (id == 8) {
        int caught = 0;
        for (double p : {-0.1,1.1,std::numeric_limits<double>::quiet_NaN()}) {
            try { run_single_simulation(p); } catch (const std::invalid_argument&) { ++caught; }
        }
        require(caught == 3, "Invalid probabilities accepted");
        for (int i = 0; i < 100; ++i) require(!check_probability(0) && check_probability(1), "Probability endpoints");
    } else if (id == 9) {
        auto r = run_single_simulation(0);
        auto s = calculate_statistics({r});
        require(write_daily_csv("test_daily.csv", {0}, {s}), "Daily CSV write");
        require(write_summary_csv("test_summary.csv", {0}, {s}), "Summary CSV write");
        require(write_runs_csv("test_runs.csv", {0}, {{r}}), "Runs CSV write");
        require(!write_daily_csv("missing-directory/test.csv", {0}, {s}), "Open error ignored");
        require(!write_summary_csv("unused.csv", {0}, {}), "Mismatched scenarios accepted");
        std::ifstream f("test_runs.csv", std::ios::binary);
        std::string text((std::istreambuf_iterator<char>(f)), {});
        require(text.substr(0,3) == "\xEF\xBB\xBF", "Missing UTF-8 BOM");
        require(std::count(text.begin(), text.end(), '\n') == 2, "CSV rows");
        require(text.find("не достигнуто") != std::string::npos, "Missing unsuccessful label");
    } else if (id == 10) {
        auto r = run_single_simulation(1, late_chain);
        check_run(r);
        for (int d = 0; d < SIMULATION_DAYS; ++d)
            require(r.informed_per_day[d] == INITIAL_INFORMED + d + 1, "Last-day update lost");
        require(attempts[SIMULATION_DAYS][INITIAL_INFORMED + SIMULATION_DAYS - 1] == 0,
                "Last-day recipient sent immediately");
    } else throw std::runtime_error("Unknown test");
}
}
int main(int argc, char** argv) {
    try { require(argc == 2, "Test id required"); test(std::stoi(argv[1])); }
    catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
    return 0;
}
