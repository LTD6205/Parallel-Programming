#include "bellman_ford.hpp"

#include <mpi.h>

BellmanFordResult bellmanFordMPI(
    const Graph& graph,
    int source,
    int rank,
    int worldSize
) {
    BellmanFordResult result;
    result.source = source;

    const int V = graph.size();

    if (source < 0 || source >= V) {
        result.success = false;
        result.message = "Invalid source vertex";
        return result;
    }

    const double t0 = MPI_Wtime();

    const std::vector<Edge> edges = graph.toEdgeList();
    const int E = static_cast<int>(edges.size());

    // Decomposition: 1D block distribution of the edge list.
    // Rank r owns edges with index r, r + worldSize, r + 2*worldSize, ...
    std::vector<Edge> localEdges;
    for (int i = rank; i < E; i += worldSize) {
        localEdges.push_back(edges[i]);
    }

    std::vector<int> dist(V, BF_INF);
    dist[source] = 0;

    double computeTime = 0.0;
    double communicationTime = 0.0;
    int iterations = 0;

    for (int iter = 1; iter <= V - 1; ++iter) {
        const double computeStart = MPI_Wtime();

        std::vector<int> localDist = dist;
        int localChanged = 0;

        for (const Edge& e : localEdges) {
            if (dist[e.u] != BF_INF && dist[e.u] + e.weight < localDist[e.v]) {
                localDist[e.v] = dist[e.u] + e.weight;
                localChanged = 1;
            }
        }

        computeTime += MPI_Wtime() - computeStart;

        const double commStart = MPI_Wtime();

        std::vector<int> newDist(V);
        MPI_Allreduce(
            localDist.data(), newDist.data(), V,
            MPI_INT, MPI_MIN, MPI_COMM_WORLD
        );

        int globalChanged = 0;
        MPI_Allreduce(
            &localChanged, &globalChanged, 1,
            MPI_INT, MPI_LOR, MPI_COMM_WORLD
        );

        communicationTime += MPI_Wtime() - commStart;

        dist = newDist;
        iterations = iter;

        if (!globalChanged) {
            break;
        }
    }

    // Negative-cycle check, replicated the same way across ranks:
    // one more local relaxation pass, then OR the result globally.
    int localNegativeCycle = 0;
    for (const Edge& e : localEdges) {
        if (dist[e.u] != BF_INF && dist[e.u] + e.weight < dist[e.v]) {
            localNegativeCycle = 1;
            break;
        }
    }

    int globalNegativeCycle = 0;
    MPI_Allreduce(
        &localNegativeCycle, &globalNegativeCycle, 1,
        MPI_INT, MPI_LOR, MPI_COMM_WORLD
    );

    result.dist = dist;
    result.iterations = iterations;
    result.negativeCycleDetected = (globalNegativeCycle != 0);
    result.totalTime = MPI_Wtime() - t0;
    result.computeTime = computeTime;
    result.communicationTime = communicationTime;
    result.success = true;
    result.message = result.negativeCycleDetected ? "Negative cycle detected" : "OK";

    return result;
}
