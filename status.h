#pragma once
#include "mpdinfo.h"

int 	getRepeatStatus		(Config* config, int status);
char* 	getRepeatString		(Config* config, int status);
int 	getStatus		(Config* config, int status);
char* 	getStatusString		(Config* config, int status);
char* 	getTitle		(Config* config, int status);
char* 	getArtist		(Config* config, int status);
char* 	getFilename		(Config* config, int status);
char* 	getElapsedTime		(Config* config, int status);
int 	getVolume		(Config* config, int status);
char* 	getVolumeString		(Config* config, int status);
int 	getDBUpdateStatus	(Config* config, int status);
char* 	getDBUpdateString	(Config* config, int status);
int 	getRandomStatus		(Config* config, int status);
char* 	getRandomString		(Config* config, int status);
char* 	getAlbum		(Config* config, int status);
char* 	getAlbumArtist		(Config* config, int status);
char* 	getGenre		(Config* config, int status);
char* 	getTrack		(Config* config, int status);
char* 	getDisc			(Config* config, int status);
char* 	getComment		(Config* config, int status);
char* 	getDate			(Config* config, int status);
char* 	getTimeBar		(Config* config, int status);
char* 	getDuration		(Config* config, int status);
char* 	getQueueLength  (Config* config, int status);
