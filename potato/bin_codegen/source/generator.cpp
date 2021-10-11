// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "generator.h"

#include <iostream>

void Generator::fail(std::string_view message) {
    ++_errors;
    std::cerr << "Error: " << message << "\n";
}
