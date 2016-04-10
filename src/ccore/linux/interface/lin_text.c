#if defined CC_USE_ALL || defined CC_USE_GAMEPAD

#include "lin_text.h"

#include <ccore/text.h>

ccReturn ccTextInputStart(void)
{
	return CC_SUCCESS;
}

ccReturn ccTextInputStop(void)
{
	return CC_SUCCESS;
}

ccReturn ccTextInputRect(ccRect rect)
{
	return CC_SUCCESS;
}

bool ccTextInputIsActive(void)
{
	return false;
}

ccTextEvent ccTextEventPoll(void)
{
	ccTextEvent event;

	return event;
}


#endif
