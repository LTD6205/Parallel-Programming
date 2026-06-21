#ifndef KRUSKAL_HPP
#define KRUSKAL_HPP

#include <string>
#include <vector>
#include "edge.hpp"
#include "graph.hpp"

struct KruskalResult {
    std::vector<Edge> mstEdges;

    int totalWeight = 0;
    int edgesUsed = 0;

    double totalTime = 0.0;
    double computeTime = 0.0;
    double communicationTime = 0.0;

    bool success = false;
    std::string message;
};

// Sequential Kruskal's algorithm: sort all edges by weight, then add each
// edge to the MST via Union-Find as long as it does not close a cycle.
// O(E log E) for the sort, O(E) amortized for the Union-Find operations.
// Kruskal's algorithm is defined for undirected graphs; calling this on a
// directed graph returns success = false.
KruskalResult kruskalSequential(const Graph& graph);

// MPI Kruskal's algorithm.
// Decomposition: 1D block distribution of vertices across ranks. Each rank
// extracts only the edges incident to the vertices it owns (so the O(V^2)
// scan of the adjacency matrix is split across ranks instead of being
// repeated in full by everyone), then sorts its local edge slice. The
// sorted slices are gathered to rank 0 with MPI_Gatherv and merged via a
// k-way merge of the worldSize sorted runs; rank 0 then runs the inherently
// sequential Union-Find pass to build the MST. The final result (edges,
// weight, success) is broadcast back to every rank with MPI_Bcast.
// Assumes the graph matrix is already identical on every rank (e.g. after
// Graph::generateRandomMPI). Returns success = false for directed graphs.
KruskalResult kruskalMPI(
    const Graph& graph,
    int rank,
    int worldSize
);

#endif