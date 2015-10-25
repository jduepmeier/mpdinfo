#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#include "format.h"
#include "help.h"
#include "debug.h"
#include "parse.h"
#include "libs/logger.h"

#include "mpd/status.h"
#include "status.h"
#include "mpdinfo.h"

#define CAT_OUTPUT "[output]"
#define CAT_GENERAL "[general]"
#define CAT_TOKEN_DBUPDATE "[token_dbupdate]"
#define CAT_TOKEN_REPEAT "[token_repeat]"
#define CAT_TOKEN_RANDOM "[token_random]"

#define CONFIG_HOST "host"
#define CONFIG_PORT "port"

#define CONFIG_PLAY "play"
#define CONFIG_PAUSE "pause"
#define CONFIG_STOP "stop"
#define CONFIG_NONE "none"
#define CONFIG_LOGFILE "logfile"

typedef struct {

	char* key;
	char* value;

} ConfigLine;

typedef struct {

	char* stop;
	char* pause;
	char* play;
	char* none;

} TokenStruct;

struct {

	TokenStruct random;
	TokenStruct repeat;
	TokenStruct dbupdate;

} tokens;

void fillToken(TokenStruct* token, char* none, char* rest) {
	
	token->stop = malloc(strlen(rest) + 1);
	token->pause = malloc(strlen(rest) + 1);
	token->play = malloc(strlen(rest) + 1);
	token->none = malloc(strlen(none) + 1);
	
	strcpy (token->stop, rest);
	strcpy (token->pause, rest);
	strcpy (token->play, rest);
	strcpy (token->none, none);
}

void initTokens() {

	//tokens.random = //(TokenStruct) { .stop = "[Random]", .pause = "[Random]", .play = "[Random]", .none = ""};
	//tokens.repeat = malloc(sizeof(TokenStruct));// (TokenStruct) { .stop = "[Repeat]", .pause = "[Repeat]", .play = "[Repeat]", .none = ""};
	//tokens.dbupdate = malloc(sizeof(TokenStruct));// (TokenStruct) { .stop = "[Update]", .pause = "[Update]", .play = "[Update]", .none = ""};

	fillToken(&tokens.random, "", "[Random]");
	fillToken(&tokens.repeat, "", "[Repeat]");
	fillToken(&tokens.dbupdate, "", "[Update]");
}

void free_token(TokenStruct* token) {
	free(token->stop);
	free(token->pause);
	free(token->play);
	free(token->none);

//	free(token);
}

void free_tokens() {
	free_token(&tokens.random);
	free_token(&tokens.repeat);
	free_token(&tokens.dbupdate);
}

char* cropSpacesAndTabs(char* line) {

	while (line[0] != '\n' && line[0] != '\0') {

		if (line[0] != '\t' && line[0] != ' ') {
			return line;
		} else {
			line++;
		}
	}
	return line;
}

void deleteQMs(char* input, char* output) {

	int count = 0;

	if (input[0] == '"') {
		count++;
	}

	int i = 0;
	int len = strlen(input);
	while (count < len - 1) {
		output[i] = input[count];
		i++;
		count++;
	}
	if (input[len - 1] != '"') {
		output[i] = input[len - 1];
		i++;
	}
	output[i] = '\0';

}

char* getNoneToken(int category) {

	switch (category) {
		case C_TOKEN_REPEAT:
			return tokens.repeat.none;
		case C_TOKEN_RANDOM:
			return tokens.random.none;
		case C_TOKEN_DBUPDATE:
			return tokens.dbupdate.none;
		default:
			return "";
	}

}

char* getTokenByStatus(int category) {
        int status = getStatus();
	TokenStruct ts;

	switch (category) {
		case C_TOKEN_REPEAT:
			ts = tokens.repeat;
			break;
		case C_TOKEN_RANDOM:
			ts = tokens.random;
			break;
		case C_TOKEN_DBUPDATE:
			ts = tokens.dbupdate;
			break;
		default:
			return "";
	}

        switch (status) {

                case MPD_STATE_PLAY:
                        return ts.play;
                case MPD_STATE_PAUSE:
                        return ts.pause;
                case MPD_STATE_STOP:
                        return ts.stop;
                default:
                        return ts.none;
       }
}



Category parseCategory(LOGGER log, Config* config, char* cat) {

	if (!strncmp(cat, CAT_OUTPUT, strlen(CAT_OUTPUT))) {
		logprintf(log, LOG_DEBUG, "Entering output category.\n");
		return C_OUTPUT;
	} else if (!strncmp(cat, CAT_TOKEN_DBUPDATE, strlen(CAT_TOKEN_DBUPDATE))) { 
		logprintf(log, LOG_DEBUG, "Entering dbupdate category.\n");
		return C_TOKEN_DBUPDATE;
	} else if (!strncmp(cat, CAT_TOKEN_REPEAT, strlen(CAT_TOKEN_REPEAT))) {
		logprintf(log, LOG_DEBUG, "Entering repeat category.\n");
		return C_TOKEN_REPEAT;
	} else if (!strncmp(cat, CAT_TOKEN_RANDOM, strlen(CAT_TOKEN_RANDOM))) { 
		logprintf(log, LOG_DEBUG, "Entering random category.\n");
		return C_TOKEN_RANDOM;
	} else {
		logprintf(log, LOG_WARNING, "Category is general or not matched. Switching to general.\n");
		return C_GENERAL;
	}

}

