#pragma once

int getStatusStruct(struct mpd_status **mpdstatus);
int getRepeatStatus();
int getRepeatStatus();
char* getRepeatString();
int getStatus();
char* getStatusString();
char* getTitle();
char* getArtist();
char* getFilename();
char* getElapsedTime();
int getVolume();
char* getVolumeString();
int getDBUpdateStatus();
char* getDBUpdateString();
int getRandomStatus();
char* getRandomString();
