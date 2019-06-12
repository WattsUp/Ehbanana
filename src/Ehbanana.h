#ifndef _EHBANANA_H_
#define _EHBANANA_H_

#define WIN32_LEAN_AND_MEAN

#ifndef UNICODE
#define UNICODE
#endif

#include <stdint.h>

static const uint8_t VERSION[3] = {0, 1, 0};

#ifdef DEBUG
static bool DEBUG_ON = true;
#else
static bool DEBUG_ON = false;
#endif

void setupLogging(int argc, char * argv[]);

#endif /* _EHBANANA_H_ */