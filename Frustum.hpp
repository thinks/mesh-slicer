#ifndef FRUSTUM_HPP_INCLUDED
#define FRUSTUM_HPP_INCLUDED

#include <GL/GL.h>

//! DOCS
struct Frustum {
  explicit 
  Frustum(GLfloat const left = -.5f,
          GLfloat const right = .5f,
          GLfloat const bottom = -.5f,
          GLfloat const top = .5f,
          GLfloat const near = -.5f,
          GLfloat const far = .5f) 
    : left(left)
    , right(right)
    , bottom(bottom)
    , top(top)
    , near(near)
    , far(far) {
  }

  GLfloat left;
  GLfloat right;
  GLfloat bottom;
  GLfloat top;
  GLfloat near;
  GLfloat far;
};

#endif // FRUSTUM_HPP_INCLUDED
