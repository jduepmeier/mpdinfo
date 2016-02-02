#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <mpd/status.h>

#include "mpdinfo.h"
#include "status.h"
#include "format.h"

#include "libs/logger.h"

const MPD_TOKEN MPD_FORMAT_TAGS[] = {
        [0] = {
                .name = "text",
                .action = NULL
        },
        [1] = {
                .name = "if",
                .action = NULL
        },
        [2] = {
                .name = "ifnot",
                .action = NULL
        },
        {
                .name = "%artist%",
                .action = &getArtist
        },
        {
                .name = "%title%",
                .action = &getTitle
        },
        {
                .name = "%volume%",
                .action = &getVolume
        },
        {
                .name = "%status%",
                .action = &getStatusString
        },
        {
                .name = "%repeat%",
                .action = &getRepeatString
        },
        {
                .name = "%dbupdate%",
                .action = &getDBUpdateString
        },
        {
                .name = "%random%",
                .action = &getRandomString
        },
        {
                .name = "%filename%",
                .action = &getFilename
        },
        {
                .name = "%elapsed_time%",
                .action = &getElapsedTime
        },
        {
                .name = "%album%",
                .action = &getAlbum
        },
        {
                .name = "%album_artist%",
                .action = &getAlbumArtist
        },
        {
                .name = "%genre%",
                .action = &getGenre
        },
        {
                .name = "%track%",
                .action = &getTrack
        }
};


char* getUserTokenString(Config* config, char* str) {

	DecisionToken* token = config->decTokens;

	while (token) {

		// check name
		if (!strncmp(str, token->name, strlen(token->name))) {
			return token->name;
		}

		token = token->next;
	}

	return NULL;
}

const MPD_TOKEN* getUserToken(Config* config, char* str) {

	DecisionToken* token = config->decTokens;
	while(token) {

		if (!strncmp(str, token->name, strlen(token->name))) {
			return token->type;
		}

		token = token->next;
	}

	return NULL;
}
	
const MPD_TOKEN* getMPDToken(Config* config, char* str) {
	logprintf(config->log, LOG_INFO, "Current Token Str: %s\n", str);

	unsigned i;
	for (i = 0; i < sizeof(MPD_TOKEN); i++) {
		if (!strncmp(str, MPD_FORMAT_TAGS[i].name, strlen(MPD_FORMAT_TAGS[i].name))) {
			return &MPD_FORMAT_TAGS[i];
		}
	}

	return getUserToken(config, str);
}

char* generateOutputStringFromToken(Config* config, FormatToken* token, int status);

char* getIfNotToken(Config* config, int status, DecisionToken* token) {

	char* a = generateOutputStringFromToken(config, token->a, status);
	logprintf(config->log, LOG_DEBUG, "a string: (%s)\n", a);
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

	return generateOutputStringFromToken(config, token->b, status);
}

