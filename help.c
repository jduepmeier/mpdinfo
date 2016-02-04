#include <stdio.h>
#include <stdlib.h>

int usage(int argc, char** argv, void* c) {
	char* fn = "mpdinfo";
	printf("%s utility\n", fn);

	//options
	printf("Usage: %s [options] <mode> [mode-args]\n", fn);
	printf("Available options:\n");
	printf("\t-c   | --config <config>\tPath to Config file\n");
	printf("\t-f   | --format <string>\tFormat string for all\n");
	printf("\t-fpa | --format=pause <string>\tFormat string for status paused\n");
	printf("\t-fpl | --format=play <string>\tFormat string for status playing\n");
	printf("\t-fs  | --format=stop <string>\tFormat string for status stopped\n");
	printf("\t-hp  | --help\t\t\tShow this help\n");
	printf("\t-h   | --host <host>\t\tSet the hostname\n");
	printf("\t-p   | --port <port>\t\tSet the port\n");
	printf("\t-v   | --verbosity <int>\tSet verbosity level between 0 (Error) and 5 (DEBUG)\n");
	
	//examples
	printf("\nExamples:\n");
	printf("%s --format \"%%artist%% - %%title%%\"\t\t returns the title and artist in format *artist* - *title*\n", fn);
	
	return -1;
}

