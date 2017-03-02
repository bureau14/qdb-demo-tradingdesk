#pragma once

#include <utils/time.hpp>
#include <utils/timespec.hpp>
#include <string>

#include <vector>

struct quote
{
    quote() {}

    template <typename Generator>
    quote(const std::string & o, const std::string & s, Generator & g) : timestamp{utils::timestamp()}, origin{o}, symbol{s}, columns{g()}
    {
    }

    utils::timespec timestamp;

    std::string origin;
    std::string symbol;

    struct values
    {
        values(double v = 0.0, double o = 0.0, double h = 0.0, double l = 0.0, double c = 0.0)
            : volume{v}, open{o}, high{h}, low{l}, close{c}
        {
        }

        double volume;
        double open;
        double high;
        double low;
        double close;
    };

    values columns;
};

qdb_error_t insert_into_qdb(qdb_handle_t h, const quote & q);