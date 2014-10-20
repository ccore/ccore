#include <ccore/display.h>
#include <ccore/window.h>
#include <ccore/time.h>
#include <ccore/opengl.h>

#include <GL/gl.h>

int main(int argc, char** argv)
{
    ccRect windowRect;
    ccReturn returnValue;
    ccError error;
    bool loop = true;

    returnValue = ccDisplayInitialize();
    if(returnValue == CC_FAIL){
        goto outputError;
    }

    windowRect.x = 0;
    windowRect.y = 0;
    windowRect.width = 800;
    windowRect.height = 600;

    returnValue = ccWindowCreate(windowRect, "A ccore window", CC_WINDOW_FLAG_NORESIZE);
    if(returnValue == CC_FAIL){
        goto outputError;
    }

    returnValue = ccGLContextBind();
    if(returnValue == CC_FAIL){
        goto outputError;
    }

    returnValue = ccWindowSetCentered();
    if(returnValue == CC_FAIL){
        goto outputError;
    }

    while(loop) {
        while(ccWindowEventPoll()) {
            switch(ccWindowEventGet().type) {
                case CC_EVENT_WINDOW_QUIT:
                    loop = false;
                    break;
                case CC_EVENT_KEY_DOWN:
                    switch(ccWindowEventGet().keyCode) {
                        case CC_KEY_M:
                            returnValue = ccWindowSetMaximized();
                            if(returnValue == CC_FAIL){
                                goto outputError;
                            }
                            break;
                        case CC_KEY_W:
                            returnValue = ccWindowSetWindowed(NULL);
                            if(returnValue == CC_FAIL){
                                goto outputError;
                            }

                            returnValue = ccWindowResizeMove(windowRect);
                            if(returnValue == CC_FAIL){
                                goto outputError;
                            }

                            returnValue = ccWindowSetCentered();
                            if(returnValue == CC_FAIL){
                                goto outputError;
                            }
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

        returnValue = ccGLBuffersSwap();
        if(returnValue == CC_FAIL){
            goto outputError;
        }
    }

    ccFree();

    return 0;

outputError:

    while((error = ccErrorPop()) != CC_ERROR_NONE){
        ccPrintf("Error catched:\n%s\n", ccErrorString(error));
    }

    ccFree();

    return -1;
}
