#include <stdio.h>

#include <GL/gl.h>

#include <ccore/display.h>
#include <ccore/window.h>
#include <ccore/time.h>
#include <ccore/opengl.h>

#define EXIT_ON_E(x) {\
	ccError e = x; \
	if(e != CC_E_NONE){ \
		fprintf(stderr, "Line %d error: %s\n\t" #x ";\n", __LINE__, ccErrorString(e)); \
		ccFree(); \
		exit(1); \
	} \
}

int main(int argc, char** argv)
{
	EXIT_ON_E(ccDisplayInitialize());

	ccRect windowRect = {.x = 0, .y = 0, .width = 800, .height = 600};
	EXIT_ON_E(ccWindowCreate(windowRect, "ccore examples: opengl", CC_WINDOW_FLAG_NORESIZE));

	EXIT_ON_E(ccGLContextBind());

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

		glBegin(GL_TRIANGLES);
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-1.0f, 1.0f, 0.0f);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 1.0f, 0.0f);
		glEnd();

		EXIT_ON_E(ccGLBuffersSwap());
	}

	ccFree();

	return 0;
}
