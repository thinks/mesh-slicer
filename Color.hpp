#ifndef COLOR_HPP_INCLUDED
#define COLOR_HPP_INCLUDED

#include "Types.hpp"

class Color {
public:
  struct UniformData {
    vec4f color;
  };

  explicit 
  Color(vec4f const& color) {
    _data.color = color;
  }

  UniformData const&
  uniformData() const {
    return _data;
  }

private: // Member variables.
  UniformData _data;
};

#endif // COLOR_HPP_INCLUDED
