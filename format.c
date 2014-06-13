#include <string.h>
#include <stdlib.h>

char formatString[500]  = "Current Track (Vol %volume%%): \n -%status%- %artist% - %title%)";

void format(char* format) {

	strcpy(formatString, format);
}

char* getFormatString() {

	return formatString;
}
