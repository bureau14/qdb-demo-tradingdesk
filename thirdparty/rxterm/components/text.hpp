#ifndef RXTERM_COMPONENTS_TEXT_HPP
#define RXTERM_COMPONENTS_TEXT_HPP

#include <rxterm/image.hpp>
#include <rxterm/utils.hpp>
#include <rxterm/style.hpp>
#include <rxterm/reflow.hpp>
#include <algorithm>
#include <numeric>

namespace rxterm {

struct Text {
  Style const style;
  std::string const content;

  template<class T>
  Text(Style const& style, T const& content)
    : style{style}
    , content{toString(content)}
  {}

  template<class T>
  Text(T const& content)
    : style{Style{}}
    , content{toString(content)}
  {}

  Image render(size_t maxWidth) const {
    auto const lines = split(reflow(maxWidth, content), "\n");
    auto const height = lines.size();
    auto const width = lines.empty() ? size_t{ 0 } :  std::max_element(
      lines.begin(),
      lines.end(),
      [](auto a, auto b){
        return a.size() < b.size();
    })->size();

    auto image = Image::create(width, height, Pixel{'\0', style});
    size_t y = 0;
    for(auto const& line : lines) {
      for(size_t x=0; x < line.size(); ++x) {
        image(x,y).c = line[x];
      }
      ++y;
    }

    return image;
  }

};

Text Spacer(size_t s)
{
    return Text{Style::Default(), std::string(s, ' ')};
}

}
#endif
