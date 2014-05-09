/*/
===========================================================================
OSPx :: g_admin_cmds.c

Admin commands.

Created: 2 May / 2014
===========================================================================
*/
#include "g_admin.h"

/*
===========
Login

NOTE: Hooked under g_cmds.c
===========
*/
void cmd_doLogin(gentity_t *ent, qboolean silent) {
	char str[MAX_TOKEN_CHARS];
	qboolean error;
	char *log;

	error = qfalse;
	trap_Argv(1, str, sizeof(str));

	// Make sure user is not already logged in.
	if (ent->client->sess.admin >= ADMIN_1) {
		CP("print \"^1Error: ^7You are already logged in!\n\"");
		return;
	}

	// Prevent bogus logins	
	if ((!Q_stricmp(str, "\0"))
		|| (!Q_stricmp(str, ""))
		|| (!Q_stricmp(str, "\""))
		|| (!Q_stricmp(str, "none")))
	{
		CP("print \"^1Error: ^7Incorrect password!\n\"");
		// No log here to avoid login by error..
		return;
	}

	// Else let's see if there's a password match.
	if ((Q_stricmp(str, a1_pass.string) == 0)
		|| (Q_stricmp(str, a2_pass.string) == 0)
		|| (Q_stricmp(str, a3_pass.string) == 0)
		|| (Q_stricmp(str, a4_pass.string) == 0)
		|| (Q_stricmp(str, a5_pass.string) == 0))
	{
		// Always start with lower level as if owner screws it up 
		// and sets the same passes for more levels, the lowest is the safest bet.
		if (Q_stricmp(str, a1_pass.string) == 0) {
			ent->client->sess.admin = ADMIN_1;
		}
		else if (Q_stricmp(str, a2_pass.string) == 0) {
			ent->client->sess.admin = ADMIN_2;
		}
		else if (Q_stricmp(str, a3_pass.string) == 0) {
			ent->client->sess.admin = ADMIN_3;
		}
		else if (Q_stricmp(str, a4_pass.string) == 0) {
			ent->client->sess.admin = ADMIN_4;
		}
		else if (Q_stricmp(str, a5_pass.string) == 0) {
			ent->client->sess.admin = ADMIN_5;
		}
		else {
			error = qtrue;
		}
		// Something went to hell..
		if (error == qtrue) {
			// User shouldn't be anything but regular so make sure..
			ent->client->sess.admin = USER_REGULAR;
			CP("print \"^1Error: ^7Error has occured while trying to log you in!\n\"");
			return;
		}

		// We came so far so go with it..
		if (silent) {
			CP("print \"^3Info: ^7Silent Login successful!\n\"");
			ent->client->sess.incognito = 1; // Hide them

			// Log it
			log = va("Time: %s\nPlayer %s (IP: %s) has silently logged in as %s.%s",
				getTime(),
				ent->client->pers.netname,
				clientIP(ent, qtrue),
				sortTag(ent),
				LOGLINE);

			if (g_extendedLog.integer)
				logEntry(ADMLOG, log);
		}
		else {
			AP(va("chat \"^7console: %s ^7has ^3logged in^7 as %s^7.\n\"", ent->client->pers.netname, sortTag(ent)));

			// Log it
			log = va("Time: %s\nPlayer %s (IP: %s) has logged in as %s.%s",
				getTime(), ent->client->pers.netname, clientIP(ent, qtrue), sortTag(ent), LOGLINE);

			if (g_extendedLog.integer)
				logEntry(ADMLOG, log);
		}

		// Make sure logged in user bypasses any spec lock instantly.
		ent->client->sess.specInvited = 3;

		return;
	}
	else
	{
		CP("print \"^1Error: ^7Incorrect password!\n\"");

		// Log it
		log = va("Time: %s\nPlayer %s (IP: %s) has tried to login using password: %s%s",
			getTime(), ent->client->pers.netname, clientIP(ent, qtrue), str, LOGLINE);

		if (g_extendedLog.integer)
			logEntry(PASSLOG, log);

		return;
	}
}

/*
===========
Logout

NOTE: Hooked under g_cmds.c
===========
*/
void cmd_doLogout(gentity_t *ent) {

	// If user is not logged in do nothing
	if (ent->client->sess.admin == USER_REGULAR) {
		return;
	}
	else {
		// Admin is hidden so don't print 
		if (ent->client->sess.incognito)
			CP("print \"^3Info: ^7You have successfully logged out!\n\"");
		else
			AP(va("chat \"console: %s ^7has ^3logged out^7.\n\"", ent->client->pers.netname));

		// Log them out now
		ent->client->sess.admin = USER_REGULAR;

		// Set incognito to visible..
		ent->client->sess.incognito = 0;

		// Clear speclock		
		ent->client->sess.specInvited = 0;

		// Black out client if needed
		G_setClientSpeclock( ent );
		return;
	}
}

