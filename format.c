#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <mpd/status.h>

#include "debug.h"
#include "mpdinfo.h"
#include "status.h"
#include "parse.h"

#define STR_ARTIST "%artist%"
#define STR_TITLE "%title%"
#define STR_VOLUME "%volume%"
#define STR_STATUS "%status%"
#define STR_REPEAT "%repeat%"
#define STR_DBUPDATE "%dbupdate%"

char* formatString  = "Current Track (Vol %volume%%):\n -%status%-\n %artist% - %title%";
char* formatStoppedString = "-stopped-";

typedef enum {

	TOKEN_ARTIST, TOKEN_TITLE, TOKEN_STATUS, TOKEN_VOLUME, TOKEN_REPEAT, TOKEN_DBUPDATE, TOKEN_TEXT 

} TOKEN_TYPE;

typedef struct {

	TOKEN_TYPE type;
	void* data;

} FormatToken;

struct {
	FormatToken** ppTokens;
	FormatToken** stopTokens;
	FormatToken** pauseTokens;
} tokenStruct;

char* getTokenStr(char* str) {
	if (strncmp(str, STR_ARTIST,strlen(STR_ARTIST)) == 0) {
		return STR_ARTIST;
	}

	if (strncmp(str, STR_TITLE, strlen(STR_TITLE)) == 0) {
		return STR_TITLE;
	}
	if (strncmp(str, STR_VOLUME, strlen(STR_VOLUME)) == 0) {
		return STR_VOLUME;
	}

	if (strncmp(str, STR_STATUS, strlen(STR_STATUS)) == 0) {
		return STR_STATUS;
	}
	if (strncmp(str, STR_REPEAT, strlen(STR_REPEAT)) == 0) {
		return STR_REPEAT;
	}
	if (strncmp(str, STR_DBUPDATE, strlen(STR_DBUPDATE)) == 0) {
                return STR_DBUPDATE;
        }
	return "";
}

	
TOKEN_TYPE getTokenEnum(char* str) {
	if (strncmp(str, STR_ARTIST,strlen(STR_ARTIST)) == 0) {
		return TOKEN_ARTIST;
	}

	if (strncmp(str, STR_TITLE, strlen(STR_TITLE)) == 0) {
		return TOKEN_TITLE;
	}
	if (strncmp(str, STR_VOLUME, strlen(STR_VOLUME)) == 0) {
		return TOKEN_VOLUME;
	}

	if (strncmp(str, STR_STATUS, strlen(STR_STATUS)) == 0) {
		return TOKEN_STATUS;
	}
	if (strncmp(str, STR_REPEAT, strlen(STR_REPEAT)) == 0) {
		return TOKEN_REPEAT;
	}
	if (strncmp(str, STR_DBUPDATE, strlen(STR_DBUPDATE)) == 0) {
                return TOKEN_DBUPDATE;
        }
	return -1;
}

void* getTokenAction(TOKEN_TYPE type) {

	switch (type) {

		case TOKEN_ARTIST:
			return &getArtist;
		case TOKEN_TITLE:
			return &getTitle;
		case TOKEN_REPEAT:
			return &getRepeatString;
		case TOKEN_VOLUME:
			return &getVolumeString;
		case TOKEN_STATUS:
			return &getStatusString;
		case TOKEN_DBUPDATE:
			return &getDBUpdateString;
		default:
			return NULL;
	}
}

