#ifndef PERSP_CAMERA_HPP_INCLUDED
#define PERSP_CAMERA_HPP_INCLUDED

#include "Camera.hpp"
#include "Plane.hpp"
#include "Model.hpp"
#include "Frustum.hpp"
#include "Types.hpp"

class PerspCamera : public Camera {
public:
  explicit
  PerspCamera(Plane const& plane, 
              Model const& planeModel,
              Frustum const& frustum = 
                Frustum(-.5f, .5f, -.5f, .5f, .1f, 1.f)) 
    : Camera(computeView(plane, planeModel), 
             computePerspProjection(frustum), 
             frustum) {
  }

  explicit
  PerspCamera(vec3f const& position = vec3f(0.f, 0.f, 0.f), 
              vec3f const& rotation = vec3f(0.f, 0.f, 0.f), 
              Frustum const& frustum = 
                Frustum(-.5f, .5f, -.5f, .5f, .1f, 1.f)) 
    : Camera(computeView(position, rotation), 
             computePerspProjection(frustum), 
             frustum) {
  }

  virtual
  ~PerspCamera() {
  }

  void
  setProjection(Frustum const& frustum) {
    Camera::setProjection(computePerspProjection(frustum), frustum);
  }
};

#endif // PERSP_CAMERA_HPP_INCLUDED
