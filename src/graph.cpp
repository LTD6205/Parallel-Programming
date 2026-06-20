#include "graph.hpp"

#include <algorithm>
#include <iostream>
#include <mpi.h>
#include <random>
#include <stdexcept>

Graph::Graph()
    : vertices(0), directed(false) {}

Graph::Graph(int n, bool isDirected)
    : vertices(n), directed(isDirected), matrix(n, std::vector<int>(n, 0)) {
    if (n < 0) {
        throw std::invalid_argument("Number of vertices must be non-negative");
    }
}

int Graph::size() const {
    return vertices;
}

bool Graph::isDirectedGraph() const {
    return directed;
}

void Graph::addEdge(int u, int v, int weight) {
    if (u < 0 || u >= vertices || v < 0 || v >= vertices) {
        throw std::out_of_range("Invalid vertex index in addEdge");
    }

    if (u == v) {
        return;
    }

    matrix[u][v] = weight;

    if (!directed) {
        matrix[v][u] = weight;
    }
}

int Graph::getWeight(int u, int v) const {
    if (u < 0 || u >= vertices || v < 0 || v >= vertices) {
        throw std::out_of_range("Invalid vertex index in getWeight");
    }

    return matrix[u][v];
}

const std::vector<std::vector<int>>& Graph::getMatrix() const {
    return matrix;
}

void Graph::generateRandom(
    double density,
    int minWeight,
    int maxWeight,
    bool allowNegative,
    int seed
) {
    if (density < 0.0 || density > 1.0) {
        throw std::invalid_argument("Density must be between 0 and 1");
    }

    if (minWeight > maxWeight) {
        throw std::invalid_argument("minWeight must be <= maxWeight");
    }

    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> probability(0.0, 1.0);
    std::uniform_int_distribution<int> weightDist(minWeight, maxWeight);

    matrix.assign(vertices, std::vector<int>(vertices, 0));

    if (directed) {
        for (int u = 0; u < vertices; ++u) {
            for (int v = 0; v < vertices; ++v) {
                if (u == v) {
                    continue;
                }

                if (probability(gen) <= density) {
                    int w = weightDist(gen);

                    if (!allowNegative && w < 0) {
                        w = -w;
                    }

                    if (w == 0) {
                        w = 1;
                    }

                    matrix[u][v] = w;
                }
            }
        }
    } else {
        for (int u = 0; u < vertices; ++u) {
            for (int v = u + 1; v < vertices; ++v) {
                if (probability(gen) <= density) {
                    int w = weightDist(gen);

                    if (!allowNegative && w < 0) {
                        w = -w;
                    }

                    if (w == 0) {
                        w = 1;
                    }

                    matrix[u][v] = w;
                    matrix[v][u] = w;
                }
            }
        }
    }
}

void Graph::generateRandomMPI(
    double density,
    int minWeight,
    int maxWeight,
    bool allowNegative,
    int seed,
    int rank,
    int /*worldSize*/
) {
    if (rank == 0) {
        generateRandom(density, minWeight, maxWeight, allowNegative, seed);
    }

    std::vector<int> flat(vertices * vertices, 0);

    if (rank == 0) {
        for (int i = 0; i < vertices; ++i) {
            for (int j = 0; j < vertices; ++j) {
                flat[i * vertices + j] = matrix[i][j];
            }
        }
    }

    MPI_Bcast(
        flat.data(),
        static_cast<int>(flat.size()),
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    if (rank != 0) {
        matrix.assign(vertices, std::vector<int>(vertices, 0));

        for (int i = 0; i < vertices; ++i) {
            for (int j = 0; j < vertices; ++j) {
                matrix[i][j] = flat[i * vertices + j];
            }
        }
    }
}

std::vector<Edge> Graph::toEdgeList() const {
    std::vector<Edge> edges;

    if (directed) {
        for (int u = 0; u < vertices; ++u) {
            for (int v = 0; v < vertices; ++v) {
                if (u != v && matrix[u][v] != 0) {
                    edges.emplace_back(u, v, matrix[u][v]);
                }
            }
        }
    } else {
        for (int u = 0; u < vertices; ++u) {
            for (int v = u + 1; v < vertices; ++v) {
                if (matrix[u][v] != 0) {
                    edges.emplace_back(u, v, matrix[u][v]);
                }
            }
        }
    }

    return edges;
}

int Graph::edgeCount() const {
    return static_cast<int>(toEdgeList().size());
}

void Graph::printSmallGraph() const {
    std::cout << "Adjacency matrix (" << vertices << " vertices):\n";

    for (int i = 0; i < vertices; ++i) {
        for (int j = 0; j < vertices; ++j) {
            std::cout << matrix[i][j] << ' ';
        }

        std::cout << '\n';
    }
}