/*
===========
Deals with customm commands

NOTE: Only called directly from cmds_admin..
===========
*/
void cmd_custom(gentity_t *ent) {
	char *log;
	
	if (!strcmp(ent->client->pers.cmd2, "")) {
		CP(va("print \"^1Error^7: Command ^1%s ^7must have a value!\n\"", ent->client->pers.cmd1));
		return;
	}
	else {
		// Rconpasswords or (allowed) sensitve commands can be changed without public print..
		if (!strcmp(ent->client->pers.cmd3, "@"))
			CP(va("print \"Info: ^2%s ^7was silently changed to ^2%s^7!\n\"", ent->client->pers.cmd1, ent->client->pers.cmd2));
		else
			AP(va("chat \"console: %s ^7changed ^3%s ^7to ^3%s %s\n\"", sortTag(ent), ent->client->pers.cmd1, ent->client->pers.cmd2, ent->client->pers.cmd3));

		// Change the stuff
		trap_SendConsoleCommand(EXEC_APPEND, va("%s %s %s", ent->client->pers.cmd1, ent->client->pers.cmd2, ent->client->pers.cmd3));

		// Log it
		log = va("Player %s (IP: %s) has changed %s to %s %s.",
			ent->client->pers.netname, clientIP(ent, qtrue), ent->client->pers.cmd1, ent->client->pers.cmd2, ent->client->pers.cmd3);
		admLog(log);

		return;
	}
}

/*
===========
Toggle incognito
===========
*/
void cmd_incognito(gentity_t *ent, qboolean fParam) {
	if (ent->client->sess.admin < ADMIN_1)
		return;

	if (ent->client->sess.incognito == 0) {
		ent->client->sess.incognito = 1;
		CP("cp \"You are now ^3incognito^7!\n\"2");
		return;
	}
	else {
		ent->client->sess.incognito = 0;
		CP("cp \"Your status is now set to ^3visible^7!\n\"2");
		return;
	}
}

/*
===========
Un/Ignore user
===========
*/
void cmd_ignoreHandle(gentity_t *ent, qboolean dIgnore) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *log;
	char *action = (dIgnore ? "ignored" : "unignored");	
	
	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
	if (count == 0){
		CP("print \"^1Error^7: Client not on server!\n\"");
		return;
	}
	else if (count > 1) {
		CP(va("print \"^1Error^7: Too many people with ^1%s ^7in their name!\n\"", ent->client->pers.cmd2));
		return;
	}

	for (i = 0; i < count; i++){

		if (isHigher(ent, &g_entities[nums[i]]))
			return;

		if (g_entities[nums[i]].client->sess.ignored == dIgnore){
			CP(va("print \"^1Error^7: Player %s ^7is already %s!\n\"", g_entities[nums[i]].client->pers.netname, action));
			return;
		}
		else
			g_entities[nums[i]].client->sess.ignored = dIgnore;

		AP(va("chat \"console: %s has ^3%s ^7player %s^7!\n\"", sortTag(ent), action, g_entities[nums[i]].client->pers.netname));

		// Log it
		log = va("Player %s (IP: %s) has %s user %s.",
			ent->client->pers.netname, clientIP(ent, qtrue), action, g_entities[nums[i]].client->pers.netname);
		admLog(log);
	}
	return;
}

/*
===========
Un/Ready all

Ready or unready all..
===========
*/
void cmd_readyHandle(gentity_t *ent, qboolean unready) {
	char *msg = ((unready) ? "^3UNREADY^7" : "^3READY^7");
	char *log;

	if (!g_doWarmup.integer) {
		CP("print \"Tourny mode is disabled! Command ignored..\n\"");
		return;
	}

	if (!unready) {
		if (g_gamestate.integer != GS_WARMUP) {
			CP("print \"^3ALLREADY ^7command can only be used in warmup!\n\"");
			return;
		}
		G_readyStart();
		AP(va("chat \"console: ^7%s has %s ALL players..\n\"", sortTag(ent), msg));
	}
	else {
		if (g_gamestate.integer != GS_WARMUP_COUNTDOWN) {
			CP("print \"^3UNREADYALL ^7command can only be used during countdown!\n\"");
			return;
		}
		G_readyReset(qtrue);
		AP(va("chat \"console: Countdown has been ^3cancelled ^7by %s..\n\"", sortTag(ent)));
	}

	// Log it
	log = va("Player %s (IP: %s) has %s users.",
		ent->client->pers.netname, clientIP(ent, qtrue), msg);
	admLog(log);
}

