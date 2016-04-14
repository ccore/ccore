#include "x11_display.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include <X11/extensions/Xinerama.h>

#include <ccore/display.h>
#include <ccore/assert.h>
#include <ccore/print.h>

static ccError ccXFindDisplaysXinerama(Display *display, char *displayName)
{
	int eventBase, errorBase;
	if(CC_UNLIKELY(!XineramaQueryExtension(display, &eventBase, &errorBase) || !XineramaIsActive(display))) {
		return CC_E_DISPLAY_NONE;
	}

	ccDisplayData currentResolution = {.bitDepth = -1};
	_ccDisplays->primary = 0;

	Window root = RootWindow(display, 0);
	XRRScreenResources *resources = XRRGetScreenResources(display, root);
	if(CC_UNLIKELY(resources->noutput <= 0)) {
		return CC_E_DISPLAY_NONE;
	}

	int i;
	for(i = 0; i < resources->noutput; i++) {
		XRROutputInfo *outputInfo = XRRGetOutputInfo(display, resources, resources->outputs[i]);

		// Ignore disconnected devices
		if(outputInfo->connection != 0) {
			continue;
		}

		_ccDisplays->amount++;
		if(_ccDisplays->amount == 1) {
			_ccDisplays->display = malloc(sizeof(ccDisplay));
			if(_ccDisplays->display == NULL){
				return CC_E_MEMORY_OVERFLOW;
			}
		} else {
			_ccDisplays->display = realloc(_ccDisplays->display, sizeof(ccDisplay) * _ccDisplays->amount);
			if(_ccDisplays->display == NULL){
				return CC_E_MEMORY_OVERFLOW;
			}
		}
		ccDisplay *currentDisplay = _ccDisplays->display + _ccDisplays->amount - 1;

		currentDisplay->data = malloc(sizeof(ccDisplay_x11));
		if(currentDisplay->data == NULL){
			return CC_E_MEMORY_OVERFLOW;
		}

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

					currentResolution.data = malloc(sizeof(ccDisplayData_x11));
					if(currentResolution.data == NULL){
						return CC_E_MEMORY_OVERFLOW;
					}

					((ccDisplayData_x11 *)currentResolution.data)->XMode = outputInfo->modes[j];
					currentResolution.refreshRate = resources->modes[k].dotClock / (resources->modes[k].hTotal * vTotal);
					currentResolution.width = resources->modes[k].width;
					currentResolution.height = resources->modes[k].height;

					currentDisplay->amount++;
					if(currentDisplay->amount == 1) {
						currentDisplay->resolution = malloc(sizeof(ccDisplayData));
						if(currentDisplay->resolution == NULL){
							return CC_E_MEMORY_OVERFLOW;
						}
					} else {
						currentDisplay->resolution = realloc(currentDisplay->resolution, sizeof(ccDisplayData) * currentDisplay->amount);
						if(currentDisplay->resolution == NULL){
							return CC_E_MEMORY_OVERFLOW;
						}
					}
					memcpy(currentDisplay->resolution + (currentDisplay->amount - 1), &currentResolution, sizeof(ccDisplayData));
					break;
				}
			}
		}

		XRRFreeOutputInfo(outputInfo);
	}

	XRRFreeScreenResources(resources);

	return CC_E_NONE;
}

ccError ccDisplayInitialize(void)
{
	if(CC_UNLIKELY(_ccDisplays != NULL)) {
		return CC_E_DISPLAY_NONE;
	}

	_ccDisplays = malloc(sizeof(ccDisplays));
	if(_ccDisplays == NULL){
		return CC_E_MEMORY_OVERFLOW;
	}
	_ccDisplays->amount = 0;

	DIR *dir = opendir("/tmp/.X11-unix");
	if(CC_UNLIKELY(dir == NULL)) {
		return CC_E_DISPLAY_NONE;
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
				return CC_E_WM;
			}
			XCloseDisplay(display);
		}
	}

	return CC_E_NONE;
}

