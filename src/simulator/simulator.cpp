
#include "trader.hpp"
#include "products.hpp"

#include <fmt/format.h>

#include <thread>
#include <random>

int main(int, char **)
{
    products prods;

    prods.insert(std::make_pair("APPL", product{"APPL", 200.0, 5.0}));
    prods.insert(std::make_pair("CSCO", product{"CSCO", 30.0, 1.0}));
    prods.insert(std::make_pair("CAT", product{"CAT", 25.0, 2.0}));

    brokers brks;

    brks.insert(std::make_pair("bl00m", std::make_unique<broker>("bl00m", prods)));

    trader<greedy> bob{"bob", brks, prods};

    std::random_device rnd;
    std::minstd_rand generator;

    std::uniform_int_distribution<std::uint32_t> wait_interval{10, 300};

    for(int i = 0; i < 1000; ++i)
    {
        trade t = bob();

        fmt::print("Trader {} : Broker {} Product {} Volume {} Value {}\n", t.trader, t.counterparty, t.product, t.volume, t.value);

        std::this_thread::sleep_for(std::chrono::milliseconds(wait_interval(generator)));
    }


    return 0;
}