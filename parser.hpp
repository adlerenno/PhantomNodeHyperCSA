#pragma once
#include <sdsl/bit_vectors.hpp>
#include <sdsl/enc_vector.hpp>
#include <sdsl/int_vector.hpp>
#include "type_definitions.hpp"

using namespace sdsl;

int parse_graph(const char *input_file, HyperGraph& graph, uint64_t lines_to_read);

int write_hyper_rdf(const char *output_file, HyperRDF& r);

HyperRDF load_hyper_rdf(const char *input_file);