#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <mpd/status.h>

#include "mpdinfo.h"
#include "status.h"

#include "libs/logger.h"

#define STR_ARTIST "%artist%"
#define STR_TITLE "%title%"
#define STR_VOLUME "%volume%"
#define STR_STATUS "%status%"
#define STR_REPEAT "%repeat%"
#define STR_DBUPDATE "%dbupdate%"
#define STR_RANDOM "%random%"
#define STR_FILENAME "%filename%"
#define STR_ELAPSED_TIME "%elapsed_time%"

#define DEFAULT_FORMAT_STRING "Current Track (Vol %volume%%):\n -%status%-\n %artist% - %title%"
#define DEFAULT_STOPPED_STRING "-stopped-"

char* checkForUserToken(Config* config, char* str) {

	DecisionToken* token = config->decTokens;

	while (token) {

		if (!strncmp(str, token->name, strlen(token->name))) {
			return token->name;
		}

		token = token->next;
	}

	return "";
}

char* getTokenStr(Config* config, char* str) {
	if (!strncmp(str, STR_ARTIST, strlen(STR_ARTIST))) {
		return STR_ARTIST;
	} else if (!strncmp(str, STR_TITLE, strlen(STR_TITLE))) {
		return STR_TITLE;
	} else if (!strncmp(str, STR_VOLUME, strlen(STR_VOLUME))) {
		return STR_VOLUME;
	} else if (!strncmp(str, STR_STATUS, strlen(STR_STATUS))) {
		return STR_STATUS;
	} else if (!strncmp(str, STR_REPEAT, strlen(STR_REPEAT))) {
		return STR_REPEAT;
	} else if(!strncmp(str, STR_FILENAME, strlen(STR_FILENAME))) {
		return STR_FILENAME;
	} else if (!strncmp(str, STR_DBUPDATE, strlen(STR_DBUPDATE))) {
		return STR_DBUPDATE;
	} else if (!strncmp(str, STR_RANDOM, strlen(STR_RANDOM))) {
		return STR_RANDOM;
	} else if (!strncmp(str, STR_ELAPSED_TIME, strlen(STR_ELAPSED_TIME))) {
		return STR_ELAPSED_TIME;
	}

	return checkForUserToken(config, str);
}

TOKEN_TYPE getUserToken(Config* config, char* str) {

	DecisionToken* token = config->decTokens;
	TOKEN_TYPE type = TOKEN_UNKOWN;	
		
	while(token) {

		if (!strncmp(str, token->name, strlen(token->name))) {
			type = token->type;
			break;
		}

		token = token->next;
	}

	return type;
}
	
TOKEN_TYPE getTokenEnum(Config* config, char* str) {
	TOKEN_TYPE type = TOKEN_UNKOWN;

	printf("Current Token Str: %s\n", str);

	if (!strncmp(str, STR_ARTIST, strlen(STR_ARTIST))) {
		type = TOKEN_ARTIST;
	} else if (!strncmp(str, STR_TITLE, strlen(STR_TITLE))) {
		type = TOKEN_TITLE;
	} else if (!strncmp(str, STR_VOLUME, strlen(STR_VOLUME))) {
		type = TOKEN_VOLUME;
	} else if (!strncmp(str, STR_STATUS, strlen(STR_STATUS))) {
		type = TOKEN_STATUS;
	} else if (!strncmp(str, STR_REPEAT, strlen(STR_REPEAT))) {
		type = TOKEN_REPEAT;
	} else if (!strncmp(str, STR_DBUPDATE, strlen(STR_DBUPDATE))) {
                type = TOKEN_DBUPDATE;
        } else if (!strncmp(str, STR_RANDOM, strlen(STR_RANDOM))) {
                type = TOKEN_RANDOM;
        } else if (!strncmp(str, STR_FILENAME, strlen(STR_FILENAME))) {
		type = TOKEN_FILENAME;
	} else if (!strncmp(str, STR_ELAPSED_TIME, strlen(STR_ELAPSED_TIME))) {
		type = TOKEN_ELAPSED_TIME;
	} else {
		type = getUserToken(config, str);
	}

	return type;
}

char* generateOutputStringFromToken(LOGGER log, Config* config, struct mpd_connection* conn, FormatToken* token, int status);

char* getIfNotToken(LOGGER log, struct mpd_connection* conn, Config* config, int status, DecisionToken* token) {

	char* a = generateOutputStringFromToken(log, config, conn, token->a, status);
	logprintf(log, LOG_DEBUG, "a string: (%s)\n", a);
	int i;
	char* out;

	for (i = 0; i < strlen(a); i++) {
		if (!isspace(a[i])) {
			out = malloc(1);
			out[0] = 0;
			free(a);	
			return out;
		}
	}

	free(a);

	return generateOutputStringFromToken(log, config, conn, token->b, status);
}

