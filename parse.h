#pragma once

typedef enum {

        C_GENERAL, C_OUTPUT, C_TOKEN_DBUPDATE, C_TOKEN_REPEAT, C_TOKEN_RANDOM

} Category;
char* getNoneToken(int category);
char* getTokenByStatus(int category);
void parseArguments(int argc, char* argv[]);
char* getMPDHost();
unsigned long int getMPDPort();
void free_connection_info();
