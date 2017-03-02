#pragma once

#include "quote.hpp"

#include <algorithm>
#include <random>
#include <tuple>

class quote_generator
{
public:
    explicit quote_generator(double amplitude)
        : _amplitude{amplitude}, _random{_random_device()}, _volume_distribution{100, 1'000}, _value_distribution{-amplitude / 2.0,
                                                                                                  amplitude / 2.0},
          _last(abs(_value_distribution(_random)))
    {
    }

    quote_generator(const quote_generator & ) = delete;

private:
    template <typename Iterator>
    std::pair<double, double> fill_values(Iterator first, Iterator last)
    {
        std::uint32_t dst = static_cast<std::uint32_t>(std::distance(first, last));

        assert(dst >= 2);

        const double front_value = *first;
        const double back_value = *last;

        double current_min = std::min(front_value, back_value);
        double current_max = std::max(front_value, back_value);

        if (dst > 2u)
        {
            // inspired by diamond-square algorithm
            const double new_value = (front_value + back_value) / 2.0 + _value_distribution(_random);

            current_min = std::min(current_min, new_value);
            current_max = std::max(current_max, new_value);

            Iterator middle = first;

            std::advance(middle, std::min(dst / 2, 1u));

            *middle = new_value;

            double new_min;
            double new_max;

            std::tie(new_min, new_max) = fill_values(first, middle);

            current_min = std::min(current_min, new_min);
            current_max = std::max(current_max, new_max);

            std::tie(new_min, new_max) = fill_values(middle, last);

            current_min = std::min(current_min, new_min);
            current_max = std::max(current_max, new_max);
        }

        return std::make_pair(current_min, current_max);
    }

public:
    quote::values operator()()
    {
        // generate a bunch of prices
        std::vector<double> values(_volume_distribution(_random));

        double next = _value_distribution(_random);

        values.front() = _last;
        values.back() = next;

        double low;
        double high;

        std::tie(low, high) = fill_values(values.begin(), values.end());

        return quote::values{static_cast<double>(values.size()), values.front(), low, high, values.back()};
    }

private:
    const double _amplitude;

    std::random_device _random_device;
    std::minstd_rand _random;

    std::uniform_int_distribution<std::uint32_t> _volume_distribution;

    std::uniform_real_distribution<double> _value_distribution;

    double _last;
};