#pragma once
#include <mpd/client.h>
#include <mpd/song.h>

#include "libs/logger.h"

#include "format.h"

typedef struct {
	char* host;
	unsigned long int port;
} ConnectionInfo;


#ifndef CONFIG_STRUCT
#define CONFIG_STRUCT 1

typedef struct Config Config;

#endif

struct Config {
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
};

// in libs/logger.c
void logconfig(LOGGER log, unsigned level, Config* config);
