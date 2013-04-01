#ifndef PLANE_HPP_INCLUDED
#define PLANE_HPP_INCLUDED

#include "Types.hpp"

//! Store plane object space.
struct Plane {
public:
  explicit 
  Plane(//mat4f const& modelMatrix,
        vec3f const& position = vec3f(0.f, 0.f, 0.f),
        vec3f const& base0 = vec3f(1.f, 0.f, 0.f),
        vec3f const& base1 = vec3f(0.f, 1.f, 0.f))
    : /*modelMatrix(modelMatrix)
    , */base0(base0)
    , base1(base1)
    , base2(thx::cross(base0, base1))
    , position(position) {
  }

#if 0
  vec3f
  worldBase0() const {
    return modelMatrix*base0;
  }

  vec3f
  worldBase1() const {
    return modelMatrix*base1;
  }

  vec3f
  worldBase2() const {
    return modelMatrix*base2;
  }

  vec3f worldPosition() const {
    return modelMatrix*position;
  }
#endif

  //mat4f modelMatrix;
  vec3f const base0;
  vec3f const base1;
  vec3f const base2;
  vec3f const position;
};

#endif // PLANE_HPP_INCLUDED
