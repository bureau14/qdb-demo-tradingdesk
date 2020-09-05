#pragma once

#include <boost/filesystem/path.hpp>
#include <system_error>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <cstdio>
#endif

namespace utils
{

struct read_status
{
    bool good() const noexcept
    {
        return !ec;
    }

    read_status() noexcept = default;
    explicit read_status(std::error_code e, size_t lr = 0) noexcept
        : ec{e}
        , last_read{lr}
        , total_read{lr}
    {}

    read_status & operator+=(const read_status & other) noexcept
    {
        ec        = ec ? ec : other.ec;
        last_read = other.last_read;
        total_read += other.total_read;

        return *this;
    }

    std::error_code ec;
    size_t last_read{0};
    size_t total_read{0};
};

struct capi_impl
{
    using file_handle = FILE *;

    static constexpr file_handle empty_handle() noexcept
    {
        return nullptr;
    }

    static std::error_code get_last_error() noexcept
    {
        return std::error_code{errno, std::generic_category()};
    }

    static read_status open(const boost::filesystem::path & p, file_handle & h)
    {
        const auto str = p.generic_string();
        h              = std::fopen(str.c_str(), "rb");
        return !h ? read_status{get_last_error()} : read_status{};
    }

    static read_status read(file_handle h, void * p, size_t s)
    {
        const auto r = std::fread(p, 1, s, h);
        return (r == s) ? read_status{std::error_code{}, r} : read_status{get_last_error(), r};
    }

    static read_status seek(file_handle h, size_t s)
    {
        if (!std::fseek(h, static_cast<long>(s), SEEK_CUR))
        {
            return read_status{std::error_code{}, s};
        }

        return read_status{get_last_error()};
    }

    static void close(file_handle & h)
    {
        if (h)
        {
            std::fclose(h);
            h = nullptr;
        }
    }
};

struct winapi_impl
{
    using file_handle = HANDLE;

    static constexpr file_handle empty_handle() noexcept
    {
        return INVALID_HANDLE_VALUE;
    }

    static std::error_code get_last_error() noexcept
    {
        return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
    }

    static read_status open(const boost::filesystem::path & p, file_handle & h)
    {
        const auto str = p.generic_string();

        h = ::CreateFile(str.c_str(),                          //
            GENERIC_READ,                                      //
            FILE_SHARE_READ,                                   //
            nullptr,                                           //
            OPEN_EXISTING,                                     //
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, //
            nullptr);

        return (h == INVALID_HANDLE_VALUE) ? read_status{get_last_error()} : read_status{};
    }

    static read_status read(file_handle h, void * p, size_t s)
    {
        DWORD r = 0;
        if (!::ReadFile(h, p, static_cast<DWORD>(s), &r, nullptr))
        {
            return read_status{get_last_error(), static_cast<size_t>(r)};
        }

        return read_status{std::error_code{}, static_cast<size_t>(r)};
    }

    static read_status seek(file_handle h, size_t s)
    {
        LARGE_INTEGER li;
        li.QuadPart = static_cast<LONGLONG>(s);

        li.LowPart = ::SetFilePointer(h, li.LowPart, &li.HighPart, FILE_CURRENT);
        if (li.LowPart == INVALID_SET_FILE_POINTER)
        {
            return read_status{get_last_error()};
        }

        return read_status{std::error_code{}, s};
    }

    static void close(file_handle & h)
    {
        if (h != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(h);
            h = INVALID_HANDLE_VALUE;
        }
    }
};

template <typename Implementation>
class basic_file_stream
{

private:
    using file_handle = typename Implementation::file_handle;

public:
    explicit basic_file_stream(const boost::filesystem::path & p)
        : _path{p}
        , _handle{Implementation::empty_handle()}
    {
        _status = Implementation::open(p, _handle);
    }

    ~basic_file_stream()
    {
        Implementation::close(_handle);
    }

public:
    basic_file_stream<Implementation> & read(void * p, size_t s)
    {
        if (_status.good())
        {
            _status += Implementation::read(_handle, p, s);
        }

        return *this;
    }

    basic_file_stream<Implementation> & seek(size_t s)
    {
        if (_status.good())
        {
            _status += Implementation::seek(_handle, s);
        }

        return *this;
    }

public:
    bool good() const noexcept
    {
        return _status.good();
    }

    std::error_code last_error() const noexcept
    {
        return _status.ec;
    }

    size_t last_read() const noexcept
    {
        return _status.last_read;
    }

    size_t total_read() const noexcept
    {
        return _status.total_read;
    }

private:
    read_status _status;
    boost::filesystem::path _path;
    file_handle _handle;
};

#ifdef _WIN32
using file_stream = basic_file_stream<winapi_impl>;
#else
using file_stream = basic_file_stream<capi_impl>;
#endif

} // namespace utils