ccError ccDisplayFree(void)
{
	if(CC_UNLIKELY(_ccDisplays == NULL)) {
		return CC_E_DISPLAY_NONE;
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

	return CC_E_NONE;
}

ccError ccDisplayResolutionSet(ccDisplay *display, int resolutionIndex)
{
	if(CC_UNLIKELY(display == NULL)) {
		return CC_E_DISPLAY_NONE;
	}

	if(CC_UNLIKELY(resolutionIndex > display->amount || resolutionIndex < -1)) {
		return CC_E_INVALID_ARGUMENT;
	}

	if(resolutionIndex == display->current) {
		return CC_E_NONE;
	}

	Display *XDisplay = XOpenDisplay(display->deviceName);
	Window root = DefaultRootWindow(XDisplay);
	XGrabServer(XDisplay);

	XRRScreenResources *resources = XRRGetScreenResources(XDisplay, root);
	if(CC_UNLIKELY(!resources)) {
		goto fail;
	}

	XRROutputInfo *outputInfo = XRRGetOutputInfo(XDisplay, resources, DISPLAY_DATA(display)->XOutput);
	if(CC_UNLIKELY(!outputInfo || outputInfo->connection == RR_Disconnected)) {
		XRRFreeOutputInfo(outputInfo);
		goto fail;
	}

	XRRCrtcInfo *crtcInfo = XRRGetCrtcInfo(XDisplay, resources, outputInfo->crtc);
	if(CC_UNLIKELY(!crtcInfo)) {
		XRRFreeOutputInfo(outputInfo);
		XRRFreeCrtcInfo(crtcInfo);
		goto fail;
	}

	if(resolutionIndex != CC_DEFAULT_RESOLUTION) {
		ccDisplayData *displayData = display->resolution + resolutionIndex;

		if(CC_UNLIKELY(displayData->width <= 8 || displayData->height <= 8)) {
			XRRFreeOutputInfo(outputInfo);
			XRRFreeCrtcInfo(crtcInfo);
			goto fail;
		}

		int minX, minY, maxX, maxY;
		if(CC_UNLIKELY(!XRRGetScreenSizeRange(XDisplay, root, &minX, &minY, &maxX, &maxY))) {
			XRRFreeOutputInfo(outputInfo);
			XRRFreeCrtcInfo(crtcInfo);
			goto fail;
		}

		if(CC_UNLIKELY(displayData->width < minX || displayData->height < minY)) {
			XRRFreeOutputInfo(outputInfo);
			XRRFreeCrtcInfo(crtcInfo);
			goto fail;
		} else if(CC_UNLIKELY(displayData->width > maxX || displayData->height > maxY)) {
			XRRFreeOutputInfo(outputInfo);
			XRRFreeCrtcInfo(crtcInfo);
			goto fail;
		}

		XRRSetCrtcConfig(XDisplay, resources, outputInfo->crtc, CurrentTime, crtcInfo->x, crtcInfo->y, ((ccDisplayData_x11 *)displayData->data)->XMode, crtcInfo->rotation, &DISPLAY_DATA(display)->XOutput, 1);
	} else {
		XRRSetCrtcConfig(XDisplay, resources, outputInfo->crtc, CurrentTime, crtcInfo->x, crtcInfo->y, DISPLAY_DATA(display)->XOldMode, crtcInfo->rotation, &DISPLAY_DATA(display)->XOutput, 1);
	}

	XRRFreeScreenResources(resources);
	XRRFreeOutputInfo(outputInfo);
	XRRFreeCrtcInfo(crtcInfo);

	XSync(XDisplay, False);
	XUngrabServer(XDisplay);
	XCloseDisplay(XDisplay);

	return CC_E_NONE;

fail:
	XRRFreeScreenResources(resources);

	XSync(XDisplay, False);
	XUngrabServer(XDisplay);
	XCloseDisplay(XDisplay);

	return CC_E_DISPLAY_RESOLUTIONCHANGE;
}
