#pragma once

#include "products.hpp"
#include "quote.hpp"
#include "quote_generator.hpp"
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

class broker
{
public:
    explicit broker(const products & p)
    {
        for (const auto & e : p)
        {
            _quoters.insert(std::make_pair(e.second.name, std::make_unique<quote_generator>(e.second.initial, e.second.amplitude)));
        }
    }

    broker(const broker &) = delete;

    quote_generator & get_pricer(const std::string & p)
    {
        auto it = _quoters.find(p);
        if (it == _quoters.end()) throw std::runtime_error("unknown product");
        return *(it->second);
    }

private:
    std::unordered_map<std::string, std::unique_ptr<quote_generator>> _quoters;
};

class broker_with_margin
{
public:
    explicit broker_with_margin(const std::string & n, broker & brk)
        : _generator{_rnd()}
        , _margin_generator{0.001, 2.0}
        , _name{n}
        , _broker{brk}
    {}

    broker_with_margin(const broker_with_margin &) = delete;

    quote request_quote(const std::string & p, std::uint32_t volume)
    {
        quote q{_name, p, volume, _broker.get_pricer(p)};

        const double margin = get_margin(p);

        q.columns.high += margin;
        q.columns.low += margin;
        q.columns.open += margin;
        q.columns.close += margin;

        return q;
    }

    double get_margin(const std::string & p)
    {
        auto it = _product_margin.find(p);
        if (it == _product_margin.end())
        {
            std::tie(it, std::ignore) = _product_margin.insert(std::make_pair(p, _margin_generator(_generator)));
        }

        assert(it != _product_margin.end());

        assert(!std::isnan(it->second));

        return it->second;
    }

private:
    std::random_device _rnd;
    std::minstd_rand _generator;
    std::uniform_real_distribution<double> _margin_generator;

    std::string _name;

    std::unordered_map<std::string, double> _product_margin;

    broker & _broker;
};

using brokers = std::unordered_map<std::string, std::unique_ptr<broker_with_margin>>;

inline void add_broker(brokers & brks, const std::string & name, broker & master_broker)
{
    brks.insert(std::make_pair(name, std::make_unique<broker_with_margin>(name, master_broker)));
}