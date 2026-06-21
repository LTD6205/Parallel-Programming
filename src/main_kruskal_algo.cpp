#include "graph.hpp"
#include "kruskal.hpp"

#include <iostream>
#include <mpi.h>

using namespace std;

// Ham nay duoc goi tu dispatcher chinh (file main_), KHONG tu goi MPI_Init/
// MPI_Finalize ben trong - viec do thuoc trach nhiem cua dispatcher, giong
// nhu main_bellman_ford / main_coloring.
int main_kruskal(int argc, char** argv) {
    int rank, worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    if (rank == 0) {
        cout << "=== DEMO THUAT TOAN KRUSKAL ===" << endl;
        cout << "So tien trinh MPI: " << worldSize << endl;
    }

    int graphSize = 6;
    if (argc > 1) {
        graphSize = std::atoi(argv[1]);
    }

    Graph graph(graphSize, false);

    // Sinh do thi ngau nhien tren rank 0 roi Bcast cho cac rank con lai,
    // dam bao moi rank co cung mot do thi.
    graph.generateRandomMPI(0.5, 1, 10, false, 42, rank, worldSize);

    if (rank == 0) {
        graph.printSmallGraph();
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // ---------- Chay ban tuan tu (chi tren rank 0) ----------
    if (rank == 0) {
        cout << "\n=== KRUSKAL - BAN TUAN TU ===" << endl;

        KruskalResult result = kruskalSequential(graph);

        if (result.success) {
            cout << result.message << endl;
            cout << "Tong trong so: " << result.totalWeight << endl;
            cout << "Cac canh trong MST (" << result.mstEdges.size() << " canh):\n";
            for (const auto& edge : result.mstEdges) {
                cout << edge.u << " - " << edge.v << " (trong so: " << edge.weight << ")\n";
            }
        } else {
            cout << "Loi: " << result.message << endl;
        }
        cout << "Thoi gian thuc hien: " << result.totalTime * 1000 << " ms" << endl;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // ---------- Chay ban phan tan MPI (tren tat ca cac rank) ----------
    if (rank == 0) {
        cout << "\n=== KRUSKAL - BAN PHAN TAN (MPI) ===" << endl;
    }

    KruskalResult distResult = kruskalMPI(graph, rank, worldSize);

    if (rank == 0) {
        if (distResult.success) {
            cout << distResult.message << endl;
            cout << "Tong trong so: " << distResult.totalWeight << endl;
            cout << "Cac canh trong MST (" << distResult.mstEdges.size() << " canh):\n";
            for (const auto& edge : distResult.mstEdges) {
                cout << edge.u << " - " << edge.v << " (trong so: " << edge.weight << ")\n";
            }
        } else {
            cout << "Loi: " << distResult.message << endl;
        }
        cout << "Thoi gian thuc hien: " << distResult.totalTime * 1000 << " ms" << endl;
        cout << "  - Compute: " << distResult.computeTime * 1000 << " ms" << endl;
        cout << "  - Communication: " << distResult.communicationTime * 1000 << " ms" << endl;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    return 0;
}