/*
==================
Speclock/unlock
==================
*/
qboolean specAlready(int team, qboolean lock) {
	if (team > 0 && team < 3) {
		if (teamInfo[team].spec_lock == lock)
			return qtrue;
		else
			return qfalse;
	}
	else if (team == 3) {
		if ((teamInfo[TEAM_RED].spec_lock == lock) &&
			(teamInfo[TEAM_BLUE].spec_lock == lock))
			return qtrue;
		else
			return qfalse;
	}
	return qfalse;
}

void cmd_specHandle(gentity_t *ent, qboolean lock) {
	int team;
	char *act = ((lock) ? "locked" : "unlocked");
	char *log;

	if (!ent->client->pers.cmd2) {
		CP(va("print \"^1Error: ^7You need to specify a team!\nUse ^3?spec%s ^7for help.\n\"", (lock ? "lock" : "unlock")));
		return;
	}

#define STM(x) !(strcmp(ent->client->pers.cmd2,x))

	if (STM("both")) {
		team = 3;
	}
	else if (STM("red") || STM("axis"))	{
		team = TEAM_RED;
	}
	else if (STM("blue") || STM("allied") || STM("allies")) {
		team = TEAM_BLUE;
	}
	else {
		CP(va("print \"^1Error: ^7Unknown argument ^1%s^7!\nUse ^1?spec%s ^7for help.\n\"",
			ent->client->pers.cmd2, (lock ? "lock" : "unlock")));
		return;
	}

	if (specAlready(team, lock)) {
		CP(va("print \"^1Error^7: %s already spec%s!\n\"",
			((team == 3) ? "^3Both ^7teams are" : va("Team %s is", aTeams[team])), act));
		return;
	}

	// Sanity check
	if (lock) {
		if (team == TEAM_BLUE && !level.alliedPlayers) {
			CP(va("print \"^1Error^7: %s team has no players!\n\"", aTeams[team]));
			return;
		}
		else if (team == TEAM_RED && !level.axisPlayers) {
			CP(va("print \"^1Error^7: %s team has no players!\n\"", aTeams[team]));
			return;
		}
		else if (team == TEAM_SPECTATOR && (!level.axisPlayers || !level.alliedPlayers)) {
			CP("print \"^1Error^7: Not all teams have players!\n\"");
			return;
		}
	}

	if (team != 3) {
		G_updateSpecLock(team, lock);
	}
	else {
		G_updateSpecLock(TEAM_RED, lock);
		G_updateSpecLock(TEAM_BLUE, lock);
	}

	aTeams[team] = (team == 3) ? "^3Both^7" : aTeams[team];
	AP(va("chat \"console: ^7%s has spec%s %s team%s\"", sortTag(ent), act, aTeams[team], ((team == 3) ? "s" : "")));

	// Log it
	log = va("Player %s (IP: %s) has %s team(s).",
		ent->client->pers.netname, clientIP(ent, qtrue), act);
	admLog(log);
}

/*
==================
Pause/Unpause
==================
*/
void cmd_pauseHandle(gentity_t *ent, qboolean fPause) {
	char *status[2] = { "^3UN", "^3" };	
	char *log;

	if ((!level.alliedPlayers || !level.axisPlayers) && fPause) {
		CP("print \"^1Error^7: Pause can only be used when both teams have players!\n\"");
		return;
	}

	if ((PAUSE_UNPAUSING >= level.match_pause && !fPause) || (PAUSE_NONE != level.match_pause && fPause)) {
		CP(va("print \"^1Error^7: The match is already %sPAUSED!\n\"", status[fPause]));
		return;
	}

	// Trigger the auto-handling of pauses
	if (fPause) {
		level.match_pause = 100 + ((ent) ? (1 + ent - g_entities) : 0);		
		G_spawnPrintf(DP_PAUSEINFO, level.time + 15000, NULL);
		AP(va("chat \"console: %s ^3PAUSED^7 the match!\n", sortTag(ent)));		
	}
	else {
		AP(va("chat \"console: %s ^3UNPAUSED^7 the match.\n\n\"", sortTag(ent)));
		level.match_pause = PAUSE_UNPAUSING;		
		G_spawnPrintf(DP_UNPAUSING, level.time + 10, NULL);
	}

	// Log it
	log = va("Player %s (IP: %s) has %s a match.",
		ent->client->pers.netname, clientIP(ent, qtrue), (fPause ? "paused" : "resumed") );
	admLog(log);
}

