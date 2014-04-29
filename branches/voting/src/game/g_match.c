/*
===========================================================================

Return to Castle Wolfenstein multiplayer GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of the Return to Castle Wolfenstein multiplayer GPL Source Code (RTCW MP Source Code).

RTCW MP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW MP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW MP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW MP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW MP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
g_match.c

Handle match related stuff, much like in et..

Author: Nate 'L0
Created: 11.Mar/14
===========================================================================
*/
#include "g_local.h"
#include "../../MAIN/ui_mp/menudef.h"

/*
=================
Match settings

Pretty much a dump from et..
=================
*/
void G_loadMatchGame(void) {
	unsigned int i, dwBlueOffset, dwRedOffset;
	unsigned int aRandomValues[MAX_REINFSEEDS];
	char strReinfSeeds[MAX_STRING_CHARS];

	if (server_autoconfig.integer && (!(z_serverflags.integer & ZSF_COMP) || level.newSession)) {
		G_configSet(g_gametype.integer, (server_autoconfig.integer == 1));
		trap_Cvar_Set("z_serverflags", va("%d", z_serverflags.integer | ZSF_COMP));
	}

	// Set up the random reinforcement seeds for both teams and send to clients
	dwBlueOffset = rand() % MAX_REINFSEEDS;
	dwRedOffset = rand() % MAX_REINFSEEDS;
	strcpy(strReinfSeeds, va("%d %d", (dwBlueOffset << REINF_BLUEDELT) + (rand() % (1 << REINF_BLUEDELT)),
		(dwRedOffset << REINF_REDDELT) + (rand() % (1 << REINF_REDDELT))));

	for (i = 0; i < MAX_REINFSEEDS; i++) {
		aRandomValues[i] = (rand() % REINF_RANGE) * aReinfSeeds[i];
		strcat(strReinfSeeds, va(" %d", aRandomValues[i]));
	}

	level.dwBlueReinfOffset = 1000 * aRandomValues[dwBlueOffset] / aReinfSeeds[dwBlueOffset];
	level.dwRedReinfOffset = 1000 * aRandomValues[dwRedOffset] / aReinfSeeds[dwRedOffset];

	trap_SetConfigstring(CS_REINFSEEDS, strReinfSeeds);
}

/*
=================
Reset Round state
=================
*/
void G_resetRoundState(void) {
	if (g_gametype.integer == GT_WOLF_STOPWATCH) {
		trap_Cvar_Set("g_currentRound", "0");
	}
}

/*
=================
Reset mode state
=================
*/
void G_resetModeState(void) {
	if (g_gametype.integer == GT_WOLF_STOPWATCH) {
		trap_Cvar_Set("g_nextTimeLimit", "0");
	}
}

/*
=================
Update configstring for vote info
=================
*/
int G_checkServerToggle(vmCvar_t *cv) {
	int nFlag;

	if (cv == &match_mutespecs) {
		nFlag = CV_SVS_MUTESPECS;
	}
	else if (cv == &g_friendlyFire) {
		nFlag = CV_SVS_FRIENDLYFIRE;
	}
	else if (cv == &g_antilag) {
		nFlag = CV_SVS_ANTILAG;
	}
	else if (cv == &g_teamForceBalance) {
		nFlag = CV_SVS_BALANCEDTEAMS;
	}
	// special case for 2 bits
	else if (cv == &match_warmupDamage) {
		if (cv->integer > 0) {
			level.server_settings &= ~CV_SVS_WARMUPDMG;
			nFlag = (cv->integer > 2) ? 2 : cv->integer;
			nFlag = nFlag << 2;
		}
		else {
			nFlag = CV_SVS_WARMUPDMG;
		}
	}
	else if (cv == &g_nextmap ) {
		if (*cv->string) {
			level.server_settings |= CV_SVS_NEXTMAP;
		}
		else {
			level.server_settings &= ~CV_SVS_NEXTMAP;
		}
		return(qtrue);
	}	
	else { return(qfalse); }

	if (cv->integer > 0) {
		level.server_settings |= nFlag;
	}
	else {
		level.server_settings &= ~nFlag;
	}

	return(qtrue);
}

