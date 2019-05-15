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
  "void main() {\n"
  "  vec4 eyePos = mat_ModelView * vec4(vertexPosition_modelspace, 1.0);\n"
  "  vec4 projVoxel = mat_Projection * vec4(spriteSize,spriteSize,eyePos.z,eyePos.w);\n"
  "  vec2 projSize = screenSize * projVoxel.xy / projVoxel.w;\n"
  "  gl_PointSize = 0.25 * (projSize.x + projSize.y);\n"
  "  gl_Position = mat_Projection * eyePos;\n"
  "  vel = abs(normalize(vertexVelocity_modelspace)-vec3(0.5));\n"
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
  "  color = vec4(sqdist)*vec4(vel, 0.8);\n"
  "}\n";

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
  if ( linkInfoLen > 0 ){
    char* text = new char[linkInfoLen+1];
    glGetProgramInfoLog(result, linkInfoLen, NULL, text);
    printf("Link: %s\n", text);
    free(text);
  }
  
  glDeleteShader(VertexShader);
  glDeleteShader(FragmentShader);

  return result;
}