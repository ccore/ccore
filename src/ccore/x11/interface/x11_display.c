#include "x11_display.h"

static ccReturn ccXFindDisplaysXinerama(Display *display, char *displayName)
{
	int i, j, k, displayNameLength, eventBase, errorBase;
	bool foundCrtc;
	unsigned int vTotal;
	ccDisplay *currentDisplay;
	ccDisplayData currentResolution;
	Window root;
	XRRScreenResources *resources;
	XRROutputInfo *outputInfo;
	XRRCrtcInfo *crtcInfo;

	if(CC_UNLIKELY(!XineramaQueryExtension(display, &eventBase, &errorBase) || !XineramaIsActive(display))){
		ccPrintf("Xinerama not supported or active\n");
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	currentResolution.bitDepth = -1;
	_ccDisplays->primary = 0;

	root = RootWindow(display, 0);
	resources = XRRGetScreenResources(display, root);
	if(CC_UNLIKELY(resources->noutput <= 0)){
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	for(i = 0; i < resources->noutput; i++){
		outputInfo = XRRGetOutputInfo(display, resources, resources->outputs[i]);
		/* Ignore disconnected devices */
		if(outputInfo->connection != 0){
			continue;
		}

		_ccDisplays->amount++;
		if(_ccDisplays->amount == 1){
			ccMalloc(_ccDisplays->display, sizeof(ccDisplay));
		}else{
			ccRealloc(_ccDisplays->display, sizeof(ccDisplay) * _ccDisplays->amount);
		}
		currentDisplay = _ccDisplays->display + _ccDisplays->amount - 1;

		ccMalloc(currentDisplay->data, sizeof(ccDisplay_x11));

		displayNameLength = strlen(displayName);
		currentDisplay->deviceName = malloc(displayNameLength + 1);
		memcpy(currentDisplay->deviceName, displayName, displayNameLength);

		currentDisplay->monitorName = malloc(outputInfo->nameLen + 1);
		memcpy(currentDisplay->monitorName, outputInfo->name, outputInfo->nameLen);

		currentDisplay->deviceName[displayNameLength] = '\0';
		currentDisplay->gpuName = "Undefined";

		foundCrtc = false;
		for(j = 0; j < resources->ncrtc; j++){	
			if(resources->crtcs[j] != outputInfo->crtc){
				continue;
			}
			crtcInfo = XRRGetCrtcInfo(display, resources, resources->crtcs[j]);
			if(crtcInfo->mode == None){
				continue;
			}

			currentDisplay->x = crtcInfo->x;
			currentDisplay->y = crtcInfo->y;
			DISPLAY_DATA(currentDisplay)->XOldMode = crtcInfo->mode;
			foundCrtc = true;

			XRRFreeCrtcInfo(crtcInfo);
			break;
		}
		if(!foundCrtc){
			currentDisplay->x = -1;
			currentDisplay->y = -1;
		}

		DISPLAY_DATA(currentDisplay)->XineramaScreen = i;
		DISPLAY_DATA(currentDisplay)->XScreen = 0;
		DISPLAY_DATA(currentDisplay)->XOutput = resources->outputs[i];
		currentDisplay->current = 0;
		currentDisplay->amount = 0;

		for(j = 0; j < outputInfo->nmode; j++){
			for(k = 0; k < resources->nmode; k++){
				if(outputInfo->modes[j] == resources->modes[k].id){
					vTotal = resources->modes[k].vTotal;
					if(resources->modes[k].modeFlags & RR_DoubleScan){
						vTotal <<= 1;
					}
					if(resources->modes[k].modeFlags & RR_Interlace){
						vTotal >>= 1;
					}

					ccMalloc(currentResolution.data, sizeof(ccDisplayData_x11));

					((ccDisplayData_x11*)currentResolution.data)->XMode = outputInfo->modes[j];
					currentResolution.refreshRate = resources->modes[k].dotClock / (resources->modes[k].hTotal * vTotal);
					currentResolution.width = resources->modes[k].width;
					currentResolution.height = resources->modes[k].height;

					currentDisplay->amount++;
					if(currentDisplay->amount == 1){
						ccMalloc(currentDisplay->resolution, sizeof(ccDisplayData));
					}else{
						ccRealloc(currentDisplay->resolution, sizeof(ccDisplayData) * currentDisplay->amount);
					}
					memcpy(currentDisplay->resolution + (currentDisplay->amount - 1), &currentResolution, sizeof(ccDisplayData));
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
	char displayName[64];
	DIR *dir;
	struct dirent *direntry;
	Display *display;

	if(CC_UNLIKELY(_ccDisplays != NULL)){
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	ccMalloc(_ccDisplays, sizeof(ccDisplays));
	_ccDisplays->amount = 0;

	dir = opendir("/tmp/.X11-unix");
	if(CC_UNLIKELY(dir == NULL)){
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	while((direntry = readdir(dir)) != NULL){
		if(direntry->d_name[0] != 'X'){
			continue;
		}
		snprintf(displayName, 64, ":%s", direntry->d_name + 1);
		display = XOpenDisplay(displayName);
		if(display != NULL){
			if(CC_UNLIKELY(ccXFindDisplaysXinerama(display, displayName))){
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
	int i,j;

	if(CC_UNLIKELY(_ccDisplays == NULL)){
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	for(i = 0; i < _ccDisplays->amount; i++){
		free(_ccDisplays->display[i].data);
		free(_ccDisplays->display[i].monitorName);
		free(_ccDisplays->display[i].deviceName);

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
	int minX, minY, maxX, maxY;
	ccDisplayData *displayData;
	Display *XDisplay;
	Window root;
	XRRScreenResources *resources;
	XRROutputInfo *outputInfo;
	XRRCrtcInfo *crtcInfo;

	if(CC_UNLIKELY(display == NULL)){
		ccErrorPush(CC_ERROR_DISPLAY_NONE);
		return CC_FAIL;
	}

	if(CC_UNLIKELY(resolutionIndex > display->amount)){
		ccErrorPush(CC_ERROR_INVALID_ARGUMENT);
		return CC_FAIL;
	}

	XDisplay = XOpenDisplay(display->deviceName);
	root = RootWindow(XDisplay, DISPLAY_DATA(display)->XScreen);
	XGrabServer(XDisplay);

	resources = XRRGetScreenResources(XDisplay, root);
	if(CC_UNLIKELY(!resources)){
		ccPrintf("X: Couldn't get screen resources");
		ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
		return CC_FAIL;
	}

	outputInfo = XRRGetOutputInfo(XDisplay, resources, DISPLAY_DATA(display)->XOutput);
	if(CC_UNLIKELY(!outputInfo || outputInfo->connection == RR_Disconnected)){
		ccPrintf("X: Couldn't get output info");
		XRRFreeScreenResources(resources);
		ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
		return CC_FAIL;
	}

	crtcInfo = XRRGetCrtcInfo(XDisplay, resources, outputInfo->crtc);
	if(CC_UNLIKELY(!crtcInfo)){
		ccPrintf("X: Couldn't get crtc info");
		XRRFreeScreenResources(resources);
		XRRFreeOutputInfo(outputInfo);
		ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
		return CC_FAIL;
	}

	if(resolutionIndex != CC_DEFAULT_RESOLUTION){
		displayData = display->resolution + resolutionIndex;
		if(display->resolution->width == displayData->width && display->resolution->height == displayData->height){
			return CC_SUCCESS;
		}

		if(CC_UNLIKELY(display->resolution->width <= 0 || display->resolution->height <= 0)){
			ccPrintf("Error: Resolution supplied not valid\n");
			ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
			return CC_FAIL;
		}

		if(CC_UNLIKELY(!XRRGetScreenSizeRange(XDisplay, root, &minX, &minY, &maxX, &maxY))){
			ccPrintf("X: Unable to get screen size range\n");
			ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
			return CC_FAIL;
		}

		if(CC_UNLIKELY(displayData->width < minX || displayData->height < minY)){
			ccPrintf("X: Unable to set size of screen below the minimum of %dx%d\n", minX, minY);
			ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
			return CC_FAIL;
		} else if(CC_UNLIKELY(displayData->width > maxX || displayData->height > maxY)){
			ccPrintf("X: Unable to set size of screen above the maximum of %dx%d\n", maxX, maxY);
			ccErrorPush(CC_ERROR_DISPLAY_RESOLUTIONCHANGE);
			return CC_FAIL;
		}

		XRRSetCrtcConfig(XDisplay, resources, outputInfo->crtc, CurrentTime, crtcInfo->x, crtcInfo->y, ((ccDisplayData_x11*)displayData->data)->XMode, crtcInfo->rotation, &DISPLAY_DATA(display)->XOutput, 1);
	}else{
		XRRSetCrtcConfig(XDisplay, resources, outputInfo->crtc, CurrentTime, crtcInfo->x, crtcInfo->y, DISPLAY_DATA(display)->XOldMode, crtcInfo->rotation, &DISPLAY_DATA(display)->XOutput, 1);
	}

	XRRFreeScreenResources(resources);
	XRRFreeOutputInfo(outputInfo);
	XRRFreeCrtcInfo(crtcInfo);

	XSync(XDisplay, False);
	XUngrabServer(XDisplay);
	XCloseDisplay(XDisplay);

	return CC_SUCCESS;
}
