#pragma once

#include "quote.hpp"

#include <algorithm>
#include <random>
#include <tuple>

class quote_generator
{
public:
    explicit quote_generator(double initial, double amplitude)
        : _amplitude{amplitude}, _random{_random_device()}, _value_distribution{-amplitude / 2.0, amplitude / 2.0}, _last{initial}
    {
    }

    quote_generator(const quote_generator &) = delete;

private:
    template <typename Iterator>
    std::pair<double, double> fill_values(Iterator first, Iterator last)
    {
        std::uint32_t dst = static_cast<std::uint32_t>(std::distance(first, last));

        assert(dst >= 2);

        const double front_value = *first;
        const double back_value = *(last - 1);

        double current_min = std::min(front_value, back_value);
        double current_max = std::max(front_value, back_value);

        if (dst > 2u)
        {
            // inspired by diamond-square algorithm
            const double new_value = std::max(0.01, (front_value + back_value) / 2.0 + _value_distribution(_random));

            current_min = std::min(current_min, new_value);
            current_max = std::max(current_max, new_value);

            Iterator middle = first;

            std::advance(middle, std::max(dst / 2, 1u));

            *middle = new_value;

            double new_min;
            double new_max;

            if (std::distance(first, middle) > 2)
            {
                std::tie(new_min, new_max) = fill_values(first, middle);

                current_min = std::min(current_min, new_min);
                current_max = std::max(current_max, new_max);
            }

            if (std::distance(middle, last) > 2)
            {
                std::tie(new_min, new_max) = fill_values(middle, last);

                current_min = std::min(current_min, new_min);
                current_max = std::max(current_max, new_max);
            }
        }

        return std::make_pair(current_min, current_max);
    }

public:
    quote::values operator()(std::uint32_t volume)
    {
        // generate a bunch of prices
        std::vector<double> values(volume);

        values.front() = _last;
        values.back() = std::max(0.0001, _last + _value_distribution(_random) * 2.0);

        double low;
        double high;

        std::tie(low, high) = fill_values(values.begin(), values.end());

        return quote::values{static_cast<double>(values.size()), values.front(), low, high, values.back()};
    }

    quote::values operator()()
    {
        const double new_value = std::max(0.1, _last + _value_distribution(_random) * 2.0);

        const double min_val = std::max(0.1, std::min(new_value, _last) - abs(_value_distribution(_random)) * 0.25);
        const double max_val = std::max(new_value, _last) + abs(_value_distribution(_random)) * 0.25;

        const quote::values res{abs(_value_distribution(_random)) * 3.14, _last, min_val, max_val, new_value};

        _last = new_value;

        return res;
    }

private:
    const double _amplitude;

    std::random_device _random_device;
    std::minstd_rand _random;

    std::uniform_real_distribution<double> _value_distribution;

    double _last;
};