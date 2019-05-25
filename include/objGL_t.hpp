#pragma once
#ifndef __OBJ_GL_T_HEADER__
#define __OBJ_GL_T_HEADER__

#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <vector>

template<typename manGL_t>
class objGL_t: private manGL_t {

  GLuint item;

  objGL_t<manGL_t>& operator = (const objGL_t<manGL_t>& nocopy);
  objGL_t(const objGL_t<manGL_t>& nocopy);

public:

  void Bind() const {
    manGL_t::BindItem(this->item);
  }

  void Free() {
    if (this->item != 0) {
      manGL_t::FreeItem(this->item);
      this->item = 0;
    }
  }

  objGL_t<manGL_t>& operator = (objGL_t<manGL_t>&& move) {
    if (this != &move) {
      manGL_t::FreeItem(this->item);
      this->item = move.item;
      move.item = 0;
    }
    return *this;
  }

  objGL_t(objGL_t<manGL_t>&& move):item(move.item) {
    move.item = 0;
  }

  objGL_t(GLuint item):item(item) {
    ;
  }

  objGL_t():item(0) {
    ;
  }

  virtual ~objGL_t() {
    if (this->item != 0) {
      manGL_t::FreeItem(this->item);
      this->item = 0;
    }
  }
};

#endif /* __OBJ_GL_T_HEADER__ */