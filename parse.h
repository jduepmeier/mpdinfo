#pragma once
#include "mpdinfo.h"
#include "format.h"
#include "libs/logger.h"

typedef enum {

        C_GENERAL, C_OUTPUT, C_TOKEN_DBUPDATE, C_TOKEN_REPEAT, C_TOKEN_RANDOM

} Category;

void parseConfigFile(LOGGER log, Config* config, char* path);
void free_tokens();
void freeMPDHost();
char* getNoneToken(int category);
char* getTokenByStatus(int category);
char* getMPDHost();
unsigned long int getMPDPort();
void free_connection_info();
