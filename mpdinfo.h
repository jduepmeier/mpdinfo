#pragma once
#include <mpd/client.h>
#include <mpd/song.h>

#include "format_structure.h"
#include "libs/logger.h"

typedef struct {
	char* host;
	unsigned long int port;
} ConnectionInfo;


#ifndef CONFIG_STRUCT
#define CONFIG_STRUCT 1

typedef struct Config Config;

#endif

struct Config {
        char* configPath;
        char* format;
	LOGGER log;
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

void logconfig(LOGGER log, unsigned level, Config* config);
char* getArtist();
char* getTitle();
char* getVolumeString();
int getStatus();
char* getStatusString();
int getRepeatStatus();
char* getRepeatString();
void getConnection(struct mpd_connection** conn);
