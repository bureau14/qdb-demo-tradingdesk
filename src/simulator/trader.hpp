#pragma once

#include "broker.hpp"
#include "products.hpp"
#include "quote.hpp"
#include "trade.hpp"

#include <random>
#include <set>
#include <unordered_map>

template <typename Behavior>
class trader
{

public:
    trader(const std::string & name, brokers & b, const products & p) : _brokers{b}, _products{p}, _name{name}
    {
    }

    trader(const trader & t) : _brokers{t._brokers}, _products{t._products}, _name{t._name}
    {
    }

public:
    const std::string & name() const
    {
        return _name;
    }

private:
    std::vector<quote> get_quotes(const std::string & p, double volume)
    {
        std::vector<quote> quotes{_brokers.size()};

        std::transform(
            _brokers.begin(), _brokers.end(), quotes.begin(), [&p, volume](auto & b) { return b.second->request_quote(p, volume); });

        return quotes;
    }

public:
    std::pair<trade, std::vector<quote>> operator()(std::uint32_t volume)
    {
        const std::vector<quote> quotes = get_quotes(_behavior.choose_product(_products), volume);
        return std::make_pair(trade{_name, _behavior.choose_quote(quotes)}, quotes);
    }

private:
    Behavior _behavior;

    brokers & _brokers;
    const products & _products;

    std::string _name;
};

struct greedy
{
    greedy() : _random{_random_device()}
    {
    }

    const std::string & choose_product(const products & p)
    {
        std::uniform_int_distribution<products::size_type> indices{0, p.size() - 1};

        auto idx = indices(_random);

        assert(idx < p.size());

        auto it_begin = p.begin();
        std::advance(it_begin, idx);

        return it_begin->first;
    }

    const quote & choose_quote(const std::vector<quote> & quotes) const noexcept
    {
        return *std::min_element(
            quotes.begin(), quotes.end(), [](const quote & left, const quote & right) { return left.columns.close < right.columns.close; });
    }

protected:
    std::random_device _random_device;
    std::minstd_rand _random;
};

// chooses the first broker 80% of the time
struct cheater : greedy
{
    cheater() : _percent{0, 100}
    {
    }

    const quote & choose_quote(const std::vector<quote> & quotes)
    {
        return (_percent(_random) < 80) ? quotes[0] : greedy::choose_quote(quotes);
    }

    std::uniform_int_distribution<std::uint16_t> _percent;
};