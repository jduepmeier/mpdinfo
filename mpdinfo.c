#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#include <mpd/async.h>
#include <mpd/status.h>
#include <mpd/song.h>
#include <mpd/entity.h>
#include <mpd/search.h>
#include <mpd/tag.h>
#include <mpd/client.h>

#include "libs/easy_args.h"
#include "libs/logger.h"
#include "libs/easy_config.h"

#include "help.h"
#include "format.h"
#include "status.h"
#include "mpdinfo.h"

int REFRESH = 1;
// only for signals
volatile sig_atomic_t QUIT;

// sets mpd host from arguments
int setHost(int argc, char** argv, void* c) {
	Config* config = (Config*) c;

	free(config->connectionInfo->host);
	config->connectionInfo->host = calloc(strlen(argv[1]) + 1, sizeof(char));
	strncpy(config->connectionInfo->host,argv[1], strlen(argv[1]) + 1);

	return 0;
}

// sets mpd port from arguments
int setPort(int argc, char** argv, void*c ) {
	Config* config = (Config*) c;
	config->connectionInfo->port = strtoul(argv[1], NULL, 10);

	return 0;
}

// set verbosity from arguments
int setVerbosity(int argc, char** argv, void* c) {

	Config* config = (Config*) c;
	config->log.verbosity = strtoul(argv[1], NULL, 10);

	return 0;
}

int setFormat(int argc, char** argv, void* c) {
	Config* config = (Config*) c;

	// check if there are empty formats
	if (!config->play) {
		config->play = parseTokenString(config, argv[1]);
	}
	if (!config->pause) {
		config->pause = parseTokenString(config, argv[1]);
	}
	if (!config->stop) {
		config->stop = parseTokenString(config, argv[1]);
	}
	if (!config->none) {
		config->none = parseTokenString(config, argv[1]);
	}

	config->format = argv[1];

	return 0;
}

// sets the pause format string from arguments
int setPauseFormat(int argc, char** argv, void* c) {
	Config* config = (Config*) c;
	config->pause = parseTokenString(config, argv[1]);

	return 0;
}

// sets the play format string from arguments
int setPlayFormat(int argc, char** argv, void* c) {
	Config* config = (Config*) c;
	config->play = parseTokenString(config, argv[1]);

	return 0;
}

// sets the stop format string from arguments
int setStopFormat(int argc, char** argv, void* c) {
	Config* config = (Config*) c;
	config->stop = parseTokenString(config, argv[1]);

	return 0;
}

struct mpd_connection* mpdinfo_connect(Config* config) {

	logprintf(config->log, LOG_INFO, "Trying to connect to: %s:%d\n", config->connectionInfo->host, config->connectionInfo->port);
	struct mpd_connection* conn = mpd_connection_new(config->connectionInfo->host, config->connectionInfo->port, 0);

	enum mpd_error error = mpd_connection_get_error(conn);

	if (error != MPD_ERROR_SUCCESS && error != MPD_ERROR_TIMEOUT) {
		logprintf(config->log, LOG_ERROR, "%s\n", mpd_connection_get_error_message(conn));
		mpd_connection_free(conn);
		return NULL;
	}
	logprintf(config->log, LOG_INFO, "Connected to %s\n", config->connectionInfo->host);
	return conn;
}


// reconnects to mpd
struct mpd_connection* mpdinfo_reconnect(Config* config) {
	struct mpd_connection* conn = NULL;

	while (!conn) {
		// check if status is quit
		if (QUIT) {
			logprintf(config->log, LOG_INFO, "Quitting.\n");
			return NULL;
		} else {
			// try it later
			logprintf(config->log, LOG_WARNING, "Connection lost, reconnection in 5 seconds.\n\f");
			if (sleep(5) > 0) {
				logprintf(config->log, LOG_WARNING, "Sleep interrupted..");
				return NULL;
			}
			conn = mpdinfo_connect(config);
		}
	}
	return conn;
}

