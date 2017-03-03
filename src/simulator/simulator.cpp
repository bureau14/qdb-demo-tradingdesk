
#include "dow_jones.hpp"
#include "products.hpp"
#include "trader.hpp"

#include <fmt/format.h>

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <random>
#include <thread>

class trading
{
public:
    explicit trading(qdb_handle_t h) : _generator{_random()}, _handle{h}, _wait_interval{10, 300}, _volume_distribution{100, 1'000}
    {
    }

    template <typename Trader>
    void operator()(Trader & trd) const
    {
        trade t;
        std::vector<quote> quotes;

        std::tie(t, quotes) = trd(_volume_distribution(_generator));

        insert_into_qdb(_handle, t);

        for (const quote & q : quotes)
        {
            insert_into_qdb(_handle, q);
        }

        fmt::print("Trader {} : Broker {} Product {} Volume {} Value {}\n", t.trader, t.counterparty, t.product, t.volume, t.value);

        std::this_thread::sleep_for(std::chrono::milliseconds(_wait_interval(_generator)));
    }

private:
    std::random_device _random;
    mutable std::minstd_rand _generator;

    qdb_handle_t _handle;

    std::uniform_int_distribution<std::uint32_t> _wait_interval;
    std::uniform_int_distribution<std::uint32_t> _volume_distribution;
};

int main(int, char **)
{
    qdb_handle_t h = 0;

    try
    {
        products prods;

        add_dow_jones(prods);

        broker master_broker{prods};

        brokers brks;

        add_broker(brks, "bloom", master_broker);
        add_broker(brks, "tom", master_broker);
        add_broker(brks, "xtnd", master_broker);
        add_broker(brks, "nile", master_broker);

        boost::fusion::vector<trader<greedy>, trader<greedy>, trader<greedy>, trader<cheater>> traders(trader<greedy>{"Bob", brks, prods},
            trader<greedy>{"Alice", brks, prods}, trader<greedy>{"Carry", brks, prods}, trader<cheater>{"Cobra", brks, prods});

        std::random_device rnd;
        std::minstd_rand generator{rnd()};

        qdb_handle_t h = qdb_open_tcp();
        qdb_error_t err = qdb_connect(h, "qdb://127.0.0.1:2836");
        if (QDB_FAILURE(err)) throw std::runtime_error("connection error");

        trading trd{h};

        for (int i = 0; i < 1000; ++i)
        {
            boost::fusion::for_each(traders, trd);
        }

        qdb_close(h);

        return EXIT_SUCCESS;
    }
    catch (const std::exception & e)
    {
        fmt::print("exception caught: {}", e.what());
        qdb_close(h);
        return EXIT_FAILURE;
    }
}