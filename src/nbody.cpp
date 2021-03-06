#define GL_GLEXT_PROTOTYPES
#define _USE_MATH_DEFINES

#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_INLINE GLM_ENABLE
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES GLM_ENABLE
#define GLM_FORCE_INTRINSICS GLM_ENABLE
#define GLM_FORCE_SIMD_AVX2 GLM_ENABLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>
#include <random>
#include <time.h>
#include <string.h>

#ifndef __USE_CUDA__
#include <omp.h>
#endif 

#include <octree.hpp>

#include <math.h>

#include "resources.inl"

#include <nbody.h>
#include <fb_t.hpp>
#include <vao_t.hpp>

//#define __BRUTE_FORCE__
//#define __USE_CUDA__
//#define __DEBUG_OCTREE__
#define __WND_MODE__

glm::vec2 ScreenSize = { 1024.0f, 768.0f };

const int totalPoints = 1024*64;

const float maxRadius = 200.0f;
const float fadeOut = 0.98f;

const float nearPlane = 10.0f;
const float farPlane = 10000.0f;

float WorldAngle = 0.0f;
float WorldDistance = 500.0f;

float maxStarVelocity = 0.0f;

glm::vec3 gPoints[totalPoints];
glm::vec3 gVel[totalPoints];
glm::vec3 gForce[totalPoints];
glm::vec3 gBarycenter;

#ifdef __DEBUG_OCTREE__
GLsizei totalLines = 0;
GLuint aabbVAO;
GLuint aabbVBO;
GLuint SecondPassProgram;
GLuint SPModelViewUniform;
GLuint SPProjectionUniform;
#endif /* __DEBUG_OCTREE__ */

fb_t screen;
fb_t fpass;
fb_t spass;

vao_t VAO;
vbo_t VBO;
vbo_t VelBO;

GLuint FirstPassProgram;

GLuint ModelViewUniform;
GLuint ProjectionUniform;
GLuint screenSizeUniform;
GLuint spriteSizeUniform;
GLuint maxVelocityUniform;

glm::mat4 Projection;
glm::mat4 ModelView;

GLuint QuadPassProgram;

vao_t QuadVAO;
vbo_t QuadVBO;
vbo_t QuadTexBO;

GLuint TempTextureUniform;
GLuint AlphaUniform;

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
	glBufferData(GL_ARRAY_BUFFER, bounds.size() * sizeof(glm::vec3), bounds.data(), GL_STREAM_DRAW);

	totalLines = (GLsizei)bounds.size();
#endif /* __DEBUG_OCTREE__ */
	return tmp;
}

void Initialize()
{
	std::random_device rd;
	std::default_random_engine gen(rd());
	std::uniform_real_distribution<float> dis(0.0f, maxRadius);
	std::uniform_real_distribution<float> ang(0.0f, 2.0f * ((float)M_PI));
	std::uniform_real_distribution<float> theta(0.0f, (float)M_PI);

	for (int i = 0; i < totalPoints; ++i) {
//#define GALAXY_DISC
#ifndef GALAXY_DISC
		float r = dis(gen);
		float t = theta(gen);
		float p = ang(gen);

		gPoints[i].x = r * std::sin(t)*std::cos(p);
		gPoints[i].y = r * std::sin(t)*std::sin(p);
		gPoints[i].z = r * std::cos(t);

		r = dis(gen)/5000.0f;
		t += theta(gen)/10.0f;
		p += ang(gen)/10.0f;

		gVel[i].x = r * std::sin(t)*std::cos(p);
		gVel[i].y = r * std::sin(t)*std::sin(p);
		gVel[i].z = r * std::cos(t);
#endif

#ifdef GALAXY_DISC
#define GALAXY_SPREAD
#ifdef GALAXY_SPREAD
		float len = dis(gen);
		float rad = ang(gen);

		gPoints[i].x = len * std::cos(rad);
		gPoints[i].y = std::log10(dis(gen) / len) * 3.0f;
		gPoints[i].z = len * std::sin(rad);
#else
		gPoints[i].x = dis(gen);
		gPoints[i].y = dis(gen);
		gPoints[i].z = dis(gen);
#endif

#define ZERO_SPEED
#ifdef ZERO_SPEED
		gVel[i].x = 0.0f;
		gVel[i].y = 0.0f;
		gVel[i].z = 0.0f;
#else
		gVel[i].x = -gPoints[i].z;
		gVel[i].y = dis(gen) / 100.0f;
		gVel[i].z = gPoints[i].x;
		gVel[i] = glm::normalize(gVel[i])*pow(maxRadius-len, 2.0f)/600000.0f;
#endif /* ZERO_SPEED */
#endif /* GALAXY_DISC */
	}

#ifdef __DEBUG_OCTREE__

	glGenVertexArrays(1, &aabbVAO);
	glBindVertexArray(aabbVAO);

	glGenBuffers(1, &aabbVBO);
	glBindBuffer(GL_ARRAY_BUFFER, aabbVBO);

	SecondPassProgram = CreateProgram(sp_vprog, sp_fprog);

#endif /* __DEBUG_OCTREE__ */

#ifndef __BRUTE_FORCE__
	auto* octree = UpdateOctree();
	gBarycenter = octree->Root().Barycenter();
	delete octree;
#else

#ifdef __USE_CUDA__

	simInitialize(totalPoints, gPoints, gVel);

#endif /* __USE_CUDA__ */

#endif /* __BRUTE_FORCE__ */


	VAO = VAOCreate();
	VAO.Bind();

	VBO = VBOCreate(gPoints, sizeof(gPoints), GL_STREAM_DRAW);
	VelBO = VBOCreate(gVel, sizeof(gVel), GL_STREAM_DRAW);

	QuadVAO = VAOCreate();
	QuadVAO.Bind();

	QuadVBO = VBOCreate(v_quad, sizeof(v_quad), GL_STATIC_DRAW);
	QuadTexBO = VBOCreate(t_quad, sizeof(t_quad), GL_STATIC_DRAW);

	FirstPassProgram = CreateProgram(fp_vprog, fp_fprog);
	QuadPassProgram = CreateProgram(quad_vprog, quad_fprog);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	Projection = glm::perspective(glm::radians(90.0f), 4.0f / 3.0f, nearPlane, farPlane);
	ModelView = glm::lookAt(
		gBarycenter + glm::vec3(WorldDistance),
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

	TempTextureUniform = glGetUniformLocation(QuadPassProgram, "star_map");
	AlphaUniform = glGetUniformLocation(QuadPassProgram, "alpha");

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_MAX);
	glEnable(GL_PROGRAM_POINT_SIZE);

	fpass = fb_t(ScreenSize);
	spass = fb_t(ScreenSize);
}

