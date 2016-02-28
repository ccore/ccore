#include "x11_display.h"

#include <stdio.h>

static ccReturn ccXFindDisplaysXinerama(Display *display, char *displayName)
{
	int eventBase, errorBase;
	if(CC_UNLIKELY(!XineramaQueryExtension(display, &eventBase, &errorBase) ||
								 !XineramaIsActive(display))) {
		ccPrintf("Xinerama not supported or active\n");
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	ccDisplayData currentResolution = {.bitDepth = -1};
	_ccDisplays->primary = 0;

	Window root = RootWindow(display, 0);
	XRRScreenResources *resources = XRRGetScreenResources(display, root);
	if(CC_UNLIKELY(resources->noutput <= 0)) {
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	int i;
	for(i = 0; i < resources->noutput; i++) {
		XRROutputInfo *outputInfo =
				XRRGetOutputInfo(display, resources, resources->outputs[i]);
		/* Ignore disconnected devices */
		if(outputInfo->connection != 0) {
			continue;
		}

		_ccDisplays->amount++;
		if(_ccDisplays->amount == 1) {
			ccMalloc(_ccDisplays->display, sizeof(ccDisplay));
		} else {
			ccRealloc(_ccDisplays->display, sizeof(ccDisplay) * _ccDisplays->amount);
		}
		ccDisplay *currentDisplay = _ccDisplays->display + _ccDisplays->amount - 1;

		ccMalloc(currentDisplay->data, sizeof(ccDisplay_x11));

		int displayNameLength = strlen(displayName);
		currentDisplay->deviceName = malloc(displayNameLength + 1);
		memcpy(currentDisplay->deviceName, displayName, displayNameLength);

		currentDisplay->monitorName = malloc(outputInfo->nameLen + 1);
		memcpy(currentDisplay->monitorName, outputInfo->name, outputInfo->nameLen);

		currentDisplay->deviceName[displayNameLength] = '\0';
		//TODO find gpu name
		currentDisplay->gpuName = "Undefined";

		bool foundCrtc = false;
		int j;
		for(j = 0; j < resources->ncrtc; j++) {
			if(resources->crtcs[j] != outputInfo->crtc) {
				continue;
			}
			XRRCrtcInfo *crtcInfo =
					XRRGetCrtcInfo(display, resources, resources->crtcs[j]);
			if(crtcInfo->mode == None) {
				continue;
			}

			currentDisplay->x = crtcInfo->x;
			currentDisplay->y = crtcInfo->y;
			DISPLAY_DATA(currentDisplay)->XOldMode = crtcInfo->mode;
			foundCrtc = true;

			XRRFreeCrtcInfo(crtcInfo);
			break;
		}
		if(!foundCrtc) {
			currentDisplay->x = -1;
			currentDisplay->y = -1;
		}

		int xscreen = 0;
		DISPLAY_DATA(currentDisplay)->XineramaScreen = i;
		DISPLAY_DATA(currentDisplay)->XScreen = xscreen;
		DISPLAY_DATA(currentDisplay)->XOutput = resources->outputs[i];
		currentDisplay->dpi = ((double)DisplayWidth(display, xscreen) * 25.4) / (double)DisplayWidthMM(display, xscreen);

		currentDisplay->current = 0;
		currentDisplay->amount = 0;

		for(j = 0; j < outputInfo->nmode; j++) {
			int k;
			for(k = 0; k < resources->nmode; k++) {
				if(outputInfo->modes[j] == resources->modes[k].id) {
					unsigned int vTotal = resources->modes[k].vTotal;
					if(resources->modes[k].modeFlags & RR_DoubleScan) {
						vTotal <<= 1;
					}
					if(resources->modes[k].modeFlags & RR_Interlace) {
						vTotal >>= 1;
					}

					ccMalloc(currentResolution.data, sizeof(ccDisplayData_x11));

					((ccDisplayData_x11 *)currentResolution.data)->XMode =
							outputInfo->modes[j];
					currentResolution.refreshRate = resources->modes[k].dotClock /
																					(resources->modes[k].hTotal * vTotal);
					currentResolution.width = resources->modes[k].width;
					currentResolution.height = resources->modes[k].height;

					currentDisplay->amount++;
					if(currentDisplay->amount == 1) {
						ccMalloc(currentDisplay->resolution, sizeof(ccDisplayData));
					} else {
						ccRealloc(currentDisplay->resolution,
											sizeof(ccDisplayData) * currentDisplay->amount);
					}
					memcpy(currentDisplay->resolution + (currentDisplay->amount - 1),
								 &currentResolution,
								 sizeof(ccDisplayData));
					break;
				}
			}
		}

		XRRFreeOutputInfo(outputInfo);
	}

	XRRFreeScreenResources(resources);

	return CC_SUCCESS;
}

ccReturn ccDisplayInitialize(void)
{
	if(CC_UNLIKELY(_ccDisplays != NULL)) {
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	ccMalloc(_ccDisplays, sizeof(ccDisplays));
	_ccDisplays->amount = 0;

	DIR *dir = opendir("/tmp/.X11-unix");
	if(CC_UNLIKELY(dir == NULL)) {
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	struct dirent *direntry;
	while((direntry = readdir(dir)) != NULL) {
		if(direntry->d_name[0] != 'X') {
			continue;
		}
		char displayName[64];
		snprintf(displayName, 64, ":%s", direntry->d_name + 1);
		Display *display = XOpenDisplay(displayName);
		if(display != NULL) {
			if(CC_UNLIKELY(ccXFindDisplaysXinerama(display, displayName))) {
				XCloseDisplay(display);
				return CC_FAIL;
			}
			XCloseDisplay(display);
		}
	}

	return CC_SUCCESS;
}

ccReturn ccDisplayFree(void)
{
	if(CC_UNLIKELY(_ccDisplays == NULL)) {
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	int i;
	for(i = 0; i < _ccDisplays->amount; i++) {
		free(_ccDisplays->display[i].data);
		free(_ccDisplays->display[i].monitorName);
		free(_ccDisplays->display[i].deviceName);

		int j;
		for(j = 0; j < _ccDisplays->display[i].amount; j++) {
			free(_ccDisplays->display[i].resolution[j].data);
		}

		free(_ccDisplays->display[i].resolution);
	}
	free(_ccDisplays->display);
	free(_ccDisplays);

	_ccDisplays = NULL;

	return CC_SUCCESS;
}

ccReturn ccDisplayResolutionSet(ccDisplay *display, int resolutionIndex)
{
	if(CC_UNLIKELY(display == NULL)) {
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	if(CC_UNLIKELY(resolutionIndex > display->amount || resolutionIndex < -1)) {
		ccErrorPush(CC_ERROR_INVALID_ARGUMENT);
		return CC_FAIL;
	}

	if(resolutionIndex == display->current) {
		return CC_SUCCESS;
	}

	Display *XDisplay = XOpenDisplay(display->deviceName);
	Window root = DefaultRootWindow(XDisplay);
	XGrabServer(XDisplay);

	XRRScreenResources *resources = XRRGetScreenResources(XDisplay, root);
	if(CC_UNLIKELY(!resources)) {
		ccPrintf("X: Couldn't get screen resources");
		ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
		goto fail;
	}

	XRROutputInfo *outputInfo =
			XRRGetOutputInfo(XDisplay, resources, DISPLAY_DATA(display)->XOutput);
	if(CC_UNLIKELY(!outputInfo || outputInfo->connection == RR_Disconnected)) {
		XRRFreeOutputInfo(outputInfo);
		ccPrintf("X: Couldn't get output info");
		ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
		goto fail;
	}

	XRRCrtcInfo *crtcInfo = XRRGetCrtcInfo(XDisplay, resources, outputInfo->crtc);
	if(CC_UNLIKELY(!crtcInfo)) {
		XRRFreeOutputInfo(outputInfo);
		XRRFreeCrtcInfo(crtcInfo);
		ccPrintf("X: Couldn't get crtc info");
		ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
		goto fail;
	}

	if(resolutionIndex != CC_DEFAULT_RESOLUTION) {
		ccDisplayData *displayData = display->resolution + resolutionIndex;

		if(CC_UNLIKELY(displayData->width <= 8 || displayData->height <= 8)) {
			XRRFreeOutputInfo(outputInfo);
			XRRFreeCrtcInfo(crtcInfo);
			ccPrintf("Error: Resolution supplied not valid\n");
			ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
			goto fail;
		}

		int minX, minY, maxX, maxY;
		if(CC_UNLIKELY(!XRRGetScreenSizeRange(
											 XDisplay, root, &minX, &minY, &maxX, &maxY))) {
			XRRFreeOutputInfo(outputInfo);
			XRRFreeCrtcInfo(crtcInfo);
			ccPrintf("X: Unable to get screen size range\n");
			ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
			goto fail;
		}

		if(CC_UNLIKELY(displayData->width < minX || displayData->height < minY)) {
			XRRFreeOutputInfo(outputInfo);
			XRRFreeCrtcInfo(crtcInfo);
			ccPrintf("X: Unable to set size of screen below the minimum of %dx%d\n",
							 minX,
							 minY);
			ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
			goto fail;
		} else if(CC_UNLIKELY(displayData->width > maxX ||
													displayData->height > maxY)) {
			XRRFreeOutputInfo(outputInfo);
			XRRFreeCrtcInfo(crtcInfo);
			ccPrintf("X: Unable to set size of screen above the maximum of %dx%d\n",
							 maxX,
							 maxY);
			ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
			goto fail;
		}

		XRRSetCrtcConfig(XDisplay,
										 resources,
										 outputInfo->crtc,
										 CurrentTime,
										 crtcInfo->x,
										 crtcInfo->y,
										 ((ccDisplayData_x11 *)displayData->data)->XMode,
										 crtcInfo->rotation,
										 &DISPLAY_DATA(display)->XOutput,
										 1);
	} else {
		XRRSetCrtcConfig(XDisplay,
										 resources,
										 outputInfo->crtc,
										 CurrentTime,
										 crtcInfo->x,
										 crtcInfo->y,
										 DISPLAY_DATA(display)->XOldMode,
										 crtcInfo->rotation,
										 &DISPLAY_DATA(display)->XOutput,
										 1);
	}

	XRRFreeScreenResources(resources);
	XRRFreeOutputInfo(outputInfo);
	XRRFreeCrtcInfo(crtcInfo);

	XSync(XDisplay, False);
	XUngrabServer(XDisplay);
	XCloseDisplay(XDisplay);

	return CC_SUCCESS;

fail:
	XRRFreeScreenResources(resources);

	XSync(XDisplay, False);
	XUngrabServer(XDisplay);
	XCloseDisplay(XDisplay);

	return CC_FAIL;
}
