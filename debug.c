#include <stdio.h>

int DEBUG = 0;

void debug(char* status, const char* message) {

	if (DEBUG) {
		printf("%s: %s\n", status, message);
	}
}

void setDebug(int debug) {
	DEBUG = debug;
}
