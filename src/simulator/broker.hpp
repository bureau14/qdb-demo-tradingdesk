#pragma once

#include "products.hpp"
#include "quote.hpp"
#include "quote_generator.hpp"

#include <string>
#include <unordered_map>
#include <iterator>
#include <memory>

class broker
{
public:
    explicit broker(const std::string & n, const products & p) : _name{n}, _random{_random_device()}, _volume_distribution{100, 1'000}
    {
        for(const auto & e : p)
        {
            _quoters.insert(std::make_pair(e.second.name, std::make_unique<quote_generator>(e.second.initial, e.second.amplitude)));
        }
    }

    broker(const broker &) = delete;

    quote request_quote(const std::string & p)
    {
        auto it = _quoters.find(p);
        if (it == _quoters.end()) throw std::runtime_error("unknown product");

        return quote{_name, it->first, _volume_distribution(_random), *(it->second)};
    }

private:
    std::string _name;
    std::unordered_map<std::string, std::unique_ptr<quote_generator>> _quoters;

    std::random_device _random_device;
    std::minstd_rand _random;

    std::uniform_int_distribution<std::uint32_t> _volume_distribution;
};

using brokers = std::unordered_map<std::string, std::unique_ptr<broker>>;