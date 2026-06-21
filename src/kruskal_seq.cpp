#include "kruskal.hpp"

#include <algorithm>
#include <chrono>

namespace {

// Union-Find (Disjoint Set Union) - chi tiet hien thuc noi bo cua Kruskal,
// khong can dua ra header.
class UnionFind {
public:
    explicit UnionFind(int n) : parent(n), rankOf(n, 0) {
        for (int i = 0; i < n; ++i) {
            parent[i] = i;
        }
    }

    int find(int x) {
        if (parent[x] != x) {
            parent[x] = find(parent[x]);
        }
        return parent[x];
    }

    // Tra ve true neu hai dinh thuoc hai thanh phan khac nhau (va da duoc hop nhat).
    bool unite(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);

        if (rootX == rootY) {
            return false;
        }

        if (rankOf[rootX] < rankOf[rootY]) {
            parent[rootX] = rootY;
        } else if (rankOf[rootX] > rankOf[rootY]) {
            parent[rootY] = rootX;
        } else {
            parent[rootY] = rootX;
            ++rankOf[rootX];
        }

        return true;
    }

private:
    std::vector<int> parent;
    std::vector<int> rankOf;
};

// Buoc Union-Find tuan tu: nhan danh sach canh DA SAP XEP va xay dung MST.
KruskalResult buildMSTFromSortedEdges(const std::vector<Edge>& sortedEdges, int n) {
    KruskalResult result;

    UnionFind uf(n);

    for (const Edge& e : sortedEdges) {
        if (uf.unite(e.u, e.v)) {
            result.mstEdges.push_back(e);
            result.totalWeight += e.weight;
            ++result.edgesUsed;

            if (result.edgesUsed == n - 1) {
                break;
            }
        }
    }

    result.success = (n <= 1) || (result.edgesUsed == n - 1);
    result.message = result.success
        ? "Cay khung nho nhat da duoc xay dung"
        : "Do thi khong lien thong, khong the xay dung MST day du";

    return result;
}

KruskalResult makeDirectedGraphError() {
    KruskalResult result;
    result.success = false;
    result.message = "Thuat toan Kruskal chi ap dung cho do thi vo huong";
    return result;
}

} // namespace

KruskalResult kruskalSequential(const Graph& graph) {
    if (graph.isDirectedGraph()) {
        return makeDirectedGraphError();
    }

    using Clock = std::chrono::high_resolution_clock;
    auto startTotal = Clock::now();

    int n = graph.size();
    std::vector<Edge> edges = graph.toEdgeList();

    auto startCompute = Clock::now();
    std::sort(edges.begin(), edges.end());
    KruskalResult result = buildMSTFromSortedEdges(edges, n);
    result.computeTime = std::chrono::duration<double>(Clock::now() - startCompute).count();

    result.communicationTime = 0.0;
    result.totalTime = std::chrono::duration<double>(Clock::now() - startTotal).count();

    return result;
}