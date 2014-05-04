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
OSPX - g_match.c

Handle match related stuff, much like in et..

Created: 11.Mar/14
===========================================================================
*/
#include "g_local.h"

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
Match Info Dump

Dumps end-of-match info
=================
*/
void G_matchInfoDump(unsigned int dwDumpType) {
	int i, ref;
	gentity_t *ent;
	gclient_t *cl;

	for (i = 0; i < level.numConnectedClients; i++) {
		ref = level.sortedClients[i];
		ent = &g_entities[ref];
		cl = ent->client;

		if (cl->pers.connected != CON_CONNECTED) {
			continue;
		}

		if (dwDumpType == EOM_WEAPONSTATS) {			
			// If client wants to write stats to a file, don't auto send this stuff
			if (!(cl->pers.clientFlags & CGF_STATSDUMP)) {
				if ((cl->pers.autoaction & AA_STATSALL) /*|| cl->pers.mvCount > 0*/) {
					G_statsall_cmd(ent, 0, qfalse);
				}
				else if (cl->sess.sessionTeam != TEAM_SPECTATOR) {
					if (cl->pers.autoaction & AA_STATSTEAM) {
						G_statsall_cmd(ent, cl->sess.sessionTeam, qfalse);
					}
					else { CP(va("ws %s\n", G_createStats(ent))); }

				}
				else if (cl->sess.spectatorState != SPECTATOR_FREE) {
					int pid = cl->sess.spectatorClient;

					if ((cl->pers.autoaction & AA_STATSTEAM)) {
						G_statsall_cmd(ent, level.clients[pid].sess.sessionTeam, qfalse); 
					}
					else { CP(va("ws %s\n", G_createStats(g_entities + pid))); }
				}				
			}			

			// Log it
			if (cl->sess.sessionTeam != TEAM_SPECTATOR) {
				G_LogPrintf("WeaponStats: %s\n", G_createStats(ent));
			}

		}
		else if (dwDumpType == EOM_MATCHINFO) {
			// Don't dump score table for users with stats dump enabled
			if (!(cl->pers.clientFlags & CGF_STATSDUMP)) {
				G_printMatchInfo(ent);
			}

			if (g_gametype.integer == GT_WOLF_STOPWATCH) {
				if (g_currentRound.integer == 1) {   // We've already missed the switch
					CP(va("print \">>> ^3Clock set to: %d:%02d\n\"",
						g_nextTimeLimit.integer,
						(int)(60.0 * (float)(g_nextTimeLimit.value - g_nextTimeLimit.integer))));
				}
				else {
					float val = (float)((level.timeCurrent - (level.startTime + level.time - level.intermissiontime)) / 60000.0);
					if (val < g_timelimit.value) {
						CP(va("print \">>> ^3Objective reached at %d:%02d (original: %d:%02d)\n\"",
							(int)val,
							(int)(60.0 * (val - (int)val)),
							g_timelimit.integer,
							(int)(60.0 * (float)(g_timelimit.value - g_timelimit.integer))));
					}
					else {
						CP(va("print \">>> ^3Objective NOT reached in time (%d:%02d)\n\"",
							g_timelimit.integer,
							(int)(60.0 * (float)(g_timelimit.value - g_timelimit.integer))));
					}
				}
			}
		}
	}
}
