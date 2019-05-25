#pragma once
#ifndef __VAO_T_HEADER__
#define __VAO_T_HEADER__

#include <objGL_t.hpp>

class vaoImpl_t {
protected:
  static void FreeItem(GLuint item) {
    glDeleteVertexArrays(1, &item);
  }
  static void BindItem(GLuint item) {
    glBindVertexArray(item);
  }
};

class vboImpl_t {
protected:
  static void FreeItem(GLuint item) {
    glDeleteBuffers(1, &item);
  }
  static void BindItem(GLuint item) {
    glBindBuffer(GL_ARRAY_BUFFER, item);
  }
};

typedef objGL_t<vaoImpl_t> vao_t;
typedef objGL_t<vboImpl_t> vbo_t;

vao_t VAOCreate();

vao_t& VAONone();

vbo_t VBOCreate(const void* data, GLsizei dataSize, GLenum usage);

void VBOUse(const vbo_t& vbo, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);

void VBONoUse(GLuint index);

class VBOUseAs {

  GLuint index;

  VBOUseAs(const VBOUseAs&) = delete;
  VBOUseAs(VBOUseAs&&) = delete;

  VBOUseAs& operator = (const VBOUseAs&) = delete;
  VBOUseAs& operator = (VBOUseAs&&) = delete;

public:
  VBOUseAs(const vbo_t& vbo, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer):index(index) {
    VBOUse(vbo, index, size, type, normalized, stride, pointer);
  }

  ~VBOUseAs() {
    VBONoUse(index);
  }
};


#endif /* __VAO_T_HEAEDER__ */