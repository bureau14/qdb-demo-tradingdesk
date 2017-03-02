#pragma once

#include <unordered_map>

struct product
{
    product(const std::string & n = {}, double i = 0.0, double a = 0.0) : name{n}, initial{i}, amplitude{a}
    {
    }

    std::string name;
    double initial;
    double amplitude;
};

using products = std::unordered_map<std::string, product>;
