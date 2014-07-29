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
#include "status.h"

int REFRESH = 1;
int QUIT = 0;
struct mpd_connection* conn = NULL;

void getConnection(struct mpd_connection** mpdconn) {
	*mpdconn = conn;
}

int mpdinfo_connect(struct mpd_connection ** conn) {
	/*char * mpdhost = getenv("MPDHOST");
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
	}*/

	char* host = getMPDHost();
	unsigned long int port = getMPDPort();
	*conn = mpd_connection_new(host, port,0);
	if (*conn == NULL) {
		debug("FAIL", "Out of memory");
		return -2;
	}

	if (mpd_connection_get_error(*conn) != MPD_ERROR_SUCCESS) {
		debug("FAIL", mpd_connection_get_error_message(*conn));
		mpd_connection_free(*conn);
		*conn = NULL;
		return -1;
	}
	return 0;
}

int mpdinfo_reconnect() {
	if (conn) {
		mpd_connection_free(conn);
	}
	if (mpdinfo_connect(&conn) == -1) {
		if (QUIT) {
			return 1;
		} else {
			printf("Connection lost, reconnection in 5 seconds.\n\f");
			if (sleep(5) < 0) {
				debug("WARNING", "Sleep interrupted..");
				return 1;
			}
			mpdinfo_reconnect();
		}
	}
	return 0;
}

int refresh() {
	
	char* out = generateOutputString();
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		printf("Connection lost, reconnecting.\n\f");
		if (mpdinfo_reconnect()) {
			return 1;
		}
		refresh();
	} else {
		printf("%s\f\n", out);
		fflush(stdout);
		free(out);
	}
	return 0;
	
}

void force_refresh() {
	debug("DEBUG","forcing refresh");
	mpd_send_noidle(conn);
}

void* wait_for_action() {

	do {
		if (!refresh()) {
			mpd_run_idle(conn);
			 debug("DEBUG", "refresh");

		}
	} while (!QUIT);
	mpd_connection_free(conn);	
	free_token_structs();
	free_connection_info();

	return 0;
}

void quit() {

	QUIT = 1;
	debug("DEBUG", "now quitting");
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
