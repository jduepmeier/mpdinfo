#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>

#include <mpd/async.h>
#include <mpd/status.h>
#include <mpd/song.h>
#include <mpd/entity.h>
#include <mpd/search.h>
#include <mpd/tag.h>
#include <mpd/client.h>

#include "parse.h"
#include "debug.h"
#include "format.h"
#include "replace.h"

int REFRESH = 1;
int QUIT = 0;
struct mpd_connection* conn = NULL;

int mpdinfo_connect(struct mpd_connection ** conn) {
	char * mpdhost = getenv("MPDHOST");
	if (mpdhost == NULL) {
		debug("FAIL", "no host is set, using localhost");
		mpdhost = "localhost";
	}
	char* mpdport_string = getenv("MPDPORT");
	
	unsigned long int mpdport = 6600;
	
	if (mpdport_string == NULL) {
		debug("WARNING", "no port is set, using default");
	} else {
		mpdport = strtoul(mpdport_string, NULL, 10);
	}
	*conn = mpd_connection_new(mpdhost, mpdport,0);
	if (*conn == NULL) {
		debug("FAIL", "Out of memory");
		return -1;
	}

	if (mpd_connection_get_error(*conn) != MPD_ERROR_SUCCESS) {
		debug("FAIL", mpd_connection_get_error_message(*conn));
		mpd_connection_free(*conn);
		*conn = NULL;
		return -1;
	}
	return 0;
}

int getStatusStruct(struct mpd_status **mpdstatus) {

	*mpdstatus = mpd_run_status(conn);
	if (!*mpdstatus) {
		debug("FAIL", mpd_connection_get_error_message(conn));
		return 0;
	}
	return 1;
}

int getStatus() {
	struct mpd_status *mpdstatus = NULL;

	if (!getStatusStruct(&mpdstatus)) {
		return -1;
	}
	int status = mpd_status_get_state(mpdstatus);

	mpd_status_free(mpdstatus);

	return status;
}

char* getStatusString() {

	char* output = malloc(8);
	int status = getStatus();

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

char* getTitle() {
	struct mpd_song *song = mpd_run_current_song(conn);

	if (song == NULL) {
		return "";
	}

	const char* tit = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	
	if (tit == NULL) {
		return "";
	}
	
	if (strcmp(tit, "") == 0) {
		return "";
	}

	char* title = malloc(strlen(tit) +1);
	strcpy(title, tit);
	mpd_song_free(song);

	return title;
}

char* getArtist() {

	struct mpd_song *song = mpd_run_current_song(conn);
	
	if (song == NULL) {
		return "";
	}

	const char* art = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);

	if (art == NULL) {
		return "";
	}

	if (strcmp(art,"") == 0) {
		return "";
	}

	char* artist = malloc(strlen(art) +1);
	strcpy(artist, art);
	mpd_song_free(song);
	return artist;
}

int getVolume() {
	struct mpd_status* status = NULL;
	if(!getStatusStruct(&status)) {
		return -1;
	}

	int volume = mpd_status_get_volume(status);
	mpd_status_free(status);
	return volume;
}

char* getVolumeString() {
	int vol = getVolume();
	char* volString = malloc(4);
	sprintf(volString, "%d", vol);
	return volString;
}

void refresh() {
	
	char* out = generateOutputString();
	free(out);
}

void force_refresh() {
	mpd_send_noidle(conn);
}

void* wait_for_action() {

	do {
		refresh();
		mpd_run_idle(conn);
	} while (!QUIT);
	mpd_connection_free(conn);	
	free_token_structs();


	return 0;
}

void quit() {

	QUIT = 1;
	force_refresh();
}

int main(int argc, char** argv) {

	parseArguments(argc, argv);

	int value = mpdinfo_connect(&conn);

	if (value < 0) {
		return -1;
	}

	pthread_t pid = 0;

	pthread_create(&pid, NULL, wait_for_action, NULL);

	signal(SIGHUP, force_refresh);
	signal(SIGQUIT, quit);

	pthread_join(pid, NULL);

	return 0;
}
