#pragma once
void init_random();
void init_random(unsigned seed);
bool check_probability(double probability);
// Inclusive upper bound.
int get_random_int(int max_val);
int get_random_recipient(int sender);
