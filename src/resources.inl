static const char fp_vprog[] = 
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
  "uniform float maxVelocity;\n"
  "void main() {\n"
  "  vec4 eyePos = mat_ModelView * vec4(vertexPosition_modelspace, 1.0);\n"
  "  vec4 projVoxel = mat_Projection * vec4(spriteSize,spriteSize,eyePos.z,eyePos.w);\n"
  "  vec2 projSize = screenSize * projVoxel.xy / projVoxel.w;\n"
  "  gl_PointSize = 0.25 * (projSize.x + projSize.y);\n"
  "  gl_Position = mat_Projection * eyePos;\n"
  "  vel = abs(vertexVelocity_modelspace/maxVelocity-vec3(0.5));\n"
  "}\n";

static const char fp_fprog[] = 
  "#version 330 core\n"
  "\n"
  "layout(location = 0) out vec4 color;\n"
  "in vec3 vel;\n"
  "\n"
  "void main(){\n"
  "  vec2 dvec = gl_PointCoord - vec2(0.5,0.5);\n"
  "  float sqdist = max(0.0, 0.25 - dot(dvec, dvec))*4.0;\n"
  "  color = vec4(sqdist)*vec4(vel*2.0 - 0.2, 0.2);\n"
  "}\n";

static const char sp_vprog[] = 
  "#version 330 core\n"
  "\n"
  "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
  "layout(location = 1) in vec2 vertexTexCoord;\n"
  "out vec2 texCoord;\n"
  "\n"
  "uniform mat4 mat_Projection;\n"
  "uniform mat4 mat_ModelView;\n"
  "\n"
  "void main() {\n"
  "  gl_Position = mat_Projection * mat_ModelView * vec4(vertexPosition_modelspace, 1.0);\n"
  "  texCoord = vertexTexCoord;\n"
  "}\n";

static const char sp_fprog[] = 
  "#version 330 core\n"
  "\n"
  "layout(location = 0) out vec4 color;\n"
  "\n"
  "void main(){\n"
  "  color = vec4(0.2);\n"
  "}\n";

static const char quad_vprog[] = 
  "#version 330 core\n"
  "\n"
  "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
  "layout(location = 1) in vec2 vertexTexCoord;\n"
  "out vec2 texCoord;\n"
  "\n"
  "void main() {\n"
  "  gl_Position = vec4(vertexPosition_modelspace, 1.0);\n"
  "  texCoord = vertexTexCoord;\n"
  "}\n";


static const char quad_fprog[] = 
  "#version 330 core\n"
  "\n"
  "layout(location = 0) out vec4 color;\n"
  "uniform sampler2D star_map;\n"
  "uniform float alpha;"
  "in vec2 texCoord;"
  "\n"
  "void main(){\n"
  "  color = texture(star_map, texCoord)*alpha;\n"
  "}\n";

static glm::vec3 v_quad[6] = {
  {-1.0f, -1.0f, 0.0f},
  { 1.0f, -1.0f, 0.0f},
  { 1.0f,  1.0f, 0.0f},
  {-1.0f, -1.0f, 0.0f},
  { 1.0f,  1.0f, 0.0f},
  {-1.0f,  1.0f, 0.0f}
};

static glm::vec2 t_quad[6] = {
  { 0.0f,  0.0f},
  { 1.0f,  0.0f},
  { 1.0f,  1.0f},
  { 0.0f,  0.0f},
  { 1.0f,  1.0f},
  { 0.0f,  1.0f}
};

GLuint LoadShader(GLenum shaderType, const char* shaderText) {
  GLuint result = glCreateShader(shaderType);
  glShaderSource(result, 1, &shaderText, NULL);
  glCompileShader(result);

  GLint compileResult = GL_FALSE;
  GLint compileInfoLen = 0;

  glGetShaderiv(result, GL_COMPILE_STATUS, &compileResult);
  glGetShaderiv(result, GL_INFO_LOG_LENGTH, &compileInfoLen);
  if ( compileInfoLen > 1 ){
	compileInfoLen += 1;
    char* text = new char[compileInfoLen];
    glGetShaderInfoLog(result, compileInfoLen, NULL, text);
    printf("Compilation[%s]: %s\n", ((compileResult==GL_TRUE)?"OK":"FAILED"), text);
    delete[] text;
  }
  return result;
}

GLuint CreateProgram(const char* vshader, const char* fshader) {
  GLuint VertexShader = LoadShader(GL_VERTEX_SHADER, vshader);
  GLuint FragmentShader = LoadShader(GL_FRAGMENT_SHADER, fshader);

  int result = glCreateProgram();
  glAttachShader(result, VertexShader);
  glAttachShader(result, FragmentShader);
  glLinkProgram(result);

  GLint linkResult = GL_FALSE;
  GLint linkInfoLen = 0;

  glGetProgramiv(result, GL_LINK_STATUS, &linkResult);
  glGetProgramiv(result, GL_INFO_LOG_LENGTH, &linkInfoLen);
  if ( linkInfoLen > 1 ){
	linkInfoLen += 1;
    char* text = new char[linkInfoLen];
    glGetProgramInfoLog(result, linkInfoLen, NULL, text);
    printf("Link[%s]: %s\n", ((linkResult == GL_TRUE) ? "OK" : "FAILED"), text);
    delete[] text;
  }
  
  glDeleteShader(VertexShader);
  glDeleteShader(FragmentShader);

  return result;
}