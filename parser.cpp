// STD
#include <iostream>

// SDSL
//#include <sdsl/suffix_arrays.hpp>
#include <sdsl/rank_support.hpp>
#include <sdsl/select_support.hpp>
#include <sdsl/int_vector.hpp>
//#include <sdsl/enc_vector.hpp>

// HYPERRDF
//#include "construct.hpp"
#include "type_definitions.hpp"


using namespace std;
using namespace sdsl;


#define MAX_LINE_LENGTH (1024).
int parse_graph(const char *input_file, HyperGraph& graph, uint64_t lines_to_read) {
    std::ifstream infile(input_file);
    std::string line;
    EdgeList edges;

    uint64_t num_lines = 0;

    while (std::getline(infile, line)) {

        // Construct edge
        Edge edge;
        uint64_t num;
        std::istringstream ss;
        ss.str(line);
        while (ss >> num) {
            edge.push_back(num);
        }

        edges.push_back(edge);
        ++num_lines;
        if (num_lines >= lines_to_read) {
            break;
        }
    }

    graph.edge_count = edges.size();
    graph.edges = edges;

    return 0;
}


int write_hyper_rdf(const char *output_file, HyperRDF& r)
{
    std::ofstream out(output_file);
    r.d.serialize(out);
    r.psi.serialize(out);
    out.flush();
    out.close();
    return 0;
}

HyperRDF load_hyper_rdf(const char *input_file) {
    ifstream in(input_file);
    if (!in.is_open()) {
        throw std::runtime_error(std::string("Could not open file: ") + input_file);
    }
    sdsl::enc_vector<> psi;
    sdsl::bit_vector d;
    try {
        d.load(in);
        psi.load(in);
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Error loading SDSL data: ") + e.what());
    }
    in.close();
    HyperRDF r = HyperRDF();
    r.d = move(d);
    r.psi = move(psi);
    return r;
}