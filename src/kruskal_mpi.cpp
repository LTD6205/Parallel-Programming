#include "kruskal.hpp"

#include <algorithm>
#include <mpi.h>
#include <queue>

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
// Chi duoc goi tren rank 0, sau khi da gop xong tat ca canh tu moi rank.
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

void packEdges(const std::vector<Edge>& edges, std::vector<int>& flat) {
    flat.clear();
    flat.reserve(edges.size() * 3);

    for (const Edge& e : edges) {
        flat.push_back(e.u);
        flat.push_back(e.v);
        flat.push_back(e.weight);
    }
}

std::vector<Edge> unpackEdges(const std::vector<int>& flat) {
    std::vector<Edge> edges;
    edges.reserve(flat.size() / 3);

    for (std::size_t i = 0; i + 2 < flat.size(); i += 3) {
        edges.emplace_back(flat[i], flat[i + 1], flat[i + 2]);
    }

    return edges;
}

// Trich xuat cac canh ma rank nay "so huu" theo block-partition tren chi so dinh.
std::vector<Edge> extractLocalEdges(const Graph& graph, int rank, int worldSize) {
    int n = graph.size();
    const auto& matrix = graph.getMatrix();

    int chunkSize = n / worldSize;
    int remainder = n % worldSize;
    int start = rank * chunkSize + std::min(rank, remainder);
    int end = start + chunkSize + (rank < remainder ? 1 : 0);

    std::vector<Edge> localEdges;

    for (int i = start; i < end; ++i) {
        for (int j = i + 1; j < n; ++j) {
            int w = matrix[i][j];
            if (w != 0) {
                localEdges.emplace_back(i, j, w);
            }
        }
    }

    return localEdges;
}

KruskalResult makeDirectedGraphError() {
    KruskalResult result;
    result.success = false;
    result.message = "Thuat toan Kruskal chi ap dung cho do thi vo huong";
    return result;
}

} // namespace

KruskalResult kruskalMPI(const Graph& graph, int rank, int worldSize) {
    if (graph.isDirectedGraph()) {
        return makeDirectedGraphError();
    }

    double startTotal = MPI_Wtime();

    int n = graph.size();

    // Buoc 1: moi rank chi quet phan ma tran ke ung voi cac dinh minh so huu
    // (1D block distribution theo chi so dinh), tranh phai quet toan bo O(V^2)
    // tren tat ca cac rank.
    std::vector<Edge> localEdges = extractLocalEdges(graph, rank, worldSize);

    double startCompute = MPI_Wtime();
    std::sort(localEdges.begin(), localEdges.end());
    double localComputeTime = MPI_Wtime() - startCompute;

    double startComm = MPI_Wtime();

    std::vector<int> localFlat;
    packEdges(localEdges, localFlat);
    int localFlatSize = static_cast<int>(localFlat.size());

    std::vector<int> recvCounts(worldSize);
    MPI_Gather(&localFlatSize, 1, MPI_INT, recvCounts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> displs(worldSize, 0);
    std::vector<int> allFlat;

    if (rank == 0) {
        for (int r = 1; r < worldSize; ++r) {
            displs[r] = displs[r - 1] + recvCounts[r - 1];
        }
        allFlat.resize(displs[worldSize - 1] + recvCounts[worldSize - 1]);
    }

    MPI_Gatherv(
        localFlat.data(), localFlatSize, MPI_INT,
        allFlat.data(), recvCounts.data(), displs.data(), MPI_INT,
        0, MPI_COMM_WORLD
    );

    KruskalResult result;
    double rootMergeTime = 0.0;

    if (rank == 0) {
        // Buoc 2: k-way merge cac doan da sap xep tu tung rank (thay vi sort
        // lai toan bo tu dau) bang mot min-heap theo trong so canh.
        double startMerge = MPI_Wtime();

        struct HeapItem {
            Edge edge;
            int rankIdx;
            int posInRank;

            bool operator>(const HeapItem& other) const {
                return edge.weight > other.edge.weight;
            }
        };

        std::vector<int> rankEdgeCounts(worldSize);
        for (int r = 0; r < worldSize; ++r) {
            rankEdgeCounts[r] = recvCounts[r] / 3;
        }

        std::priority_queue<HeapItem, std::vector<HeapItem>, std::greater<HeapItem>> heap;

        for (int r = 0; r < worldSize; ++r) {
            if (rankEdgeCounts[r] > 0) {
                int base = displs[r];
                Edge e(allFlat[base], allFlat[base + 1], allFlat[base + 2]);
                heap.push(HeapItem{e, r, 0});
            }
        }

        std::vector<Edge> mergedEdges;
        mergedEdges.reserve(allFlat.size() / 3);

        while (!heap.empty()) {
            HeapItem top = heap.top();
            heap.pop();
            mergedEdges.push_back(top.edge);

            int nextPos = top.posInRank + 1;
            if (nextPos < rankEdgeCounts[top.rankIdx]) {
                int base = displs[top.rankIdx] + nextPos * 3;
                Edge e(allFlat[base], allFlat[base + 1], allFlat[base + 2]);
                heap.push(HeapItem{e, top.rankIdx, nextPos});
            }
        }

        // Buoc 3: Union-Find tuan tu (khong the song song hoa buoc nay).
        result = buildMSTFromSortedEdges(mergedEdges, n);

        rootMergeTime = MPI_Wtime() - startMerge;
    }

    // Buoc 4: phat (broadcast) ket qua MST cuoi cung tu rank 0 cho tat ca cac rank.
    int edgesUsedBcast = result.edgesUsed;
    MPI_Bcast(&edgesUsedBcast, 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> mstFlat;
    if (rank == 0) {
        packEdges(result.mstEdges, mstFlat);
    } else {
        mstFlat.resize(static_cast<std::size_t>(edgesUsedBcast) * 3);
    }
    MPI_Bcast(mstFlat.data(), edgesUsedBcast * 3, MPI_INT, 0, MPI_COMM_WORLD);

    int totalWeightBcast = result.totalWeight;
    MPI_Bcast(&totalWeightBcast, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int successFlag = result.success ? 1 : 0;
    MPI_Bcast(&successFlag, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        result.mstEdges = unpackEdges(mstFlat);
        result.edgesUsed = edgesUsedBcast;
        result.totalWeight = totalWeightBcast;
        result.success = (successFlag != 0);
        result.message = result.success
            ? "MST da duoc xay dung theo cach phan tan"
            : "Do thi khong lien thong, khong the xay dung MST day du";
    }

    double commWindow = MPI_Wtime() - startComm;

    result.computeTime = (rank == 0) ? (localComputeTime + rootMergeTime) : localComputeTime;
    result.communicationTime = (rank == 0) ? (commWindow - rootMergeTime) : commWindow;
    result.totalTime = MPI_Wtime() - startTotal;

    return result;
}