/*
===========
Kick player + optional <msg>
===========
*/
void cmd_kick(gentity_t *ent, qboolean fParam) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *log, *msg;	

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
	if (count == 0){
		CP("print \"^1Error^7: Client not on server!\n\"");
		return;
	}
	else if (count > 1){
		CP(va("print \"^1Error^7: Too many people with ^1%s ^7in their name!\n\"", ent->client->pers.cmd2));
		return;
	}

	msg = (ent->client->pers.cmd3 ? va("\n^3Reason^7: %s", ent->client->pers.cmd3) : "");
	for (i = 0; i < count; i++){
		trap_DropClient(nums[i], va("^3kicked by ^3%s%s", sortTag(ent), msg));
		AP(va("chat \"console: %s has ^3kicked ^7player %s^7!%s\n\"", sortTag(ent), g_entities[nums[i]].client->pers.netname, msg));

		// Log it
		log = va("Player %s (IP: %s) has kicked user %s. %s",
			ent->client->pers.netname, clientIP(ent, qtrue), g_entities[nums[i]].client->pers.netname, ent->client->pers.cmd3);
		logEntry(ADMACT, log);
	}
	return;
}

/*
===========
Kick player based upon clientnumber + optional <msg>
===========
*/
void cmd_clientkick(gentity_t *ent, qboolean fParam) {
	int	player_id;
	gentity_t	*targetclient;
	char *msg, *log;

	player_id = ClientNumberFromString(ent, ent->client->pers.cmd2);
	if (player_id == -1) {
		CP("print \"^1Error^7: Client not on server!\n\"");
		return;
	}

	targetclient = g_entities + player_id;
	msg = (ent->client->pers.cmd3 ? va("\n^3Reason^7: %s", ent->client->pers.cmd3) : "");

	//kick the client
	trap_DropClient(player_id, va("^3kicked by ^3%s%s", sortTag(ent), msg));
	AP(va("chat \"console: %s has ^3kicked^7 player %s^7!%s\n\"", sortTag(ent), targetclient->client->pers.netname, msg));

	// Log it
	log = va("Player %s (IP: %s) has clientkicked user %s. %s",
		ent->client->pers.netname, clientIP(ent, qtrue), targetclient->client->pers.netname, ent->client->pers.cmd3);
	logEntry(ADMACT, log);
	return;
}

/*
===========
Rename player
===========
*/
void cmd_rename(gentity_t *ent, qboolean fParam) {
	int          clientNum;
	gclient_t	 *client;
	gentity_t *target;
	char *log;
	char userinfo[MAX_INFO_STRING];

	clientNum = ClientNumberFromString(ent, ent->client->pers.cmd2);
	if (clientNum == -1) {
		return;
	}

	target = g_entities + clientNum;
	client = target->client;

	// Print first..
	AP(va("chat \"console: %s has renamed player %s ^7to %s!\n\"", sortTag(ent), client->pers.netname, ConcatArgs(3)));

	// Rename..
	trap_GetUserinfo(client->ps.clientNum, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "name", ConcatArgs(3));
	trap_SetUserinfo(client->ps.clientNum, userinfo);
	ClientUserinfoChanged(client->ps.clientNum);

	// Log it
	log = va("Player %s (IP: %s) has renamed user %s",
		ent->client->pers.netname, clientIP(ent, qtrue), ConcatArgs(3));
	// Not vital..
	if (g_extendedLog.integer > 1)
		logEntry(ADMACT, log);
	return;
}

/*
===========
Slap player
===========
*/
void cmd_slap(gentity_t *ent, qboolean fParam) {
	int clientid;
	int damagetodo;
	char *log;
	gentity_t *target;

	clientid = atoi(ent->client->pers.cmd2);
	target = &g_entities[clientid];
	damagetodo = 20; 
	
	if ((clientid < 0) || (clientid >= MAX_CLIENTS) || 
		(!g_entities[clientid].client) || 
		(level.clients[clientid].pers.connected != CON_CONNECTED))	{
		CP("print \"^1Error^7: Invalid client number!\n\"");
		return;
	}
	if (g_entities[clientid].client->sess.sessionTeam == TEAM_SPECTATOR) {
		CP("print \"^1Error^7: You cannot slap a spectator!\n\"");
		return;
	}
	
	if (target->client->ps.stats[STAT_HEALTH] <= 20) {
		G_Damage(target, NULL, NULL, NULL, NULL, damagetodo, DAMAGE_NO_PROTECTION, MOD_ADMKILL);
		AP(va("chat \"console: %s ^7was ^3slapped to death ^7by %s!\n\"", target->client->pers.netname, sortTag(ent)));
		player_die(target, target, target, target->health, MOD_ADMKILL);

		// Log it
		log = va("%s (IP: %s) has slapped to death player %s.", 
			ent->client->pers.netname, clientIP(ent, qtrue), target->client->pers.netname);
	}
	else {
		G_Damage(target, NULL, NULL, NULL, NULL, damagetodo, DAMAGE_NO_PROTECTION, MOD_ADMKILL);
		AP(va("chat \"console: %s ^7was slapped by %s!\n\"", ent->client->pers.netname, sortTag(ent)));
		APRS(target, "sound/multiplayer/vo_revive.wav");
		
		// Log it
		log = va("%s (IP: %s) has slapped player %s.",
			ent->client->pers.netname, clientIP(ent, qtrue), target->client->pers.netname);
	}

	if (g_extendedLog.integer > 1)
		logEntry(ADMACT, log);
	return;
}