void FirstPass()
{
	glUseProgram(FirstPassProgram);

	glUniformMatrix4fv(ModelViewUniform, 1, GL_FALSE, &ModelView[0][0]);
	glUniformMatrix4fv(ProjectionUniform, 1, GL_FALSE, &Projection[0][0]);
	glUniform2fv(screenSizeUniform, 1, &ScreenSize[0]);
	glUniform1f(spriteSizeUniform, 1.0f);
	glUniform1f(maxVelocityUniform, maxStarVelocity);

	VAO.Bind();
	VBOUseAs useVBO(VBO, 0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	VBOUseAs useVelBO(VelBO, 1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, totalPoints);
}

void QuadPass(const fb_t& pass, float alpha)
{
	glUseProgram(QuadPassProgram);
	glUniform1i(TempTextureUniform, 0);
	glUniform1f(AlphaUniform, alpha);

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, pass.Texture());

	QuadVAO.Bind();
	VBOUseAs useQuadVBO(QuadVBO, 0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	VBOUseAs useQuadTexBO(QuadTexBO, 1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
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

void onDraw()
{
	spass.Bind();
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, 1024, 768);
	QuadPass(fpass, fadeOut);

	fpass.Bind();
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, 1024, 768);
	QuadPass(spass, 1.0f);
	FirstPass();

	screen.Bind();
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)ScreenSize.x, (GLsizei)ScreenSize.y);
	QuadPass(fpass, 1.0f);

#ifdef __DEBUG_OCTREE__
	SecondPass();
#endif /* __DEBUG_OCTREE__ */

	glutSwapBuffers();
}

#ifdef _WIN32

class Frequency {
public:
	LARGE_INTEGER self;
	Frequency() :self() {
		QueryPerformanceFrequency(&this->self);
	}
} G_FREQ;


#define TIMESTAMP LARGE_INTEGER

#define MARK_TIME(MARKER) QueryPerformanceCounter(&(MARKER))

#define DELTA_TIME(RESULT, START, END) (RESULT) = (((END).QuadPart - (START).QuadPart))/((double)G_FREQ.self.QuadPart)

#else

#define TIMESTAMP timespec

#define MARK_TIME(MARKER) clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &(MARKER))

#define DELTA_TIME(RESULT, START, END) if (((END).tv_nsec-(START).tv_nsec)<0) {\
  (RESULT) = ((END).tv_sec - (START).tv_sec - 1) + (1000000000 + (END).tv_nsec - (START).tv_nsec) / 1.0e9;\
} else {\
  (RESULT) = ((END).tv_sec - (START).tv_sec) + ((END).tv_nsec - (START).tv_nsec) / 1.0e9;\
}

#endif

