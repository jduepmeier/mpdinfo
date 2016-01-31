#pragma once

typedef enum {
	TOKEN_UNKOWN = -1, 
	TOKEN_ARTIST,
	TOKEN_TITLE,
	TOKEN_STATUS,
	TOKEN_VOLUME,
	TOKEN_REPEAT,
	TOKEN_DBUPDATE,
	TOKEN_RANDOM,
	TOKEN_TEXT,
	TOKEN_FILENAME,
	TOKEN_ELAPSED_TIME,
	TOKEN_IF,
	TOKEN_IFNOT
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

typedef struct DecisionToken DecisionToken;

struct DecisionToken {
	char* name;
	TOKEN_TYPE type;
	FormatToken* a;
	FormatToken* b;
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
FormatToken* parseTokenString(Config* config, const char* input);
