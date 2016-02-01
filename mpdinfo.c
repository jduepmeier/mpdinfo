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
int QUIT = 0;
// only for signals
struct mpd_connection* conn = NULL;

// sets mpd host from arguments
int setHost(int argc, char** argv, void* c) {
	Config* config = (Config*) c;
	config->connectionInfo->host = argv[1];

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


// only for signals
void setConnection(struct mpd_connection* c) {
	conn = c;
}

struct mpd_connection* mpdinfo_connect(Config* config) {
	setConnection(NULL);
	
	logprintf(config->log, LOG_INFO, "Trying to connect to: %s:%d\n", config->connectionInfo->host, config->connectionInfo->port);
	struct mpd_connection* conn = mpd_connection_new(config->connectionInfo->host, config->connectionInfo->port, 0);

	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		logprintf(config->log, LOG_ERROR, "%s\n", mpd_connection_get_error_message(conn));
		mpd_connection_free(conn);
		return NULL;
	}
	logprintf(config->log, LOG_INFO, "Connected to %s\n", config->connectionInfo->host);
	
	// needed for signals
	setConnection(conn);
	return conn;
}


// reconnects to mpd
int mpdinfo_reconnect(Config* config, struct mpd_connection * conn) {
	if (conn) {
		mpd_connection_free(conn);
	}
	if ((conn = mpdinfo_connect(config))) {
		// check if status is quit
		if (QUIT) {
			return 1;
		} else {
			// try it later
			logprintf(config->log, LOG_WARNING, "Connection lost, reconnection in 5 seconds.\n\f");
			if (sleep(5) > 0) {
				logprintf(config->log, LOG_WARNING, "Sleep interrupted..");
				return 1;
			}
			// and try it again
			mpdinfo_reconnect(config, conn);
		}
	}
	return 0;
}

// refresh the output
int refresh(Config* config, struct mpd_connection* conn) {
	if (!config) {
		return 1;
	}
	
	logprintf(config->log, LOG_DEBUG, "Starting refresh.\n");	

	if (!conn) {
		logprintf(config->log, LOG_WARNING, "No connection");
		mpdinfo_reconnect(config, conn);
	}

	// cache current mpd status and current song info
	config->curr_song = mpd_run_current_song(conn);
	config->mpd_status = mpd_run_status(conn);

	// check for errors
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		logprintf(config->log, LOG_ERROR, "%s\n", mpd_connection_get_error_message(conn));
		printf("Connection lost, reconnecting.\n\f");
		if (mpdinfo_reconnect(config, conn)) {
			return 1;
		}
		refresh(config, conn);
	} else {
		// generate output
		char* out = generateOutputString(config);
		// and free the cache again
		
		if (config->mpd_status) {
			mpd_status_free(config->mpd_status);
		}
		
		if  (config->curr_song) {
			mpd_song_free(config->curr_song);
		}
	
		// we can print it
		printf("\f%s", out);
		fflush(stdout);
		free(out);
	}	
	
	return 0;
	
}

// force refresh signal function
void force_refresh() {
	if (conn) {
		mpd_send_noidle(conn);
	}
}

// main wait function
void* wait_for_action(Config* config, struct mpd_connection* conn) {

	logprintf(config->log, LOG_INFO, "letsgo :)\n");
	do {
		// refresh output and wait for any change on mpd
		if (!refresh(config, conn)) {
			mpd_run_idle(conn);
			logprintf(config->log, LOG_DEBUG, "refresh");
		}
	} while (!QUIT);

	// cleaning up
	logprintf(config->log, LOG_DEBUG, "Freeing connections.\n");
	mpd_connection_free(conn);
	freeTokenStructs(config);
	freeTokenConfig(config->tokens);
	free(config->connectionInfo->host);
	return 0;
}

