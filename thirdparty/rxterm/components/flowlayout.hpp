#ifndef RXTERM_COMPONENTS_FLOWLAYOUT_HPP
#define RXTERM_COMPONENTS_FLOWLAYOUT_HPP

#include <rxterm/image.hpp>
#include <rxterm/components/component.hpp>
#include <rxterm/utils.hpp>
#include <rxterm/style.hpp>
#include <rxterm/reflow.hpp>
#include <algorithm>

namespace rxterm {

template<class T = Component>
struct FlowLayout {
  std::vector<T> children;
  Pixel bg;

  FlowLayout(std::vector<T> const& children, Pixel bg = Pixel{})
    : children(children)
    , bg{bg}
  {}

  template<class...Xs>
  FlowLayout(Xs const&...xs)
    : children{xs...}
    , bg{Pixel{}}
  {}

  template<class...Xs>
  FlowLayout(Pixel const& bg, Xs const&...xs)
    : children{xs...}
    , bg{bg}
  {}

  Image render(size_t maxWidth) const {

    auto const images = map(children, [maxWidth](auto const& c) {
      auto const image = c.render(maxWidth);
      return image;
    });

    size_t width = 0;
    size_t x = 0;
    size_t curHeight = 0;
    std::vector<size_t> xs;
    std::vector<size_t> ys;
    size_t y = 0;

    for (auto const& image : images) {
      if (x + image.width > maxWidth) {
        x = 0;
        xs.push_back(x);
        x += image.width;
        y += curHeight;
        ys.push_back(y);
        curHeight = image.height;
        width = std::max({width, image.width});
      } else {
        xs.push_back(x);
        ys.push_back(y);
        x += image.width;
        width = std::max(width, x);
        curHeight = std::max(curHeight, image.height);
      }
    }

    size_t const height = y + curHeight;
    auto canvas = Image::create(width, height, bg);

    auto yi = ys.cbegin();
    auto xi = xs.cbegin();
    for(auto const& image : images) {
      canvas = drawOnBackground(canvas, *xi, *yi, image);
      ++xi;
      ++yi;
    }

    return canvas;
  }
};

}

#endif
