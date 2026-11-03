#pragma once

// ===== 1. Параметры модели (из ТЗ) ==========================================
const int TOTAL_USERS = 200;   // размер сообщества, человек
const int INITIAL_INFORMED = 5;     // сколько случайных людей знают новость в 1-й день
const int SIMULATION_DAYS = 30;    // длительность одного запуска, дней
const int ATTEMPTS_PER_DAY = 2;     // попыток пересылки в день
const int SPREAD_DAYS = 3;     // сколько дней подряд человек пересылает новость (со дня, следующего за тем, когда узнал)
const int RUNS_COUNT = 1000;  // независимых запусков для каждой вероятности
const int REACH_TARGET = 100;   // цель для проверки (100 человек)

// ===== 2. Вероятности успешной пересылки (в долях единицы): 10%, 30%, 50% ===
constexpr double PROBABILITIES[] = { 0.10, 0.30, 0.50 };
constexpr int NUM_PROBABILITIES = static_cast<int>(sizeof(PROBABILITIES) / sizeof(PROBABILITIES[0]));

// Контрольный запуск: пересылка заведомо невозможна
constexpr double CONTROL_PROBABILITY = 0.0;

// ===== 3. Генератор случайных чисел =========================================
constexpr bool     USE_FIXED_SEED = true;   // true — результаты повторяются от запуска к запуску (удобно для отчёта)
constexpr unsigned FIXED_SEED = 12345;  // используется, если USE_FIXED_SEED == true

// ===== 4. Вывод в консоль ===================================================
constexpr int  DAYS_PER_BLOCK = 10;     // дней в одной консольной таблице (30 дней -> 3 блока по 10)
constexpr int  CONSOLE_DECIMALS = 1;      // знаков после запятой в консоли
constexpr bool UNICODE_BORDERS = false;  // false: +---+ и |  (работает везде); true: ┌──┐ и │ (красивее, нужен UTF-8)
constexpr const char* NO_DATA = "—";    // вместо числа, если его нет (например, «средний день» при 0 успешных запусков)

// ===== 5. CSV для Excel =====================================================
constexpr const char* CSV_DAILY_FILE = "results_daily.csv";    // среднее по дням (для графика)
constexpr const char* CSV_SUMMARY_FILE = "results_summary.csv";  // итоговая таблица по вероятностям
constexpr const char* CSV_CONTROL_FILE = "results_control.csv";  // контрольный запуск (p = 0)

// Русская (и финская) локаль Excel: разделитель «;» и десятичная запятая.
// Английская локаль Excel: CSV_DELIMITER = ',' и CSV_DECIMAL_COMMA = false.
constexpr char CSV_DELIMITER = ';';
constexpr bool CSV_DECIMAL_COMMA = true;
constexpr bool CSV_WRITE_BOM = true;   // метка UTF-8 в начале файла, чтобы Excel не ломал кириллицу
constexpr int  CSV_DECIMALS = 2;      // знаков после запятой в CSV

// ===== 6. Проверка констант при компиляции ==================================
static_assert(TOTAL_USERS >= 2, "TOTAL_USERS must be at least 2");
static_assert(SIMULATION_DAYS >= 1, "SIMULATION_DAYS must be at least 1");
static_assert(INITIAL_INFORMED >= 1 && INITIAL_INFORMED <= TOTAL_USERS,
    "INITIAL_INFORMED must be in [1, TOTAL_USERS]");
static_assert(SPREAD_DAYS >= 1 && ATTEMPTS_PER_DAY >= 1,
    "SPREAD_DAYS and ATTEMPTS_PER_DAY must be positive");
static_assert(RUNS_COUNT >= 1, "RUNS_COUNT must be at least 1");
static_assert(REACH_TARGET >= 1 && REACH_TARGET <= TOTAL_USERS,
    "REACH_TARGET must be in [1, TOTAL_USERS]");
static_assert(NUM_PROBABILITIES >= 1, "PROBABILITIES must not be empty");
static_assert(DAYS_PER_BLOCK >= 1, "DAYS_PER_BLOCK must be at least 1");
static_assert(!(CSV_DECIMAL_COMMA&& CSV_DELIMITER == ','),
    "CSV: decimal comma conflicts with ',' as delimiter");