/*
===========
Slap player
===========
*/
void cmd_kill(gentity_t *ent, qboolean fParam) {
	int clientid;
	int damagetodo;
	char *log;
	gentity_t *target;

	clientid = atoi(ent->client->pers.cmd2);
	target = &g_entities[clientid];
	damagetodo = 20;

	if ((clientid < 0) || (clientid >= MAX_CLIENTS) ||
		(!g_entities[clientid].client) ||
		(level.clients[clientid].pers.connected != CON_CONNECTED))	{
		CP("print \"^1Error^7: Invalid client number!\n\"");
		return;
	}
	if (g_entities[clientid].client->sess.sessionTeam == TEAM_SPECTATOR) {
		CP("print \"^1Error^7: You cannot kill a spectator!\n\"");
		return;
	}

	if (target->client->ps.stats[STAT_HEALTH] > 0) {
		G_Damage(target, NULL, NULL, NULL, NULL, damagetodo, DAMAGE_NO_PROTECTION, MOD_ADMKILL);
		AP(va("chat \"console: %s ^7was ^3killed ^7by %s!\n\"", target->client->pers.netname, sortTag(ent)));
		player_die(target, target, target, target->health, MOD_ADMKILL);

		// Log it
		log = va("%s (IP: %s) has slapped to death player %s.",
			ent->client->pers.netname, clientIP(ent, qtrue), target->client->pers.netname);
	}
	else {		
		CPx(ent->client->ps.clientNum, va("print \"^3Denied^7: %s ^7is already dead..\n\"", target->client->pers.netname));
		return;
	}

	if (g_extendedLog.integer > 1)
		logEntry(ADMACT, log);
	return;
}

/*
===========
Force user to a desired team..
===========
*/
void cmd_forceToTeam(gentity_t *ent, qboolean tSpecs) {
	int count = 0;
	int i;
	int nums[MAX_CLIENTS];
	char *log;
	int team;

	count = ClientNumberFromNameMatch(ent->client->pers.cmd2, nums);
	if (count == 0){
		CP("print \"^1Error^7: Client not on server!\n\"");
		return;
	}
	else if (count > 1){
		CP(va("print \"^1Error^7: Too many people with ^1%s ^7in their name!\n\"", ent->client->pers.cmd2));
		return;
	}
			
	team = (tSpecs ? TEAM_SPECTATOR : 
		(!Q_stricmp(ent->client->pers.cmd1, "!axis") ? TEAM_RED : TEAM_BLUE));

	for (i = 0; i < count; i++) {
		if (g_entities[nums[i]].client->sess.sessionTeam == team){
			CP(va("print \"^1Error^7: Player %s ^7is already in %s team!\n\"", g_entities[nums[i]].client->pers.netname, aTeams[team]));
			return;
		}
		else
			SetTeam(&g_entities[nums[i]], (tSpecs ? "s" : (team == TEAM_RED ? "r" : "b")), qtrue);
		AP(va("chat \"console: %s has ^3forced^7 player %s ^7to %s^7!\n\"", 
			sortTag(ent), g_entities[nums[i]].client->pers.netname, aTeams[team]));
		
		log = va("Player %s (IP: %s) has forced user %s to %s.",
			ent->client->pers.netname, clientIP(ent, qtrue), g_entities[nums[i]].client->pers.netname), aTeams[team];

		if (g_extendedLog.integer >= 2) 
			logEntry(ADMACT, log);
	}
	return;
}

/*
===========
Execute command (simpleton..)
===========
*/
void cmd_exec(gentity_t *ent, qboolean fParam) {
	char *log;
	
	if (!strcmp(ent->client->pers.cmd3, "@"))
		CP(va("print \"^3Info: ^7%s has been silently executed.\n\"", ent->client->pers.cmd2));
	else
		AP(va("print \"console: %s has ^3executed ^7%s config.\n\"", sortTag(ent), ent->client->pers.cmd2));

	trap_SendConsoleCommand(EXEC_INSERT, va("exec \"%s\"", ent->client->pers.cmd2));

	// Log it
	log = va("Player %s (IP: %s) has executed %s config.",
		ent->client->pers.netname, clientIP(ent, qtrue), ent->client->pers.cmd2);
	logEntry(ADMACT, log);
	return;
}

/*
===========
Nextmap
===========
*/
void cmd_nextmap(gentity_t *ent, qboolean fParam) {
	char *log;

	AP(va("chat \"console:^7 %s has set ^3nextmap ^7in rotation.\n\"", sortTag(ent)));
	trap_SendConsoleCommand(EXEC_APPEND, va("vstr nextmap"));

	log = va("Player %s (IP: %s) has set nextmap.", 	ent->client->pers.netname, clientIP(ent, qtrue));
	if (g_extendedLog.integer >= 2)
		logEntry(ADMACT, log);
	return;
}

