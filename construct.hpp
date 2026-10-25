#pragma once
#include <sdsl/bit_vectors.hpp>
#include <sdsl/enc_vector.hpp>
#include <sdsl/int_vector.hpp>
#include "type_definitions.hpp"

using namespace sdsl;

int compute_linear_representation(HyperGraph& graph, LinearRepresentation *linear);

HyperRDF create(LinearRepresentation *linear);