void onIdle()
{
	TIMESTAMP start;
	TIMESTAMP end;

	MARK_TIME(start);

#ifndef __BRUTE_FORCE__
	octree_t<glm::vec3*>* octree = UpdateOctree();
#endif /* __BRUTE_FORCE__ */

#if defined(__BRUTE_FORCE__) && defined(__USE_CUDA__)

	simStep(gPoints, gVel, totalPoints);

	for (int i = 0; i < totalPoints; ++i)
	{
		gPoints[i] += gVel[i];
		float tMaxVel = glm::length(gVel[i]);
		if (tMaxVel > maxStarVelocity) {
			maxStarVelocity = tMaxVel;
		}
	}

#else
#ifndef __BRUTE_FORCE__
#pragma omp parallel firstprivate(octree)
#else
#pragma omp parallel
#endif
	{
#pragma omp for
		for (int i = 0; i < totalPoints; ++i)
		{
			gForce[i].x = 0.0f;
			gForce[i].y = 0.0f;
			gForce[i].z = 0.0f;
		}

#pragma omp for
		for (int i = 0; i < totalPoints; ++i)
		{

#ifdef __BRUTE_FORCE__

			for (int j = i + 1; j < totalPoints; ++j)
			{
				glm::vec3 dlt = gPoints[j] - gPoints[i];
				float len = glm::length(dlt);
				dlt /= len;
				len = (len < 1.0f) ? 1.0f : len;
				gForce[i] += dlt * G / (len * len);
				gForce[j] -= dlt * G / (len * len);
			}

#ifdef __HEAVY_IN_CENTER__
			glm::vec3 heavy_dlt = -gPoints[i];
			float heavy_len = glm::length(heavy_dlt);
			heavy_dlt /= heavy_len;
			heavy_len = (heavy_len < 1.0f) ? 1.0f : heavy_len;
			gForce[i] += heavy_dlt * G / (heavy_len * heavy_len) * 1000.0f;
#endif 

#else
			gForce[i] = octree->Root().GetForceOnPoint(gPoints[i]);

#endif /* __BRUTE_FORCE__  */

			gVel[i] += gForce[i];
		}

#pragma omp for
		for (int i = 0; i < totalPoints; ++i)
		{
			gPoints[i] += gVel[i];
			float tMaxVel = glm::length(gVel[i]);
			if (tMaxVel > maxStarVelocity) {
				maxStarVelocity = tMaxVel;
			}
		}
	}

#ifndef __BRUTE_FORCE__
	gBarycenter = octree->Root().Barycenter();
	delete octree;
#endif /* __BRUTE_FORCE__ */

#endif 

	MARK_TIME(end);

	char buffer[256];
	double deltaTime = 0.0;

	DELTA_TIME(deltaTime, start, end);

	snprintf(buffer, 255, "Physics: %7.4f sec; Max Velocity: %7.4f; Barycenter: [%7.4f, %7.4f, %7.4f]",
		deltaTime,
		maxStarVelocity,
		gBarycenter.x, gBarycenter.y, gBarycenter.z
	);

	glutSetWindowTitle(buffer);

	ModelView = glm::lookAt(
		gBarycenter + glm::vec3(WorldDistance),
		gBarycenter,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	ModelView = glm::rotate(ModelView, WorldAngle, glm::vec3(0.0f, 1.0f, 0.0f));

	VAO.Bind();

	VBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gPoints), gPoints);

	VelBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gVel), gVel);

	glutPostRedisplay();
}

void onReshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	Projection = glm::perspective(glm::radians(60.0f), ScreenSize.x / ScreenSize.y, nearPlane, farPlane);

	ScreenSize.x = (float)w;
	ScreenSize.y = (float)h;
}

void OnKeyDown(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':
	case 'Q':
	case 27:
#ifndef __WND_MODE__
		glutLeaveGameMode();
#endif
		VBO.Free();
		VelBO.Free();
		VAO.Free();
		QuadVAO.Free();
		glDeleteProgram(FirstPassProgram);
		glutLeaveMainLoop();

#ifdef __BRUTE_FORCE__
#ifdef __USE_CUDA__

		simCleanup();

#endif /* __USE_CUDA__ */
#endif /* __BRUTE_FORCE__ */
		return;
	case '[':
		WorldAngle += 0.02f;
		return;
	case ']':
		WorldAngle -= 0.02f;
		return;
	case '+':
		WorldDistance *= 1.01f;
		return;
	case '-':
		WorldDistance *= 0.99f;
		return;
	}
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize((int)ScreenSize.x, (int)ScreenSize.y);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_CORE_PROFILE);

#ifndef __WND_MODE__
	glutGameModeString("1920x1080:32");
	glutEnterGameMode();
#else
	glutCreateWindow("N-Body Simulation Demo");
#endif

	int glewError = glewInit();

	if (glewError != GLEW_OK) {
		printf("GLEW Error: %s\n", glewGetErrorString(glewError));
		return -1;
	}

	glutDisplayFunc(onDraw);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(OnKeyDown);
	glutIdleFunc(onIdle);

	Initialize();


	glutMainLoop();
	return 0;
}