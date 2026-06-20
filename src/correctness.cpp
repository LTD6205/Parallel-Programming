#include "correctness.hpp"

#include <set>

bool isValidColoring(const Graph& graph, const std::vector<int>& colors) {
    int n = graph.size();

    if (static_cast<int>(colors.size()) != n) {
        return false;
    }

    for (int u = 0; u < n; ++u) {
        if (colors[u] < 0) {
            return false;
        }

        for (int v = u + 1; v < n; ++v) {
            if (graph.getWeight(u, v) != 0 && colors[u] == colors[v]) {
                return false;
            }
        }
    }

    return true;
}

int countColors(const std::vector<int>& colors) {
    std::set<int> used;

    for (int color : colors) {
        if (color >= 0) {
            used.insert(color);
        }
    }

    return static_cast<int>(used.size());
}

int countColorConflicts(const Graph& graph, const std::vector<int>& colors) {
    int n = graph.size();

    if (static_cast<int>(colors.size()) != n) {
        return -1;
    }

    int conflicts = 0;

    for (int u = 0; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) {
            if (graph.getWeight(u, v) != 0 &&
                colors[u] != -1 &&
                colors[u] == colors[v]) {
                ++conflicts;
            }
        }
    }

    return conflicts;
}