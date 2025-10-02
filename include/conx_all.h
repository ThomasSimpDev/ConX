#ifndef CONX_ALL_H
#define CONX_ALL_H

// Include all ConX headers
#include "conx.h"
#include "conx_2d.h"
#include "conx_3d.h"
#include "conx_lua.h"
#include "conx_math.h"

// Initialize all subsystems
bool conx_initialize_all(const ConXConfig *config);
void conx_shutdown_all(void);

#endif
