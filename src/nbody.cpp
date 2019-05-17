#define GL_GLEXT_PROTOTYPES

#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>
#include <random>
#include <time.h>
#include <string.h>
#include <omp.h>
#include <octree.hpp>
#include <math.h>

#include "resources.inl"

#define __BRUTE_FORCE__
#define __DEBUG_OCTREE__

glm::vec2 ScreenSize = {1024.0f, 768.0f};

const int totalPoints = 10000;

float WorldAngle = 0.0f;

float maxStarVelocity = 1.0f;

glm::vec3 gPoints[totalPoints];
glm::vec3 gVel[totalPoints];
glm::vec3 gForce[totalPoints];
glm::vec3 gBarycenter;

#ifdef __DEBUG_OCTREE__
int totalLines = 0;
GLuint aabbVAO;
GLuint aabbVBO;
GLuint SecondPassProgram;
GLuint SPModelViewUniform;
GLuint SPProjectionUniform;
#endif /* __DEBUG_OCTREE__ */

GLuint VAO;
GLuint VBO;
GLuint VelBO;
GLuint FirstPassProgram;

GLuint ModelViewUniform;
GLuint ProjectionUniform;
GLuint screenSizeUniform;
GLuint spriteSizeUniform;
GLuint FinalTextureUniform;
GLuint maxVelocityUniform;

glm::mat4 Projection;
glm::mat4 ModelView;

octree_t<glm::vec3*>* UpdateOctree() {
  aabb_t globalAABB;

  globalAABB.tl = glm::vec3(std::numeric_limits<float>::lowest());
  globalAABB.br = glm::vec3(std::numeric_limits<float>::max());

  for (int i = 0; i < totalPoints; ++i) {
    if (gPoints[i].x < globalAABB.br.x) {
      globalAABB.br.x = gPoints[i].x;
    }
    if (gPoints[i].y < globalAABB.br.y) {
      globalAABB.br.y = gPoints[i].y;
    }
    if (gPoints[i].z < globalAABB.br.z) {
      globalAABB.br.z = gPoints[i].z;
    }
    if (gPoints[i].x > globalAABB.tl.x) {
      globalAABB.tl.x = gPoints[i].x;
    }
    if (gPoints[i].y > globalAABB.tl.y) {
      globalAABB.tl.y = gPoints[i].y;
    }
    if (gPoints[i].z > globalAABB.tl.z) {
      globalAABB.tl.z = gPoints[i].z;
    }
  }

  auto* tmp = new octree_t<glm::vec3*>(globalAABB);

  for (int i = 0; i < totalPoints; ++i) {
    tmp->Push(gPoints[i]);
  }

#ifdef __DEBUG_OCTREE__
  std::vector<glm::vec3> bounds = tmp->Root().BuildBox();

  glBindVertexArray(aabbVAO);
  glBindBuffer(GL_ARRAY_BUFFER, aabbVBO);
  glBufferData(GL_ARRAY_BUFFER, bounds.size()*sizeof(glm::vec3), bounds.data(), GL_STREAM_DRAW);

  totalLines = bounds.size();
#endif /* __DEBUG_OCTREE__ */
  return tmp;
}

