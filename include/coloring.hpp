#ifndef COLORING_HPP
#define COLORING_HPP

#include <string>
#include <vector>
#include "graph.hpp"

struct ColoringResult {
    std::vector<int> colors;

    int numberOfColors = 0;
    int iterations = 0;
    int conflicts = 0;
    int localVertices = 0;

    double totalTime = 0.0;
    double computeTime = 0.0;
    double communicationTime = 0.0;

    bool success = false;
    std::string message;
};

ColoringResult greedyColoringSequential(const Graph& graph);

ColoringResult greedyColoringMPI(
    const Graph& graph,
    int rank,
    int worldSize,
    int maxIterations = 1000
);

#endif