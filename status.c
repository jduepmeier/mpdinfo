#include <mpd/status.h>
#include <mpd/song.h>
#include <mpd/client.h>
#include <mpd/tag.h>

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

char* getId3Tag(LOGGER log, struct mpd_connection* conn, int status, Config* config, 
		enum mpd_tag_type tag_type) {
        struct mpd_song *song = mpd_run_current_song(conn);
	char* out = malloc(1);
	out[0] = 0;

	if (song == NULL) {
		mpd_song_free(song);
		return out;
	}

	const char* tag = mpd_song_get_tag(song, tag_type, 0);
	
	if (tag == NULL || !strcmp(tag, "")) {
		mpd_song_free(song);
		return out;
	}
	
	free(out);
	
	out = malloc(strlen(tag) +1);
        strncpy(out, tag, strlen(tag) + 1);
        mpd_song_free(song);
	return out;

}
char* getTitle(LOGGER log, struct mpd_connection* conn, int status, Config* config) {
        return getId3Tag(log, conn, status, config, MPD_TAG_TITLE);
}
char* getArtist(LOGGER log, struct mpd_connection* conn, int status, Config* config) {
	return getId3Tag(log, conn, status, config, MPD_TAG_ARTIST);
}

char* getFilename(LOGGER log, struct mpd_connection* conn, int status, Config* config) {
	struct mpd_song* song = mpd_run_current_song(conn);

	char* out = malloc(1);
	out[0] = 0;

	if (song == NULL) {
		return out;
	}

	const char* uri = mpd_song_get_uri(song);

	if (uri == NULL) {
		return out;
	}

	if (strcmp(uri, "") == 0) {
		return out;
	}

	const char* str = strrchr(uri, '/');

	// check if more then the / is in the string
	if (!str || strlen(str) < 2) {
		str = uri;
	}

	str++;

	free(out);
	out = malloc(strlen(str) + 1);
	strncpy(out, str, strlen(str) + 1);
	mpd_song_free(song);
	
	return out;
}

char* getElapsedTime(LOGGER log, struct mpd_connection* conn, int status, Config* config) {

	char* out = malloc(1);
	out[0] = 0;
	
	struct mpd_status* mstatus = NULL;
	if (!getStatusStruct(log, conn, &mstatus)) {
		return out;
	}

	unsigned time = mpd_status_get_elapsed_time(mstatus);

	unsigned sec;
	unsigned min;

	sec = time % 60;
	min = time / 60;

	free(out);

	int length = snprintf(NULL, 0, "%d:%02d", min, sec) + 1;
	out = malloc(length);

	snprintf(out, length, "%d:%02d", min, sec);

	mpd_status_free(mstatus);

	return out; 
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

