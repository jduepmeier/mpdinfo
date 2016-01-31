#include "format_structure.h"
#include "format.h"
#include "string.h"

FormatToken* parseTokenString(Config* config, const char* input) {

	if (!input || !strcmp(input, "")) {
		logprintf(config->log, LOG_WARNING, "Empty input string in token parsing\n");
		return NULL;
	}

	return buildTokenStructure(config, input);
}
