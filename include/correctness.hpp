#ifndef CORRECTNESS_HPP
#define CORRECTNESS_HPP

#include <vector>
#include "graph.hpp"

bool isValidColoring(const Graph& graph, const std::vector<int>& colors);

int countColors(const std::vector<int>& colors);

int countColorConflicts(const Graph& graph, const std::vector<int>& colors);

#endif