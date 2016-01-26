#pragma once
#include "format_structure.h"
#include "mpdinfo.h"

FormatToken* buildTokenStructure(LOGGER log, Config* config, const char* input);
void formatControls(const char* format, char* output);
char* generateOutputString(LOGGER log, Config* config, struct mpd_connection* conn);
void freeTokenStruct(LOGGER log, FormatToken* token);
void freeTokenConfig(TokenConfig* config);
void freeTokenStructs(LOGGER log, Config* config);
