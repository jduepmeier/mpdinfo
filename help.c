#include <stdio.h>

void printHelp() {
    char* fn = "mpdinfo";
    printf("%s utility\n", fn);

    //options
    printf("Usage: %s [options] <mode> [mode-args]\n", fn);
    printf("Available options:\n");
    printf("\t-d | --debug\t\tPrint some debug info\n");
    printf("\t-h | --help\t\tShow this help\n");

    //examples
    printf("\nExamples:\n");
    printf("%s --format \"%%artist%% - %%title%%\"\t\t returns the title and artist in format *artist* - *title*\n", fn);
    
}