// refresh the output
struct mpd_connection* refresh(Config* config, struct mpd_connection* conn) {
	if (!config) {
		return NULL;
	}

	logprintf(config->log, LOG_DEBUG, "Starting refresh.\n");

	if (!conn) {
		logprintf(config->log, LOG_WARNING, "No connection");


		conn = mpdinfo_reconnect(config);

		if (!conn) {
			return NULL;
		}
	}

	// cache current mpd status and current song info
	config->curr_song = mpd_run_current_song(conn);
	config->mpd_status = mpd_run_status(conn);

	// check for errors
	if (config->mpd_status) {
		// generate output
		char* out = generateOutputString(config);
		// and free the cache again

		mpd_status_free(config->mpd_status);
		if (config->curr_song) {
			mpd_song_free(config->curr_song);
		}

		// we can print it
		printf("%s", out);
		fflush(stdout);
		free(out);
	} else {
		logprintf(config->log, LOG_ERROR, "%s\n", mpd_connection_get_error_message(conn));
		printf("Connection lost, reconnecting.\n\f");
		mpd_connection_free(conn);
		conn = mpdinfo_reconnect(config);
		if (!conn) {
			return NULL;
		}
		return refresh(config, conn);
	}

	return conn;
}

int run_select(Config* config, struct mpd_connection* conn) {
	struct timeval tv;
	fd_set readfds;

	int conn_fd = mpd_connection_get_fd(conn);

	tv.tv_sec = config->update;
	tv.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(conn_fd, &readfds);

	return select(conn_fd + 1, &readfds, NULL, NULL, &tv);
}

// main wait function
void* wait_for_action(Config* config, struct mpd_connection* conn) {

	logprintf(config->log, LOG_INFO, "letsgo :)\n");

	do {
		// refresh output and wait for any change on mpd
		conn = refresh(config, conn);
		if (conn) {
			if (config->update > 0) {
				mpd_send_idle(conn);

				int error = run_select(config, conn);
				if (error < 0) {
					if (errno == EINTR) {
						logprintf(config->log, LOG_INFO, "%s\n", strerror(errno));
						continue;
					} else {
						logprintf(config->log, LOG_ERROR, "%s\n" , strerror(errno));
						break;
					}
				} else if (error == 0) {
					mpd_send_noidle(conn);
				}
				mpd_recv_idle(conn, true);
			} else {
				mpd_run_idle(conn);
			}

			logprintf(config->log, LOG_DEBUG, "refresh");
		}
	} while (!QUIT);

	// cleaning up
	logprintf(config->log, LOG_DEBUG, "Freeing connections.\n");
	if (conn) {
		mpd_connection_free(conn);
	}
	freeTokenStructs(config);
	freeTokenConfig(config->tokens);
	free(config->connectionInfo->host);
	return 0;
}

// quit signal function
void quit() {
	QUIT = 1;
}

// set mpd host from config file
int setConfigHost(const char* category, char* key, char* value, EConfig* econfig, void* c) {	
	Config* config = (Config*) c;

	logprintf(config->log, LOG_DEBUG, "Set config host");

	if (!config->connectionInfo) {
		return -1;
	}

	if (config->connectionInfo->host) {
		free(config->connectionInfo->host);
	}

	config->connectionInfo->host = malloc(strlen(value) +1);
	strcpy(config->connectionInfo->host, value);

	return 0;
}

// set mpd port from config file
int setConfigPort(const char* cat, char* key, char* value, EConfig* econfig, void* c) {
	Config* config = (Config*) c;

	if (config->connectionInfo) {
		config->connectionInfo->port = strtoul(value, NULL, 10);
	}

	return 0;
}

// get the token config item from the category
TokenConfigItem* getTokenConfigItem(const char* cat, Config* config) {

	if (!strcmp(cat, "token_repeat")) {
		return config->tokens->repeat;
	} else if (!strcmp(cat, "token_random")) {
		return config->tokens->random;
	} else if (!strcmp(cat, "token_dbupdate")) {
		return config->tokens->dbupdate;
	} else {
		return NULL;
	}
}

