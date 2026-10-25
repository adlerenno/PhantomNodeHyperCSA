// STD
#include <iostream>
#include <optional>

// SDSL
//#include <sdsl/suffix_arrays.hpp>
#include <sdsl/rank_support.hpp>
#include <sdsl/select_support.hpp>
#include <sdsl/int_vector.hpp>
//#include <sdsl/enc_vector.hpp>

// HYPERRDF
//#include "construct.hpp"
#include "parser.hpp"
#include "type_definitions.hpp"


using namespace std;
using namespace sdsl;

// credit: Enno Adler
struct SearchInterval {
    uint64_t lower_bound;
    uint64_t higher_bound;

    SearchInterval(uint64_t low, uint64_t high)
            : lower_bound(low), higher_bound(high) {}

    void print() const {
        cout << "interval[" << lower_bound << ", " << higher_bound << "]";
    }

    uint64_t size() const {
        return higher_bound - lower_bound;
    }
};


std::optional<Edge> decompress_edge(enc_vector<> *psi, rank_support_v<1>& rank_d, map<int64_t, uint64_t> checklist, uint64_t index, set<int64_t>* already_decomp) {
    bool debug_prints = false;
    int64_t first_node = rank_d(index+1)-1;
    if (debug_prints) {
        cout << "start decompress edge" << endl;
        cout << "d[" << index << "] -> " << first_node << endl;
    }
    Edge decomp;
    decomp.push_back(first_node);
    size_t j = (*psi)[index];
    while (j != index)
    {
        int64_t current_node = rank_d(j+1)-1;
        if (current_node == first_node) {
            (*already_decomp).insert(j);
        }
        decomp.push_back(current_node);
        if (auto it = checklist.find(current_node); it != checklist.end()) {
            it->second--;
            if (it->second <= 0) {
                checklist.erase(current_node);
            }
        }
        // early return possible?
        for (auto n: checklist) {
            bool jumped_down = current_node < first_node;
            bool skipped =                  !jumped_down && n.first > first_node && n.first < current_node;
            bool jump_down_early =           jumped_down && n.first > first_node;
            bool smallest_nodes_skipped =    jumped_down && n.first < current_node;
            if (skipped || jump_down_early || smallest_nodes_skipped) {
                if (debug_prints) {
                    cout << " early return" << endl;
                }
                return std::nullopt;
            }
        }
        j = (*psi)[j];
    }
    // remove the last node (which is the same as first_node) manually
    if (auto it = checklist.find(first_node); it != checklist.end()) {
        if (it->second == 1) {
            checklist.erase(first_node);
        }
    }
    // all nodes in e found?
    if (!checklist.empty()) {
        return std::nullopt;
    }
    return decomp;
}

EdgeList query(HyperRDF r, Edge e) {

    for (size_t i = 0; i < e.size(); i++) {
        e[i] = e[i]-1; // because the data had 1 as lowest node, but got 0-based during construction
    }

    rank_support_v<1> rank_d(&r.d);
    select_support_mcl<1> select_d(&r.d);

    // TODO what if e contains numbers >= omega -> use rank to find out

    SearchInterval best_interval(0, 0); 
    bool first = true;

    for (int node : e) {
        SearchInterval current(select_d(node+1),
                            select_d(node+2)); // this is why we need the extra 1 in d
        if (first || current.size() < best_interval.size()) {
            best_interval = current;
            first = false;
        }
    }

    map<int64_t, uint64_t> checklist_e;
    for (size_t i = 0; i < e.size(); ++i) {
        checklist_e[e[i]]++;
    }

    EdgeList solution;
    set<int64_t> already_decomp;
    for (size_t i = best_interval.lower_bound; i < best_interval.higher_bound; i++) {
        if (already_decomp.count(i) >= 1) {
            continue; // this node was in an edge that was already decompressed
        }
        auto decomp_opt = decompress_edge(&r.psi, rank_d, map<int64_t, uint64_t>(checklist_e), i, &already_decomp);
        if (decomp_opt.has_value()) {
            for (size_t i = 0; i < (*decomp_opt).size(); i++) {
                (*decomp_opt)[i]++; // see the -1 at the beginning of this function
            }
            solution.push_back(*decomp_opt);
        }
    }

    return solution;
}

/*
bool query_contains(HyperRDF r, Edge e) {
    
}*/


int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Command line parameters have to be given: first name of hyper rdf file, then at least one node to search" << endl;
    }
    // In case of command line arguments
    string input_file = argv[1];
    Edge query_edge = Edge();
    for (int i = 2; i < argc; ++i) {
        try {
            query_edge.push_back(static_cast<uint64_t>(stoull(argv[i])));
        } catch (...) {
            cerr << "invalid argument: " << argv[i] << " is not a number\n";
            return 1;
        }
    }
    //cout << "Using parameters:\n"
    //         << " - Input: " << input_file << "\n"
    //         << " - Query: " << query_edge << "\n\n";

    HyperRDF r = load_hyper_rdf(input_file.c_str());
    
    EdgeList sol = query(r, query_edge);
    //cout << "Result:" << endl;
    for (Edge e: sol) {
        cout << e << "\n";
    }
    return 0;
}