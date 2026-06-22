#include "graph.hpp"
#include "kruskal.hpp"
#include "correctness.hpp"

#include <iostream>
#include <mpi.h>
#include <cstdlib>
#include <string>

using namespace std;

namespace {
    // Hàm hỗ trợ đọc tham số cấu hình từ Terminal
    int getIntArg(int argc, char** argv, const std::string& key, int defaultValue) {
        for (int i = 1; i + 1 < argc; ++i) {
            if (argv[i] == key) {
                return std::atoi(argv[i + 1]);
            }
        }
        return defaultValue;
    }

    double getDoubleArg(int argc, char** argv, const std::string& key, double defaultValue) {
        for (int i = 1; i + 1 < argc; ++i) {
            if (argv[i] == key) {
                return std::atof(argv[i + 1]);
            }
        }
        return defaultValue;
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank = 0;
    int worldSize = 1;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    // Lấy thông số từ lệnh mpirun (giống hệt thuật toán Tô màu đồ thị)
    int vertices = getIntArg(argc, argv, "--vertices", 1000);
    double density = getDoubleArg(argc, argv, "--density", 0.1);
    int seed = getIntArg(argc, argv, "--seed", 42);

    Graph graph(vertices, false);

    // Sinh đồ thị trên rank 0 và Broadcast đi toàn mạng
    graph.generateRandomMPI(density, 1, 10, false, seed, rank, worldSize);

    if (rank == 0) {
        cout << "Kruskal MST test\n";
        cout << "Vertices: " << vertices << "\n";
        cout << "Edges: " << graph.edgeCount() << "\n";
        cout << "Density: " << density << "\n";
        cout << "MPI processes: " << worldSize << "\n\n";
    }

    KruskalResult seqResult;

    // ----- CHẠY TUẦN TỰ -----
    if (rank == 0) {
        seqResult = kruskalSequential(graph);

        cout << "[Sequential]\n";
        cout << "Correct: " << (seqResult.success ? "YES" : "NO") << "\n";
        cout << "Total weight: " << seqResult.totalWeight << "\n";
        // Bỏ nhân 1000 để hiển thị đơn vị giây (seconds) thay vì mili-giây
        cout << "Total time: " << seqResult.totalTime << " seconds\n\n"; 
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // ----- CHẠY SONG SONG MPI -----
    KruskalResult mpiResult = kruskalMPI(graph, rank, worldSize);

    double maxTotalTime = 0.0;
    double maxComputeTime = 0.0;
    double maxCommunicationTime = 0.0;

    // Gom thời gian chạy lớn nhất từ các máy ảo về Master
    MPI_Reduce(&mpiResult.totalTime, &maxTotalTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&mpiResult.computeTime, &maxComputeTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&mpiResult.communicationTime, &maxCommunicationTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "[MPI]\n";
        cout << "Correct: " << (mpiResult.success ? "YES" : "NO") << "\n";
        cout << "Total weight: " << mpiResult.totalWeight << "\n";
        cout << "Max total time: " << maxTotalTime << " seconds\n";
        cout << "Max compute time: " << maxComputeTime << " seconds\n";
        cout << "Max communication time: " << maxCommunicationTime << " seconds\n";
        
        if (seqResult.totalWeight != mpiResult.totalWeight) {
            cout << "\nWARNING: MPI weight (" << mpiResult.totalWeight 
                 << ") does not match Sequential weight (" << seqResult.totalWeight << ")!\n";
        }
    }

    MPI_Finalize();
    return 0;
}