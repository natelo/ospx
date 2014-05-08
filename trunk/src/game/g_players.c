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
			else if (cl->pers.ready || (g_entities[idnum].r.svFlags & SVF_BOT)) {
				strcpy(ready, ((ent) ? "^3(READY)^1  :" : "(READY)  :"));
			}
			else {
				strcpy(ready, ((ent) ? "NOTREADY^1 :" : "NOTREADY :"));
			}
		}
		
		if (cl->sess.admin && !cl->sess.incognito) {			
			strcpy(ref, sortTag(ent));
		}
		/*
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

/*
===================
READY / NOTREADY

Sets a player's "ready" status.

Tardo - rewrote this because the parameter handling to the function is different in rtcw.
===================
*/
void G_ready_cmd(gentity_t *ent, qboolean state) {
	char *status[2] = { "NOT READY", "READY" };

	if (!g_doWarmup.integer) {
		return;
	}

	if (g_gamestate.integer == GS_PLAYING || g_gamestate.integer == GS_INTERMISSION) {
		CP("@print \"Match is already in progress.\n\"");
		return;
	}

	if (!state && g_gamestate.integer == GS_WARMUP_COUNTDOWN) {
		CP("print \"Countdown started, ^3notready^7 ignored.\n\"");
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		CP(va("print \"Specs cannot use ^3%s ^7command.\n\"", status[state]));
		return;
	}

	// Move them to correct ready state
	if (ent->client->pers.ready == state) {
		CP(va("print \"You are already ^3%s^7!\n\"", status[state]));
	}
	else {
		ent->client->pers.ready = state;
		if (!level.intermissiontime) {
			if (state) {
				ent->client->pers.ready = qtrue;
				ent->client->ps.powerups[PW_READY] = INT_MAX;
			}
			else {
				ent->client->pers.ready = qfalse;
				ent->client->ps.powerups[PW_READY] = 0;
			}

			// Doesn't rly matter..score tab will show slow ones..
			AP(va("cp \"\n%s \n^7is %s%s!\n\"", ent->client->pers.netname, (state ? "^n" : "^z"), status[state]));
		}
	}
}

/*
===================
Invite player to spectate

NOTE: Admin can still be invited..so in case logout occurs..
===================
*/
void cmd_specInvite(gentity_t *ent) {
	int	target;
	gentity_t	*player;
	char arg[MAX_TOKEN_CHARS];
	int team = ent->client->sess.sessionTeam;

	if (team_nocontrols.integer) {
		CP("print \"Team commands are not enabled on this server.\n\"");
		return;
	}

	if (team == TEAM_RED || team == TEAM_BLUE) {
		if (!teamInfo[team].spec_lock) {
			CP("print \"Your team isn't locked from spectators!\n\"");
			return;
		}

		trap_Argv(1, arg, sizeof(arg));
		if ((target = ClientNumberFromString(ent, arg)) == -1) {
			return;
		}

		player = g_entities + target;

		// Can't invite self
		if (player->client == ent->client) {
			CP("print \"You can't specinvite yourself!\n\"");
			return;
		}

		// Can't invite an active player.
		if (player->client->sess.sessionTeam != TEAM_SPECTATOR) {
			CP("print \"You can't specinvite a non-spectator!\n\"");
			return;
		}

		// If player it not viewing anyone, force them..
		if (!player->client->sess.specInvited &&
			!(player->client->sess.spectatorClient == SPECTATOR_FOLLOW)) {
			player->client->sess.spectatorClient = ent->client->ps.clientNum;
			player->client->sess.spectatorState = SPECTATOR_FOLLOW;
		}

		player->client->sess.specInvited |= team;

		// Notify sender/recipient
		CP(va("print \"%s^7 has been sent a spectator invitation.\n\"", player->client->pers.netname));
		CPx(player - g_entities, va("cp \"%s ^7invited you to spec the %s team.\n\"2",
			ent->client->pers.netname, aTeams[team]));

	}
	else { CP("print \"Spectators can't specinvite players!\n\""); }
}

