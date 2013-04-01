#ifndef VERTEX_ATTRIB_SIZE_HPP_INCLUDED
#define VERTEX_ATTRIB_SIZE_HPP_INCLUDED

#include <thx.hpp>
#include <GL/GL.h>

//! Generic.
template<class T>
struct VertexAttribSize;

//! Specialization.
template<std::size_t N, typename S>
struct VertexAttribSize<typename thx::vec<N,S>> {
  static GLint const VALUE = 
    static_cast<GLint>(typename thx::vec<N,S>::linear_size);
};

#endif // VERTEX_ATTRIB_SIZE_HPP_INCLUDED
