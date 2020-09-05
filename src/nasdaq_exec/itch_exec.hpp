#pragma once

#include "itch_messages.hpp"
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/endian/conversion.hpp>
#include <rh/robin_hood.h>
#include <utils/timespec.hpp>
#include <cmath>
#include <cstdint>
#include <vector>

namespace itch
{

// an order at instant X, can be buy or sell
struct order
{
    order() = default;
    order(std::uint32_t p, std::uint32_t s)
        : price{p}
        , shares{s}
    {}

    std::uint32_t price; // fixed point, 4 decimal precision on Nasdaq
    std::uint32_t shares;

    bool operator==(const order & other) const noexcept
    {
        return (shares == other.shares) && (price == other.price);
    }

    bool operator!=(const order & other) const noexcept
    {
        return !(*this == other);
    }

    bool operator<(const order & other) const noexcept
    {
        return (price < other.price) || ((price == other.price) && (shares < other.shares));
    }
};

// reference => order
using order_map  = robin_hood::unordered_flat_map<std::uint64_t, order>;
using order_book = boost::container::flat_multimap<order, std::uint64_t>;
// price share collapsed book
using collapsed_book = boost::container::flat_map<std::uint32_t, std::uint32_t>;

template <typename Integer>
inline void serialize_integer(std::uint8_t *& p, size_t & l, Integer v) noexcept
{
    *reinterpret_cast<Integer *>(p) = boost::endian::native_to_little(v);

    p += sizeof(v);
    l -= sizeof(v);
}

template <typename Integer>
inline bool deserialize_integer(const std::uint8_t *& p, size_t & l, Integer & v) noexcept
{
    if (l < sizeof(v)) return false;
    v = *reinterpret_cast<const Integer *>(p);

    p += sizeof(v);
    l -= sizeof(v);
    return true;
}

inline void serialize_order_map(std::uint8_t *& p, size_t & l, const order_map & orders) noexcept
{
    serialize_integer(p, l, static_cast<std::uint64_t>(orders.size()));

    for (const auto & o : orders)
    {
        serialize_integer(p, l, o.first);
        serialize_integer(p, l, o.second.price);
        serialize_integer(p, l, o.second.shares);
    }
}

inline bool deserialize_order_map(const std::uint8_t *& p, size_t & l, order_map & orders)
{
    std::uint64_t s;
    if (!deserialize_integer(p, l, s)) return false;

    orders.clear();
    orders.reserve(s);

    for (std::uint64_t i = 0; i < s; ++i)
    {
        std::uint64_t reference;
        if (!deserialize_integer(p, l, reference)) return false;

        order o;
        if (!deserialize_integer(p, l, o.price)) return false;

        if (!deserialize_integer(p, l, o.shares)) return false;

        orders.emplace(reference, o);
    }

    return true;
}

static const size_t serialized_size(const order_map & m) noexcept
{
    return sizeof(std::uint64_t) + m.size() * (sizeof(std::uint64_t) + sizeof(std::uint32_t) * 2);
}

struct order_record
{
    std::uint64_t reference;
    std::uint64_t new_reference;
    std::uint32_t shares;
    double price;
    char order_type;
    bool is_buy;
};

using order_records = boost::container::flat_multimap<utils::timespec, order_record>;

class execution_engine
{

private:
    static constexpr std::uint32_t convert_to_fix(double p) noexcept
    {
        return static_cast<std::uint32_t>(p * 1000.0);
    }

private:
    void run_add_order(bool is_buy, std::uint64_t reference, std::uint32_t shares, double price)
    {
        auto & m = is_buy ? _all_buy_orders : _all_sell_orders;

        m.emplace(reference, order{convert_to_fix(price), shares});
    }

    template <typename Map>
    bool run_execute_order(Map & m, std::uint64_t reference, std::uint32_t shares)
    {
        auto it = m.find(reference);
        if (it != m.end())
        {
            it->second.shares -= shares;
            if (!it->second.shares)
            {
                m.erase(it);
            }
            return true;
        }

        return false;
    }

