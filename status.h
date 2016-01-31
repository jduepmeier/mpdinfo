#pragma once
#include "libs/logger.h"
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
