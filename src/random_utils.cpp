#include "random_utils.h"
#include "config.h"
#include <random>
#include <stdexcept>
#include <cmath>
namespace { std::mt19937 generator; }
void init_random(unsigned seed) { generator.seed(seed); }
void init_random() {
    if (USE_FIXED_SEED) init_random(FIXED_SEED);
    else init_random(std::random_device{}());
}
bool check_probability(double probability) {
    if (!std::isfinite(probability) || probability < 0 || probability > 1)
        throw std::invalid_argument("Probability must be in [0, 1]");
    return std::bernoulli_distribution(probability)(generator);
}
int get_random_int(int max_val) {
    if (max_val < 0) throw std::invalid_argument("Negative random bound");
    return std::uniform_int_distribution<int>(0, max_val)(generator);
}
int get_random_recipient(int sender) {
    if (sender < 0 || sender >= TOTAL_USERS) throw std::invalid_argument("Invalid sender");
    const int recipient = get_random_int(TOTAL_USERS - 2);
    return recipient >= sender ? recipient + 1 : recipient;
}
