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
OSPx - g_stats.c

Mostly a ET code dump with few modifications..

Created: 3 May / 2014
===========================================================================
*/
#include "g_local.h"


int iWeap = WS_MAX;

static const weap_ws_convert_t aWeapMOD[MOD_NUM_MODS] = {
	{ MOD_UNKNOWN,              WS_MAX },
	{ MOD_MACHINEGUN,           WS_MG42 },
	{ MOD_GRENADE,              WS_GRENADE },
	{ MOD_ROCKET,               WS_PANZERFAUST },

	{ MOD_KNIFE2,               WS_KNIFE },
	{ MOD_KNIFE,                WS_KNIFE },
	{ MOD_KNIFE_STEALTH,        WS_KNIFE },
	{ MOD_LUGER,                WS_LUGER },
	{ MOD_COLT,                 WS_COLT },
	{ MOD_MP40,                 WS_MP40 },
	{ MOD_THOMPSON,             WS_THOMPSON },
	{ MOD_STEN,                 WS_STEN },
//	{ MOD_GARAND,               WS_RIFLE },
	{ MOD_SNIPERRIFLE,          WS_RIFLE },
	{ MOD_FG42,                 WS_FG42 },
	{ MOD_FG42SCOPE,            WS_FG42 },
	{ MOD_PANZERFAUST,          WS_PANZERFAUST },
	{ MOD_GRENADE_LAUNCHER,     WS_GRENADE },
	{ MOD_FLAMETHROWER,         WS_FLAMETHROWER },
	{ MOD_VENOM,				WS_VENOM },
	{ MOD_GRENADE_PINEAPPLE,    WS_GRENADE },

	{ MOD_DYNAMITE,             WS_DYNAMITE },
	{ MOD_AIRSTRIKE,            WS_AIRSTRIKE },
	{ MOD_SYRINGE,              WS_SYRINGE },
	{ MOD_ARTY,                 WS_ARTILLERY }
};

// Get right stats index based on weapon mod
unsigned int G_weapStatIndex_MOD( int iWeaponMOD ) {
	unsigned int i;

	for ( i = 0; i < MOD_NUM_MODS; i++ ) if ( iWeaponMOD == aWeapMOD[i].iWeapon ) {
			return( aWeapMOD[i].iWS );
		}
	return( WS_MAX );
}

// +wstats
char *G_createStats( gentity_t *refEnt ) {
	unsigned int i, dwWeaponMask = 0, dwSkillPointMask = 0;
	char strWeapInfo[MAX_STRING_CHARS] = {0};
	char strSkillInfo[MAX_STRING_CHARS] = {0};

	if ( !refEnt ) {
		return( NULL );
	}	

	// Add weapon stats as necessary
	for ( i = WS_KNIFE; i < WS_MAX; i++ ) {
		if ( refEnt->client->sess.aWeaponStats[i].atts || refEnt->client->sess.aWeaponStats[i].hits ||
			 refEnt->client->sess.aWeaponStats[i].deaths ) {
			dwWeaponMask |= ( 1 << i );
			Q_strcat( strWeapInfo, sizeof( strWeapInfo ), 
						va( " %d %d %d %d %d",
							refEnt->client->sess.aWeaponStats[i].hits, refEnt->client->sess.aWeaponStats[i].atts,
							refEnt->client->sess.aWeaponStats[i].kills, refEnt->client->sess.aWeaponStats[i].deaths,
							refEnt->client->sess.aWeaponStats[i].headshots ) );
		}
	}

	// Additional info
	Q_strcat( strWeapInfo, sizeof( strWeapInfo ), 
				va( " %d %d %d %d",
					refEnt->client->sess.damage_given,
					refEnt->client->sess.damage_received,
					refEnt->client->sess.team_damage,
					refEnt->client->sess.gibs) );	

	return( va( "%d %d %d%s %d%s", 
				(int)(refEnt - g_entities),
				refEnt->client->sess.rounds,
				dwWeaponMask,
				strWeapInfo,
				dwSkillPointMask,
				strSkillInfo) );		
}

