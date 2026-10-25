#pragma once
#include <sdsl/bit_vectors.hpp>
#include <sdsl/enc_vector.hpp>
#include <sdsl/int_vector.hpp>

using namespace std;
using namespace sdsl;

typedef uint64_t Node;
typedef vector<Node> Edge;
typedef vector<Edge> EdgeList;

struct HyperGraph {
    uint64_t edge_count;
    EdgeList edges;
};

struct LinearRepresentation {
    int_vector<64> nodes;
    Node omega;
    size_t size;
};

struct HyperRDF {
    enc_vector<> psi;
    bit_vector d;
};
