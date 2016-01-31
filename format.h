#pragma once
#include "format_structure.h"
#include "mpdinfo.h"

FormatToken* buildTokenStructure(Config* config, const char* input);
void formatControls(const char* format, char* output);
char* generateOutputString(Config* config);
void freeTokenStruct(LOGGER log, FormatToken* token);
void freeTokenConfig(TokenConfig* config);
void freeTokenStructs(Config* config);