    bool run_execute_order(std::uint64_t reference, std::uint32_t shares)
    {
        if (run_execute_order(_all_buy_orders, reference, shares)) return true;
        return run_execute_order(_all_sell_orders, reference, shares);
    }

    bool run_cancel_order(std::uint64_t reference, std::uint32_t shares)
    {
        return run_execute_order(reference, shares);
    }

    bool run_delete_order(std::uint64_t reference)
    {
        if (_all_buy_orders.erase(reference) > 0u) return true;
        return _all_sell_orders.erase(reference) > 0u;
    }

    template <typename Map>
    bool run_replace_order(Map & m, bool is_buy, std::uint64_t reference, std::uint64_t new_reference, std::uint32_t shares, double price)
    {
        auto it = m.find(reference);
        if (it == m.end()) return false;

        m.erase(it);
        run_add_order(is_buy, new_reference, shares, price);
        return true;
    }

    bool run_replace_order(std::uint64_t reference, std::uint64_t new_reference, std::uint32_t shares, double price)
    {
        if (run_replace_order(_all_buy_orders, true, reference, new_reference, shares, price)) return true;

        return run_replace_order(_all_sell_orders, false, reference, new_reference, shares, price);
    }

public:
    bool run_order(const order_record & record)
    {
        switch (record.order_type)
        {
        case itch::messages::add_order_with_attribution::message_code:
            [[fallthrough]];
        case itch::messages::add_order_without_attribution::message_code:
            run_add_order(record.is_buy, record.reference, record.shares, record.price);
            return true;

        case itch::messages::order_executed::message_code:
            [[fallthrough]];
        case itch::messages::order_executed_with_price::message_code:
            return run_execute_order(record.reference, record.shares);

        case itch::messages::order_cancel::message_code:
            return run_cancel_order(record.reference, record.shares);

        case itch::messages::order_delete::message_code:
            return run_delete_order(record.reference);

        case itch::messages::order_replace::message_code:
            return run_replace_order(record.reference, record.new_reference, record.shares, record.price);

        default:
            return false;
        }
    }

private:
    template <typename Map>
    order_book make_book(Map & m) const
    {
        boost::container::vector<std::pair<order, std::uint64_t>> orders;

        orders.reserve(m.size());

        for (const auto & e : m)
        {
            orders.emplace_back(e.second, e.first);
        }

        std::sort(orders.begin(), orders.end());

        order_book res;

        res.adopt_sequence(boost::container::ordered_range_t{}, std::move(orders));

        return res;
    }

public:
    void reserve(size_t s)
    {
        _all_sell_orders.reserve(s);
        _all_buy_orders.reserve(s);
    }

public:
    order_book buy_book() const
    {
        return make_book(_all_buy_orders);
    }

    order_book sell_book() const
    {
        return make_book(_all_sell_orders);
    }

public:
    static collapsed_book collapse_book(const order_book & orders)
    {
        collapsed_book res;

        if (orders.empty()) return res;

        res.reserve(orders.size());

        auto it_src  = orders.cbegin();
        auto it_dest = res.emplace(it_src->first.price, it_src->first.shares).first;

        for (++it_src; it_src != orders.cend(); ++it_src)
        {
            if (it_dest->first == it_src->first.price)
            {
                it_dest->second += it_src->first.shares;
            }
            else
            {
                it_dest = res.emplace_hint(res.end(), it_src->first.price, it_src->first.shares);
            }
        }

        return res;
    }

public:
    std::vector<std::uint8_t> serialize_state() const
    {
        std::vector<std::uint8_t> res(serialized_size(_all_buy_orders) + serialized_size(_all_sell_orders));

        auto * p = res.data();
        size_t l = res.size();

        serialize_order_map(p, l, _all_buy_orders);
        serialize_order_map(p, l, _all_sell_orders);

        return res;
    }

    bool deserialize_state(const std::uint8_t *& p, size_t & l)
    {
        if (!deserialize_order_map(p, l, _all_buy_orders)) return false;
        return deserialize_order_map(p, l, _all_sell_orders);
    }

private:
    order_map _all_buy_orders;
    order_map _all_sell_orders;
};

} // namespace itch