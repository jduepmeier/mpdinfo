#include <stdio.h>
#include <stdlib.h>
#include "mpdinfo.h"

int usage(int argc, char** argv, void* c) {
	Config* config = (Config*) c;
	printf("%s utility\n", config->programName);

	//options
	printf("Usage: %s [options] <mode> [mode-args]\n", config->programName);
	printf("Available options:\n");
	printf("    -c  , --config <config>        Path to Config file\n");
	printf("    -f  , --format <string>        Format string for all\n");
	printf("    -fpa, --format=pause <string>  Format string for status paused\n");
	printf("    -fpl, --format=play <string>   Format string for status playing\n");
	printf("    -fs , --format=stop <string>   Format string for status stopped\n");
	printf("    -hp , --help                   Show this help\n");
	printf("    -h  , --host <host>            Set the hostname\n");
	printf("    -p  , --port <port>            Set the port\n");
	printf("    -v  , --verbosity <int>        Set verbosity level between 0 (Error) and 5 (DEBUG)\n");

	//examples
	printf("\nExamples:\n");
	printf("%s --format \"%%artist%% - %%title%%\"    returns the title and artist in format *artist* - *title*\n", config->programName);

	return -1;
}

