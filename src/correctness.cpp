#include "correctness.hpp"

#include <algorithm>
#include <iterator>
#include <set>
#include <utility>

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

// --- MST / Kruskal correctness (added) ---

namespace {

int findRoot(std::vector<int>& parent, int x) {
    while (parent[x] != x) {
        x = parent[x];
    }
    return x;
}

} // namespace

bool isValidMST(const Graph& graph, const std::vector<Edge>& mstEdges) {
    if (graph.isDirectedGraph()) {
        return false;
    }

    int n = graph.size();

    if (n == 0) {
        return mstEdges.empty();
    }

    const auto& matrix = graph.getMatrix();

    // Moi canh trong mstEdges phai thuc su ton tai trong graph voi dung trong so.
    for (const Edge& e : mstEdges) {
        if (e.u < 0 || e.u >= n || e.v < 0 || e.v >= n || e.u == e.v) {
            return false;
        }
        if (matrix[e.u][e.v] != e.weight || e.weight == 0) {
            return false;
        }
    }

    // Khong duoc co chu trinh; dong thoi dem so thanh phan ma mstEdges tao ra.
    std::vector<int> mstParent(n);
    for (int i = 0; i < n; ++i) {
        mstParent[i] = i;
    }

    for (const Edge& e : mstEdges) {
        int ru = findRoot(mstParent, e.u);
        int rv = findRoot(mstParent, e.v);

        if (ru == rv) {
            return false;
        }

        mstParent[ru] = rv;
    }

    int mstComponents = 0;
    for (int i = 0; i < n; ++i) {
        if (findRoot(mstParent, i) == i) {
            ++mstComponents;
        }
    }

    // Dem so thanh phan lien thong cua graph goc (doc lap voi Kruskal) de doi chieu.
    std::vector<int> graphParent(n);
    for (int i = 0; i < n; ++i) {
        graphParent[i] = i;
    }

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (matrix[i][j] != 0) {
                int ri = findRoot(graphParent, i);
                int rj = findRoot(graphParent, j);

                if (ri != rj) {
                    graphParent[ri] = rj;
                }
            }
        }
    }

    int graphComponents = 0;
    for (int i = 0; i < n; ++i) {
        if (findRoot(graphParent, i) == i) {
            ++graphComponents;
        }
    }

    return mstComponents == graphComponents;
}

bool mstWeightsMatch(int totalWeightA, int totalWeightB) {
    return totalWeightA == totalWeightB;
}

int countEdgeSetDifferences(const std::vector<Edge>& a, const std::vector<Edge>& b) {
    auto normalize = [](const Edge& e) {
        return e.u < e.v ? std::make_pair(e.u, e.v) : std::make_pair(e.v, e.u);
    };

    std::vector<std::pair<int, int>> normalizedA;
    std::vector<std::pair<int, int>> normalizedB;
    normalizedA.reserve(a.size());
    normalizedB.reserve(b.size());

    for (const Edge& e : a) {
        normalizedA.push_back(normalize(e));
    }
    for (const Edge& e : b) {
        normalizedB.push_back(normalize(e));
    }

    std::sort(normalizedA.begin(), normalizedA.end());
    std::sort(normalizedB.begin(), normalizedB.end());

    std::vector<std::pair<int, int>> diff;
    std::set_symmetric_difference(
        normalizedA.begin(), normalizedA.end(),
        normalizedB.begin(), normalizedB.end(),
        std::back_inserter(diff)
    );

    return static_cast<int>(diff.size());
}