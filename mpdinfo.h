#pragma once

char* getArtist();
char* getTitle();
char* getVolumeString();
int getStatus();
char* getStatusString();
int getRepeatStatus();
char* getRepeatString();
void getConnection(struct mpd_connection** conn);
