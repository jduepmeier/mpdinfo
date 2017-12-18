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

typedef struct FormatToken {
	const MPD_TOKEN* type;
	void* data;
	struct FormatToken* next;
} FormatToken;

typedef struct DecisionToken {
	char* name;
	const MPD_TOKEN* type;
	FormatToken* condition;
	FormatToken* yes;
	FormatToken* no;
	struct DecisionToken* next;
} DecisionToken;

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

#define TOKEN_TEXT 0
#define TOKEN_IF 1

extern const MPD_TOKEN MPD_FORMAT_TAGS[];
