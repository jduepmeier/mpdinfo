#include <stdio.h>
#include <stdlib.h>

void printHelp() {
	char* fn = "mpdinfo";
	printf("%s utility\n", fn);

	//options
	printf("Usage: %s [options] <mode> [mode-args]\n", fn);
	printf("Available options:\n");
	printf("\t-d   | --debug\t\tPrint some debug info\n");
	printf("\t-h   | --help\t\tShow this help\n");
	printf("\t-c   | --config <config>\tPath to Config file\n");
	printf("\t-fpl | --format=play <string>\tFormat string for status playing\n");
	printf("\t-fpa | --format=pause <string>\tFormat string for status paused\n");
	printf("\t-fs  | --format=stop <string>\tFormat string for status stopped\n");
	
	//examples
	printf("\nExamples:\n");
	printf("%s --format \"%%artist%% - %%title%%\"\t\t returns the title and artist in format *artist* - *title*\n", fn);
	
	exit(1);    
}

