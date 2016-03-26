#include <ccore/string.h>

#include <string.h>

void ccStringTrimToChar(char* str, char ch, bool includeChar)
{
	int i;
	for(i = (int)strlen(str); str[i] != ch; i--);
	str[i + includeChar] = 0;
}

void ccStringReplaceChar(char *str, char ch, char newCh)
{
	int i;
	for(i = (int)strlen(str); i >= 0; i--)
	{
		if(str[i] == ch) str[i] = newCh;
	}
}

char *ccStringConcatenate(int amount, ...)
{
	va_list strings;
	int i;
	int l = 0;
	int *lengths;
	char **elements;
	char *newStr;

	lengths = malloc(amount*sizeof(int));
	elements = malloc(amount*sizeof(char*));
	
	va_start(strings, amount);
	for(i = 0; i < amount; i++) {
		elements[i] = va_arg(strings, char*);
		lengths[i] = (int)strlen(elements[i]);
		l += lengths[i];
	}
	va_end(strings);

	newStr = malloc(l+1);
	newStr[l] = '\0';
	l = 0;

	for(i = 0; i < amount; i++) {
		memcpy(&newStr[l], elements[i], lengths[i]);
		l += lengths[i];
	}

	free(lengths);
	free(elements);
	return newStr;
}
