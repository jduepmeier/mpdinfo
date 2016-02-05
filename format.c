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
                .action = &getVolumeString
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

	if (strlen(str) < 1 || str[0] != '%') {
		return NULL;
	}


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

	unsigned len_out = 0;
	unsigned len_args = 0;
	char* args;

	while(token) {
		
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
		
		if (args) {
			logprintf(config->log, LOG_DEBUG, "%s\n", args);
			len_args = strlen(args);
			output = realloc(output, len_out + strlen(args) + 1);
			strncpy(output + len_out, args, len_args + 1);
			len_out += len_args;

			free(args);
		} else {
			logprintf(config->log, LOG_INFO, "args not defined.\n");

		}
		token = token->next;
	}

	return output;
}

FormatToken* buildToken(Config* config, const MPD_TOKEN* type, char* tokenstr, void* data) {
	FormatToken* token = malloc(sizeof(FormatToken));
	token->data = NULL;

	if (!type) {
		logprintf(config->log, LOG_WARNING, "type is null.\n");
		return NULL;
	}

	token->type = type;

	// check for user tokens
	if (token->type == &MPD_FORMAT_TAGS[TOKEN_IF] || token->type == &MPD_FORMAT_TAGS[TOKEN_IF_NOT]) {
		
		// search the right token
		DecisionToken* dtoken = config->decTokens;
		while (dtoken) {
			logprintf(config->log, LOG_DEBUG, "curr dtoken: %s (%s)\n", dtoken->name, tokenstr);
			if (!strncmp(tokenstr, dtoken->name, strlen(dtoken->name))) {
				token->data = dtoken;
				break;
			}
			dtoken = dtoken->next;
		}
		free(data);

		if (!dtoken) {
			logprintf(config->log, LOG_WARNING, "UserToken not found %s.\n", tokenstr);
			free(token);
			return NULL;
		}
	} else {
		token->data = data;
	}

	token->next = NULL;

	return token;
}

void freeTokenStruct(LOGGER log, FormatToken* token) {

	if (!token) {
		return;
	}
	freeTokenStruct(log, token->next);

	if (token->type != &MPD_FORMAT_TAGS[TOKEN_IF] && token->type != &MPD_FORMAT_TAGS[TOKEN_IF_NOT]) {
		logprintf(log, LOG_DEBUG,  "FREE %s\n", token->data);
		free(token->data);
	}

	free(token);
}

FormatToken* nextBuildToken(Config* config, char* token, int size) {

	const MPD_TOKEN* type = getMPDToken(config, token);

	char* name = malloc(size + 1);
	strncpy(name, token, size + 1);
	name[size] = 0;

	if (!type) {	
		logprintf(config->log, LOG_INFO, "Text token found.\n");
		type = &MPD_FORMAT_TAGS[TOKEN_TEXT];
	} else {
		logprintf(config->log, LOG_INFO, "%s token found.\n", type->name);
	}
	return buildToken(config, type, token, name);

}

FormatToken* buildTokenStructure(Config* config, const char* input) {

	logprintf(config->log, LOG_DEBUG, "build token structure from: %s\n", input);

	FormatToken* tokens = NULL;
	FormatToken* out = NULL;
	FormatToken* currToken = NULL;
	char format[strlen(input) + 1];
	formatControls(input, format);


	char* curr = format;
	char* last = format;


	if (strlen(curr) < 1) {
		return NULL;
	}

	if (curr[0] == '%') {
		curr++;
	}

	while ((curr = strchr(curr, '%'))) {

		logprintf(config->log, LOG_INFO, "Found seperator. Last string is: (%*s)\n", curr - last, last);
		currToken = nextBuildToken(config, last, curr - last);


		if (!currToken) {
			freeTokenStruct(config->log, out);
			return NULL;
		}

		if (out) {
			tokens->next = currToken;
			tokens = currToken;
		} else {
			out = currToken;
			tokens = currToken;
		}

		// check if last was a token
		if (currToken->type != &MPD_FORMAT_TAGS[TOKEN_TEXT]) {
			logprintf(config->log, LOG_DEBUG, "cur++, %s\n", currToken->type->name);
			curr++;
		}
		logprintf(config->log, LOG_DEBUG, "curr = (%s)\n", curr);
		last = curr;
		// check if we are now at the end of the string
		if (curr[0] == 0) {
			break;
		}
		
		curr++;
	};

	currToken = nextBuildToken(config, last, strlen(last));

	if (!currToken) {
		freeTokenStruct(config->log, out);
		return NULL;
	}

	if (out) {
		tokens->next = currToken;
		tokens = currToken;
	} else {
		out = currToken;
	}
 
	return out;
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

	free(item->play);
	free(item->pause);
	free(item->stop);
	free(item->none);
	free(item->off);
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