// OSPx - Typical "1.0" info based stats (+stats)
char *G_createClientStats( gentity_t *refEnt ) {	
	char strClientInfo[MAX_STRING_CHARS] = {0};

	if ( !refEnt ) {
		return( NULL );
	}	

	// Info
	Q_strcat( strClientInfo, sizeof( strClientInfo ), 
		va( "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			refEnt->client->sess.kills,
			refEnt->client->sess.headshots,
			refEnt->client->sess.deaths,
			refEnt->client->sess.team_kills,
			refEnt->client->sess.suicides,
			refEnt->client->sess.acc_shots,
			refEnt->client->sess.acc_hits,
			refEnt->client->sess.damage_given,
			refEnt->client->sess.damage_received,
			refEnt->client->sess.team_damage,
			refEnt->client->sess.gibs,
			refEnt->client->sess.med_given,
			refEnt->client->sess.ammo_given,
			refEnt->client->sess.revives,
			refEnt->client->sess.killPeak
			));	

	return( va( "%d %s", (int)(refEnt - g_entities), strClientInfo) );		
}

// Sends a player's stats to the requesting client.
void G_statsPrint( gentity_t *ent, int nType ) {
	int pid;
	char *cmd = ( nType == 0 ) ? "ws" : ( "wws" /* ( nType == 1 ) ? "wws" : "gstats" */ );   // Yes, not the cleanest
	char arg[MAX_TOKEN_CHARS];

	if ( !ent || ( ent->r.svFlags & SVF_BOT ) ) {
		return;
	}

	// If requesting stats for self, its easy.
	if ( trap_Argc() < 2 ) {
		if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
			CP( va( "%s %s\n", cmd, G_createStats( ent ) ) );
			// Specs default to players they are chasing
		} else if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			CP( va( "%s %s\n", cmd, G_createStats( g_entities + ent->client->sess.spectatorClient ) ) );
		} else {
			CP( "print \"Info: ^7Type ^3\\wstats <player_id>^7 to see stats on an active player.\n\"" );
			return;
		}
	} else {
		// Find the player to poll stats.
		trap_Argv( 1, arg, sizeof( arg ) );
		if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
			return;
		}
		CP( va( "%s %s\n", cmd, G_createStats( g_entities + pid ) ) );
	}
}

// Sends a player's stats to the requesting client.
void G_clientStatsPrint( gentity_t *ent, int nType, qboolean toWindow ) {
	int pid;
	char *cmd = (toWindow) ? "cgs" : "cgsp"; 
	char arg[MAX_TOKEN_CHARS];

	if ( !ent || ( ent->r.svFlags & SVF_BOT ) ) {
		return;
	}

	// If requesting stats for self, its easy.
	if ( trap_Argc() < 2 ) {
		if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
			CP( va( "%s %s\n", cmd, G_createClientStats( ent ) ) );
			// Specs default to players they are chasing
		} else if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			CP( va( "%s %s\n", cmd, G_createClientStats( g_entities + ent->client->sess.spectatorClient ) ) );
		} else {
			CP( "print \"Info: ^7Type ^3\\stats <player_id>^7 to see stats on an active player.\n\"" );
			return;
		}
	} else {
		// Find the player to poll stats.
		trap_Argv( 1, arg, sizeof( arg ) );
		if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
			return;
		}
		CP( va( "%s %s\n", cmd, G_createClientStats( g_entities + pid ) ) );
	}
}