void setLogFile(LOGGER log, Config* config, char* logFile) {

	if (log.stream != stderr) {
		fclose(log.stream);
	}
	logprintf(log, LOG_DEBUG, "Switching logging to file (%s)\n", logFile);
	log.stream = fopen(logFile, "w");

}

void setMPDHost(LOGGER log, Config* config, char* host) {
	if (config->connectionInfo->host) {
		free(config->connectionInfo->host);
	}
	char* qm = malloc(strlen(host) + 1);
	deleteQMs(host, qm);
	logprintf(log, LOG_DEBUG, "Switching to host '%s'\n", qm);
	config->connectionInfo->host = qm;
}
char* getMPDHost(Config* config) {
	return config->connectionInfo->host;
}

unsigned long int getMPDPort(Config* config) {
	return config->connectionInfo->port;
}

void setMPDPort(LOGGER log, Config* config, char* port) {
	config->connectionInfo->port = strtoul(port, NULL, 10);

	if (config->connectionInfo->port == 0) {
		logprintf(log, LOG_WARNING,"port not correct, using default port 6600\n");
		config->connectionInfo->port = 6600;
	}
}

void parseConfigLineToken(LOGGER log, Config* config, ConfigLine* cl, TokenStruct* tk) {

	char output2[strlen(cl->value) +1];
	memset(output2, 0, strlen(cl->value + 1));
	formatControls(cl->value, output2);
	char output[strlen(output2) + 1];
	memset(output, 0, strlen(output2) + 1);
	deleteQMs(output2,output);

	if (!strncmp(cl->key, CONFIG_PLAY, strlen(CONFIG_PLAY))) {
		free(tk->play);
		tk->play = malloc(strlen(output) + 1);
		strcpy(tk->play,output);
	} else if (!strncmp(cl->key, CONFIG_PAUSE, strlen(CONFIG_PAUSE))) {
		free(tk->pause);
		tk->pause = malloc(strlen(output) + 1);
		strcpy(tk->pause,output);
	} else if (!strncmp(cl->key, CONFIG_STOP, strlen(CONFIG_STOP))) {
		free(tk->stop);
		tk->stop = malloc(strlen(output) + 1);
		strcpy(tk->stop,output);
	} else if (!strncmp(cl->key, CONFIG_NONE, strlen(CONFIG_NONE))) {
		free(tk->none);
		tk->none = malloc(strlen(output) + 1);
		strcpy(tk->none,output);
	}
}

void parseConfigLineOutput(LOGGER log, Config* config, ConfigLine* cl) {
	//logprintf(log, LOG_DEBUG, "Error in output category parsing.\n");

	char* output = malloc(strlen(cl->value) + 1);
	deleteQMs(cl->value, output);

	if (!strncmp(cl->key, CONFIG_PLAY, strlen(CONFIG_PLAY))) {
		logprintf(log, LOG_DEBUG, "Parsing play output string.\n");
		free_token_struct(log, config->playFormat);
		config->playFormat = parseTokenString(log, output);
	} else if (!strncmp(cl->key, CONFIG_PAUSE, strlen(CONFIG_PAUSE))) {
		logprintf(log, LOG_DEBUG, "Parsing pause output string.\n");
		free_token_struct(log, config->pauseFormat);
		config->pauseFormat = parseTokenString(log, output);
	} else if (!strncmp(cl->key, CONFIG_STOP, strlen(CONFIG_STOP))) {
		logprintf(log, LOG_DEBUG, "Parsing stop output string.\n");
		free_token_struct(log, config->stopFormat);
		config->stopFormat = parseTokenString(log, output);
	} else {
		logprintf(log, LOG_WARNING, "Unkown key in output category (%s).\n", cl->key);	
	}

	free(output);
}

void parseConfigLineGeneral(LOGGER log, Config* config, ConfigLine* cl) {


	if (!strncmp(cl->key, CONFIG_HOST, strlen(CONFIG_HOST))) {
		setMPDHost(log, config, cl->value);
		return;
	}
	if (!strncmp(cl->key, CONFIG_PORT, strlen(CONFIG_PORT))) {
		setMPDPort(log, config, cl->value);
		return;
	}

	if (!strncmp(cl->key, CONFIG_LOGFILE, strlen(CONFIG_LOGFILE))) {
		setLogFile(log, config, cl->value);
		return;
	}
}

