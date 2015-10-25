#include <mpd/status.h>
#include <mpd/song.h>
#include <mpd/client.h>
#include <mpd/tag.h>

#include "debug.h"
#include "mpdinfo.h"
//#include "parse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int getStatusStruct(LOGGER log, struct mpd_connection* conn, struct mpd_status **mpdstatus) {

        *mpdstatus = mpd_run_status(conn);
        if (!*mpdstatus) {
                logprintf(log, LOG_ERROR, "%s\n", mpd_connection_get_error_message(conn));
                return 0;
        }
        return 1;
}

int getStatusByFunc(LOGGER log, struct mpd_connection* conn, void* func) {

	struct mpd_status *mpdstatus = NULL;

	if (!getStatusStruct(log, conn, &mpdstatus)) {
		return -1;
	}
	int (*f)() = func;
	int status = f(mpdstatus);

	mpd_status_free(mpdstatus);

	return status;
}

int getDBUpdateStatus(LOGGER log, struct mpd_connection* conn, int status, Config* config) {

	int update = getStatusByFunc(log, conn, &mpd_status_get_update_id);

	if (update != 0) {
		return 1;
	}
	return 0;
}

int getRandomStatus(LOGGER log, struct mpd_connection* conn, int status, Config* config) {
	return getStatusByFunc(log, conn, &mpd_status_get_random);
}

int getRepeatStatus(LOGGER log, struct mpd_connection* conn, int status, Config* config) {
        return getStatusByFunc(log, conn, &mpd_status_get_repeat);
}

int getStatus(LOGGER log, struct mpd_connection* conn, int status, Config* config) {
        return getStatusByFunc(log, conn, &mpd_status_get_state);
}

char* getTokenStatusString(int status, int playStatus, TokenConfigItem* item) {

	char* output = NULL;

	if (status) {
		switch	(playStatus) {
			case MPD_STATE_STOP:
				output = item->stop;
				break;
			case MPD_STATE_PAUSE:
				output = item->pause;
				break;
			case MPD_STATE_PLAY:
				output = item->play;
				break;
			default:
				output = item->none;
				break;
		}

	} else {
		output = item->off;
	}

	// check for NULL pointer
	char* str_status;
	if (!output) {
		str_status = malloc(1);
		str_status[0] = 0;
	} else {
		str_status = malloc(strlen(output) + 1);
		strcpy(str_status, output);
	}
	return str_status;
}

char* getRepeatString(LOGGER log, struct mpd_connection* conn, int status, Config* config) {
	return getTokenStatusString(getRepeatStatus(log, conn, status, config), status, config->tokens->repeat);
}

char* getRandomString(LOGGER log, struct mpd_connection* conn, int status, Config* config) {
	return getTokenStatusString(getRandomStatus(log, conn, status, config), status, config->tokens->random);
}
char* getDBUpdateString(LOGGER log, struct mpd_connection* conn, int status, Config* config) {
	return getTokenStatusString(getDBUpdateStatus(log, conn, status, config), status, config->tokens->dbupdate);
}

char* getStatusString(LOGGER log, struct mpd_connection* conn, int status, Config* config) {

        char* output = malloc(8);

        switch (status) {

                case MPD_STATE_PLAY:
                        strcpy(output,"playing");
                        return output;
                case MPD_STATE_PAUSE:
                        strcpy(output,"pause");
                        return output;
                case MPD_STATE_STOP:
                        strcpy(output,"stopped");
                        return output;
                default:
                        strcpy(output,"unkown");
                        return output;
        }
}

char* getTitle(LOGGER log, struct mpd_connection* conn, int status, Config* config) {

	struct mpd_song *song = mpd_run_current_song(conn);

	char* title = malloc(1);
	title[0] = 0;

        if (song == NULL) {
                return title;
        }

        const char* tit = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
        
        if (tit == NULL) {
                return title;
        }
        
        if (strcmp(tit, "") == 0) {
                return title;
        }
	free(title);
        title = malloc(strlen(tit) +1);
        strcpy(title, tit);
        mpd_song_free(song);

        return title;
}

char* getArtist(LOGGER log, struct mpd_connection* conn, int status, Config* config) {

        struct mpd_song *song = mpd_run_current_song(conn);
	char* artist = malloc(1);
	artist[0] = 0;

	if (song == NULL) {
		return artist;
	}

	const char* art = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
	
	if (art == NULL) {
		return artist;
	}
        if (strcmp(art,"") == 0) {
		return artist;
	}

	free(artist);
	
	artist = malloc(strlen(art) +1);
        strcpy(artist, art);
        mpd_song_free(song);
	return artist;	
}

int getVolume(LOGGER log, struct mpd_connection* conn, int status, Config* config) {
        struct mpd_status* mstatus = NULL;
	if(!getStatusStruct(log, conn, &mstatus)) {
		return -1;
	}
	int volume = mpd_status_get_volume(mstatus);
        mpd_status_free(mstatus);
        return volume;
}

char* getVolumeString(LOGGER log, struct mpd_connection* conn, int status, Config* config) {
        int vol = getVolume(log, conn, status, config);
        char* volString = malloc(4);
        sprintf(volString, "%d", vol);
        return volString;
}

