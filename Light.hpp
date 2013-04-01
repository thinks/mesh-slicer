#ifndef LIGHT_HPP_INCLUDED
#define LIGHT_HPP_INCLUDED

#include "Types.hpp"

class Light {
public:
  struct UniformData {
    vec4f ambientColor;
    vec4f diffuseColor;
    vec4f specularColor;
    vec4f position;
  };

  explicit 
  Light(vec4f const& ambientColor = vec4f(1.f, 1.f, 1.f, 1.f),
        vec4f const& diffuseColor = vec4f(1.f, 1.f, 1.f, 1.f),
        vec4f const& specularColor = vec4f(1.f, 1.f, 1.f, 1.f),
        vec4f const& position = vec4f(0.f, 0.f, 0.f, 1.f)) {
    _data.ambientColor = ambientColor;
    _data.diffuseColor = diffuseColor;
    _data.specularColor = specularColor;
    _data.position = position;
  }

  UniformData const&
  uniformData() const {
    return _data;
  }

  void
  setPosition(vec4f const& position) {
    _data.position = position;
  }

private: // Member variables.
  UniformData _data;
};

#endif // LIGHT_HPP_INCLUDED
