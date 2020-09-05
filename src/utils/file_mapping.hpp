#pragma once

#include <boost/filesystem/path.hpp>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#    include <windows.h>
#endif

namespace utils
{

using slice = std::pair<const std::uint8_t *, size_t>;

class file_mapping
{

private:
    static std::error_code get_last_error() noexcept
    {
        return std::error_code{static_cast<int>(::GetLastError()), std::system_category()};
    }

    void map_file(const boost::filesystem::path & p)
    {
        const auto str = p.generic_string();

        const auto file_handle = CreateFile(                   //
            str.data(),                                        //
            GENERIC_READ,                                      //
            FILE_SHARE_READ,                                   //
            /* lpSecurityAttributes */ nullptr,                //
            OPEN_EXISTING,                                     //
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, //
            /* hTemplateFile */ nullptr);

        if (file_handle == INVALID_HANDLE_VALUE) throw get_last_error();

        LARGE_INTEGER s;
        if (!::GetFileSizeEx(file_handle, &s)) throw get_last_error();

        _size = static_cast<size_t>(s.QuadPart);

        if ((s.HighPart > 0) && (sizeof(size_t) < sizeof(std::uint64_t))) throw std::runtime_error("File cannot fit in memory.");

        _file_mapping = ::CreateFileMapping(file_handle, nullptr, PAGE_READONLY, 0, 0, nullptr);

        ::CloseHandle(file_handle);

        if (!_file_mapping) throw get_last_error();

        _p = static_cast<const std::uint8_t *>(::MapViewOfFile(_file_mapping, FILE_MAP_READ, 0, 0, 0));
        if (!_p) throw get_last_error();
    }

public:
    explicit file_mapping(const boost::filesystem::path & p)
    {
        map_file(p);
    }

    ~file_mapping()
    {
        close();
    }

public:
    slice view() const noexcept
    {
        return slice{_p, _size};
    }

public:
    void close()
    {
        if (_p)
        {
            ::UnmapViewOfFile(_p);
            _p    = nullptr;
            _size = 0;
        }

        if (_file_mapping)
        {
            ::CloseHandle(_file_mapping);
            _file_mapping = nullptr;
        }
    }

private:
    HANDLE _file_mapping{nullptr};
    const std::uint8_t * _p{nullptr};
    size_t _size{0};
};

} // namespace utils