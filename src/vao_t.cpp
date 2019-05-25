#include <vao_t.hpp>

vao_t VAOCreate() {
  GLuint result;
  glGenVertexArrays(1, &result);
  return vao_t(result);
}

vao_t& VAONone() {
  static vao_t none(0);
  return none;
}

vbo_t VBOCreate(const void* data, GLsizei dataSize, GLenum usage) {
  GLuint result;

  glGenBuffers(1, &result);
  glBindBuffer(GL_ARRAY_BUFFER, result);
  glBufferData(GL_ARRAY_BUFFER, dataSize, data, usage);
  return vbo_t(result);
}

void VBOUse(const vbo_t& vbo, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer) {
  glEnableVertexAttribArray(index);
  vbo.Bind();
  glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void VBONoUse(GLuint index) {
  glDisableVertexAttribArray(index);
}
