#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <mpd/status.h>

#include "debug.h"
#include "mpdinfo.h"


char* formatString  = "Current Track (Vol %volume%%):\n -%status%-\n %artist% - %title%";
char* formatStoppedString = "-stopped-";

#define STR_ARTIST "%artist%"
#define STR_TITLE "%title%"
#define STR_VOLUME "%volume%"
#define STR_STATUS "%status%"


typedef enum {

	TOKEN_ARTIST, TOKEN_TITLE, TOKEN_STATUS, TOKEN_VOLUME, TOKEN_TEXT 

} TOKEN_TYPE;

typedef struct {

	TOKEN_TYPE type;
	void* data;

} FormatToken;

FormatToken** ppTokens = NULL;
FormatToken** stopTokens = NULL;

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
	return -1;
}

void* getTokenAction(TOKEN_TYPE type) {

	switch (type) {

		case TOKEN_ARTIST:
			return &getArtist;
		case TOKEN_TITLE:
			return &getTitle;
		case TOKEN_VOLUME:
			return &getVolumeString;
		case TOKEN_STATUS:
			return &getStatusString;
		default:
			return NULL;
	}
}

char* generateOutputString() {

	FormatToken** tokens = NULL;

	if (getStatus() == MPD_STATE_STOP) {
		tokens = stopTokens;
	} else {
		tokens = ppTokens;
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
		debug("DEBUG: args", args);
		next = malloc(strlen(output) + strlen(args) + 1);
		sprintf(next, "%s%s", output, args);
		free(args);
		free(output);
		debug("DEBUG: next", next);
		output = next;
		t++;
	}

	debug("DEBUG: output", output);

	printf("%s\f\n", output);

	fflush(stdout);

	return output;
}

FormatToken** buildTokenStructure(char* format) {

	FormatToken** tokens = NULL;

	debug("DEBUG","in buildTokenStructure");
	int i = 0;
	int t = 0;

	char* current = format;

	tokens = realloc(tokens, (t + 1) * sizeof(FormatToken*));
	tokens[t] = NULL;


	// for every char
	while (i < strlen(format)) {
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
	return tokens;

}

void checkFormat() {

	if (!stopTokens) {
		stopTokens = buildTokenStructure(formatStoppedString);	
	}
	if (!ppTokens) {
		ppTokens = buildTokenStructure(formatString);
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
	free_token_struct(ppTokens);
	free_token_struct(stopTokens);
}


void format(char* format) {

	ppTokens = buildTokenStructure(format);
}

void formatStopped(char* format) {

	stopTokens = buildTokenStructure(format);
}