char* getIfToken(LOGGER log, struct mpd_connection* conn, Config* config, int status, DecisionToken* token) {

	char* a = generateOutputStringFromToken(log, config, conn, token->a, status);
	int i;
	char* out = NULL;

	logprintf(log, LOG_DEBUG, "a string: (%s)\n", a);
	for (i = 0; i < strlen(a); i++) {
		if (!isspace(a[i])) {
			out = generateOutputStringFromToken(log, config, conn, token->b, status);
			break;
		}
	}

	free(a);

	if (!out) {
		out = malloc(1);
		out[0] = 0;
	}

	return out;
}

void* getTokenAction(TOKEN_TYPE type) {

	switch (type) {

		case TOKEN_ARTIST:
			return &getArtist;
		case TOKEN_TITLE:
			return &getTitle;
		case TOKEN_FILENAME:
			return &getFilename;
		case TOKEN_REPEAT:
			return &getRepeatString;
		case TOKEN_VOLUME:
			return &getVolumeString;
		case TOKEN_STATUS:
			return &getStatusString;
		case TOKEN_DBUPDATE:
			return &getDBUpdateString;
		case TOKEN_RANDOM:
			return &getRandomString;
		case TOKEN_ELAPSED_TIME:
			return &getElapsedTime;
		default:
			return NULL;
	}
}

// parse special characters
void formatControls(const char* format, char* output) {
	
	int i = 0;
	int j = 0;
	int length = strlen(format);

	while (i < length) {

		if (format[i] == '\\') {
			i++;
			if (i < length) {
				switch (format[i]) {

					case 'f':
						output[j] = '\f';
						break;
					case 'n':
						output[j] = '\n';
						break;
					case 't':
						output[j] = '\t';
						break;
					default:
						output[j] = '\\';
						j++;
						i--;
						break;
				}
				j++;
				i++;
			} else {
				output[j] = '\\';
				j++;
			}
		} else {
			output[j] = format[i];
			j++;
			i++;
		}
	}
	output[j] = '\0';
}

char* generateOutputString(LOGGER log, Config* config, struct mpd_connection* conn) {

	FormatToken* token = NULL;
	int status = getStatus(log, conn);

	// get right output string
	switch (status) {
		case MPD_STATE_STOP:
			token = config->stop;
			break;
		case MPD_STATE_PAUSE:
			token = config->pause;
			break;
		case MPD_STATE_PLAY:
			token = config->play;
			break;
		default:
			logprintf(log, LOG_WARNING, "Unkown playback status. Use default output format.\n");
			token = config->none;
			break;
	}

	return generateOutputStringFromToken(log, config, conn, token, status);

}

char* generateOutputStringFromToken(LOGGER log, Config* config, struct mpd_connection* conn, FormatToken* token, int status) {

	logprintf(log, LOG_DEBUG, "begin output string generation\n");
	char* output = malloc(1);
	output[0] = 0;

	while(token) {
		char* next;
		char* args;
		
		// text token have the string in the data section. The others have a function in data section.
		
		if (token->type == TOKEN_IF) {
			args = getIfToken(log, conn, config, status, token->data);
		} else if (token->type == TOKEN_IFNOT) {
			args = getIfNotToken(log, conn, config, status, token->data);
		} else if (token->type != TOKEN_TEXT) {
			
			char* (*p)(LOGGER log, struct mpd_connection* conn, int status, Config* config) = token->data;
			if (!p) {
				logprintf(log, LOG_ERROR, "Function pointer is NULL (token->type = %d)\n", token->type);
				break;
			}
			args = p(log, conn, status, config);
		} else {
			args = malloc(strlen((char*) token->data) + 1);
			strcpy(args, token->data);
		}

		logprintf(log, LOG_DEBUG, "%s\n", args);
		next = malloc(strlen(output) + strlen(args) + 1);
		sprintf(next, "%s%s", output, args);
		
		free(args);

		free(output);
		output = next;
		token = token->next;
	}

	return output;
}