/*
===========
Load map
===========
*/
void cmd_map(gentity_t *ent, qboolean fParam) {
	char *log;
	
	AP(va("chat \"console: %s has loaded ^3%s ^7map. \n\"", sortTag(ent), ent->client->pers.cmd2));
	trap_SendConsoleCommand(EXEC_APPEND, va("map %s", ent->client->pers.cmd2));

	// Log it
	log = va("Player %s (IP: %s) has loaded %s map.", ent->client->pers.netname, clientIP(ent, qtrue), ent->client->pers.cmd2);
	logEntry(ADMACT, log);
	return;
}

/*
===========
Vstr

Loads next map in rotation (if any)
===========
*/
void cmd_vstr(gentity_t *ent, qboolean fParam) {
	char *log;

	AP(va("chat \"console:^7 %s set ^3vstr ^7to ^3%s^7.\n\"", sortTag(ent), ent->client->pers.cmd2));
	trap_SendConsoleCommand(EXEC_APPEND, va("vstr %s", ent->client->pers.cmd2));
	
	log = va("Player %s (IP:%i.%i.%i.%i) has set vstr to %s",
		ent->client->pers.netname, clientIP(ent, qtrue), ent->client->pers.cmd2);
	logEntry(ADMACT, log);
	return;
}

/*
===========
Center prints message to all
===========
*/
void cmd_cpa(gentity_t *ent, qboolean fParam) {
	char *s, *log;

	s = ConcatArgs(2);
	AP(va("cp \"%s^7 Issued a Warning!\n%s\n\"3", sortTag(ent), s));
		
	log = va("Player %s (IP: %s) issued CPA warning: %s", ent->client->pers.netname, clientIP(ent, qtrue), s);

	if (g_extendedLog.integer >= 2)
		logEntry(ADMACT, log);
	return;
}

/*
===========
Shows message to selected user in center print
===========
*/
void cmd_cp(gentity_t *ent, qboolean fParam) {
	int	player_id;
	gentity_t	*targetclient;
	char *s, *log;

	s = ConcatArgs(3);

	player_id = ClientNumberFromString(ent, ent->client->pers.cmd2);
	if (player_id == -1) {
		CP("print \"^1Error^7: Client not on server!\n\"");
		return;
	}

	// CP to user	
	targetclient = g_entities + player_id;	
	CPx(targetclient - g_entities, va("cp \"%s ^3Issued you a warning!\n^7%s\n\"3", sortTag(ent), s));

	// Log it
	log = va("Player %s (IP: %s) issued to user %s CP warning: %s",
		ent->client->pers.netname, clientIP(ent, qtrue), targetclient->client->pers.netname, s);

	if (g_extendedLog.integer >= 2) 
		logEntry(ADMACT, log);
	return;
}

/*
===========
Shows message to all in console and center print
===========
*/
void cmd_warn(gentity_t *ent, qboolean fParam) {
	char *s, *log;

	s = ConcatArgs(2);
	AP(va("cp \"%s^7 Issued a Warning!\n%s\n\"3", sortTag(ent), s));
	AP(va("chat \"%s^7 Issued a Warning!\n%s\n\"3", sortTag(ent), s));

	log = va("Player %s (IP: %s) issued a global warning: %s",
		ent->client->pers.netname, clientIP(ent, qtrue), s);

	if (g_extendedLog.integer >= 2) 
		logEntry(ADMACT, log);
	return;
}

/*
===========
Shows message to all in console
===========
*/
void cmd_chat(gentity_t *ent, qboolean fParam) {
	char *s, *log;

	s = ConcatArgs(2);
	AP(va("chat \"%s^7 Issued a Warning!\n%s\n\"3", sortTag(ent), s));

	log = va("Player %s (IP: %s) issued a CHAT warning: %s",
		ent->client->pers.netname, clientIP(ent, qtrue), s);

	if (g_extendedLog.integer >= 2) 
		logEntry(ADMACT, log);
	return;
}
/*
===========
Cancels any vote in progress
===========
*/
void cmd_cancelvote(gentity_t *ent, qboolean fParam) {
	char *log;

	if (level.voteTime) {
		level.voteNo = level.numConnectedClients;
		CheckVote();
		AP(va("cp \"%s has ^3Cancelled ^7the vote.\n\"2", sortTag(ent)));
		AP("chat \"console: Turns out everyone voted ^3No^7!\n\"");

		// Log it
		log = va("Player %s (IP: %s) cancelled a vote.",
			ent->client->pers.netname, clientIP(ent, qtrue));

		if (g_extendedLog.integer >= 2) 
			logEntry(ADMACT, log);
		return;
	}
	else {
		CP("print \"^1Error^7: No vote in progress..\n\"");
	}
	return;
}

