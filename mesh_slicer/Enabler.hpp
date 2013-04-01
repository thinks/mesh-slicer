#ifndef ENABLER_HPP_INCLUDED
#define ENABLER_HPP_INCLUDED

#include <nDjinn.hpp>
#include <GL/GL.h>

//! DOCS
class Enabler {
public:
  explicit
  Enabler(GLenum const flag) 
    : _flag(flag) {
    ndj::enable(_flag);
  }

  ~Enabler() {
    ndj::disable(_flag);
  }

private: // Member variables.
  GLenum const _flag;
};

#endif // ENABLER_HPP_INCLUDED
