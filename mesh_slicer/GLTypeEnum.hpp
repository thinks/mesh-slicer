#ifndef GL_TYPE_ENUM_HPP_INCLUDED
#define GL_TYPE_ENUM_HPP_INCLUDED

#include <thx.hpp>
#include <GL/GL.h>

//! Generic.
template <class T>
struct GLTypeEnum;

//! Specialization.
template<>
struct GLTypeEnum<thx::float32> {
  static GLenum const value = GL_FLOAT;
};

//! Specialization.
template<>
struct GLTypeEnum<thx::uint32> {
  static GLenum const value = GL_UNSIGNED_INT;
};


#endif // GL_TYPE_ENUM_HPP_INCLUDED
