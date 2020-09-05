#pragma once

#include <chrono>
#include <cstdint>

namespace itch
{
struct read_status
{
    struct messages_status
    {
        std::uint64_t stall{0};
        std::uint64_t loaded{0};
        std::uint64_t skipped{0};
        std::uint64_t read{0};
    };

    messages_status messages;

    std::uint64_t bytes_read{0};
    std::uint64_t total_bytes{0};
};

struct write_status
{
    std::uint64_t errors{0};
    std::uint64_t unmatched{0};
    std::uint64_t tables_created{0};
    std::uint64_t rows_written{0};
    std::uint64_t bytes_written{0};
};

struct global_status
{
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point read_end_time;
    std::chrono::high_resolution_clock::time_point write_end_time;

    read_status read;
    write_status write;
};
} // namespace itch