void Initialize()
{
  std::random_device rd;
  std::default_random_engine gen(rd());
  std::uniform_real_distribution<> dis(0.0f, 100.0f);
  std::uniform_real_distribution<> ang(0.0f, 2.0f*((float)M_PI));

  for (int i = 0; i < totalPoints; ++i) {
    float len = dis(gen);
    float rad = ang(gen);

    gPoints[i].x = len * std::cos(rad);
    gPoints[i].y = std::log10(dis(gen)/len)*3.0f;
    gPoints[i].z = len * std::sin(rad);
#ifdef ZERO_SPEED
    gVel[i].x = 0.0f;
    gVel[i].y = 0.0f;
    gVel[i].z = 0.0f;
#else
    gVel[i].x = -gPoints[i].z;
    gVel[i].y = dis(gen)/100.0f;
    gVel[i].z = gPoints[i].x;
    gVel[i] = glm::normalize(gVel[i])/50.0f;
#endif /* ZERO_SPEED */
  }

#ifdef __DEBUG_OCTREE__

	glGenVertexArrays(1, &aabbVAO);
  glBindVertexArray(aabbVAO);

  glGenBuffers(1, &aabbVBO);
  glBindBuffer(GL_ARRAY_BUFFER, aabbVBO);

  SecondPassProgram = CreateProgram(sp_vprog, sp_fprog);

#endif /* __DEBUG_OCTREE__ */

  auto* octree = UpdateOctree();
  gBarycenter = octree->Root().Barycenter();
  delete octree;

	glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(gPoints), gPoints, GL_STREAM_DRAW);

  glGenBuffers(1, &VelBO);
  glBindBuffer(GL_ARRAY_BUFFER, VelBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(gVel), gVel, GL_STREAM_DRAW);

  FirstPassProgram = CreateProgram(fp_vprog, fp_fprog);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  Projection = glm::perspective(glm::radians(60.0f), 4.0f/3.0f, 0.1f, 1000.0f);
  ModelView = glm::lookAt(
    glm::vec3(300.0f, 100.0f, 300.0f),
    gBarycenter,
    glm::vec3(0.0f, 1.0f, 0.0f)
  );

#ifdef __DEBUG_OCTREE__
  SPModelViewUniform = glGetUniformLocation(SecondPassProgram, "mat_ModelView");
  SPProjectionUniform = glGetUniformLocation(SecondPassProgram, "mat_Projection");
#endif /* __DEBUG_OCTREE__ */

  ModelViewUniform = glGetUniformLocation(FirstPassProgram, "mat_ModelView");
  ProjectionUniform = glGetUniformLocation(FirstPassProgram, "mat_Projection");
  screenSizeUniform = glGetUniformLocation(FirstPassProgram, "screenSize");
  spriteSizeUniform = glGetUniformLocation(FirstPassProgram, "spriteSize");
  maxVelocityUniform = glGetUniformLocation(FirstPassProgram, "maxVelocity");

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_PROGRAM_POINT_SIZE);

}

void FirstPass()
{
  glUseProgram(FirstPassProgram);

  glUniformMatrix4fv(ModelViewUniform, 1, GL_FALSE, &ModelView[0][0]);
  glUniformMatrix4fv(ProjectionUniform, 1, GL_FALSE, &Projection[0][0]);
  glUniform2fv(screenSizeUniform, 1, &ScreenSize[0]);
  glUniform1f(spriteSizeUniform, 2.0f);
  glUniform1f(maxVelocityUniform, maxStarVelocity);

  glBindVertexArray(VAO);
  
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, VelBO);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glDrawArrays(GL_POINTS, 0, totalPoints);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}

#ifdef __DEBUG_OCTREE__

void SecondPass()
{
  glUseProgram(SecondPassProgram);

  glUniformMatrix4fv(SPModelViewUniform, 1, GL_FALSE, &ModelView[0][0]);
  glUniformMatrix4fv(SPProjectionUniform, 1, GL_FALSE, &Projection[0][0]);

  glBindVertexArray(aabbVAO);
  
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, aabbVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glDrawArrays(GL_LINES, 0, totalLines);

  glDisableVertexAttribArray(0);
}

#endif /* __DEBUG_OCTREE__ */

void onDraw ()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0,0,ScreenSize.x,ScreenSize.y); 
#ifdef __DEBUG_OCTREE__
  SecondPass();
#endif /* __DEBUG_OCTREE__ */
  FirstPass();  
  glutSwapBuffers ();
}

