#pragma once
#include <string>
#include "statistics.h"

void append_stats_to_csv(double probability, const FinalStats& stats, const std::string& filename);
void print_summary(double probability, const FinalStats& stats);
