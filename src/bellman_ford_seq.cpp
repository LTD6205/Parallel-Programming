#include "bellman_ford.hpp"

#include <chrono>

BellmanFordResult bellmanFordSequential(const Graph& graph, int source) {
    BellmanFordResult result;
    result.source = source;

    const int V = graph.size();

    if (source < 0 || source >= V) {
        result.success = false;
        result.message = "Invalid source vertex";
        return result;
    }

    const auto startTotal = std::chrono::high_resolution_clock::now();

    std::vector<int> dist(V, BF_INF);
    dist[source] = 0;

    const std::vector<Edge> edges = graph.toEdgeList();

    int iterations = 0;
    for (int i = 1; i <= V - 1; ++i) {
        bool changed = false;

        for (const Edge& e : edges) {
            if (dist[e.u] != BF_INF && dist[e.u] + e.weight < dist[e.v]) {
                dist[e.v] = dist[e.u] + e.weight;
                changed = true;
            }
        }

        iterations = i;
        if (!changed) {
            break;
        }
    }

    // One extra pass: if anything can still be relaxed, the graph
    // contains a negative-weight cycle reachable from `source`.
    bool negativeCycle = false;
    for (const Edge& e : edges) {
        if (dist[e.u] != BF_INF && dist[e.u] + e.weight < dist[e.v]) {
            negativeCycle = true;
            break;
        }
    }

    const auto endTotal = std::chrono::high_resolution_clock::now();

    result.dist = dist;
    result.iterations = iterations;
    result.negativeCycleDetected = negativeCycle;
    result.totalTime = std::chrono::duration<double>(endTotal - startTotal).count();
    result.computeTime = result.totalTime; // no communication in the sequential path
    result.communicationTime = 0.0;
    result.success = true;
    result.message = negativeCycle ? "Negative cycle detected" : "OK";

    return result;
}