void onIdle()
{
  timespec start;
  timespec end;

  maxStarVelocity = 0.0f;

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  for (int i = 0; i < totalPoints; ++i) 
  {
    gForce[i].x = 0.0f;
    gForce[i].y = 0.0f;
    gForce[i].z = 0.0f;
  }

  octree_t<glm::vec3*>* octree = UpdateOctree();

  #pragma omp for
  for (int i = 0; i < totalPoints; ++i) 
  {

#ifdef __BRUTE_FORCE__

    for (int j = i+1; j < totalPoints; ++j) 
    {
      glm::vec3 dlt = gPoints[j] - gPoints[i];
      float len = glm::length(dlt);
      dlt /= len;
      len = (len < 1.0f)?1.0f:len;
      gForce[i] += dlt*G/(len*len);
      gForce[j] -= dlt*G/(len*len);
    }

#ifdef __HEAVY_IN_CENTER__
    glm::vec3 heavy_dlt = -gPoints[i];
    float heavy_len = glm::length(heavy_dlt);
    heavy_dlt /= heavy_len;
    heavy_len = (heavy_len < 1.0f)?1.0f:heavy_len;
    gForce[i] += heavy_dlt*G/(heavy_len*heavy_len)*1000.0f;
#endif 

#else

    gForce[i] = octree->Root().GetForceOnPoint(gPoints[i]);

#endif /* __BRUTE_FORCE__  */

    gVel[i] += gForce[i];
  }
  for (int i = 0; i < totalPoints; ++i) 
  {
    gPoints[i] += gVel[i];
    float tMaxVel = glm::length(gVel[i]);
    if (tMaxVel > maxStarVelocity) {
      maxStarVelocity = tMaxVel;
    }
  }

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

  char buffer[256];
  double deltaTime;
  
  if ((end.tv_nsec-start.tv_nsec)<0) {
    deltaTime = (end.tv_sec-start.tv_sec-1) + (1000000000+end.tv_nsec-start.tv_nsec)/1.0e9;
  } else {
    deltaTime = (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec)/1.0e9;
  }

  snprintf(buffer, 255, "Physics: %7.4f sec", deltaTime);

  glutSetWindowTitle(buffer);

  gBarycenter = octree->Root().Barycenter();
  delete octree;

  ModelView = glm::lookAt(
    glm::vec3(200.0f, 200.0f, 200.0f),
    gBarycenter,
    glm::vec3(0.0f, 1.0f, 0.0f)
  );

  ModelView = glm::rotate(ModelView, WorldAngle, glm::vec3(0.0f, 1.0f, 0.0f));

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gPoints), gPoints);

  glBindBuffer(GL_ARRAY_BUFFER, VelBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gVel), gVel);

  glutPostRedisplay();
}

void onReshape ( int w, int h )
{
  glViewport ( 0, 0, (GLsizei)w, (GLsizei)h );

  Projection = glm::perspective(glm::radians(60.0f), ScreenSize.x/ScreenSize.y, 0.1f, 1000.0f);

  ScreenSize.x = w;
  ScreenSize.y = h;
}

void OnKeyDown ( unsigned char key, int x, int y )
{
  switch(key) {
    case 'q':
    case 'Q':
    case 27:
      glDeleteBuffers(1, &VBO);
      glDeleteBuffers(1, &VelBO);
      glDeleteVertexArrays(1, &VAO);
      glDeleteProgram(FirstPassProgram);
      glutLeaveMainLoop();
      return;
    case '[':
      WorldAngle += 0.02;
      return;
    case ']':
      WorldAngle -= 0.02;
      return;
 }
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize((int)ScreenSize.x, (int)ScreenSize.y);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_CONTINUE_EXECUTION);

  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  glutCreateWindow("N-Body Simulation Demo");

  glutDisplayFunc(onDraw);
  glutReshapeFunc(onReshape);
  glutKeyboardFunc(OnKeyDown);
  glutIdleFunc(onIdle);

  Initialize();

  glutMainLoop();
  return 0;
}