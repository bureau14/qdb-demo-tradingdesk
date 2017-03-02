#pragma once

#include "broker.hpp"
#include "quote.hpp"
#include "trade.hpp"

#include <set>
#include <unordered_map>
#include <random>

template <typename Behavior>
class trader
{

public:
    trader(const std::string & name, brokers & b, const std::set<std::string> & products) : _brokers{b}, _products{products}, _name{name}
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
            return b.second.request_quote(p);
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
    const std::set<std::string> & _products;

    std::string _name;
};

struct greedy
{

    greedy() : _random{_random_device()} {}

    const std::string & choose_product(const std::set<std::string> & products)
    {
        std::uniform_int_distribution<std::set<std::string>::size_type> indices{0, products.size() - 1};

        auto idx = indices(_random);

        assert(idx < products.size());

        auto it_begin = products.begin();
        std::advance(it_begin, idx);

        return *it_begin;
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