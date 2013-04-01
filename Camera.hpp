#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include "Frustum.hpp"
#include "Plane.hpp"
#include "Model.hpp"
#include "Types.hpp"

class Camera {
public:
  struct UniformData {
    explicit 
    UniformData(mat4f const& view, mat4f const& projection)
      : viewMatrix(view)
      , projectionMatrix(projection) {
    }

    mat4f viewMatrix;
    mat4f projectionMatrix;
  };

  explicit
  Camera(mat4f const& view, mat4f const& projection, Frustum const& frustum)
    : _data(view, projection) 
    , _frustum(frustum) {
  }

  virtual
  ~Camera() = 0;

  UniformData const& 
  uniformData() const {
    return _data;
  }

  void 
  setView(Plane const& plane, Model const& model) {
    _data.viewMatrix = computeView(plane, model);
  }

  void 
  setView(vec3f const& position, vec3f const& rotation) {
    _data.viewMatrix = computeView(position, rotation);
  }

  vec3f
  position() const {
    mat4f const& v = _data.viewMatrix;
    mat3f const rt(
      -v(0, 0), -v(1, 0), -v(2, 0),
      -v(0, 1), -v(1, 1), -v(2, 1),
      -v(0, 2), -v(1, 2), -v(2, 2));
    return rt*vec3f(v(0, 3), v(1, 3), v(2, 3));
  }

  Frustum const& 
  frustum() const {
    return _frustum;
  }

protected:
  void
  setProjection(mat4f const& projection, Frustum const& frustum) {
    _data.projectionMatrix = projection;
    _frustum = frustum;
  }

  static mat4f
  computeView(Plane const& plane, Model const& model) {
    mat4f const& m = model.uniformData().modelMatrix;
    mat3f const r = 
      mat3f(m(0, 0), m(0, 1), m(0, 2),
            m(1, 0), m(1, 1), m(1, 2),
            m(2, 0), m(2, 1), m(2, 2));

    vec3f const wb0 = thx::normalized(r*plane.base0);
    vec3f const wb1 = thx::normalized(r*plane.base1);
    vec3f const wb2 = thx::normalized(r*plane.base2);
    vec3f const wp = m*plane.position;

    return mat4f(
      wb0[0], wb0[1], wb0[2], -thx::dot(wb0, wp), // Right.
      wb1[0], wb1[1], wb1[2], -thx::dot(wb1, wp), // Up.
      wb2[0], wb2[1], wb2[2], -thx::dot(wb2, wp), // Back.
      0.f,    0.f,    0.f,    1.f);
  }

  static mat4f
  computeView(vec3f const& position, vec3f const& rotation) {
    mat4f const rx(thx::rotation_x(-thx::deg_to_rad(rotation[0])));
    mat4f const ry(thx::rotation_y(-thx::deg_to_rad(rotation[1])));
    mat4f const rz(thx::rotation_z(-thx::deg_to_rad(rotation[2])));
    mat4f const t(thx::translation3(-position));
    return rz*ry*rx*t;
  }

  static mat4f 
  computeOrthoProjection(Frustum const& frustum) {
    return thx::ortho_projection(
      frustum.left, 
      frustum.right, 
      frustum.bottom, 
      frustum.top, 
      frustum.near, 
      frustum.far);
  }

  static mat4f 
  computePerspProjection(Frustum const& frustum) {
    return thx::persp_projection(
      frustum.left, 
      frustum.right, 
      frustum.bottom, 
      frustum.top, 
      frustum.near, 
      frustum.far);
  }

private: // Member variables.
  UniformData _data;
  Frustum _frustum;
};

inline 
Camera::~Camera() {
}

#endif // CAMERA_HPP_INCLUDED