// Records accuracy, damage, and kill/death stats.
void G_addStats( gentity_t *targ, gentity_t *attacker, int dmg_ref, int mod ) {
	int dmg, ref;

	// Keep track of only active player-to-player interactions in a real game
	if ( !targ || !targ->client ||
		 g_gamestate.integer != GS_PLAYING ||
		 mod == MOD_ADMKILL ||
		 mod == MOD_SWITCHTEAM ||
		 ( g_gametype.integer >= GT_WOLF && ( targ->client->ps.pm_flags & PMF_LIMBO ) ) ||
		 ( g_gametype.integer < GT_WOLF && ( targ->s.eFlags == EF_DEAD || targ->client->ps.pm_type == PM_DEAD ) ) ) {
		return;
	}

	// Special hack for intentional gibbage
	if ( targ->health <= 0 && targ->client->ps.pm_type == PM_DEAD ) {
		if ( mod < MOD_CROSS && attacker && attacker->client ) {
			int x = attacker->client->sess.aWeaponStats[G_weapStatIndex_MOD( mod )].atts--;
			if ( x < 1 ) {
				attacker->client->sess.aWeaponStats[G_weapStatIndex_MOD( mod )].atts = 1;
			}
		}
		return;
	}

	// Suicides only affect the player specifically
	if ( targ == attacker || !attacker || !attacker->client || mod == MOD_SUICIDE || mod == MOD_SELFKILL ) {	
		if ( !attacker || !attacker->client )
		return;
	}

	// Telefrags only add 100 points.. not 100k!!
	if ( mod == MOD_TELEFRAG ) {
		dmg = 100;
	} else { dmg = dmg_ref;}

	// Player team stats
	if ( g_gametype.integer >= GT_WOLF &&
		 targ->client->sess.sessionTeam == attacker->client->sess.sessionTeam ) {
		attacker->client->sess.team_damage += dmg;
		// Don't count self kill as team kill..because it ain't!
		if ( targ->health <= 0 && !(mod == MOD_SUICIDE || mod == MOD_SELFKILL)) {
			attacker->client->sess.team_kills++;
			targ->client->sess.deaths++;	// Record death when TK occurs
		}
		return;
	}

	// General player stats
	if ( mod != MOD_SYRINGE ) {
		attacker->client->sess.damage_given += dmg;
		targ->client->sess.damage_received += dmg;
		if ( targ->health <= 0 ) {
			attacker->client->sess.kills++;
			targ->client->sess.deaths++;

			// OSPx - Life(s) Kill peak
			if (attacker->client->pers.life_kills > attacker->client->sess.killPeak)
				attacker->client->sess.killPeak++;
		}
	}

	// Player weapon stats
	ref = G_weapStatIndex_MOD( mod );
	if ( dmg > 0 ) {
		attacker->client->sess.aWeaponStats[ref].hits++;
	}
	if ( targ->health <= 0 ) {
		attacker->client->sess.aWeaponStats[ref].kills++;
		targ->client->sess.aWeaponStats[ref].deaths++;
	}
}

// Records weapon headshots
void G_addStatsHeadShot( gentity_t *attacker, int mod ) {
	if ( g_gamestate.integer != GS_PLAYING ) {
		return;
	}

	if ( !attacker || !attacker->client ) {
		return;
	}
	attacker->client->sess.aWeaponStats[G_weapStatIndex_MOD( mod )].headshots++;
	// Store headshot in session as well for overall count
	attacker->client->sess.headshots++;
}

// Resets player's current stats
void G_deleteStats( int nClient ) {
	gclient_t *cl = &level.clients[nClient];

	cl->sess.damage_given = 0;
	cl->sess.damage_received = 0;
	cl->sess.deaths = 0;
	cl->sess.rounds = 0;
	cl->sess.kills = 0;
	cl->sess.suicides = 0;
	cl->sess.team_damage = 0;
	cl->sess.team_kills = 0;
	cl->sess.headshots = 0;
	cl->sess.med_given = 0;
	cl->sess.ammo_given = 0;
	cl->sess.gibs = 0;
	cl->sess.revives = 0;
	cl->sess.acc_hits = 0;
	cl->sess.acc_shots = 0;
	cl->sess.killPeak = 0;

	memset( &cl->sess.aWeaponStats, 0, sizeof( cl->sess.aWeaponStats ) );
	trap_Cvar_Set( va( "wstats%i", nClient ), va( "%d", nClient ) );
}

// Parses weapon stat info for given ent
//	---> The given string must be space delimited and contain only integers
void G_parseStats( char *pszStatsInfo ) {
	gclient_t *cl;
	const char *tmp = pszStatsInfo;
	unsigned int i, dwWeaponMask, dwClientID = atoi( pszStatsInfo );

	if ( dwClientID < 0 || dwClientID > MAX_CLIENTS ) {
		return;
	}

	cl = &level.clients[dwClientID];

#define GETVAL( x ) if ( ( tmp = strchr( tmp, ' ' ) ) == NULL ) {return;} x = atoi( ++tmp );

	GETVAL( cl->sess.rounds );
	GETVAL( dwWeaponMask );
	for ( i = WS_KNIFE; i < WS_MAX; i++ ) {
		if ( dwWeaponMask & ( 1 << i ) ) {
			GETVAL( cl->sess.aWeaponStats[i].hits );
			GETVAL( cl->sess.aWeaponStats[i].atts );
			GETVAL( cl->sess.aWeaponStats[i].kills );
			GETVAL( cl->sess.aWeaponStats[i].deaths );
			GETVAL( cl->sess.aWeaponStats[i].headshots );
		}
	}

	GETVAL( cl->sess.damage_given );
	GETVAL( cl->sess.damage_received );
	GETVAL( cl->sess.team_damage );
	GETVAL( cl->sess.deaths );
	GETVAL( cl->sess.kills );
	GETVAL( cl->sess.suicides );
	GETVAL( cl->sess.team_kills );
	GETVAL( cl->sess.headshots );
	GETVAL( cl->sess.med_given );
	GETVAL( cl->sess.ammo_given );
	GETVAL( cl->sess.gibs );
	GETVAL( cl->sess.revives );
	GETVAL( cl->sess.acc_shots );
	GETVAL( cl->sess.acc_hits );
	GETVAL( cl->sess.killPeak );
}

