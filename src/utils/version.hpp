#pragma once

static constexpr const char * qdb_demo_version = QDB_DEMO_VERSION;

namespace utils
{

inline const char * get_version()
{
    return qdb_demo_version;
}

} // namespace utils