void formatControls(char* format, char* output) {
	
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

char* generateOutputString() {

	FormatToken** tokens = NULL;

	switch (getStatus()) {
		case MPD_STATE_STOP:
			tokens = tokenStruct.stopTokens;
			break;
		case MPD_STATE_PAUSE:
			if (tokenStruct.pauseTokens) {
				tokens = tokenStruct.pauseTokens;
			} else {
				tokens = tokenStruct.ppTokens;
			}
			break;
		case MPD_STATE_PLAY:
			tokens = tokenStruct.ppTokens;
			break;
		default:
			tokens = tokenStruct.ppTokens;
			break;
	}

	debug("VERBOSE", "begin output string generation");

	int t = 0;

	char* output = malloc(1);
	output[0] = '\0';

	while(tokens[t]) {

		char* next;
		char* args;
		if (tokens[t]->type != TOKEN_TEXT) {
			
			char* (*p)() = tokens[t]->data;
			args = p();
		} else {
			args = malloc(strlen((char*) tokens[t]->data) + 1);
			strcpy(args, tokens[t]->data);
		}
		debug("DEBUG", args);
		next = malloc(strlen(output) + strlen(args) + 1);
		sprintf(next, "%s%s", output, args);
		if (strcmp(args, "") != 0) {
			free(args);
		}
		free(output);
		output = next;
		t++;
	}

	return output;
}

FormatToken** buildTokenStructure(char* f) {

	char* format = malloc(strlen(f) + 1);
	formatControls(f, format);

	FormatToken** tokens = NULL;

	debug("DEBUG","in buildTokenStructure");
	int i = 0;
	int t = 0;

	char* current = format;

	tokens = realloc(tokens, (t + 1) * sizeof(FormatToken*));
	tokens[t] = NULL;
	int len = strlen(format);

	// for every char
	while (i < len) {
		// delimiter for tokens
		if (format[i] == '%') {
			char* tokenstr = getTokenStr(format + i);

			if (strlen(tokenstr) > 0 ) {
				// check for text token
				if (current != (format + i)) {
					debug("DEBUG", "find TextToken");
					tokens = realloc(tokens, (t + 2) * sizeof(FormatToken*));
					tokens[t] = malloc(sizeof(FormatToken));

					tokens[t]->type = TOKEN_TEXT;
					int length = strlen(current) - strlen(format + i);
					tokens[t]->data = malloc(length + 1);
					strncpy(tokens[t]->data, current, length);
					((char*)tokens[t]->data)[length] = '\0';
					current = (format + i);
					debug("DEBUG",tokens[t]->data);
					t++;
					tokens[t] = NULL;
				}
				
				tokens = realloc(tokens,(t + 2) * sizeof(FormatToken*));
				tokens[t] = malloc(sizeof(FormatToken));

				tokens[t]->type = getTokenEnum(tokenstr);
				tokens[t]->data = getTokenAction(tokens[t]->type);

				t++;
				tokens[t] = NULL; 
				
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
		debug("DEBUG", "Last TextToken");

		tokens = realloc(tokens, (t + 2) * sizeof(FormatToken*));
		tokens[t] = malloc(sizeof(FormatToken));

		tokens[t]->type = TOKEN_TEXT;
		int length = strlen(current) - strlen(format + i);
		tokens[t]->data = malloc(length + 1);
		strcpy(tokens[t]->data, current);
		debug("DEBUG", tokens[t]->data);
		t++;
		tokens[t] = NULL;

	}
	free(format);
	return tokens;

}

void checkFormat() {

	if (!tokenStruct.stopTokens) {
		tokenStruct.stopTokens = buildTokenStructure(formatStoppedString);	
	}
	if (!tokenStruct.ppTokens) {
		tokenStruct.ppTokens = buildTokenStructure(formatString);
	}


}

void free_token_struct(FormatToken** token) {
	int t = 0;

	while (token[t]) {

		if (token[t]->type == TOKEN_TEXT) {
			free(token[t]->data);
		}

		free(token[t]);
		t++;
	}
	free(token);
}

void free_token_structs() {
	free_token_struct(tokenStruct.ppTokens);
	free_token_struct(tokenStruct.stopTokens);
	if (tokenStruct.pauseTokens) {
		free_token_struct(tokenStruct.pauseTokens);
	}
}


void formatPlay(char* format) {

	tokenStruct.ppTokens = buildTokenStructure(format);
}

void formatStop(char* format) {

	tokenStruct.stopTokens = buildTokenStructure(format);
}

void formatPause(char* format) {
	tokenStruct.pauseTokens = buildTokenStructure(format);
}
