#pragma once
#include "libs/logger.h"

typedef enum {
	TOKEN_UNKOWN = -1, TOKEN_ARTIST, TOKEN_TITLE, TOKEN_STATUS, TOKEN_VOLUME, TOKEN_REPEAT, TOKEN_DBUPDATE, TOKEN_RANDOM, TOKEN_TEXT
} TOKEN_TYPE;

typedef enum {
	FORMAT_PLAY, FORMAT_STOP, FORMAT_PAUSE
} FORMAT_TYPE;

typedef struct FormatToken FormatToken;

struct FormatToken {
	TOKEN_TYPE type;
	void* data;
	FormatToken* next;
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

FormatToken* parseTokenString(LOGGER log, const char* input);