char* getIfToken(Config* config, int status, DecisionToken* token) {

	char* a = generateOutputStringFromToken(config, token->a, status);
	int i;
	char* out = NULL;

	logprintf(config->log, LOG_DEBUG, "a string: (%s)\n", a);
	for (i = 0; i < strlen(a); i++) {
		if (!isspace(a[i])) {
			out = generateOutputStringFromToken(config, token->b, status);
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

char* generateOutputString(Config* config) {

	FormatToken* token = NULL;
	int status = getStatus(config, -1);

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
			logprintf(config->log, LOG_WARNING, "Unkown playback status. Use default output format.\n");
			token = config->none;
			break;
	}

	return generateOutputStringFromToken(config, token, status);

}

char* generateOutputStringFromToken(Config* config, FormatToken* token, int status) {

	logprintf(config->log, LOG_DEBUG, "begin output string generation\n");
	char* output = malloc(1);
	output[0] = 0;

	while(token) {
		char* next;
		char* args;
		
		// text token have the string in the data section. The others have a function in data section.
		
		if (token->type == &MPD_FORMAT_TAGS[TOKEN_IF]) {
			
			logprintf(config->log, LOG_DEBUG, "Found if token\n");
			args = getIfToken(config, status, token->data);
		} else if (token->type == &MPD_FORMAT_TAGS[TOKEN_IF_NOT]) {
			logprintf(config->log, LOG_DEBUG, "Found if not token\n");
			args = getIfNotToken(config, status, token->data);
		} else if (token->type != &MPD_FORMAT_TAGS[TOKEN_TEXT]) {
			logprintf(config->log, LOG_DEBUG, "Found %s token\n", token->type->name);	
			char* (*p)(Config* config, int status) = token->type->action;
			if (!p) {
				logprintf(config->log, LOG_ERROR, "Function pointer is NULL (token->type = %d)\n", token->type);
				break;
			}
			args = p(config, status);
		} else {
			logprintf(config->log, LOG_DEBUG, "Found text token\n");
			args = malloc(strlen((char*) token->data) + 1);
			strncpy(args, token->data, strlen((char*) token->data) + 1);
		}
		if (!args) {
			logprintf(config->log, LOG_WARNING, "args not defined.\n");
		} else {
			logprintf(config->log, LOG_DEBUG, "%s\n", args);
			next = malloc(strlen(output) + strlen(args) + 1);
			sprintf(next, "%s%s", output, args);
		
			free(args);
		}
		free(output);
		output = next;
		token = token->next;
	}

	return output;
}

FormatToken* buildToken(Config* config, const MPD_TOKEN* type, char* tokenstr, void* data) {
	FormatToken* token = malloc(sizeof(FormatToken));

	token->type = type;

	if (token->type == &MPD_FORMAT_TAGS[TOKEN_IF] || token->type == &MPD_FORMAT_TAGS[TOKEN_IF_NOT]) {
		DecisionToken* dtoken = config->decTokens;
		while (dtoken) {
			if (!strncmp(dtoken->name, tokenstr, strlen(tokenstr))) {
				token->data = dtoken;
				break;
			}
			dtoken = dtoken->next;
		}

		if (!dtoken) {
			logprintf(config->log, LOG_WARNING, "UserToken not found %s.\n", tokenstr);
			free(token);
			return NULL;
		}
	} else {
		if (!data) {
			logprintf(config->log, LOG_WARNING, "Unkown token (%d)\n", type);
			free(data);
			free(token);
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

	if (token->type == &MPD_FORMAT_TAGS[TOKEN_TEXT]) {
		logprintf(log, LOG_DEBUG,  "FREE %s\n", token->data);
		free(token->data);
	}
	free(token);
}

char* getTokenName(Config* config, char* str) {
	int i;
	for (i = 0; i < sizeof(MPD_TOKEN); i++) {
		if (!strncmp(str, MPD_FORMAT_TAGS[i].name, strlen(MPD_FORMAT_TAGS[i].name))) {
			return MPD_FORMAT_TAGS[i].name;
		}
	}

	return getUserTokenString(config, str);
}


FormatToken* buildTokenStructure(Config* config, const char* input) {

	logprintf(config->log, LOG_DEBUG, "build token structure from:%s\n", input);

	char format[strlen(input) + 1];
	formatControls(input, format);

	FormatToken* tokens = NULL;
	FormatToken* next = NULL;

	int i = 0;

	char* current = format;
	char* tmp;
	char* token_name;
	MPD_TOKEN* mpdToken;

	int len = strlen(format);

	// for every char
	while (i < len) {
		// delimiter for tokens
		if (format[i] == '%') {
			mpdToken = (MPD_TOKEN*) getMPDToken(config, format + i);
			token_name = getTokenName(config, format + i);
			if (mpdToken && token_name) {
				logprintf(config->log, LOG_DEBUG, "Found token: %s\n", format + i + 1);
				// check for text token
				if (current != (format + i)) {

					int length = strlen(current) - strlen(format + i);
					tmp = malloc(length + 1);
					memset(tmp, 0, length + 1);
					strncpy(tmp, current, length);
					tmp[length] = '\0';
					current = (format + i);
					
					logprintf(config->log, LOG_DEBUG, "found next text token: %s\n", tmp);

					if (!tokens) {
						tokens = buildToken(config, &MPD_FORMAT_TAGS[TOKEN_TEXT], mpdToken->name, tmp);
						next = tokens;
					} else {
						next->next = buildToken(config, &MPD_FORMAT_TAGS[TOKEN_TEXT], mpdToken->name, tmp);
						if (next->next) {
							next = next->next;
						}
					}
				}

				if (!tokens) {
					tokens = buildToken(config, mpdToken, mpdToken->name, mpdToken->action);
					next = tokens;
				} else {

					next->next = buildToken(config, mpdToken, mpdToken->name, mpdToken->action);
					
					if (next->next) {
						next = next->next;
					}
				}

				i += strlen(token_name);
				logprintf(config->log, LOG_DEBUG, "rest string (%s), name: (%s)\n", format + i, token_name);
				current = format + i;
			} else {
				logprintf(config->log, LOG_ERROR, "Unkown token (%s).\n", format + i);
				freeTokenStruct(config->log, tokens);
				return NULL;
			}
		} else {
			i++;
		}
	}

	if (current != (format + i)) {
		logprintf(config->log, LOG_DEBUG, "Last TextToken\n");

		int length = strlen(current) - strlen(format + i);
		tmp = malloc(length + 1);
		strcpy(tmp, current);
		logprintf(config->log, LOG_DEBUG, "%s\n", tmp);

		if (!tokens) {
			tokens = buildToken(config, &MPD_FORMAT_TAGS[TOKEN_TEXT], mpdToken->name, tmp);
		} else {
			next->next = buildToken(config, &MPD_FORMAT_TAGS[TOKEN_TEXT], mpdToken->name, tmp);
		}
	}
	return tokens;

}

void freeTokenStructs(Config* config) {
	freeTokenStruct(config->log, config->play);
	freeTokenStruct(config->log, config->pause);
	freeTokenStruct(config->log, config->stop);
	freeTokenStruct(config->log, config->none);
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

FormatToken* parseTokenString(Config* config, const char* input) {
	if (!input || !strcmp(input, "")) {
		logprintf(config->log, LOG_WARNING, "Empty input string in token parsing.\n");
		return NULL;
	}

	return buildTokenStructure(config, input);
}