// These map to WS_* weapon indexes
// OSPx: In other words...min shots before it qualifies for top-bottom check..
const int cQualifyingShots[WS_MAX] = {
	20,     // Knife
	14,     // Luger
	14,     // Colt
	32,     // MP40
	30,     // Thompson
	32,     // STEN
	30,     // FG42 (rapid sniper mode)
	3,      // PF
	100,    // Flamer
	5,      // Grenade
	5,      // Mortar (Was I on drugs or am I missing something?)
	5,      // Dynamite
	3,      // Airstrike
	3,      // Artillery
	5,      // Syringe
	3,      // Smoke (Completelly useless..or maybe for "AS cannister kill" when blocking it?)
	50,     // MG42
	10,     // Rifle (sniper/mauser aka scopped-unscopped)
	100		// Venom
};

// ************** TOPSHOTS/BOTTOMSHOTS
//
// Gives back overall or specific weapon rankings
int QDECL SortStats( const void *a, const void *b ) {
	gclient_t   *ca, *cb;
	float accA, accB;

	ca = &level.clients[*(int *)a];
	cb = &level.clients[*(int *)b];

	// then connecting clients
	if ( ca->pers.connected == CON_CONNECTING ) {
		return( 1 );
	}
	if ( cb->pers.connected == CON_CONNECTING ) {
		return( -1 );
	}

	if ( ca->sess.sessionTeam == TEAM_SPECTATOR ) {
		return( 1 );
	}
	if ( cb->sess.sessionTeam == TEAM_SPECTATOR ) {
		return( -1 );
	}

	if ( ( ca->sess.aWeaponStats[iWeap].atts ) < cQualifyingShots[iWeap] ) {
		return( 1 );
	}
	if ( ( cb->sess.aWeaponStats[iWeap].atts ) < cQualifyingShots[iWeap] ) {
		return( -1 );
	}

	accA = (float)( ca->sess.aWeaponStats[iWeap].hits * 100.0 ) / (float)( ca->sess.aWeaponStats[iWeap].atts );
	accB = (float)( cb->sess.aWeaponStats[iWeap].hits * 100.0 ) / (float)( cb->sess.aWeaponStats[iWeap].atts );

	// then sort by accuracy
	if ( accA > accB ) {
		return( -1 );
	}
	return( 1 );
}

// Shows the most accurate players for each weapon to the requesting client
void G_weaponStatsLeaders_cmd( gentity_t* ent, qboolean doTop, qboolean doWindow ) {
	int i, iWeap, shots, wBestAcc, cClients, cPlaces;
	int aClients[MAX_CLIENTS];
	float acc;
	char z[MAX_STRING_CHARS];
	const gclient_t* cl;

	z[0] = 0;
	for ( iWeap = WS_KNIFE; iWeap < WS_MAX; iWeap++ ) {
		wBestAcc = ( doTop ) ? 0 : 99999;
		cClients = 0;
		cPlaces = 0;

		// suckfest - needs two passes, in case there are ties
		for ( i = 0; i < level.numConnectedClients; i++ ) {
			cl = &level.clients[level.sortedClients[i]];

			if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
				continue;
			}

			shots = cl->sess.aWeaponStats[iWeap].atts;
			if ( shots >= cQualifyingShots[iWeap] ) {
				acc = (float)( ( cl->sess.aWeaponStats[iWeap].hits ) * 100.0 ) / (float)shots;
				aClients[cClients++] = level.sortedClients[i];
				if ( ( ( doTop ) ? acc : (float)wBestAcc ) > ( ( doTop ) ? wBestAcc : acc ) ) {
					wBestAcc = (int)acc;
					cPlaces++;
				}
			}
		}

		if ( !doTop && cPlaces < 2 ) {
			continue;
		}

		for ( i = 0; i < cClients; i++ ) {
			cl = &level.clients[ aClients[i] ];
			acc = (float)( cl->sess.aWeaponStats[iWeap].hits * 100.0 ) / (float)( cl->sess.aWeaponStats[iWeap].atts );

			if ( ( ( doTop ) ? acc : (float)wBestAcc + 0.999 ) >= ( ( doTop ) ? wBestAcc : acc ) ) {
				Q_strcat( z, sizeof( z ), va( " %d %d %d %d %d %d", iWeap + 1, aClients[i],
											  cl->sess.aWeaponStats[iWeap].hits,
											  cl->sess.aWeaponStats[iWeap].atts,
											  cl->sess.aWeaponStats[iWeap].kills,
											  cl->sess.aWeaponStats[iWeap].deaths ) );
			}
		}
	}
	CP( va( "%sbstats%s %s 0", ( ( doWindow ) ? "w" : "" ), ( ( doTop ) ? "" : "b" ), z ) );	
}

