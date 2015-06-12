#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "format.h"
#include "help.h"
#include "debug.h"
#include "parse.h"

#include "mpd/status.h"
#include "status.h"

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

struct {

	char* host;
	unsigned long int port;

} connectionInfo;

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

void free_connection_info() {

	free(connectionInfo.host);
}

Category parseCategory(char* cat) {

	if (!strncmp(cat, CAT_OUTPUT, strlen(CAT_OUTPUT))) {
		return C_OUTPUT;
	} else if (!strncmp(cat, CAT_TOKEN_DBUPDATE, strlen(CAT_TOKEN_DBUPDATE))) { 
		return C_TOKEN_DBUPDATE;
	} else if (!strncmp(cat, CAT_TOKEN_REPEAT, strlen(CAT_TOKEN_REPEAT))) { 
		return C_TOKEN_REPEAT;
	} else if (!strncmp(cat, CAT_TOKEN_RANDOM, strlen(CAT_TOKEN_RANDOM))) { 
		return C_TOKEN_RANDOM;
	} else {
		return C_GENERAL;
	}

}

void setMPDHost(char* host) {
	if (connectionInfo.host) {
		free(connectionInfo.host);
	}
	char* qm = malloc(strlen(host) + 1);
	deleteQMs(host, qm);
	debug("DEBUG qm", qm);
	connectionInfo.host = qm;
}
char* getMPDHost() {
	return connectionInfo.host;
}

unsigned long int getMPDPort() {
	return connectionInfo.port;
}

void setMPDPort(char* port) {
	connectionInfo.port = strtoul(port, NULL, 10);

	if (connectionInfo.port == 0) {
		debug("WARNING","port not correct, using default port 6600");
		connectionInfo.port = 6600;
	}
}

void parseConfigLineToken(ConfigLine* cl, TokenStruct* tk) {


	char* output2 = malloc(strlen(cl->value) +1);
	formatControls(cl->value, output2);
	char* output = malloc(strlen(output2) + 1);
	deleteQMs(output2,output);
	free(output2);

	if (!strncmp(cl->key, CONFIG_PLAY, strlen(CONFIG_PLAY))) {
		free(tk->play);
		tk->play = output;
	} else if (!strncmp(cl->key, CONFIG_PAUSE, strlen(CONFIG_PAUSE))) {
		free(tk->pause);
		tk->pause = output;
	} else if (!strncmp(cl->key, CONFIG_STOP, strlen(CONFIG_STOP))) {
		free(tk->stop);
		tk->stop = output;
	} else if (!strncmp(cl->key, CONFIG_NONE, strlen(CONFIG_NONE))) {
		free(tk->none);
		tk->none = output;
	}
}

void parseConfigLineOutput(ConfigLine* cl) {
	debug("DEBUG", "in output category parsing");

	char* output = malloc(strlen(cl->value) + 1);
	deleteQMs(cl->value, output);

	if (!strncmp(cl->key, CONFIG_PLAY, strlen(CONFIG_PLAY))) {
		formatPlay(output);
	} else if (!strncmp(cl->key, CONFIG_PAUSE, strlen(CONFIG_PAUSE))) {
		formatPause(output);
	} else if (!strncmp(cl->key, CONFIG_STOP, strlen(CONFIG_STOP))) {
		formatStop(output);
	}

	free(output);
}

void parseConfigLineGeneral(ConfigLine* cl) {


	if (!strncmp(cl->key, CONFIG_HOST, strlen(CONFIG_HOST))) {
		setMPDHost(cl->value);
		return;
	}
	if (!strncmp(cl->key, CONFIG_PORT, strlen(CONFIG_PORT))) {
		setMPDPort(cl->value);
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

ConfigLine* gettingLineArgs(char* line) {
	line = cropSpacesAndTabs(line);
	int i = 0;

	while (line[i] != '=') {
		if (line[i] == '\n') {
			printf("Error parsing line: %s\n", line);
			return NULL;
		}
		i++;
	}

	ConfigLine* cl = malloc(sizeof(ConfigLine));

	
	cl->key = strncpyN(cl->key, line, i);

	cl->key = cropSpacesAndTabsB(cl->key);

	line += i + 1;

	line = cropSpacesAndTabs(line);
	cl->value = malloc(strlen(line) + 1);
	strcpy(cl->value, line);
	cl->value = cropSpacesAndTabsB(cl->value);

	return cl;
}



void parseConfigLine(Category cat, char* line) {


	ConfigLine* cl = gettingLineArgs(line);

	if (cl == NULL) {
		
		printf("lineargs not correct\n");
		return;
	}

	switch (cat) {
	
		case C_GENERAL:
			parseConfigLineGeneral(cl);
			break;
		case C_OUTPUT:
			parseConfigLineOutput(cl);
			break;
		case C_TOKEN_RANDOM:
			parseConfigLineToken(cl, &tokens.random);
			break;
		case C_TOKEN_REPEAT:
			parseConfigLineToken(cl, &tokens.repeat);
			break;
		case C_TOKEN_DBUPDATE:
			parseConfigLineToken(cl, &tokens.dbupdate);
			break;
		default:
			parseConfigLineGeneral(cl);
			break;
	}

	free(cl->key);
	free(cl->value);
	free(cl);
}


void parseConfigFile(char* path) {

	debug("DEBUG", "starting config reader");

	FILE* file = fopen(path, "r");

	if (file == NULL) {

		perror("Cannot read config file.\n");
		return;
	}

	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	Category category = C_GENERAL;

	while ( (read = getline(&line, &len, file)) != -1) {
		char* free_line = line;
		debug("DEBUG", line);
		
		line = cropSpacesAndTabs(line);
		if (line[0] != '#' && line[0] != '\n') {
			
			if (line[0] == '[') {
				debug("DEBUG", line);
				category = parseCategory(line);
			} else {
				parseConfigLine(category, line);
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
                printHelp();
            } else if (!strcmp(argv[i], "--debug") || !strcmp(argv[i], "-d")) {
                setDebug(1);
	    } else if (i + 1 < argc && (!strcmp(argv[i], "--config") || !strcmp(argv[i], "-c"))) {
	    	parseConfigFile(argv[i + 1]);
		i++;
	    } else {
                printf("Invalid arguments.\n");
                printHelp();
            }
        }
    
    }
    checkFormat();
}
