#pragma once

void parseArguments(int argc, char* argv[]);
char* getMPDHost();
unsigned long int getMPDPort();
void free_connection_info();