/*
===========
Passes any vote in progress
===========
*/
void cmd_passvote(gentity_t *ent, qboolean fParam) {
	char *log;

	if (level.voteTime) {
		level.voteYes = level.numConnectedClients;
		CheckVote();

		AP(va("cp \"%s has ^3Passed ^7the vote.\n\"2", sortTag(ent)));
		AP("chat \"console: Turns out everyone voted ^7Yes^7!\n\"");

		log = va("Player %s (IP: %s) passed a vote.",
			ent->client->pers.netname, clientIP(ent, qtrue));

		if (g_extendedLog.integer >= 2) 
			logEntry(ADMACT, log);
	}
	else {
		CP("print \"^1Error^7: No vote in progress..\n\"");
	}
	return;
}

/*
===========
Map restart
===========
*/
void cmd_restart(gentity_t *ent, qboolean fParam) {
	char *log;

	AP(va("chat \"console: %s has ^3restarted ^7map.\n\"", sortTag(ent)));
	trap_SendConsoleCommand(EXEC_APPEND, va("map_restart"));

	// Log it
	log = va("Player %s (IP: %s) has restarted map.",
		ent->client->pers.netname, clientIP(ent, qtrue));

	if (g_extendedLog.integer >= 2) 
		logEntry(ADMACT, log);
	return;
}

/*
===========
Reset match
===========
*/
void cmd_resetmatch(gentity_t *ent, qboolean fParam) {
	char *log;
		
	AP(va("chat \"console: %s has ^3reset ^7match.\n\"", sortTag(ent)));
	trap_SendConsoleCommand(EXEC_APPEND, va("reset_match"));

	// Log it
	log = va("Player %s (IP: %s) has reset match.",
		ent->client->pers.netname, clientIP(ent, qtrue));

	if (g_extendedLog.integer >= 2)
		logEntry(ADMACT, log);
	return;
}

/*
===========
Swap teams
===========
*/
void cmd_swap(gentity_t *ent, qboolean fParam) {
	char *log;

	AP(va("chat \"console:^7 %s has ^3swapped ^7the teams.\n\"", sortTag(ent)));
	trap_SendConsoleCommand(EXEC_APPEND, va("swap_teams"));

	// Log it
	log = va("Player %s (IP: %s) has swapped teams.",
		ent->client->pers.netname, clientIP(ent, qtrue));

	if (g_extendedLog.integer >= 2)
		logEntry(ADMACT, log);
	return;
}

/*
===========
Shuffle
===========
*/
void cmd_shuffle(gentity_t *ent, qboolean fParam) {
	char *log;
	int count = 0, tmpCount, i;
	int players[MAX_CLIENTS];

	G_teamReset(TEAM_RED, qtrue);
	G_teamReset(TEAM_BLUE, qtrue);

	memset(players, -1, sizeof(players));
	for (i = 0; i < MAX_CLIENTS; i++) {
		if ((!g_entities[i].inuse) || (level.clients[i].pers.connected != CON_CONNECTED))
			continue;

		if ((level.clients[i].sess.sessionTeam != TEAM_RED) && (level.clients[i].sess.sessionTeam != TEAM_BLUE))
			continue;

		players[count] = i;
		count++;
	}

	tmpCount = count;
	for (i = 0; i < count; i++)	{
		int j;

		do {
			j = (rand() % count);
		} while (players[j] == -1);

		if (i & 1)
			level.clients[players[j]].sess.sessionTeam = TEAM_BLUE;
		else
			level.clients[players[j]].sess.sessionTeam = TEAM_RED;

		ClientUserinfoChanged(players[j]);
		ClientBegin(players[j]);

		players[j] = players[tmpCount - 1];
		players[tmpCount - 1] = -1;
		tmpCount--;
	}

	AP(va("print \"console: %s has ^3shuffled ^7teams.\n\"", sortTag(ent)));
	// If admin does !shuffle @ it wont restart the match..
	if (Q_stricmp(ent->client->pers.cmd2, "@"))
		trap_SendConsoleCommand(EXEC_APPEND, va("reset_match %i\n", GS_WARMUP));

	// Log it
	log = va("Player %s (IP: %s) has shuffled teams.",
		ent->client->pers.netname, clientIP(ent, qtrue));

	if (g_extendedLog.integer >= 2)
		logEntry(ADMACT, log);
	return;
}

