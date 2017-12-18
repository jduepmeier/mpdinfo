#pragma once
#include <mpd/client.h>
#include <mpd/song.h>

#include "libs/logger.h"

#include "config.h"

// in libs/logger.c
void logconfig(LOGGER log, unsigned level, Config* config);
