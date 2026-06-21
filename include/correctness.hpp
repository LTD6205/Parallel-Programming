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

// --- MST / Kruskal correctness (added) ---

// Structural check: verifies that mstEdges forms a valid spanning forest of
// graph - i.e. it contains no cycles, every edge actually exists in graph
// with a matching weight, and the number of components it forms matches
// the number of connected components of the original graph (so the result
// spans every component it possibly can, without missing a necessary edge).
// Only meaningful for undirected graphs; returns false for directed input.
bool isValidMST(const Graph& graph, const std::vector<Edge>& mstEdges);

// Compares total MST weight between two runs (e.g. sequential vs MPI).
bool mstWeightsMatch(int totalWeightA, int totalWeightB);

// Counts edges (by endpoint pair, order-independent) that appear in one
// edge set but not the other. A non-zero result does not necessarily mean
// an error: graphs with tied edge weights can have multiple valid MSTs
// with the same total weight but a different edge set.
int countEdgeSetDifferences(const std::vector<Edge>& a, const std::vector<Edge>& b);

#endif
