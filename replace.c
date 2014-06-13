#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char * stringReplace(char *search, char *replace, char *string) {
	char *tempString, *searchStart;
	int len=0;

	// preuefe ob Such-String vorhanden ist
	searchStart = strstr(string, search);
	if(searchStart == NULL) {
		return string;
	}

	// Speicher reservieren
	tempString = (char*) malloc(strlen(string) * sizeof(char) + 1);
	if(tempString == NULL) {
		return NULL;
	}

	// temporaere Kopie anlegen
	strcpy(tempString, string);

	// ersten Abschnitt in String setzen
	len = searchStart - string;
	string[len] = '\0';

	// zweiten Abschnitt anhaengen
	strcat(string, replace);

	// dritten Abschnitt anhaengen
	len += strlen(search);
	strcat(string, (char*)tempString+len);

	// Speicher freigeben
	free(tempString);
	
	return string;
}
