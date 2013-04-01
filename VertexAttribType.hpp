#ifndef VERTEX_ATTRIB_TYPE_HPP_INCLUDED
#define VERTEX_ATTRIB_TYPE_HPP_INCLUDED

#include <thx.hpp>
#include <nDjinn.hpp>

//! Specialization.
template <typename V>
struct VertexAttribType {
  static GLenum const VALUE = 
    ndj::VertexAttribType<typename V::value_type>::VALUE;
};

#endif // VERTEX_ATTRIB_TYPE_HPP_INCLUDED