// quit signal function
void quit() {

	QUIT = 1;
	force_refresh();
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

	if (!strcmp(key, "name")) {

		DecisionToken* dt = config->decTokens;
		config->decTokens = malloc(sizeof(DecisionToken));
		config->decTokens->next = dt;
		config->decTokens->type = -1;
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

	if (!strcmp(key, "type")) {
		if (!strcmp(value, "IF") || !strcmp(value, "if")) {
			config->decTokens->type = TOKEN_IF;
		} else if (!strcmp(value, "IFNOT") || !strcmp(value, "ifnot")) {
			config->decTokens->type = TOKEN_IFNOT;
		}else {
			logprintf(config->log, LOG_ERROR, "Unkown token type: %s.\n", value);
			return -1;
		}
	} else if (!strcmp(key, "a")) {
		config->decTokens->a = parseTokenString(config, value);
	} else if (!strcmp(key, "b")) {
		config->decTokens->b = parseTokenString(config, value);
	} else {
		logprintf(config->log, LOG_ERROR, "Unkown key %s in decision token parsing.\n", key);
		return -1;
	}

	return 0;
}

int setConfigVerbosity(const char* cat, const char* key, const char* value, EConfig* econfig, void* c) {
	Config* config = (Config*) c;

	config->log.verbosity = strtol(value, NULL, 10);

	return 0;
}

int setConfigLogfile(const char* cat, const char* key, const char* value, EConfig* econfig, void* c) {

	Config* config = (Config*) c;

	config->log.stream = fopen(value, "a");

	if (!config->log.stream) {
		config->log.stream = stderr;
		logprintf(config->log, LOG_ERROR, "Cannot open logfile (%s).\n", strerror(errno));

		return -1;
	}

	return 0;
}

int setTokenParam(const char* cat, const char* key, const char* value, EConfig* econfig, void* c) {
	
	Config* config = (Config*) c;

	TokenConfigItem* item = getTokenConfigItem(cat, config);

	if (!item) {
		logprintf(config->log, LOG_ERROR, "Category (%s) is not a valid token category\n", cat);
		return -1;
	}

	char* val = malloc(strlen(value) + 1);
	//strcpy(val, value);

	formatControls(value, val);

	if (!strcmp(key, "play")) {
		if (item->play) {
			free(item->play);
		}
		item->play = val;
	} else if (!strcmp(key, "pause")) {
		if (item->pause) {
			free(item->pause);
		}
		item->pause = val;
	} else if (!strcmp(key, "stop")) {
		if (item->stop) {
			free(item->stop);
		}
		item->stop = val;
	} else if (!strcmp(key, "none")) {
		if (item->none) {
			free(item->none);
		}
		item->none = val;
	} else if (!strcmp(key, "off")) {
		if (item->off) {
			free(item->off);
		}
		item->off = val;
	} else {
		free(val);
	}

	return 0;
}

// parses the config file from arguments
int setConfigPath(int argc, char** argv, void* c) {
	Config* prg_config = (Config*) c;
	EConfig* config = econfig_init(argv[1], c);
	
	unsigned cats[6];

	cats[0] = econfig_addCategory(config, "general");
	cats[1] = econfig_addCategory(config, "output");
	cats[2] = econfig_addCategory(config, "token_repeat");
	cats[3] = econfig_addCategory(config, "token_random");
	cats[4] = econfig_addCategory(config, "token_dbupdate");
	cats[5] = econfig_addCategory(config, "token_decision");

	econfig_addParam(config, cats[0], "host", setConfigHost);
	econfig_addParam(config, cats[0], "port", setConfigPort);
	econfig_addParam(config, cats[0], "verbosity", setConfigVerbosity);
	econfig_addParam(config, cats[0], "logfile", setConfigLogfile);

	econfig_addParam(config, cats[1], "play", setOutputParam);
	econfig_addParam(config, cats[1], "pause", setOutputParam);
	econfig_addParam(config, cats[1], "none", setOutputParam);
	econfig_addParam(config, cats[1], "stop", setOutputParam);


	int i;
	for (i = 2; i < 5; i++) {
		econfig_addParam(config, cats[i], "play", setTokenParam);
		econfig_addParam(config, cats[i], "pause", setTokenParam);
		econfig_addParam(config, cats[i], "none", setTokenParam);
		econfig_addParam(config, cats[i], "stop", setTokenParam);
		econfig_addParam(config, cats[i], "off", setTokenParam);
	}

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

	eargs_addArgument("-v", "--verbosity", setVerbosity, 1);
	eargs_addArgument("-h", "--help", usage, 0);
	eargs_addArgument("-c", "--config", setConfigPath, 1);
	eargs_addArgument("-h", "--host", setHost, 1);
	eargs_addArgument("-p", "--port", setPort, 1);
	eargs_addArgument("-fpl", "--format=play", setPlayFormat, 1);
	eargs_addArgument("-fpa", "--format=pause", setPauseFormat, 1);
	eargs_addArgument("-fs", "--format=stop", setStopFormat, 1);

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

	if (token->a) {
		freeTokenStruct(config->log, token->a);
	}

	if (token->b) {
		freeTokenStruct(config->log, token->b);
	}

	if (token->name) {
		free(token->name);
	}
	free(token);
}

int main(int argc, char** argv) {

	ConnectionInfo info = {
		.host = NULL,
		.port = 6600
	};

	info.host = malloc(strlen("localhost") + 1);
	strcpy(info.host, "localhost");

	LOGGER log = {
		.stream = stderr,
		.verbosity = 0,
		.print_timestamp = 1
	};
	char* pre_v = getenv("MPDINFO_PRE_VERBOSITY");
	if (pre_v) {
		log.verbosity = strtoul(pre_v, NULL, 10);
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

	signal(SIGINT, quit);
	signal(SIGTERM, quit);

	logconfig(config.log, LOG_DEBUG, &config);
	wait_for_action(&config, conn);
	
	cleanDecisionTokens(&config, config.decTokens);

	if (config.log.stream != stderr) {
		fclose(config.log.stream);
	}
	printf("Bye...\n");
	return 0;
}
