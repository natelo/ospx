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
OSPx - g_players.c

Various player's commands

Created: 5 May/14
===========================================================================
*/
#include "g_local.h"

// ************** PLAYERS
//
// Show client info
void pCmd_Players(gentity_t *ent) {
	int i, idnum, max_rate, cnt = 0, tteam;
	int user_rate, user_snaps;
	gclient_t *cl;
	gentity_t *cl_ent;
	char n2[MAX_NETNAME], ready[16], ref[16], rate[256];
	char *s, *tc, *coach, userinfo[MAX_INFO_STRING];


	if (g_gamestate.integer == GS_PLAYING) {
		if (ent) {
			CP("print \"\n^3 ID^1 : ^3Player                    Nudge  Rate  MaxPkts  Snaps\n\"");
			CP("print \"^1-----------------------------------------------------------^7\n\"");
		}
		else {
			G_Printf(" ID : Player                    Nudge  Rate  MaxPkts  Snaps\n");
			G_Printf("-----------------------------------------------------------\n");
		}
	}
	else {
		if (ent) {
			CP("print \"\n^3Status^1   : ^3ID^1 : ^3Player                    Nudge  Rate  MaxPkts  Snaps\n\"");
			CP("print \"^1---------------------------------------------------------------------^7\n\"");
		}
		else {
			G_Printf("Status   : ID : Player                    Nudge  Rate  MaxPkts  Snaps\n");
			G_Printf("---------------------------------------------------------------------\n");
		}
	}

	max_rate = trap_Cvar_VariableIntegerValue("sv_maxrate");

	for (i = 0; i < level.numConnectedClients; i++) {
		idnum = level.sortedClients[i]; //level.sortedNames[i];
		cl = &level.clients[idnum];
		cl_ent = g_entities + idnum;

		SanitizeString(cl->pers.netname, n2, qtrue);
		n2[26] = 0;
		ref[0] = 0;
		ready[0] = 0;

		// Rate info
		if (cl_ent->r.svFlags & SVF_BOT) {
			strcpy(rate, va("%s%s%s%s", "[BOT]", " -----", "       --", "     --"));
		}
		else if (cl->pers.connected == CON_CONNECTING) {
			strcpy(rate, va("%s", "^3>>> CONNECTING <<<"));
		}
		else {
			trap_GetUserinfo(idnum, userinfo, sizeof(userinfo));
			s = Info_ValueForKey(userinfo, "rate");
			user_rate = (max_rate > 0 && atoi(s) > max_rate) ? max_rate : atoi(s);
			s = Info_ValueForKey(userinfo, "snaps");
			user_snaps = atoi(s);

			strcpy(rate, va("%5d%6d%9d%7d", cl->pers.clientTimeNudge, user_rate, cl->pers.clientMaxPackets, user_snaps));
		}

		if (g_gamestate.integer != GS_PLAYING) {
			if (cl->sess.sessionTeam == TEAM_SPECTATOR || cl->pers.connected == CON_CONNECTING) {
				strcpy(ready, ((ent) ? "^5--------^1 :" : "-------- :"));
			}
			else if (/*cl->pers.ready ||*/ (g_entities[idnum].r.svFlags & SVF_BOT)) {
				strcpy(ready, ((ent) ? "^3(READY)^1  :" : "(READY)  :"));
			}
			else {
				strcpy(ready, ((ent) ? "NOTREADY^1 :" : "NOTREADY :"));
			}
		}
		/*/
		if (cl->sess.referee) {
			strcpy(ref, "REF");
		}

		if (cl->sess.coach_team) {
			tteam = cl->sess.coach_team;
			coach = (ent) ? "^3C" : "C";
		}
		else {*/
			tteam = cl->sess.sessionTeam;
			coach = " ";
		//}

		tc = (ent) ? "^7 " : " ";
		if (g_gametype.integer >= GT_WOLF) {
			if (tteam == TEAM_RED) {
				tc = (ent) ? "^1X^7" : "X";
			}
			if (tteam == TEAM_BLUE) {
				tc = (ent) ? "^4L^7" : "L";
			}
		}

		if (ent) {
			CP(va("print \"%s%s%2d%s^1:%s %-26s^7%s  ^3%s\n\"", ready, tc, idnum, coach, ((ref[0]) ? "^3" : "^7"), n2, rate, ref));
		}
		else { G_Printf("%s%s%2d%s: %-26s%s  %s\n", ready, tc, idnum, coach, n2, rate, ref); }

		cnt++;
	}

	if (ent) {
		CP(va("print \"\n^3%2d^7 total players\n\n\"", cnt));
	}
	else { G_Printf("\n%2d total players\n\n", cnt); }

	// Team speclock info
	if (g_gametype.integer >= GT_WOLF) {
		for (i = TEAM_RED; i <= TEAM_BLUE; i++) {
			if (teamInfo[i].spec_lock) {
				if (ent) {
					CP(va("print \"** %s team is speclocked.\n\"", aTeams[i]));
				}
				else { G_Printf("** %s team is speclocked.\n", aTeams[i]); }
			}
		}
	}
}