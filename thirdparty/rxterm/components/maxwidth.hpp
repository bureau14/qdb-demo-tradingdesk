#ifndef RXTERM_COMPONENTS_MAXWIDTH_HPP
#define RXTERM_COMPONENTS_MAXWIDTH_HPP

#include <rxterm/components/component.hpp>
#include <rxterm/image.hpp>
#include <cmath>
#include <functional>

namespace rxterm
{

auto percent(float const & p)
{
    return [=](size_t width) -> size_t { return static_cast<size_t>(p / 100.0 * width); };
}

auto px(size_t width)
{
    return [=](auto) -> size_t { return width; };
}

struct MaxWidth
{
    std::function<size_t(size_t)> maxWidth;
    Component const c;

    MaxWidth(std::function<size_t(size_t)> maxWidth, Component const & c)
        : maxWidth{maxWidth}
        , c{c}
    {}

    MaxWidth(size_t maxWidth, Component const & c)
        : maxWidth{px(maxWidth)}
        , c{c}
    {}

    Image render(size_t width) const
    {
        return c.render(std::min(width, maxWidth(width)));
    }
};

} // namespace rxterm

#endif
