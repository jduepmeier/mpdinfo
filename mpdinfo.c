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
	char* mpdhost = getenv("MPDHOST");
	if (mpdhost == NULL) {
		debug("FAIL", "no host is set, using localhost");
		mpdhost = "localhost";
		//return -1;
	}
	char* mpdport_string = getenv("MPDPORT");
	
	unsigned long int mpdport = 6600;
	
	if (mpdport_string == NULL) {
		debug("WARNING", "no port is set, using default");
	} else {
		mpdport = strtoul(mpdport_string, NULL, 10);
	}
	*conn = mpd_connection_new(mpdhost,mpdport, 0);
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

char* getStatus() {

	struct mpd_status *mpdstatus = NULL;
	char* output = "";

	if (!getStatusStruct(&mpdstatus)) {
		return output;
	}
	int status = mpd_status_get_state(mpdstatus);

	if (status == MPD_STATE_PLAY) {
		output = "playing";
	} else if (status == MPD_STATE_PAUSE) {
		output = "pause";
	} else if (status == MPD_STATE_STOP) {
		output = "stopped";
	} else {
		output = "unkown";
	}
	debug("DEBUG", output);
	mpd_status_free(mpdstatus);
	return output;

}

char* getTitle() {
	struct mpd_song *song = mpd_run_current_song(conn);

	if (song == NULL) {
		return "";
	}

	const char* tit = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
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

void refresh() {
	char* status = getStatus();
	char* title = getTitle();
	int volume = getVolume();
	char* artist = getArtist();
	
	char* format = getFormatString();

	debug("DEBUG", format);

	char volumeString[4];

	sprintf(volumeString, "%d", volume);

	format = stringReplace("%artist%", artist, format);
	format = stringReplace("%title%", title, format);
 	format = stringReplace("%status%", status, format);
	format = stringReplace("%volume%", volumeString, format);
	
	printf("%s\f", format);

	//printf("Status: %s\n", status);
	//printf("Volume: %d%%\n", volume);
	//printf("Artist: %s\n", artist);
	//printf("Title: %s\n", title);

	fflush(stdout);

	free(title);
	free(artist);
}

void force_refresh() {
	mpd_send_noidle(conn);
}

void* wait_for_action() {

	do {
		refresh(conn);
		mpd_run_idle(conn);
	} while (!QUIT);

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

	debug("debug","starting\f");
	pthread_t pid = 0;

	pthread_create(&pid, NULL, wait_for_action, NULL);

	debug("debug","threading\f");

	signal(SIGHUP, force_refresh);
	signal(SIGQUIT, quit);

	pthread_join(pid, NULL);

	mpd_connection_free(conn);	

	return 0;
}