int setOutputParam(const char* cat, const char* key, const char* value, EConfig* econfig, void* c) {
	Config* config = (Config*) c;

	FormatToken* token = parseTokenString(config, value);

	if (!token) {
		return -1;
	}

	if (!strcmp(key, "pause")) {
		config->pause = token;
	} else if (!strcmp(key, "play")) {
		config->play = token;
	} else if (!strcmp(key, "stop")) {
		config->stop = token;
	} else {
		// save in none
		config->none = token;
	}

	return 0;
}

int setDecisionParam(const char* cat, const char* key, const char* value, EConfig* econfig, void* c) {

	Config* config = (Config*) c;

	// every decision token needs a name
	if (!strcmp(key, "name")) {

		DecisionToken* dt = config->decTokens;
		config->decTokens = malloc(sizeof(DecisionToken));
		config->decTokens->next = dt;
		config->decTokens->type = NULL;
		config->decTokens->name = malloc(strlen(value) + 1);
		strncpy(config->decTokens->name, value, strlen(value) + 1);

		config->decTokens->a = NULL;
		config->decTokens->b = NULL;

		return 0;
	}

	if (!config->decTokens) {
		logprintf(config->log, LOG_ERROR, "Name must be the first entry in decision token.\n");
		return -1;
	}

	// check type
	if (!strcmp(key, "type")) {
		if (!strcmp(value, "IF") || !strcmp(value, "if")) {
			config->decTokens->type = &MPD_FORMAT_TAGS[TOKEN_IF];
		} else if (!strcmp(value, "IFNOT") || !strcmp(value, "ifnot")) {
			config->decTokens->type = &MPD_FORMAT_TAGS[TOKEN_IF_NOT];
		}else {
			logprintf(config->log, LOG_ERROR, "Unkown token type: %s.\n", value);
			return -1;
		}
	// string for checking
	} else if (!strcmp(key, "a")) {
		config->decTokens->a = parseTokenString(config, value);
	// output string
	} else if (!strcmp(key, "b")) {
		config->decTokens->b = parseTokenString(config, value);
	} else {
		logprintf(config->log, LOG_ERROR, "Unkown key %s in decision token parsing.\n", key);
		return -1;
	}

	return 0;
}

// Set verbosity from config file
int setConfigVerbosity(const char* cat, const char* key, const char* value, EConfig* econfig, void* c) {
	Config* config = (Config*) c;

	config->log.verbosity = strtol(value, NULL, 10);

	return 0;
}

// Set logfile from config file
int setConfigLogfile(const char* cat, const char* key, const char* value, EConfig* econfig, void* c) {

	Config* config = (Config*) c;

	config->log.stream = fopen(value, "a");

	// can we open log file?
	if (!config->log.stream) {
		config->log.stream = stderr;
		logprintf(config->log, LOG_ERROR, "Cannot open logfile (%s).\n", strerror(errno));

		return -1;
	}

	return 0;
}

// Sets a token parameter from config file
int setTokenParam(const char* cat, const char* key, const char* value, EConfig* econfig, void* c) {

	Config* config = (Config*) c;

	TokenConfigItem* item = getTokenConfigItem(cat, config);

	if (!item) {
		logprintf(config->log, LOG_ERROR, "Category (%s) is not a valid token category\n", cat);
		return -1;
	}

	char* val = malloc(strlen(value) + 1);

	// replace special chars
	formatControls(value, val);

	if (!strcmp(key, "play")) {
		free(item->play);
		item->play = val;
	} else if (!strcmp(key, "pause")) {
		free(item->pause);
		item->pause = val;
	} else if (!strcmp(key, "stop")) {
		free(item->stop);
		item->stop = val;
	} else if (!strcmp(key, "none")) {
		free(item->none);
		item->none = val;
	} else if (!strcmp(key, "off")) {
		free(item->off);
		item->off = val;
	} else {
		logprintf(config->log, LOG_ERROR, "Not a valid key (%s) in token param parsing.\n", key);
		free(val);
		return -1;
	}

	return 0;
}


