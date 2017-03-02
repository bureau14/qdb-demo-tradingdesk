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
    explicit broker(const std::string & n, const products & p) : _name{n}
    {
        for(const auto & e : p)
        {
            _quoters.insert(std::make_pair(e.first, std::make_unique<quote_generator>(e.second)));
        }
    }

    quote request_quote(const std::string & p)
    {
        auto it = _quoters.find(p);
        if (it == _quoters.end()) throw std::runtime_error("unknown product");

        return quote{_name, it->first, *(it->second)};
    }

private:
    std::string _name;
    std::unordered_map<std::string, std::unique_ptr<quote_generator>> _quoters;
};

using brokers = std::unordered_map<std::string, broker>;