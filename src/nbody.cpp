#define GL_GLEXT_PROTOTYPES

#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>
#include <random>

static const char*  const vprog = 
  "#version 330 core\n"
  "\n"
  "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
  "layout(location = 1) in vec3 vertexVelocity_modelspace;\n"
  "out vec3 vel;\n"
  "\n"
  "uniform mat4 mat_Projection;\n"
  "uniform mat4 mat_ModelView;\n"
  "uniform vec2 screenSize;\n"
  "uniform float spriteSize;\n"
  "void main() {\n"
  "  vec4 eyePos = mat_ModelView * vec4(vertexPosition_modelspace, 1.0);\n"
  "  vec4 projVoxel = mat_Projection * vec4(spriteSize,spriteSize,eyePos.z,eyePos.w);\n"
  "  vec2 projSize = screenSize * projVoxel.xy / projVoxel.w;\n"
  "  gl_PointSize = 0.25 * (projSize.x + projSize.y);\n"
  "  gl_Position = mat_Projection * eyePos;\n"
  "  vel = abs(normalize(vertexVelocity_modelspace)-vec3(0.5));\n"
  "}\n";

static const char* const fprog = 
  "#version 330 core\n"
  "\n"
  "layout(location = 0) out vec4 color;\n"
  "in vec3 vel;\n"
  "\n"
  "void main(){\n"
  "  vec2 dvec = gl_PointCoord - vec2(0.5,0.5);\n"
  "  float sqdist = max(0.0, 0.25 - dot(dvec, dvec))*4.0;\n"
  "  color = vec4(sqdist)*vec4(vel, 0.8);\n"
  "}\n";

const int totalPoints = 1000;

glm::vec3 gPoints[totalPoints];
glm::vec3 gVel[totalPoints];
glm::vec3 gForce[totalPoints];
glm::vec3 gBarycenter;

GLuint VAO;
GLuint VBO;
GLuint VelBO;
GLuint DrawProgram;

GLuint DepthBuffer;
GLuint FrameBuffer;
GLuint BakedTexture;

GLuint ModelViewUniform;
GLuint ProjectionUniform;
GLuint screenSizeUniform;
GLuint spriteSizeUniform;

glm::mat4 Projection;
glm::mat4 ModelView;

glm::vec2 ScreenSize(1024.0f, 768.0f);

GLuint LoadShader(GLenum shaderType, const char* shaderText) {
  GLuint result = glCreateShader(shaderType);
  glShaderSource(result, 1, &shaderText, NULL);
  glCompileShader(result);

  GLint compileResult = GL_FALSE;
  GLint compileInfoLen = 0;

  glGetShaderiv(result, GL_COMPILE_STATUS, &compileResult);
  glGetShaderiv(result, GL_INFO_LOG_LENGTH, &compileInfoLen);
  if ( compileInfoLen > 0 ){
    char* text = new char[compileInfoLen+1];
    glGetShaderInfoLog(result, compileInfoLen, NULL, text);
    printf("Compilation: %s\n", text);
    free(text);
  }
  return result;
}

void UpdateBarycenter() {
  gBarycenter = glm::vec3(0.0f);
  for (int i = 0; i < totalPoints; ++i) {
    gBarycenter += gPoints[i]/((float)totalPoints);
  }
}

