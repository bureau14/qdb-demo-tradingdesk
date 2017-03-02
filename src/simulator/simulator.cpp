
#include "trader.hpp"
#include "products.hpp"

#include <fmt/format.h>

#include <thread>
#include <random>

int main(int, char **)
{
    products prods;

    prods.insert(std::make_pair("APPL", 30.0));
    prods.insert(std::make_pair("CSCO", 30.0));

    brokers brks;

    brks.insert(std::make_pair("bl00m", broker{"bl00m", prods}));

    std::set<std::string> products = { "APPL", "CSCO" };

    trader<greedy> bob{"bob", brks, products};

    std::random_device rnd;
    std::minstd_rand generator;

    std::uniform_int_distribution<std::uint32_t> wait_interval{10, 300};

    for(int i = 0; i < 1000; ++i)
    {
        trade t = bob();

        fmt::print("Trader {} : Broker {} Product {} Volume {}", t.trader, t.counterparty, t.product, t.volume, t.value);

        std::this_thread::sleep_for(std::chrono::milliseconds(wait_interval(generator)));
    }


    return 0;
}