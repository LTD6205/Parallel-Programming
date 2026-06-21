#include "correctness.hpp"
#include "bellman_ford.hpp"

bool isValidBellmanFord(const Graph& graph, int source, const std::vector<int>& dist) {
    const int V = graph.size();

    if (static_cast<int>(dist.size()) != V) {
        return false;
    }
    if (source < 0 || source >= V) {
        return false;
    }
    if (dist[source] != 0) {
        return false;
    }

    const std::vector<Edge> edges = graph.toEdgeList();

    for (const Edge& e : edges) {
        if (dist[e.u] == BF_INF) {
            continue;
        }
        // Optimality condition: no edge should still be relaxable.
        if (dist[e.u] + e.weight < dist[e.v]) {
            return false;
        }
    }

    return true;
}

bool distancesMatch(const std::vector<int>& a, const std::vector<int>& b) {
    if (a.size() != b.size()) {
        return false;
    }

    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }

    return true;
}

int countDistanceMismatches(const std::vector<int>& a, const std::vector<int>& b) {
    int mismatches = 0;
    const std::size_t n = a.size() < b.size() ? a.size() : b.size();

    for (std::size_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            ++mismatches;
        }
    }

    mismatches += static_cast<int>(a.size() > b.size() ? a.size() - n : b.size() - n);

    return mismatches;
}