/*
==================
Move lagged out or downloading clients to spectators
==================
*/
void cmd_specs999(gentity_t *ent, qboolean fParam) {
	int i;
	qboolean moved = qfalse;
	char  *log;

	for (i = 0; i < level.maxclients; i++) {
		ent = &g_entities[i];
		if (!ent->client) continue;
		if (ent->client->pers.connected != CON_CONNECTED) continue;
		if (ent->client->ps.ping >= 999) {
			SetTeam(ent, "s", qtrue);
			moved = qtrue;
		}
	}

	if (moved == qtrue)
		AP(va("chat \"console: %s ^3moved all lagged ^7out players to spectators!\n\"", sortTag(ent)));
	else
		CP("print \"^1Error^7: No one to move to spectators!\n\"");

	// Log it
	log = va("Player %s (IP: %s) forced all 999 to spectators.",
		ent->client->pers.netname, clientIP(ent, qtrue));

	if (g_extendedLog.integer >= 2) 
		logEntry(ADMACT, log);
	return;
}

/*
==================
Reveal location of a player.
==================
*/
void cmd_revealCamper(gentity_t *ent, qboolean fParam) {
	gentity_t *target;
	char location[64];
	int	clientNum;
	char *log;
	
	clientNum = ClientNumberFromString(ent, ent->client->pers.cmd2);
	if (clientNum == -1) {
		CP("print \"^1Error^7: Client not on server!\n\"");
		return; 
	}
	target = g_entities + clientNum;
	Team_GetLocationMsg(ent, location, sizeof(location), qtrue);
	AP(va("chat \"console: %s ^3has releaved ^7that player %s^7 is hidding at ^3%s^7.\n\"", sortTag(ent), target->client->pers.netname, location));

	// Log it
	log = va("%s (IP: %s) has revealed %s location.", 
		ent->client->pers.netname, clientIP(ent, qtrue), target->client->pers.netname);

	if (g_extendedLog.integer >= 2) 
		logEntry(ADMACT, log);
	return;
}

/*
===========
Lock/Unlock team

NOTE: Somewhat messy due verbosity
TODO: Clean this eventually..
===========
*/
qboolean canTeamBeLocked(int team)
{
	if (team == TEAM_RED && level.axisPlayers < 1)
		return qfalse;
	else if (team == TEAM_BLUE && level.alliedPlayers < 1)
		return qfalse;
	else
		return qtrue;
}
// Lock/Unlock
void cmd_handleTeamLock(gentity_t *ent, qboolean tLock) {
	char *tag = sortTag(ent);
	char *cmd = ent->client->pers.cmd2;
	char *action = (tLock ? "Lock" : "Unlock");
	int team = TEAM_NUM_TEAMS;
	char *teamTag = "^3Both^7";
	char *log;

	if (!Q_stricmp(cmd, "")) {
		CP(va("print \"^1Error: ^7Please select which team you wish to %s!\n\"", action));
		return;
	}

	// Axis
	if (!Q_stricmp(cmd, "red") || !Q_stricmp(cmd, "axis") || !Q_stricmp(cmd, "r")) {
		team = TEAM_RED;
		teamTag = "^1Axis^7";
	}
	// Allies
	else if (!Q_stricmp(cmd, "blue") || !Q_stricmp(cmd, "allies") || !Q_stricmp(cmd, "allied") || !Q_stricmp(cmd, "b"))	{
		team = TEAM_BLUE;
		teamTag = "^4Allied^7";
	}
	// Both
	else if (!(Q_stricmp(cmd, "both") == 0)) {
		CP(va("print \"^1Error^7: ^7Please select which team you wish to %s!\n\"", action));
		return;
	}

	if (team != TEAM_NUM_TEAMS) {
		if (teamInfo[team].team_lock == tLock) {
			CP(va("print \"^1Error^7: %s team is already %sed!  \n\"", teamTag, action));
			return;
		}
		else {
			if (!canTeamBeLocked(team)) {
				CP(va("print \"^1Error^7: %s team is empty!\n\"", teamTag));
				return;
			}
			AP(va("chat \"console: %s has %sed %s team!\n\"", tag, action, teamTag));
			teamInfo[team].team_lock = tLock;
		}
	}
	else {
		if (teamInfo[TEAM_RED].team_lock != tLock || teamInfo[TEAM_BLUE].team_lock != tLock) {
			teamInfo[TEAM_RED].team_lock = tLock;
			teamInfo[TEAM_BLUE].team_lock = tLock;
			AP(va("chat \"console: %s has %sed %s teams!\n\"", tag, action, teamTag));
		}
		else
			CP(va("print \"Error: Both teams are already %sed!  \n\"", action));
		return;
	}

	log = va("Player %s (IP: %s) has issued %s for %s",
		ent->client->pers.netname, clientIP(ent, qtrue), action,
		(team == TEAM_RED ? "Axis team" : (team == TEAM_BLUE ? "Allied team" : "Both teams")));

	logEntry(ADMACT, log);
	return;
}
