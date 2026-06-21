#ifndef BELLMAN_FORD_HPP
#define BELLMAN_FORD_HPP

#include <string>
#include <vector>
#include "graph.hpp"

// Sentinel value representing "infinite" distance (unreachable vertex).
// Kept well below INT_MAX so that dist[u] + weight never overflows.
constexpr int BF_INF = 1000000000;

struct BellmanFordResult {
    std::vector<int> dist;

    int source = 0;
    int iterations = 0;
    bool negativeCycleDetected = false;

    double totalTime = 0.0;
    double computeTime = 0.0;
    double communicationTime = 0.0;

    bool success = false;
    std::string message;
};

// Sequential Bellman-Ford, O(V*E).
BellmanFordResult bellmanFordSequential(const Graph& graph, int source);

// MPI Bellman-Ford.
// Decomposition: 1D block distribution of the edge list across ranks.
// Each rank relaxes only its local edges, then the global distance
// array is synchronized every iteration with MPI_Allreduce(MIN),
// and the "did anything change" flag with MPI_Allreduce(LOR).
BellmanFordResult bellmanFordMPI(
    const Graph& graph,
    int source,
    int rank,
    int worldSize
);

#endif
