#include "coloring.hpp"
#include "correctness.hpp"

#include <algorithm>
#include <mpi.h>
#include <vector>

namespace {
    int ownerStart(int rank, int worldSize, int n) {
        int base = n / worldSize;
        int remainder = n % worldSize;

        return rank * base + std::min(rank, remainder);
    }

    int ownerEnd(int rank, int worldSize, int n) {
        int base = n / worldSize;
        int remainder = n % worldSize;

        return ownerStart(rank, worldSize, n) + base + (rank < remainder ? 1 : 0);
    }
}

ColoringResult greedyColoringMPI(
    const Graph& graph,
    int rank,
    int worldSize,
    int maxIterations
) {
    double totalStart = MPI_Wtime();

    ColoringResult result;
    int n = graph.size();

    result.colors.assign(n, -1);

    int startVertex = ownerStart(rank, worldSize, n);
    int endVertex = ownerEnd(rank, worldSize, n);

    result.localVertices = endVertex - startVertex;

    double computeTime = 0.0;
    double communicationTime = 0.0;

    if (n == 0) {
        result.success = true;
        result.message = "Empty graph";
        result.totalTime = MPI_Wtime() - totalStart;
        return result;
    }

    int globalHasConflict = 1;
    int iteration = 0;
    int finalConflicts = 0;

    while (globalHasConflict && iteration < maxIterations) {
        ++iteration;

        double computeStart = MPI_Wtime();

        for (int u = startVertex; u < endVertex; ++u) {
            if (result.colors[u] != -1) {
                continue;
            }

            std::vector<bool> used(n, false);

            for (int v = 0; v < n; ++v) {
                if (graph.getWeight(u, v) != 0 && result.colors[v] != -1) {
                    used[result.colors[v]] = true;
                }
            }

            int chosenColor = 0;

            while (chosenColor < n && used[chosenColor]) {
                ++chosenColor;
            }

            result.colors[u] = chosenColor;
        }

        computeTime += MPI_Wtime() - computeStart;

        double communicationStart = MPI_Wtime();

        std::vector<int> globalColors(n, -1);

        MPI_Allreduce(
            result.colors.data(),
            globalColors.data(),
            n,
            MPI_INT,
            MPI_MAX,
            MPI_COMM_WORLD
        );

        result.colors.swap(globalColors);

        communicationTime += MPI_Wtime() - communicationStart;

        computeStart = MPI_Wtime();

        int localConflicts = 0;

        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                if (graph.getWeight(u, v) != 0 &&
                    result.colors[u] != -1 &&
                    result.colors[u] == result.colors[v]) {
                    ++localConflicts;

                    /*
                        Deterministic conflict resolution:
                        keep the smaller vertex id colored,
                        uncolor the larger vertex id,
                        then recolor it in the next round.
                    */
                    result.colors[v] = -1;
                }
            }
        }

        computeTime += MPI_Wtime() - computeStart;

        communicationStart = MPI_Wtime();

        MPI_Allreduce(
            &localConflicts,
            &finalConflicts,
            1,
            MPI_INT,
            MPI_MAX,
            MPI_COMM_WORLD
        );

        globalHasConflict = finalConflicts > 0 ? 1 : 0;

        communicationTime += MPI_Wtime() - communicationStart;
    }

    result.iterations = iteration;
    result.conflicts = countColorConflicts(graph, result.colors);
    result.numberOfColors = countColors(result.colors);
    result.success = isValidColoring(graph, result.colors);

    result.message = result.success
        ? "MPI coloring passed"
        : "MPI coloring failed: invalid coloring or max iterations reached";

    result.computeTime = computeTime;
    result.communicationTime = communicationTime;
    result.totalTime = MPI_Wtime() - totalStart;

    return result;
}