int setConfigUpdateInterval(const char* cat, const char* key, const char* value, EConfig* econfig, void* c) {

	Config* config = (Config*) c;

	config->update = strtoul(value, NULL, 10);

	return 0;
}

int setConfigTimeBar(const char* cat, const char* key, const char* value, EConfig* econfig, void* c) {

	Config* config = (Config*) c;

	config->timebar = strtoul(value, NULL, 10);

	if (!config->timebar) {
		logprintf(config->log, LOG_ERROR, "Timebar value must an integer higher than zero (%s).\n", value);
		return -1;
	}


	return 0;
}

// parses the config file from arguments
int setConfigPath(int argc, char** argv, void* c) {
	Config* prg_config = (Config*) c;
	EConfig* config = econfig_init(argv[1], c);

	unsigned cats[6];

	// Setup valid categories
	cats[0] = econfig_addCategory(config, "general");
	cats[1] = econfig_addCategory(config, "output");
	cats[2] = econfig_addCategory(config, "token_repeat");
	cats[3] = econfig_addCategory(config, "token_random");
	cats[4] = econfig_addCategory(config, "token_dbupdate");
	cats[5] = econfig_addCategory(config, "token_decision");

	// Setup valid parameters to the categories
	econfig_addParam(config, cats[0], "host", setConfigHost);
	econfig_addParam(config, cats[0], "port", setConfigPort);
	econfig_addParam(config, cats[0], "verbosity", setConfigVerbosity);
	econfig_addParam(config, cats[0], "logfile", setConfigLogfile);
	econfig_addParam(config, cats[0], "timebar", setConfigTimeBar);
	econfig_addParam(config, cats[0], "update", setConfigUpdateInterval);


	econfig_addParam(config, cats[1], "play", setOutputParam);
	econfig_addParam(config, cats[1], "pause", setOutputParam);
	econfig_addParam(config, cats[1], "none", setOutputParam);
	econfig_addParam(config, cats[1], "stop", setOutputParam);


	// Tokens have the same parameters
	int i;
	for (i = 2; i < 5; i++) {
		econfig_addParam(config, cats[i], "play", setTokenParam);
		econfig_addParam(config, cats[i], "pause", setTokenParam);
		econfig_addParam(config, cats[i], "none", setTokenParam);
		econfig_addParam(config, cats[i], "stop", setTokenParam);
		econfig_addParam(config, cats[i], "off", setTokenParam);
	}

	// Setup decision token parameter
	econfig_addParam(config, cats[5], "name", setDecisionParam);
	econfig_addParam(config, cats[5], "type", setDecisionParam);
	econfig_addParam(config, cats[5], "a", setDecisionParam);
	econfig_addParam(config, cats[5], "b", setDecisionParam);

	logprintf(prg_config->log, LOG_DEBUG, "Starting to parse config file.\n");
	int out = econfig_parse(config);
	logprintf(prg_config->log, LOG_DEBUG, "Finished parsing config file.\nOutput status is %d\n", out);
	econfig_free(config);
	if (out < 0) {
		return -1;
	} else {
		return 0;
	}
}

// add arguments to the easy_args parser
int addArguments() {

	// Setup command line arguments
	eargs_addArgument("-c", "--config", setConfigPath, 1);
	eargs_addArgument("-f", "--format", setFormat, 1);	
	eargs_addArgument("-fpa", "--format=pause", setPauseFormat, 1);
	eargs_addArgument("-fpl", "--format=play", setPlayFormat, 1);
	eargs_addArgument("-fs", "--format=stop", setStopFormat, 1);
	eargs_addArgument("-h", "--host", setHost, 1);
	eargs_addArgument("-hp", "--help", usage, 0);
	eargs_addArgument("-p", "--port", setPort, 1);
	eargs_addArgument("-v", "--verbosity", setVerbosity, 1);

	return 0;
}

