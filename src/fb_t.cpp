#include <fb_t.hpp>

GLuint fb_t::Texture() const {
  return this->tex;
}

void fb_t::Bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
}

fb_t::fb_t():fbo(0),rbo(0),tex(0) {
  ;
}

void fb_t::Create(glm::ivec2 size) {
  glGenFramebuffers(1, &this->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);

  glGenTextures(1, &this->tex);
  glBindTexture(GL_TEXTURE_2D, this->tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->tex, 0);

  glGenRenderbuffers(1, &this->rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, this->rbo);

  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->rbo);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

fb_t::fb_t(fb_t&& move):fbo(move.fbo),rbo(move.rbo),tex(move.tex) {
  move.fbo = 0;
  move.rbo = 0;
  move.tex = 0;
}

fb_t& fb_t::operator = (fb_t&& move) {

  if (this != &move) {

    if (this->fbo != 0) {
      glDeleteFramebuffers(1, &this->fbo);
      glDeleteRenderbuffers(1, &this->rbo);
      glDeleteTextures(1, &this->tex);
    }

    this->fbo = move.fbo;
    this->rbo = move.rbo;
    this->tex = move.tex;

    move.fbo = 0;
    move.rbo = 0;
    move.tex = 0;
  }
  return *this;
}

fb_t::fb_t(glm::ivec2 size):fbo(0),rbo(0),tex(0) {
  this->Create(size);
}

fb_t::~fb_t() {
  if (this->fbo != 0) {
    glDeleteFramebuffers(1, &this->fbo);
    glDeleteRenderbuffers(1, &this->rbo);
    glDeleteTextures(1, &this->tex);
  }
}
