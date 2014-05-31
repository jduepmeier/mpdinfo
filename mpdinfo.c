#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#include <mpd/async.h>
#include <mpd/status.h>
#include <mpd/song.h>
#include <mpd/entity.h>
#include <mpd/search.h>
#include <mpd/tag.h>
#include <mpd/client.h>

int DEBUG = 1;
int REFRESH = 1;
int QUIT = 0;
struct mpd_connection* conn = NULL;

void debug(char* status, const char* message) {
	if (DEBUG) {
		printf("%s: %s\n", status, message);
	}
}

int mpdinfo_connect(struct mpd_connection ** conn) {
	char* mpdhost = getenv("MPDHOST");
	if (mpdhost == NULL) {
		return -1;
	}
	char* mpdport_string = getenv("MPDPORT");
	if (mpdport_string == NULL) {
		return -1;
	}
	unsigned long int mpdport = strtoul(mpdport_string, NULL, 10);
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

const char* getTitle() {
	const struct mpd_song *song = mpd_run_current_song(conn);

	if (song == NULL) {
		return "";
	}

	const char* title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	//mpd_song_free(song);

	return title;
}

const char* getArtist() {

	const struct mpd_song *song = mpd_run_current_song(conn);
	
	if (song == NULL) {
		return "";
	}
					
	const char* artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
	//mpd_song_free(song);
							
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
	const char* title = getTitle();
	int volume = getVolume();
	const char* artist = getArtist();

	printf("Status: %s\n", status);
	printf("Volume: %d%%\n", volume);
	printf("Artist: %s\n", artist);
	printf("Title: %s\n", title);
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

int main(int argc, char** argv) {

	int value = mpdinfo_connect(&conn);
	int x = 0;

	if (value < 0) {
		return -1;
	}

	debug("debug","starting");

	pthread_t pid = 0;

	pthread_create(&pid, NULL, wait_for_action, &x);

	debug("debug","threading");

	signal(SIGHUP, force_refresh);

	getchar();

	QUIT = 1;
	force_refresh();

	mpd_connection_free(conn);
	return 0;
}
