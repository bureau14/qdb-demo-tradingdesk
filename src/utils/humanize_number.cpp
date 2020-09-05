#include <boost/numeric/conversion/converter.hpp>
#include <boost/predef/os/windows.h>
#include <boost/spirit/include/karma.hpp>
#include <utils/humanize_number.hpp>
#include <utils/make_array.hpp>

namespace utils
{

std::string humanize_number(std::uint64_t n, const char * const unit, double thousands)
{
    assert(unit != nullptr);
    assert(thousands > 0.0);

    static const size_t max_unit_length = 25;

    if (strlen(unit) > max_unit_length)
    {
        return std::string();
    }

    using boost::spirit::karma::double_;
    using boost::spirit::karma::string;

    size_t stage = 0;

    // rigorous double conversion
    double val = boost::numeric::converter<double, std::uint64_t>()(n);

    static const auto units = make_array<const char * const>(" ", " Ki", " Mi", " Gi", " Ti", " Pi", " Ei", " Zi", " iY");

    while ((val >= thousands) && (stage < units.size()))
    {
        val /= thousands;
        ++stage;
    }

    assert(stage < units.size());

    static const size_t bufsize = 256;
    std::array<char, bufsize> buffer;
    char * bufp = std::data(buffer);

    std::string result;

    if (boost::spirit::karma::generate(bufp, double_ << string << string, val, units[stage], unit))
    {
        bufp[0] = '\0';
        result  = std::data(buffer);
    }

    return result;
}

} // namespace utils
