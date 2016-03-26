#if defined CC_USE_ALL || defined CC_USE_TEXT

#include "x11_text.h"

#include <ccore/text.h>

static bool _isactive;

ccTextEvent ccTextEventHandle(XEvent event)
{
	ccTextEvent te = {.type = CC_TEXT_UNHANDLED};
	if(!_isactive){
		return te;
	}

	return te;
}

ccReturn ccTextInputStart()
{
	_isactive = true;

	return CC_SUCCESS;
}

ccReturn ccTextInputStop()
{
	_isactive = false;

	return CC_SUCCESS;
}

ccReturn ccTextInputRect(ccRect rect)
{

}

bool ccTextInputIsActive()
{
	return _isactive;
}

#endif
