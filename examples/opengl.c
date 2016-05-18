#include <stdio.h>

#include <ccore/display.h>
#include <ccore/window.h>
#include <ccore/time.h>
#include <ccore/opengl.h>

#ifdef LINUX
#ifdef USE_GLEW
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#endif

#define EXIT_ON_E(x) {\
	ccError e = x; \
	if(e != CC_E_NONE){ \
		fprintf(stderr, "Line %d error: %s\n\t" #x ";\n", __LINE__, ccErrorString(e)); \
		ccFree(); \
		exit(1); \
	} \
}

static const GLfloat vdata[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
};

int main(int argc, char** argv)
{
	EXIT_ON_E(ccDisplayInitialize());

	ccRect windowRect = {.x = 0, .y = 0, .width = 800, .height = 600};
	EXIT_ON_E(ccWindowCreate(windowRect, "ccore examples: opengl", CC_WINDOW_FLAG_NORESIZE));

	EXIT_ON_E(ccGLContextBind());

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vdata), vdata, GL_STATIC_DRAW);


	EXIT_ON_E(ccWindowSetCentered());

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

  	glClearColor(0.5, 0.5, 0.5, 1.0);
  	glClear(GL_COLOR_BUFFER_BIT);
	
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(0);

		glFlush();

		EXIT_ON_E(ccGLBuffersSwap());
	}

	ccFree();

	return 0;
}
