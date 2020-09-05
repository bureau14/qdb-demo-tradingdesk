#ifndef RXTERM_COMPONENTS_STACKLAYOUT_HPP
#define RXTERM_COMPONENTS_STACKLAYOUT_HPP

#include <rxterm/components/component.hpp>
#include <rxterm/image.hpp>
#include <rxterm/reflow.hpp>
#include <rxterm/style.hpp>
#include <rxterm/utils.hpp>
#include <algorithm>

namespace rxterm
{

template <class T = Component>
struct StackLayout
{
    std::vector<T> children;
    Pixel bg;

    StackLayout(std::vector<T> const & children, Pixel bg = Pixel{})
        : children(children)
        , bg{bg}
    {}

    template <class... Xs>
    StackLayout(Xs const &... xs)
        : children{xs...}
        , bg{Pixel{}}
    {}

    template <class... Xs>
    StackLayout(Pixel const & bg, Xs const &... xs)
        : children{xs...}
        , bg{bg}
    {}

    Image render(size_t maxWidth) const
    {
        if (children.empty()) return Image{};

        auto const images = map(children, [maxWidth](auto const & c) { return c.render(maxWidth); });

        size_t const width =
            std::max_element(images.begin(), images.end(), [](auto const & a, auto const & b) { return a.width < b.width; })->width;

        size_t const height =
            std::accumulate(images.begin(), images.end(), size_t{0}, [](size_t a, const auto & b) { return a + b.height; });

        auto canvas = Image::create(width, height, bg);

        size_t y = 0;
        for (auto const & image : images)
        {
            canvas = drawOnBackground(canvas, 0, y, image);
            y += image.height;
        }

        return canvas;
    }
};

} // namespace rxterm

#endif
