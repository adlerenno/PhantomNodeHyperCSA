
#include <iostream>
#include <filesystem>
#include <chrono>

#include <sdsl/suffix_arrays.hpp>
#include "construct.hpp"
#include "parser.hpp"

using namespace std;
using namespace sdsl;

int main(int argc, char* argv[]) {
    // Standard paths
    string data_input_file = "data_sets/test.raw";
    string output_file = "test.hrdf";
    bool do_rename = false;
    bool debug_prints = false;
    int percentage = 100;

    // In case of command line arguments
    if (argc >= 3) {
        data_input_file = argv[1];
        output_file = argv[2];
    }
    if (argc >= 4) {
        string arg3 = argv[3];
        percentage = stoi(arg3);
    }
    cout << "Using values:\n"
            << " - Input:  " << data_input_file << "\n"
            << " - Output: " << output_file << "\n"
            << " - Data percentage: " << percentage << "\n\n";

    // Check if data_input_file exists
    filesystem::path input_path(data_input_file);
    if (!filesystem::exists(input_path)) {
        cerr << "Input file not found: " << data_input_file << "\n";
        return 1;
    }
    if (!filesystem::is_regular_file(input_path)) {
        cerr << "Path exists but is not a regular file: " << data_input_file << "\n";
        return 1;
    }

    // Convert data percentage to number of lines
    uint64_t total_lines = 0;
    {
        std::ifstream counter(data_input_file);
        std::string tmp;
        while (std::getline(counter, tmp))
            ++total_lines;
    }
    uint64_t lines_to_read = total_lines * percentage / 100;
    cout << "reading " << lines_to_read << " out of " << total_lines << " total lines...\n";

    // Start timer
    auto time_start = chrono::high_resolution_clock::now();

    // Parse graph
    HyperGraph graph;
    parse_graph(data_input_file.c_str(), graph, lines_to_read);
    if (debug_prints) {
        for (auto c : graph.edges) {
            cout << c << ", ";
        }
        cout << "\ncount: " << graph.edge_count << "\n";
    }

    auto time_after_parsing = chrono::high_resolution_clock::now();
    auto duration_parsing = chrono::duration_cast<chrono::milliseconds>(time_after_parsing - time_start);
    cout << " - Parsing time: " << duration_parsing.count() << " ms\n";

    // Compute linear representation
    LinearRepresentation linear;
    compute_linear_representation(graph, &linear);
    if (debug_prints) {cout << "l = " << linear.nodes << "\n";}

    auto time_after_linearizing = chrono::high_resolution_clock::now();
    auto duration_linearizing = chrono::duration_cast<chrono::milliseconds>(time_after_linearizing - time_after_parsing);
    cout << " - Linearization time: " << duration_linearizing.count() << " ms\n";

    // Create HyperRDF
    HyperRDF r = create(&linear);
    if (debug_prints) {
        cout << "psi = [";
        for (auto c : r.psi) {
            cout << c << ", ";
        }
        cout << "]\nd = [";
        for (auto c : r.d) {
            cout << c << ", ";
        }
        cout << "]\n";
    }

    auto time_after_construction = chrono::high_resolution_clock::now();
    auto duration_construction = chrono::duration_cast<chrono::milliseconds>(time_after_construction - time_after_linearizing);
    cout << " - Construction time: " << duration_construction.count() << " ms\n";

    // Write output
    write_hyper_rdf(output_file.c_str(), r);

    // End timer
    auto time_end = chrono::high_resolution_clock::now();
    auto duration_writing = chrono::duration_cast<chrono::milliseconds>(time_end - time_after_construction);
    cout << " - Writing time: " << duration_writing.count() << " ms\n";
    auto duration_total = chrono::duration_cast<chrono::milliseconds>(time_end - time_start);
    cout << "-> Total time: " << duration_total.count() << " ms\n";
    
    cout << "Output written to " << output_file << "\n";
    return 0;
}