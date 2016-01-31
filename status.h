#pragma once
#include "libs/logger.h"
#include "mpdinfo.h"


int getStatusStruct(LOGGER log, Config* config, struct mpd_connection* conn, struct mpd_status** mpdstatus);
int getRepeatStatus(LOGGER log, struct mpd_connection* conn, int status, Config* config);
char* getRepeatString(LOGGER log, struct mpd_connection* conn, int status, Config* config);
int getStatus(LOGGER log, struct mpd_connection* conn, int status, Config* config);
char* getStatusString(LOGGER log, struct mpd_connection* conn, int status, Config* config);
char* getTitle(LOGGER log, struct mpd_connection* conn, int status, Config* config);
char* getArtist(LOGGER log, struct mpd_connection* conn, int status, Config* config);
char* getFilename(LOGGER log, struct mpd_connection* conn, int status, Config* config);
char* getElapsedTime(LOGGER log, struct mpd_connection* conn, int status, Config* config);
int getVolume(LOGGER log, struct mpd_connection* conn, int status, Config* config);
char* getVolumeString(LOGGER log, struct mpd_connection* conn, int status, Config* config);
int getDBUpdateStatus(LOGGER log, struct mpd_connection* conn, int status, Config* config);
char* getDBUpdateString(LOGGER log, struct mpd_connection* conn, int status, Config* config);
int getRandomStatus(LOGGER log, struct mpd_connection* conn, int status, Config* config);
char* getRandomString(LOGGER log, struct mpd_connection* conn, int status, Config* config);
