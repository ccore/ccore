#include <stdio.h>

#include <ccore/display.h>
#include <ccore/window.h>
#include <ccore/time.h>
#include <ccore/opengl.h>

#ifdef USE_EPOXY
#include <epoxy/gl.h>
#elif defined USE_GLEW
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#define EXIT_ON_E(x) {\
	ccError e = x; \
	if(e != CC_E_NONE){ \
		fprintf(stderr, "Line %d error: %s\n\t" #x ";\n", __LINE__, ccErrorString(e)); \
		ccFree(); \
		exit(1); \
	} \
}

static const float vdata[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	0.0f,  1.0f, 0.0f,
};

static const char *vertShader =
"#version 400\n"
"in vec3 vp;"
"void main () {"
"  gl_Position = vec4 (vp, 1.0);"
"}";

static const char *fragShader =
"#version 400\n"
"out vec4 frag_colour;"
"void main () {"
"  frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
"}";

int main(int argc, char** argv)
{
	EXIT_ON_E(ccDisplayInitialize());

	ccRect windowRect = {.x = 0, .y = 0, .width = 800, .height = 600};
	EXIT_ON_E(ccWindowCreate(windowRect, "ccore examples: opengl", 0));

	EXIT_ON_E(ccWindowSetCentered());

	EXIT_ON_E(ccGLContextBind());

#ifdef USE_GLEW
	glewInit();
#endif

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), vdata, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	GLuint vs = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertShader, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragShader, NULL);
	glCompileShader(fs);

	GLuint program = glCreateProgram();
	glAttachShader(program, fs);
	glAttachShader(program, vs);
	glLinkProgram(program);

	bool loop = true;
	while(loop) {
		while(ccWindowEventPoll()) {
			switch(ccWindowEventGet().type) {
				case CC_EVENT_WINDOW_QUIT:
					loop = false;
					break;
				case CC_EVENT_KEY_DOWN:
					switch(ccWindowEventGet().keyCode) {
						case CC_KEY_M:
							EXIT_ON_E(ccWindowSetMaximized());
							break;
						case CC_KEY_W:
							EXIT_ON_E(ccWindowSetWindowed(windowRect));

							EXIT_ON_E(ccWindowSetCentered());
							break;
						case CC_KEY_ESCAPE:
							loop = false;
							break;
					}

					break;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.5, 0.5, 0.5, 1.0);

		glUseProgram(program);
		glBindVertexArray (vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glFlush();

		EXIT_ON_E(ccGLBuffersSwap());
	}

	ccFree();

	return 0;
}
