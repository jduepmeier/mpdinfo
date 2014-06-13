#include <string.h>
#include <stdlib.h>

#include "debug.h"

char formatString[500]  = "Current Track (Vol %volume%%):\n -%status%-\n %artist% - %title%";
char formatStoppedString[500] = "-stopped-";

void format(char* format) {

	if (strlen(format) < 500) {
		debug("FAIL", "format string is too long");
	}

	strcpy(formatString, format);
}

void formatStopped(char* format) {

	if (strlen(format) < 500) {
		debug("FAIL", "format string for status stopped is too long");
	}

	strcpy(formatString, format);
}

char* getFormatString() {

	return formatString;
}

char* getFormatStoppedString() {
	return formatStoppedString;
}