// ************** STATSALL
//
// Shows all players' stats to the requesting client.
void G_statsall_cmd( gentity_t *ent, unsigned int dwCommand, qboolean fDump ) {
	int i;
	gentity_t *player;

	for ( i = 0; i < level.numConnectedClients; i++ ) {
		player = &g_entities[level.sortedClients[i]];
		if ( player->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		CP( va( "ws %s\n", G_createStats( player ) ) );
	}
}


// Shows best/worst accuracy for all weapons, or sorted
// accuracies for a single weapon.
void G_weaponRankings_cmd( gentity_t *ent, unsigned int dwCommand, qboolean state ) {
	gclient_t *cl;
	int c = 0, i, shots, wBestAcc;
	char z[MAX_STRING_CHARS];

	if ( trap_Argc() < 2 ) {
		G_weaponStatsLeaders_cmd( ent, state, qfalse );
		return;
	}

	wBestAcc = ( state ) ? 0 : 99999;

	// Find the weapon
	trap_Argv( 1, z, sizeof( z ) );
	if ( ( iWeap = atoi( z ) ) == 0 || iWeap < WS_KNIFE || iWeap >= WS_MAX ) {
		for (iWeap = WS_VENOM; iWeap >= WS_KNIFE; iWeap--) {
			if ( !Q_stricmp( z, aWeaponInfo[iWeap].pszCode ) ) {
				break;
			}
		}
	}

	if ( iWeap < WS_KNIFE ) {		
		CP( va( "print \"\n^3Info: %s\n\n\"",   
			(state ? 
				"^7 Shows BEST player for each weapon. Add ^3<weapon_ID>^7 to show all stats for a weapon " : 
				"^7 Shows WORST player for each weapon. Add ^3<weapon_ID>^7 to show all stats for a weapon" ) 
		));

		Q_strncpyz( z, "^3Available weapon codes:^7\n", sizeof( z ) );
		for ( i = WS_KNIFE; i < WS_MAX; i++ ) {
			Q_strcat( z, sizeof( z ), va( "  %s - %s\n", aWeaponInfo[i].pszCode, aWeaponInfo[i].pszName ) );
		}
		CP( va( "print \"%s\"", z ) );
		return;
	}

	memcpy( &level.sortedStats, &level.sortedClients, sizeof( level.sortedStats ) );
	qsort( level.sortedStats, level.numConnectedClients, sizeof( level.sortedStats[0] ), SortStats );

	z[0] = 0;
	for ( i = 0; i < level.numConnectedClients; i++ ) {
		cl = &level.clients[level.sortedStats[i]];

		if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		shots = cl->sess.aWeaponStats[iWeap].atts;
		if ( shots >= cQualifyingShots[iWeap] ) {
			float acc = (float)( cl->sess.aWeaponStats[iWeap].hits * 100.0 ) / (float)shots;

			c++;
			wBestAcc = ( ( ( state ) ? acc : wBestAcc ) > ( ( state ) ? wBestAcc : acc ) ) ? (int)acc : wBestAcc;
			Q_strcat( z, sizeof( z ), va( " %d %d %d %d %d", level.sortedStats[i],
										  cl->sess.aWeaponStats[iWeap].hits,
										  shots,
										  cl->sess.aWeaponStats[iWeap].kills,
										  cl->sess.aWeaponStats[iWeap].deaths ) );
		}
	}
	CP( va( "astats%s %d %d %d%s", ( ( state ) ? "" : "b" ), c, iWeap, wBestAcc, z ) );
}

// Prints current player match info.
void G_printMatchInfo(gentity_t *ent, qboolean time) {
	int i, j, cnt, eff;
	float tot_acc = 0.00f;
	int tot_kills, tot_deaths, tot_gp, tot_hs, tot_sui, tot_tk, tot_dg, tot_dr, tot_td, tot_hits, tot_shots;
	gclient_t *cl;
	char *ref;
	char n1[MAX_NETNAME];
	
	CP(va("sc \"\nMod: %s \n^7Server: %s %s\n\n\"",
		GAMEVERSION, sv_hostname.string, ( time ? va("\n^7Time: ^3%s", getTime()) : "" ) ));

	cnt = 0;
	for (i = TEAM_RED; i <= TEAM_BLUE; i++) {
		if (!TeamCount(-1, i)) {
			continue;
		}

		tot_kills = 0;
		tot_deaths = 0;
		tot_hs = 0;
		tot_sui = 0;
		tot_tk = 0;
		tot_dg = 0;
		tot_dr = 0;
		tot_td = 0;
		tot_gp = 0;
		tot_hits = 0;
		tot_shots = 0;
		tot_acc = 0;

		CP(va("sc \"%s ^7Team\n"
			"^7----------------------------------------------------------------------"
			"\nPlayer          Kll Dth Sui TK ^2Eff Accrcy   HS    ^5DG    DR   TD  ^3Score\n"
			"^7----------------------------------------------------------------------\n\"", (i == TEAM_RED) ? "^1Axis" : "^4Allied"));

		for (j = 0; j < level.numPlayingClients; j++) {
			cl = level.clients + level.sortedClients[j];

			if (cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam != i) {
				continue;
			}
									
			SanitizeString(cl->pers.netname, n1, qtrue);
			Q_CleanStr(n1);						

			ref = "^7";
			tot_kills += cl->sess.kills;
			tot_deaths += cl->sess.deaths;
			tot_sui += cl->sess.suicides;
			tot_tk += cl->sess.team_kills;
			tot_hs += cl->sess.headshots;
			tot_dg += cl->sess.damage_given;
			tot_dr += cl->sess.damage_received;
			tot_td += cl->sess.team_damage;
			tot_gp += cl->ps.persistant[PERS_SCORE];
			tot_hits += cl->sess.acc_hits;
			tot_shots += cl->sess.acc_shots;

			eff = (cl->sess.deaths + cl->sess.kills == 0) ? 0 : 100 * cl->sess.kills / (cl->sess.deaths + cl->sess.kills);
			if (eff < 0) {
				eff = 0;
			}

			if (ent->client == cl ||
				(ent->client->sess.sessionTeam == TEAM_SPECTATOR &&
				ent->client->sess.spectatorState == SPECTATOR_FOLLOW &&
				ent->client->sess.spectatorClient == level.sortedClients[j])) {
				ref = "^7";
			}

			cnt++;
			CP(va("sc \"%s%-15s%4d%4d%4d%3d%s^2%4d %6.2f%5d^5%6d%6d%5d^3%7d\n\"",
				ref,
				n1,
				cl->sess.kills,
				cl->sess.deaths,
				cl->sess.suicides,
				cl->sess.team_kills,
				ref,
				eff,
				((cl->sess.acc_shots == 0) ? 0.00 : ((float)cl->sess.acc_hits / (float)cl->sess.acc_shots) * 100.00f),
				cl->sess.headshots,
				cl->sess.damage_given,
				cl->sess.damage_received,
				cl->sess.team_damage,
				cl->ps.persistant[PERS_SCORE]));
		}

		eff = (tot_kills + tot_deaths == 0) ? 0 : 100 * tot_kills / (tot_kills + tot_deaths);
		if (eff < 0) {
			eff = 0;
		}
		tot_acc = ((tot_shots == 0) ? 0.00 : ((float)tot_hits / (float)tot_shots) * 100.00f);

		CP(va("sc \"^7----------------------------------------------------------------------\n"
			"%-19s%4d%4d%4d%3d^2%4d %6.2f%5d^5%6d%6d%5d^3%7d\n\n\"",
			"^3Totals^7",
			tot_kills,
			tot_deaths,
			tot_sui,
			tot_tk,
			eff,
			tot_acc,
			tot_hs,
			tot_dg,
			tot_dr,
			tot_td,
			tot_gp));
	}
	CP(va("sc \"%s\n\" 0", ((!cnt) ? "^3\nNo scores to report." : "")));
}
