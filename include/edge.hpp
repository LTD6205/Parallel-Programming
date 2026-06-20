#ifndef EDGE_HPP
#define EDGE_HPP

struct Edge {
    int u;
    int v;
    int weight;

    Edge() : u(0), v(0), weight(0) {}

    Edge(int from, int to, int w)
        : u(from), v(to), weight(w) {}

    bool operator<(const Edge& other) const {
        return weight < other.weight;
    }
};

#endif