#ifndef ORTHO_CAMERA_HPP_INCLUDED
#define ORTHO_CAMERA_HPP_INCLUDED

#include "Camera.hpp"
#include "Plane.hpp"
#include "Model.hpp"
#include "Frustum.hpp"
#include "Types.hpp"

class OrthoCamera : public Camera {
public:
  explicit
  OrthoCamera(vec3f const& position = vec3f(0.f, 0.f, 0.f), 
              vec3f const& rotation = vec3f(0.f, 0.f, 0.f), 
              Frustum const& frustum = 
                Frustum(-.5f, .5f, -.5f, .5f, -.5f, .5f)) 
    : Camera(computeView(position, rotation), 
             computeOrthoProjection(frustum),
             frustum) {
  }

  explicit
  OrthoCamera(Plane const& plane, 
              Model const& planeModel,
              Frustum const& frustum = 
                Frustum(-.5f, .5f, -.5f, .5f, -.5f, .5f)) 
    : Camera(computeView(plane, planeModel), 
             computeOrthoProjection(frustum), 
             frustum) {
  }

  virtual
  ~OrthoCamera() {
  }

  void
  setProjection(Frustum const& frustum) {
    Camera::setProjection(computeOrthoProjection(frustum), frustum);
  }
};

#endif // ORTHO_CAMERA_HPP_INCLUDED