TokenConfigItem nullTokenConfigItem() {

	return (TokenConfigItem) {
		.play = NULL,
		.pause = NULL,
		.stop = NULL,
		.none = NULL,
		.off = NULL
	};
}

void cleanDecisionTokens(Config* config, DecisionToken* token) {

	if (!token) {
		return;
	}

	cleanDecisionTokens(config, token->next);

	freeTokenStruct(config->log, token->a);
	freeTokenStruct(config->log, token->b);
	free(token->name);

	free(token);
}

int main(int argc, char** argv) {

	ConnectionInfo info = {
		.host = NULL,
		.port = 6600
	};

	LOGGER log = {
		.stream = stderr,
		.verbosity = 0,
		.print_timestamp = 1
	};
	char* pre_v = getenv("MPDINFO_PRE_VERBOSITY");
	if (pre_v) {
		log.verbosity = strtoul(pre_v, NULL, 10);
	}

	char* env_host = getenv("MPD_HOST");

	if (!env_host) {
		info.host = strdup("localhost");
	} else {
		info.host = malloc(strlen(env_host) + 1);
		strncpy(info.host, env_host, strlen(env_host) + 1);
		logprintf(log, LOG_INFO, "Using env variable host: %s\n", info.host);
	}

	char* env_port = getenv("MPD_PORT");
	if (env_port) {
		info.port = strtoul(env_port, NULL, 10);
		logprintf(log, LOG_INFO, "Using env variable port: %d\n", info.port);
	}

	logprintf(log, LOG_DEBUG, "Finished connection info. Host=%s, Port=%d\n", info.host, info.port);

	TokenConfigItem repeat = nullTokenConfigItem();
	TokenConfigItem random = nullTokenConfigItem();
	TokenConfigItem dbupdate = nullTokenConfigItem();

	TokenConfig tokens = {
		.repeat = &repeat,
		.random = &random,
		.dbupdate = &dbupdate
	};

	Config config = {
		.log = log,
		.configPath = "",
		.format = "",
		.timebar = 10,
		.update = 0,
		.play = NULL,
		.pause = NULL,
		.stop = NULL,
		.none = NULL,
		.tokens = &tokens,
		.decTokens = NULL,
		.connectionInfo = &info,
		.curr_song = NULL,
		.mpd_status = NULL
	};

	addArguments();
	char* output[argc];
	if (eargs_parse(argc, argv, output, &config) < 0) {
		freeTokenStructs(&config);
		free(config.connectionInfo->host);
		return 1;
	}
	struct mpd_connection* conn;
	if (!(conn = mpdinfo_connect(&config))) {
		logprintf(config.log, LOG_DEBUG, "Freeing structs.\n");
		cleanDecisionTokens(&config, config.decTokens);
		freeTokenStructs(&config);
		free(config.connectionInfo->host);
		return -1;
	}

	struct sigaction act = {
		.sa_handler = &quit
	};

	if (sigaction(SIGTERM, &act, NULL) < 0 || sigaction(SIGINT, &act, NULL) < 0) {
		logprintf(log, LOG_ERROR, "Failed to set signal mask\n");
		return -1;
	}

	act.sa_handler = SIG_IGN;

	if (sigaction(SIGHUP, &act, NULL) < 0 || sigaction(SIGUSR1, &act, NULL) < 0) {
		logprintf(log, LOG_ERROR, "Failed to set signal mask\n");
		return -1;
	}

	logconfig(config.log, LOG_DEBUG, &config);
	wait_for_action(&config, conn);

	cleanDecisionTokens(&config, config.decTokens);

	if (config.log.stream != stderr) {
		fclose(config.log.stream);
	}
	printf("Bye...\n");
	return 0;
}
