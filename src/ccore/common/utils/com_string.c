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
	for(i = (int)strlen(str); i >= 0; i--){
		if(str[i] == ch){
			str[i] = newCh;
		}
	}
}

char *ccStringConcatenate(int amount, ...)
{
#if __STDC_VERSION__ >= 199901L
	int lengths[amount * sizeof(int)];
	char *elements[amount * sizeof(char*)];
#else
	int *lengths = alloca(amount * sizeof(int));
	char **elements = alloca(amount * sizeof(char*));
#endif
	
	va_list strings;
	va_start(strings, amount);

	int i, l = 0;
	for(i = 0; i < amount; i++) {
		elements[i] = va_arg(strings, char*);
		lengths[i] = (int)strlen(elements[i]);
		l += lengths[i];
	}
	va_end(strings);

	char *newStr = malloc(l + 1);
	newStr[l] = '\0';
	l = 0;

	for(i = 0; i < amount; i++) {
		memcpy(&newStr[l], elements[i], lengths[i]);
		l += lengths[i];
	}

	return newStr;
}