char* cropSpacesAndTabsB(char* s) {

	int i = strlen(s) -1;
	while (i > 0 && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n')) {
		s[i] = '\0';
		i--;
	}

	return s;
}

char* strncpyN(char* dest, char* src, int size) {
	
	if (strlen(src) < size) {
		dest = malloc(strlen(src) + 1);
		strcpy(dest, src);
		return dest;
	}
	
	dest = malloc(size + 1);

	int i = 0;

	while (i < size) {
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return dest;

}

void gettingLineArgs(LOGGER log, Config* config, ConfigLine* cl, char* line) {
	line = cropSpacesAndTabs(line);
	int i = 0;

	while (line[i] != '=') {
		if (line[i] == '\n') {
			logprintf(log, LOG_ERROR, "Error parsing line: %s\n", line);
			return;
		}
		i++;
	}

	line[i] = '\0';
	
	cl->key = line;
	cl->value = (line + i + 1);

	cl->key = cropSpacesAndTabsB(cl->key);

	cl->value = cropSpacesAndTabs(cl->value);
	cl->value = cropSpacesAndTabsB(cl->value);
	//malloc(sizeof(ConfigLine));

	
	//cl->key = strncpyN(cl->key, line, i);

	//cl->key = cropSpacesAndTabsB(cl->key);

	//line += i + 1;

	//line = cropSpacesAndTabs(line);
	//cl->value = malloc(strlen(line) + 1);
	//strcpy(cl->value, line);
	//cl->value = cropSpacesAndTabsB(cl->value);

	return;
}



void parseConfigLine(LOGGER log, Config* config, Category cat, char* line) {

	ConfigLine cl = {
		.key = "",
		.value = ""
	};

	gettingLineArgs(log, config, &cl, line);

	if (!strcmp(cl.key, "")) {
		
		logprintf(log, LOG_ERROR, "lineargs not correct\n");
		return;
	}

	switch (cat) {
	
		case C_OUTPUT:
			parseConfigLineOutput(log, config, &cl);
			break;
		case C_TOKEN_RANDOM:
			parseConfigLineToken(log, config, &cl, &tokens.random);
			break;
		case C_TOKEN_REPEAT:
			parseConfigLineToken(log, config, &cl, &tokens.repeat);
			break;
		case C_TOKEN_DBUPDATE:
			parseConfigLineToken(log, config, &cl, &tokens.dbupdate);
			break;
		case C_GENERAL:
		default:
			parseConfigLineGeneral(log, config, &cl);
			break;
	}
}


void parseConfigFile(LOGGER log, Config* config, char* path) {

	logprintf(log, LOG_DEBUG, "starting config reader\n");

	FILE* file = fopen(path, "r");

	if (file == NULL) {

		logprintf(log, LOG_ERROR, "Cannot read config file.\n");
		return;
	}

	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	Category category = C_GENERAL;

	while ( (read = getline(&line, &len, file)) != -1) {
		char* free_line = line;
		logprintf(log, LOG_DEBUG, "Current line: %s\n", line);
		
		line = cropSpacesAndTabs(line);
		if (line[0] != '#' && line[0] != '\n') {
			
			if (line[0] == '[') {
				logprintf(log, LOG_DEBUG, "Found category (%s)\n", line);
				category = parseCategory(log, config, line);
			} else {
				parseConfigLine(log, config, category, line);
			}
		}
		free(free_line);
		line = NULL;
		len = 0;

	}

	if (line != NULL) {
		free(line);
	}

	fclose(file);

}

/*
void parseArguments(int argc, char* argv[]) {

	initTokens();

	char* mpdhost = getenv("MPDHOST");
	if (mpdhost == NULL) {
		debug("DEBUG", "host is not set over env");
		mpdhost = "localhost";
	} 
	
	setMPDHost(mpdhost);
	char* mpdport = getenv("MPDPORT");

	if (mpdport != NULL) {
		setMPDPort(mpdport);
	}

    //parse arguments
    int i;
    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            if (i + 1 < argc && (!strcmp(argv[i], "--format=play") || !strcmp(argv[i], "-fpl"))) { 
		formatPlay(argv[i + 1]);
                i++;
            } else if (i + 1 < argc && (!strcmp(argv[i], "--format=pause") || !strcmp(argv[i], "-fpa"))) { 
		formatPause(argv[i + 1]);
                i++;
            } else if (i + 1 < argc && (!strcmp(argv[i], "--format=stop") || !strcmp(argv[i], "-fs"))) { 
		formatStop(argv[i + 1]);
                i++;
            }  else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
                usage();
            } else if (!strcmp(argv[i], "--debug") || !strcmp(argv[i], "-d")) {
                setDebug(1);
	    } else if (i + 1 < argc && (!strcmp(argv[i], "--config") || !strcmp(argv[i], "-c"))) {
	    	parseConfigFile(log, config, argv[i + 1]);
		i++;
	    } else {
                printf("Invalid arguments.\n");
                usage();
            }
        }
    
    }
    checkFormat();
}
*/
