#include "coloring.hpp"
#include "correctness.hpp"

#include <mpi.h>
#include <vector>

ColoringResult greedyColoringSequential(const Graph& graph) {
    double start = MPI_Wtime();

    ColoringResult result;
    int n = graph.size();

    result.colors.assign(n, -1);
    result.localVertices = n;

    if (n == 0) {
        result.success = true;
        result.message = "Empty graph";
        result.totalTime = MPI_Wtime() - start;
        return result;
    }

    for (int u = 0; u < n; ++u) {
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

    result.numberOfColors = countColors(result.colors);
    result.conflicts = countColorConflicts(graph, result.colors);
    result.success = isValidColoring(graph, result.colors);

    result.message = result.success
        ? "Sequential greedy coloring passed"
        : "Sequential greedy coloring failed";

    result.totalTime = MPI_Wtime() - start;
    result.computeTime = result.totalTime;
    result.communicationTime = 0.0;

    return result;
}