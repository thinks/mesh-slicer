#ifndef VIEWPORT_HPP_INCLUDED
#define VIEWPORT_HPP_INCLUDED

#include <GL/GL.h>

//! DOCS
struct Viewport {
  explicit
  Viewport(GLint const x = 0, 
           GLint const y = 0, 
           GLsizei const width = 0, 
           GLsizei const height = 0)
    : x(x)
    , y(y)
    , width(width)
    , height(height) {
  }

  GLint x;
  GLint y;
  GLsizei width;
  GLsizei height;
};

#endif // VIEWPORT_HPP_INCLUDED
