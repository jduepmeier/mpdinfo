#pragma once

#include "mpdinfo.h"
#include "libs/logger.h"
#include "format_structs.h"

extern const MPD_TOKEN MPD_FORMAT_TAGS[];

FormatToken* parseTokenString(Config* config, const char* input);
FormatToken* buildTokenStructure(Config* config, const char* input);
void formatControls(const char* format, char* output);
char* generateOutputString(Config* config);
void freeTokenStruct(LOGGER log, FormatToken* token);
void freeTokenConfig(TokenConfig* config);
void freeTokenStructs(Config* config);

