#ifndef CORRECTNESS_HPP
#define CORRECTNESS_HPP

#include <vector>
#include "graph.hpp"

// --- Graph coloring correctness (existing) ---

bool isValidColoring(const Graph& graph, const std::vector<int>& colors);

int countColors(const std::vector<int>& colors);

int countColorConflicts(const Graph& graph, const std::vector<int>& colors);

// --- Bellman-Ford correctness (added) ---

// Structural check: dist[source] == 0 and no edge in the graph can
// still be relaxed (the standard Bellman-Ford optimality condition).
// Does not require a reference/sequential result.
bool isValidBellmanFord(const Graph& graph, int source, const std::vector<int>& dist);

// Compares two distance vectors (e.g. sequential vs MPI) element by element.
bool distancesMatch(const std::vector<int>& a, const std::vector<int>& b);

// Counts how many vertices disagree between two distance vectors.
int countDistanceMismatches(const std::vector<int>& a, const std::vector<int>& b);

#endif
