#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include "edge.hpp"

class Graph {
private:
    int vertices;
    bool directed;
    std::vector<std::vector<int>> matrix;

public:
    Graph();
    Graph(int n, bool isDirected);

    int size() const;
    bool isDirectedGraph() const;

    void addEdge(int u, int v, int weight);
    int getWeight(int u, int v) const;

    const std::vector<std::vector<int>>& getMatrix() const;

    void generateRandom(
        double density,
        int minWeight,
        int maxWeight,
        bool allowNegative,
        int seed
    );

    void generateRandomMPI(
        double density,
        int minWeight,
        int maxWeight,
        bool allowNegative,
        int seed,
        int rank,
        int worldSize
    );

    std::vector<Edge> toEdgeList() const;
    int edgeCount() const;
    void printSmallGraph() const;
};

#endif