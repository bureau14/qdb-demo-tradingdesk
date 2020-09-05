#ifndef RXTERM_COMPONENTS_PROGRESS_HPP
#define RXTERM_COMPONENTS_PROGRESS_HPP

#include <cmath>
#include <rxterm/style.hpp>
#include <rxterm/utils.hpp>
#include <rxterm/image.hpp>

namespace rxterm {

struct Progress {
  float const progress;
  Pixel const bg;
  Pixel const fg;

  Progress(float const p,
    Pixel const& bg = Pixel{' ', {Color::Cyan}},
    Pixel const& fg = Pixel{' ', {Color::Blue}})
    : progress{p}
    , bg{bg}
    , fg{fg}
  {}

  Image render(size_t width)const {
    auto const p = std::clamp(progress, 0.0f, 1.0f);
    return drawOnBackground(
      Image::create(width, 1, bg),
      0, 0,
      Image::create(static_cast<size_t>(static_cast<float>(width)*p), 1, fg));
  }
};

}

#endif
