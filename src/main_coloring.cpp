#include "coloring.hpp"
#include "correctness.hpp"
#include "graph.hpp"

#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include <string>

namespace {
    int getIntArg(
        int argc,
        char** argv,
        const std::string& key,
        int defaultValue
    ) {
        for (int i = 1; i + 1 < argc; ++i) {
            if (argv[i] == key) {
                return std::atoi(argv[i + 1]);
            }
        }

        return defaultValue;
    }

    double getDoubleArg(
        int argc,
        char** argv,
        const std::string& key,
        double defaultValue
    ) {
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

    int vertices = getIntArg(argc, argv, "--vertices", 1000);
    double density = getDoubleArg(argc, argv, "--density", 0.1);
    int seed = getIntArg(argc, argv, "--seed", 42);
    int maxIterations = getIntArg(argc, argv, "--max-iterations", vertices * 2);

    Graph graph(vertices, false);

    graph.generateRandomMPI(
        density,
        1,
        10,
        false,
        seed,
        rank,
        worldSize
    );

    if (rank == 0) {
        std::cout << "Graph coloring test\n";
        std::cout << "Vertices: " << vertices << "\n";
        std::cout << "Edges: " << graph.edgeCount() << "\n";
        std::cout << "Density: " << density << "\n";
        std::cout << "MPI processes: " << worldSize << "\n\n";
    }

    ColoringResult seqResult;

    if (rank == 0) {
        seqResult = greedyColoringSequential(graph);

        std::cout << "[Sequential]\n";
        std::cout << "Correct: " << (seqResult.success ? "YES" : "NO") << "\n";
        std::cout << "Colors used: " << seqResult.numberOfColors << "\n";
        std::cout << "Conflicts: " << seqResult.conflicts << "\n";
        std::cout << "Total time: " << seqResult.totalTime << " seconds\n\n";
    }

    MPI_Barrier(MPI_COMM_WORLD);

    ColoringResult mpiResult = greedyColoringMPI(
        graph,
        rank,
        worldSize,
        maxIterations
    );

    double maxTotalTime = 0.0;
    double maxComputeTime = 0.0;
    double maxCommunicationTime = 0.0;

    MPI_Reduce(
        &mpiResult.totalTime,
        &maxTotalTime,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );

    MPI_Reduce(
        &mpiResult.computeTime,
        &maxComputeTime,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );

    MPI_Reduce(
        &mpiResult.communicationTime,
        &maxCommunicationTime,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        MPI_COMM_WORLD
    );

    if (rank == 0) {
        bool validMPI = isValidColoring(graph, mpiResult.colors);

        std::cout << "[MPI]\n";
        std::cout << "Correct: " << (validMPI ? "YES" : "NO") << "\n";
        std::cout << "Colors used: " << mpiResult.numberOfColors << "\n";
        std::cout << "Conflicts: " << mpiResult.conflicts << "\n";
        std::cout << "Iterations: " << mpiResult.iterations << "\n";
        std::cout << "Max total time: " << maxTotalTime << " seconds\n";
        std::cout << "Max compute time: " << maxComputeTime << " seconds\n";
        std::cout << "Max communication time: " << maxCommunicationTime << " seconds\n";

        std::cout << "\nNote:\n";
        std::cout << "MPI coloring does not need to use the same number of colors as sequential coloring.\n";
        std::cout << "Correctness means no adjacent vertices share the same color.\n";
    }

    MPI_Finalize();

    return 0;
}