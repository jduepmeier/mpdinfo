#pragma once

#define DEFAULT_FORMAT_STRING "Current Track (Vol %volume%%):\n -%status%-\n %artist% - %title%"
#define DEFAULT_STOPPED_STRING "-stopped-"

typedef enum {
	FORMAT_PLAY, FORMAT_STOP, FORMAT_PAUSE
} FORMAT_TYPE;

typedef struct {

	char* name;
	void* action;

} MPD_TOKEN;


typedef struct FormatToken FormatToken;

struct FormatToken {
	const MPD_TOKEN* type;
	void* data;
	FormatToken* next;
};

typedef struct DecisionToken DecisionToken;

struct DecisionToken {
	char* name;
	const MPD_TOKEN* type;
	FormatToken* condition;
	FormatToken* yes;
	FormatToken* no;
	DecisionToken* next;
};

typedef struct {
	char* play;
	char* pause;
	char* stop;
	char* none;
	char* off;
} TokenConfigItem;

typedef struct {
	TokenConfigItem* repeat;
	TokenConfigItem* random;
	TokenConfigItem* dbupdate;
} TokenConfig;

#ifndef CONFIG_STRUCT
#define CONFIG_STRUCT 1
typedef struct Config Config;
#endif

#define TOKEN_TEXT 0
#define TOKEN_IF 1

#include "status.h"

extern const MPD_TOKEN MPD_FORMAT_TAGS[];

FormatToken* parseTokenString(Config* config, const char* input);
FormatToken* buildTokenStructure(Config* config, const char* input);
void formatControls(const char* format, char* output);
char* generateOutputString(Config* config);
void freeTokenStruct(LOGGER log, FormatToken* token);
void freeTokenConfig(TokenConfig* config);
void freeTokenStructs(Config* config);

