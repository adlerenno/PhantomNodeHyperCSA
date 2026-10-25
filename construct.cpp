
#include <iostream>
#include <sdsl/suffix_arrays.hpp>
#include "construct.hpp"
#include "type_definitions.hpp"

using namespace std;
using namespace sdsl;

int create_d(int_vector<64> *t, bit_vector *d) {
    map<uint64_t, size_t> frequencies;
    for (auto element: *t) {
        frequencies[element] ++;
    }

    int index = 0;
    for (const auto& [key, value] : frequencies) {
        (*d)[index] = true;
        index += value;
    }

    return 0;
}

int modify_psi_and_d(enc_vector<>& psi, bit_vector& d) {
    // determine first omega (backtracking)
    int i = psi.size() - 2; // the last element of d is a 1 for the omega+1 due to sdsl. Skip this last 1.
    while (d[i] == 0) {
        i--;
    }

    int new_length = i;
    int jump_to_i;

    int_vector<> new_psi(new_length);
    for (int j = 0; j < new_length; j++) {
        if (psi[j] < i) {
            new_psi[j] = psi[j]; // used [0, .., i-1]
        } else if (psi[j] > i+1) {
            new_psi[j] = psi[psi[j] - 1]; // used [psi[i+1], psi[i+2], ..., psi[-2]]
        } else if (psi[j] == i+1) {
            new_psi[j] = psi[psi.size() - 1]; // used psi[-1]
        } else if (psi[j] == i) {
            new_psi[j] = psi[i]; // used psi[i]
            jump_to_i = j;
        }
    }
    swap(new_psi[jump_to_i], new_psi[0]); // now the 0 node is a single edge and not contained in the last edge
    // remove 0 edge
    uint64_t prev_node = 0;
    uint64_t current_node = 0;
    for (int j = new_length-1; j >= 0; j--) {
        current_node = new_psi[j];
        new_psi[j] = prev_node-1;
        prev_node = current_node;
    }
    new_psi.resize(new_length-1);

    enc_vector<> new_psi_enc(new_psi);

    // swap for new psi and d
    psi.swap(new_psi_enc);
    
    bit_vector new_d(new_length); // length of new_psi +1, because we need an additional 1 at the end for searching
    std::copy_n(d.begin() + 1, new_length, new_d.begin()); // leave out the first entry, because it was for the singular 0
    d.swap(new_d);

    return 0;
}

int assert_no_zero(int_vector<64> *t) {
    // smallest element must not be zero for sdsl
    int num_zeros = count((*t).begin(), (*t).end(), 0);
    if (num_zeros > 0) {
        for (size_t i = 0; i < (*t).size(); ++i)
            (*t)[i]++;
    }
    return 0;
}

bool compare_lexicographically(const Edge &a, const Edge &b) {
    return lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}

int compute_linear_representation(HyperGraph& graph, LinearRepresentation *linear)
{
    // Determine omega and total size
    Node omega = 1;
    size_t total_size = 0;
    for (auto const& edge : graph.edges) {
        omega = max(omega, edge[edge.size()-1]);
        total_size += edge.size() + 1; // because we will add one omega at the end of each edge
    }
    omega++;

    // Sort each edge ascendingly
    for (uint32_t i = 0; i < graph.edge_count; i++)
    {
        sort(graph.edges[i].begin(), graph.edges[i].end());
    }

    // Insert the omega at the end
    for (size_t i = 0; i < graph.edges.size(); i++) {
        graph.edges[i].push_back(omega);
    } // IMPORTANT: omegas have to be inserted before sorting lexicographically, so that [2, 3, omega] comes after [2, 3, 4, omega] (like it should)

    // Sort the edges ascendingly lexicographically
    sort(graph.edges.begin(), graph.edges.end(), compare_lexicographically);

    // Allocate space in linear->nodes
    linear->nodes.resize(total_size);

    // Copy nodes into linear representation.
    size_t pos = 0;
    for (const auto &edge : graph.edges)
    {
        copy(edge.begin(), edge.end(), linear->nodes.begin() + pos);
        pos += edge.size();
    }
    linear->omega = omega;
    linear->size = linear->nodes.size();

    /*
    // Increase all values by 1, because 0 will be added by SDSL to construct the suffix array.
    for (size_t i = 0; i < linear->size(); i++) {
        (*linear)[i]++;
    }*/

    return 0;
}


//int_vector<64> t = {1, 1, omega, 1, 4, 4, omega, 2, 3, 4, omega, 2, 3, omega};
HyperRDF create(LinearRepresentation *linear) {
    bool debug_prints = false;

    // t needs to be in the correct format
    // assert_no_zero(linear);
    //cout << "l = " << linear->nodes << "\n";

    // create l2, which is how sdsl needs the data
    int omega = linear->omega;
    int_vector<64> l2(linear->size + 2);
    l2[0] = omega + 1;
    for (size_t i = 0; i < (linear->size); ++i) {
        l2[i+1] = linear->nodes[i];
    }
    l2[l2.size() - 1] = 0;
    
    
    // Construct the suffix array directly
    int_vector<> sa;
    sdsl::qsufsort::construct_sa(sa, l2);
    if (debug_prints) {
        cout << "sa = " << sa << "\n";
    }
    

    // Result
    HyperRDF r;

    // Construct the compressed suffix array
    sdsl::csa_sada<enc_vector<>, 32, 32, sa_order_sa_sampling<>, isa_sampling<>, int_alphabet<>> csa; // TODO: Use http://vios.dc.fi.udc.es/indexing/wsi/
    sdsl::construct_im(csa, linear->nodes, 8); // Note that csa.size = linear_representation.size + 1 (auto adds the 0 at the end)
    r.psi = csa.psi;
    if (debug_prints) {
        cout << "psi_o = " << r.psi << "\n";
    }


    // Create d
    sdsl::bit_vector d(l2.size(), 0);
    create_d(&l2, &d);
    r.d = d;
    if (debug_prints) {
        cout << "d_o = " << d << "\n";
    }


    // Modify psi and d
    modify_psi_and_d(r.psi, r.d);

    // cout << "psi_mod = " << r.psi << "\n";

    // cout << "d_mod = " << r.d << "\n";


    return r;
}