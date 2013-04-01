#ifndef MODEL_HPP_INCLUDED
#define MODEL_HPP_INCLUDED

#include "Types.hpp"

class Model {
public:
  struct UniformData {
    mat4f modelMatrix;
    mat4f normalMatrix;
  };

  explicit 
  Model()
    : _translation(0.f, 0.f, 0.f)
    , _rotation(0.f, 0.f, 0.f) 
    , _scale(1.f, 1.f, 1.f)
  {
    _data.normalMatrix = mat4f(1.f); // Identity.
    _updateModelMatrix();
  }

  explicit 
  Model(vec3f const& translation, 
        vec3f const& rotation,
        vec3f const& scale)
    : _translation(translation)
    , _rotation(rotation) 
    , _scale(scale)
  {
    _data.normalMatrix = mat4f(1.f); // Identity.
    _updateModelMatrix();
  }

  UniformData const& 
  uniformData() const {
    return _data;
  }

  vec3f const& 
  translation() const {
    return _translation;
  }

  vec3f const& 
  rotation() const {
    return _rotation;
  }

  vec3f const& 
  scale() const {
    return _scale;
  }

  void 
  setTranslation(vec3f const& translation) {
    _translation = translation;
    _updateModelMatrix();
  }

  void 
  setRotation(vec3f const& rotation) {
    _rotation = rotation;
    _updateModelMatrix();
  }

  void 
  setScale(vec3f const& scale) {
    _scale = scale;
    _updateModelMatrix();
  }

  void 
  set(vec3f const& translation, vec3f const& rotation, vec3f const& scale) {
    _translation = translation;
    _rotation = rotation;
    _scale = scale;
    _updateModelMatrix();
  }

  void
  setNormalMatrix(mat4f const& viewMatrix) {
    _data.normalMatrix = 
      thx::transposed(
        thx::inverted(
          viewMatrix*_data.modelMatrix));
  }

private:
  void
  _updateModelMatrix() {
    mat4f const rx(thx::rotation_x(thx::deg_to_rad(_rotation[0])));
    mat4f const ry(thx::rotation_y(thx::deg_to_rad(_rotation[1])));
    mat4f const rz(thx::rotation_z(thx::deg_to_rad(_rotation[2])));
    mat4f const r(rx*ry*rz); // NOTE: xyz order hard-coded for now!
    mat4f const t(thx::translation3(_translation));
    mat4f const s(thx::scale3(_scale));
    _data.modelMatrix = s*r*t;
  }

private: // Member variables.
  vec3f _translation; 
  vec3f _rotation;
  vec3f _scale;
  UniformData _data;
};

#endif // MODEL_HPP_INCLUDED
