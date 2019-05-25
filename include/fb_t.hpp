#pragma once
#ifndef __FB_T_HEADER__
#define __FB_T_HEADER__

#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>

class fb_t {

  GLuint fbo;
  GLuint rbo;
  GLuint tex;

  void Create(glm::ivec2 size);

  fb_t(const fb_t& nocopy); 
  fb_t& operator = (const fb_t& nocopy);

public:

  GLuint Texture() const;
  void Bind() const;

  fb_t(fb_t&& move); 
  fb_t& operator = (fb_t&& move);

  fb_t();
  fb_t(glm::ivec2 size);
  ~fb_t();
};



#endif /* __FB_T_HEADER__ */