#pragma once

#include "broker.hpp"
#include "quote.hpp"
#include "trade.hpp"
#include "products.hpp"

#include <set>
#include <unordered_map>
#include <random>

template <typename Behavior>
class trader
{

public:
    trader(const std::string & name, brokers & b, const products & p) : _brokers{b}, _products{p}, _name{name}
    {
    }

public:
    const std::string & name() const
    {
        return _name;
    }

private:
    std::vector<quote> get_quotes(const std::string & p)
    {
        std::vector<quote> quotes{_brokers.size()};

        std::transform(_brokers.begin(), _brokers.end(), quotes.begin(), [&p](auto & b)
        {
            return b.second->request_quote(p);
        });

        return quotes;
    }

public:
    trade operator()()
    {
        return trade{_name, _behavior.choose_quote(get_quotes(_behavior.choose_product(_products)))};
    }

private:
    Behavior _behavior;

    brokers & _brokers;
    const products & _products;

    std::string _name;
};

struct greedy
{

    greedy() : _random{_random_device()} {}

    const std::string & choose_product(const products & p)
    {
        std::uniform_int_distribution<products::size_type> indices{0, p.size() - 1};

        auto idx = indices(_random);

        assert(idx < p.size());

        auto it_begin = p.begin();
        std::advance(it_begin, idx);

        return it_begin->first;
    }

    const quote & choose_quote(const std::vector<quote> & quotes)
    {
        return *std::min_element(quotes.begin(), quotes.end(), [](const quote & left, const quote & right)
        {
            return left.columns.close < right.columns.close;
        });
    }

private:
    std::random_device _random_device;
    std::minstd_rand _random;


};