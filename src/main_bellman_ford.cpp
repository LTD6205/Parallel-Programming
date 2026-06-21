#include <mpi.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "bellman_ford.hpp"
#include "correctness.hpp"
#include "graph.hpp"

namespace {

struct Options {
    int vertices = 1000;
    double density = 0.1;
    int minWeight = 1;
    int maxWeight = 20;
    bool allowNegative = false;
    int seed = 42;
    int source = 0;
};

Options parseArgs(int argc, char** argv) {
    Options opt;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--vertices" && i + 1 < argc) {
            opt.vertices = std::atoi(argv[++i]);
        } else if (arg == "--density" && i + 1 < argc) {
            opt.density = std::atof(argv[++i]);
        } else if (arg == "--min-weight" && i + 1 < argc) {
            opt.minWeight = std::atoi(argv[++i]);
        } else if (arg == "--max-weight" && i + 1 < argc) {
            opt.maxWeight = std::atoi(argv[++i]);
        } else if (arg == "--negative") {
            opt.allowNegative = true;
        } else if (arg == "--seed" && i + 1 < argc) {
            opt.seed = std::atoi(argv[++i]);
        } else if (arg == "--source" && i + 1 < argc) {
            opt.source = std::atoi(argv[++i]);
        }
    }

    return opt;
}

} // namespace

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank = 0;
    int worldSize = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    const Options opt = parseArgs(argc, argv);

    // Rank 0 generates the graph, then it is broadcast to every rank
    // via MPI_Bcast (see Graph::generateRandomMPI). This avoids redundant
    // O(V^2) random generation on every machine and exercises real
    // network communication across the cluster.
    Graph graph(opt.vertices, /*isDirected=*/true);
    graph.generateRandomMPI(
        opt.density, opt.minWeight, opt.maxWeight, opt.allowNegative,
        opt.seed, rank, worldSize
    );

    const BellmanFordResult mpiResult =
        bellmanFordMPI(graph, opt.source, rank, worldSize);

    if (rank == 0) {
        const BellmanFordResult seqResult =
            bellmanFordSequential(graph, opt.source);

        const bool structurallyValid =
            isValidBellmanFord(graph, opt.source, mpiResult.dist);
        const bool matchesSequential =
            distancesMatch(seqResult.dist, mpiResult.dist);
        const int mismatches =
            countDistanceMismatches(seqResult.dist, mpiResult.dist);

        std::cout << "=== Bellman-Ford Benchmark ===\n";
        std::cout << "Vertices:        " << opt.vertices << "\n";
        std::cout << "Density:         " << opt.density << "\n";
        std::cout << "Seed:            " << opt.seed << "\n";
        std::cout << "Source:          " << opt.source << "\n";
        std::cout << "World size:      " << worldSize << "\n";
        std::cout << "---\n";
        std::cout << "Sequential total time:   " << seqResult.totalTime << " s\n";
        std::cout << "MPI total time:          " << mpiResult.totalTime << " s\n";
        std::cout << "MPI compute time:        " << mpiResult.computeTime << " s\n";
        std::cout << "MPI communication time:  " << mpiResult.communicationTime << " s\n";
        std::cout << "Iterations (sequential): " << seqResult.iterations << "\n";
        std::cout << "Iterations (MPI):        " << mpiResult.iterations << "\n";
        std::cout << "Negative cycle (seq):    " << seqResult.negativeCycleDetected << "\n";
        std::cout << "Negative cycle (MPI):    " << mpiResult.negativeCycleDetected << "\n";
        std::cout << "---\n";
        std::cout << "MPI result structurally valid: "
                  << (structurallyValid ? "yes" : "NO") << "\n";
        std::cout << "Sequential vs MPI distances match: "
                  << (matchesSequential ? "yes" : "NO") << "\n";
        std::cout << "Mismatched vertices: " << mismatches << "\n";

        if (!structurallyValid || !matchesSequential) {
            std::cout << "\n*** CORRECTNESS CHECK FAILED ***\n";
        }
    }

    MPI_Finalize();
    return 0;
}