/*
===================
unInvite player from spectating
===================
*/
void cmd_specUnInvite(gentity_t *ent) {
	int	target;
	gentity_t	*player;
	char arg[MAX_TOKEN_CHARS];
	int team = ent->client->sess.sessionTeam;

	if (team_nocontrols.integer) {
		CP("print \"Team commands are not enabled on this server.\n\"");
		return;
	}

	if (team == TEAM_RED || team == TEAM_BLUE) {
		if (!teamInfo[team].spec_lock) {
			CP("print \"Your team isn't locked from spectators!\n\"");
			return;
		}

		trap_Argv(1, arg, sizeof(arg));
		if ((target = ClientNumberFromString(ent, arg)) == -1) {
			return;
		}

		player = g_entities + target;

		// Can't uninvite self
		if (player->client == ent->client) {
			CP("print \"You can't specuninvite yourself!\n\"");
			return;
		}

		// Can't uninvite an active player.
		if (player->client->sess.sessionTeam != TEAM_SPECTATOR) {
			CP("print \"You can't specuninvite a non-spectator!\n\"");
			return;
		}

		// Can't uninvite a already speclocked player
		if (player->client->sess.specInvited < team) {
			CP(va("print \"%s ^7already can't spectate your team!\n\"", ent->client->pers.netname));
			return;
		}

		player->client->sess.specInvited &= ~team;
		G_updateSpecLock(team, qtrue);

		// Notify sender/recipient
		CP(va("print \"%s^7 can't any longer spectate your team.\n\"", player->client->pers.netname));
		CPx(player->client->ps.clientNum, va("print \"%s ^7has revoked your ability to spectate the %s team.\n\"",
			ent->client->pers.netname, aTeams[team]));

	}
	else { CP("print \"Spectators can't specuninvite players!\n\""); }
}

/*
===================
Revoke ability from all players to spectate
===================
*/
void cmd_uninviteAll(gentity_t *ent) {
	int team = ent->client->sess.sessionTeam;

	if (team_nocontrols.integer) {
		CP("print \"Team commands are not enabled on this server.\n\"");
		return;
	}

	if (team == TEAM_RED || team == TEAM_BLUE) {
		if (!teamInfo[team].spec_lock) {
			CP("print \"Your team isn't locked from spectators!\n\"");
			return;
		}

		// Remove all specs
		G_removeSpecInvite(team);

		// Notify that team only that specs lost privilage
		TP(team, va("chat \"^3TEAM NOTICE: ^7%s ^7has revoked ALL spec's invites for your team.\n\"", ent->client->pers.netname));
		// Inform specs..
		TP(TEAM_SPECTATOR, va("print \"%s ^7revoked ALL spec invites from %s team.\n\"", ent->client->pers.netname, aTeams[team]));

	}
	else { CP("print \"Spectators can't specuninviteall!\n\""); }

}

/*
===================
Spec lock/unlock team
===================
*/
void cmd_speclock(gentity_t *ent, qboolean lock) {
	int team = ent->client->sess.sessionTeam;

	if (team_nocontrols.integer) {
		CP("print \"Team commands are not enabled on this server.\n\"");
		return;
	}

	if (team == TEAM_RED || team == TEAM_BLUE) {
		if ((lock && teamInfo[team].spec_lock) || (!lock && !teamInfo[team].spec_lock)) {
			CP(va("print \"Your team is already %s spectators!\n\"",
				(!lock ? "unlocked for" : "locked from")));
			return;
		}

		G_updateSpecLock(team, lock);
		AP(va("cp \"%s is now ^3SPEC%s\"2", aTeams[team], (lock ? "LOCKED" : "UNLOCKED")));

		if (lock) {
			CP("print \"Use ^3specinvite^7 to invite people to spectate.\n\"");
		}
	}
	else { CP(va("print \"Spectators can't use ^3spec%s ^7command!\n\"", (lock ? "lock" : "unlock"))); }
}
