#include <stdio.h>
#include <string.h>

#include "format.h"
#include "help.h"

void parseArguments(int argc, char* argv[]) {

    //parse arguments
    int i;
    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            if (i + 1 < argc && (!strcmp(argv[i], "--format") || !strcmp(argv[i], "-f"))) {
                format(argv[i + 1]);
                i++;
            }  else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
                printHelp();
            } else if (!strcmp(argv[i], "--debug") || !strcmp(argv[i], "-d")) {
                //setDebug(1);
            } else {
                printf("Invalid arguments.\n");
                printHelp();
            }
        }
    }
}
