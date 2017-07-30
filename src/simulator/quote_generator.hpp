#pragma once

#include "quote.hpp"
#include <algorithm>
#include <random>
#include <tuple>

class quote_generator
{
public:
    quote_generator(double initial, double amplitude)
        : _random{_random_device()}, _value_distribution{-amplitude / 2.0, amplitude / 2.0}, _last{initial}
    {
    }

    quote_generator(const quote_generator &) = delete;

private:
    template <typename Iterator>
    std::pair<double, double> fill_values(Iterator first, Iterator last, size_t generation)
    {
        auto dst = static_cast<std::uint32_t>(std::distance(first, last));
        assert(dst >= 2u);

        const double front = *first;
        assert(front > 0.0);
        const double back = *std::prev(last);
        assert(back > 0.0);

        double current_min = std::min(front, back);
        double current_max = std::max(front, back);

        if (dst >= 2u)
        {
            // inspired by diamond-square algorithm
            const double new_value = std::max(0.01, (front + back) / 2.0 + (_value_distribution(_random) / generation));

            current_min = std::min(current_min, new_value);
            current_max = std::max(current_max, new_value);

            Iterator middle = std::next(first, std::max(dst / 2, 1u));
            *middle = new_value;

            double new_min;
            double new_max;

            if (std::distance(first, middle) >= 2)
            {
                std::tie(new_min, new_max) = fill_values(first, std::next(middle), generation + 1);

                current_min = std::min(current_min, new_min);
                current_max = std::max(current_max, new_max);
            }

            if (std::distance(middle, last) >= 2)
            {
                std::tie(new_min, new_max) = fill_values(middle, last, generation + 1);

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
        std::tie(low, high) = fill_values(values.begin(), values.end(), /*generation=*/1);
        assert(low <= high);

        return quote::values{static_cast<double>(values.size()), /*open=*/values.front(), high, low, /*close=*/values.back()};
    }

private:
    std::random_device _random_device;
    std::minstd_rand _random;

    std::uniform_real_distribution<double> _value_distribution;

    double _last;
};
