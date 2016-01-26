#include "format_structure.h"
#include "format.h"
#include "string.h"

FormatToken* parseTokenString(LOGGER log, Config* config, const char* input) {

	if (!input || !strcmp(input, "")) {
		logprintf(log, LOG_WARNING, "Empty input string in token parsing\n");
		return NULL;
	}

	return buildTokenStructure(log, config, input);
}