void Initialize()
{
  std::random_device rd;
  std::default_random_engine gen(rd());
  std::uniform_real_distribution<> dis(0.0f, 50.0f);
  std::uniform_real_distribution<> ang(0.0f, 2.0f*((float)M_PI));

  for (int i = 0; i < totalPoints; ++i) {
    float len = dis(gen);
    float rad = ang(gen);

    gPoints[i].x = len * std::cos(rad);
    gPoints[i].y = std::log10(dis(gen)/len)*3.0f;
    gPoints[i].z = len * std::sin(rad);
/*
    gVel[i].x = 0.0f;
    gVel[i].y = 0.0f;
    gVel[i].z = 0.0f;
*/
    gVel[i].x = -gPoints[i].z;
    gVel[i].y = dis(gen)/100.0f;
    gVel[i].z = gPoints[i].x;

    gVel[i] = glm::normalize(gVel[i])/25.0f;
  }

  UpdateBarycenter();

	glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(gPoints), gPoints, GL_STREAM_DRAW);

  glGenBuffers(1, &VelBO);
  glBindBuffer(GL_ARRAY_BUFFER, VelBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(gVel), gVel, GL_STREAM_DRAW);

  GLuint VertexShader = LoadShader(GL_VERTEX_SHADER, vprog);
  GLuint FragmentShader = LoadShader(GL_FRAGMENT_SHADER, fprog);

  DrawProgram = glCreateProgram();
  glAttachShader(DrawProgram, VertexShader);
  glAttachShader(DrawProgram, FragmentShader);
  glLinkProgram(DrawProgram);

  GLint linkResult = GL_FALSE;
  GLint linkInfoLen = 0;

  glGetProgramiv(DrawProgram, GL_LINK_STATUS, &linkResult);
  glGetProgramiv(DrawProgram, GL_INFO_LOG_LENGTH, &linkInfoLen);
  if ( linkInfoLen > 0 ){
    char* text = new char[linkInfoLen+1];
    glGetProgramInfoLog(DrawProgram, linkInfoLen, NULL, text);
    printf("Link: %s\n", text);
    free(text);
  }
  
  glDeleteShader(VertexShader);
  glDeleteShader(FragmentShader);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  Projection = glm::perspective(glm::radians(60.0f), 4.0f/3.0f, 0.1f, 1000.0f);
  ModelView = glm::lookAt(
    glm::vec3(300.0f, 100.0f, 300.0f),
    gBarycenter,
    glm::vec3(0.0f, 1.0f, 0.0f)
  );

  ModelViewUniform = glGetUniformLocation(DrawProgram, "mat_ModelView");
  ProjectionUniform = glGetUniformLocation(DrawProgram, "mat_Projection");
  screenSizeUniform = glGetUniformLocation(DrawProgram, "screenSize");
  spriteSizeUniform = glGetUniformLocation(DrawProgram, "spriteSize");

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_PROGRAM_POINT_SIZE);

  glGenFramebuffers(1, &FrameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

  glGenTextures(1, &BakedTexture);
  glBindTexture(GL_TEXTURE_2D, BakedTexture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ScreenSize.x, ScreenSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glGenRenderbuffers(1, &DepthBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, DepthBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, ScreenSize.x, ScreenSize.y);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, BakedTexture, 0);

  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    printf("FrameBuffer error\n");
    glutLeaveMainLoop();
  }
}

void onDraw ()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0,0,ScreenSize.x,ScreenSize.y); 
  glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
  glUseProgram(DrawProgram);

  glUniformMatrix4fv(ModelViewUniform, 1, GL_FALSE, &ModelView[0][0]);
  glUniformMatrix4fv(ProjectionUniform, 1, GL_FALSE, &Projection[0][0]);
  glUniform2fv(screenSizeUniform, 1, &ScreenSize[0]);
  glUniform1f(spriteSizeUniform, 1.5f);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, VAO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, VelBO);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glDrawArrays(GL_POINTS, 0, totalPoints);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

  glutSwapBuffers ();
}

const float G = 6.6742867e-5f;

void onIdle()
{
  for (int i = 0; i < totalPoints; ++i) 
  {
    gForce[i].x = 0.0f;
    gForce[i].y = 0.0f;
    gForce[i].z = 0.0f;
  }

  for (int i = 0; i < totalPoints; ++i) 
  {
    for (int j = i+1; j < totalPoints; ++j) 
    {
      glm::vec3 dlt = gPoints[j] - gPoints[i];
      float len = glm::length(dlt);
      dlt /= len;
      len = (len < 1.0f)?1.0f:len;
      gForce[i] += dlt*G/(len*len);
      gForce[j] -= dlt*G/(len*len);
    }

    // Heavy in center
    glm::vec3 heavy_dlt = -gPoints[i];
    float heavy_len = glm::length(heavy_dlt);
    heavy_dlt /= heavy_len;
    heavy_len = (heavy_len < 1.0f)?1.0f:heavy_len;
    gForce[i] += heavy_dlt*G/(heavy_len*heavy_len)*1000.0f;

    gVel[i] += gForce[i];
  }
  for (int i = 0; i < totalPoints; ++i) 
  {
    gPoints[i] += gVel[i];
  }

  UpdateBarycenter();

  ModelView = glm::lookAt(
    glm::vec3(200.0f, 100.0f, 200.0f),
    gBarycenter,
    glm::vec3(0.0f, 1.0f, 0.0f)
  );

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gPoints), gPoints);

  glBindBuffer(GL_ARRAY_BUFFER, VelBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gVel), gVel);

  glutPostRedisplay();
}

void onReshape ( int w, int h )
{
  glViewport ( 0, 0, (GLsizei)w, (GLsizei)h );
  ScreenSize.x = w;
  ScreenSize.y = h;
}

void OnKeyDown ( unsigned char key, int x, int y )
{
  if ( key == 27 || key == 'q' || key == 'Q' )
  {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VelBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(DrawProgram);

    glDeleteRenderbuffers(1, &DepthBuffer);
    glDeleteTextures(1, &BakedTexture);
    glDeleteFramebuffers(1, &FrameBuffer);

    glutLeaveMainLoop();
  }
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
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