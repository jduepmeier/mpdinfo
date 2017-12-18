#pragma once
#include "format_structs.h"
#include "libs/logger.h"

typedef struct {
	char* host;
	unsigned long int port;
} ConnectionInfo;

typedef struct Config {
	char* programName; // init with argv[0]
	char* configPath;
	char* format;
	char* prefix;
	char* suffix;
	LOGGER log;
	unsigned timebar;
	unsigned update;
	FormatToken* play;
	FormatToken* pause;
	FormatToken* stop;
	FormatToken* none;
	TokenConfig* tokens;
	DecisionToken* decTokens;
	ConnectionInfo* connectionInfo;
	struct mpd_song* curr_song;
	struct mpd_status* mpd_status;
} Config;