FormatToken* buildToken(LOGGER log, Config* config, TOKEN_TYPE type, char* tokenstr, void* data) {
	FormatToken* token = malloc(sizeof(FormatToken));

	token->type = type;

	if (token->type == TOKEN_IF || token->type == TOKEN_IFNOT) {
		DecisionToken* dtoken = config->decTokens;
		while (dtoken) {
			if (!strncmp(dtoken->name, tokenstr, strlen(tokenstr))) {
				token->data = dtoken;
				break;
			}
			dtoken = dtoken->next;
		}

		if (!dtoken) {
			logprintf(log, LOG_WARNING, "UserToken not found %s.\n", tokenstr);
			free(token);
			return NULL;
		}
	} else {
		if (!data) {
			logprintf(log, LOG_WARNING, "Unkown token (%d)\n", type);
			free(data);
			return NULL;
		} else {
			token->data = data;
		}
	}

	token->next = NULL;

	return token;
}

void freeTokenStruct(LOGGER log, FormatToken* token) {

	if (!token) {
		return;
	}
	freeTokenStruct(log, token->next);

	if (token->type == TOKEN_TEXT) {
		logprintf(log, LOG_DEBUG,  "FREE %s\n", token->data);
		free(token->data);
	}
	free(token);
}

FormatToken* buildTokenStructure(LOGGER log, Config* config, const char* input) {

	logprintf(log, LOG_DEBUG, "build token structure from:%s\n", input);

	char format[strlen(input) + 1];
	formatControls(input, format);

	FormatToken* tokens = NULL;
	FormatToken* next = NULL;

	int i = 0;

	char* current = format;
	char* tmp;
	char* tokenstr;
	TOKEN_TYPE type;

	int len = strlen(format);

	// for every char
	while (i < len) {
		// delimiter for tokens
		if (format[i] == '%') {
			tokenstr = getTokenStr(config, format + i);
			logprintf(log, LOG_DEBUG, "Found token: %s\n", tokenstr);

			if (strlen(tokenstr) > 0 ) {
				// check for text token
				if (current != (format + i)) {

					int length = strlen(current) - strlen(format + i);
					tmp = malloc(length + 1);
					memset(tmp, 0, length + 1);
					strncpy(tmp, current, length);
					tmp[length] = '\0';
					current = (format + i);
					
					logprintf(log, LOG_DEBUG, "found next text token: %s\n", tmp);

					if (!tokens) {
						tokens = buildToken(log, config, TOKEN_TEXT, tokenstr, tmp);
						next = tokens;
					} else {
						next->next = buildToken(log, config, TOKEN_TEXT, tokenstr, tmp);
						next = next->next;
					}
				}
				
				type = getTokenEnum(config, tokenstr);

				if (type == TOKEN_UNKOWN) {
					logprintf(log, LOG_ERROR, "Unkown token (%s).\n", tokenstr);
					freeTokenStruct(log, tokens);
					return NULL;
				}

				if (!tokens) {
					tokens = buildToken(log, config, type, tokenstr, getTokenAction(type));
					next = tokens;
				} else {
					next->next = buildToken(log, config, type, tokenstr, getTokenAction(type));
					next = next->next;
				}

				// increment counter with size of tokenstr
				i += strlen(tokenstr);
				current = format + i;
			} else {
				i++;
			}
		} else {
			i++;
		}
	}

	if (current != (format + i)) {
		logprintf(log, LOG_DEBUG, "Last TextToken\n");

		int length = strlen(current) - strlen(format + i);
		tmp = malloc(length + 1);
		strcpy(tmp, current);
		logprintf(log, LOG_DEBUG, "%s\n", tmp);

		if (!tokens) {
			tokens = buildToken(log, config, TOKEN_TEXT, tokenstr, tmp);
		} else {
			next->next = buildToken(log, config, TOKEN_TEXT, tokenstr, tmp);
		}
	}
	return tokens;

}

void checkFormat() {

}



void freeTokenStructs(LOGGER log, Config* config) {
	freeTokenStruct(log, config->play);
	freeTokenStruct(log, config->pause);
	freeTokenStruct(log, config->stop);
	freeTokenStruct(log, config->none);
}

void freeTokenConfigItem(TokenConfigItem* item) {
	if (!item) {
		return;
	}

	if (item->play) {
		free(item->play);
	}
	if (item->pause) {
		free(item->pause);
	}
	if (item->stop) {
		free(item->stop);
	}
	if (item->none) {
		free(item->none);
	}
	if (item->off) {
		free(item->off);
	}
}

void freeTokenConfig(TokenConfig* config) {

	if (!config) {
		return;
	}

	freeTokenConfigItem(config->repeat);
	freeTokenConfigItem(config->random);
	freeTokenConfigItem(config->dbupdate);

}

void formatPlay(char* format) {
	printf("formatPlay is deprecated.\n");
}

void formatStop(char* format) {
	printf("formatStop is deprecated.\n");
}

void formatPause(char* format) {
	printf("formatPause is deprecated.\n");
}
