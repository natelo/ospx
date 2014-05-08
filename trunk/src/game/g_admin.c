/*/
===========================================================================
L0 / rtcwPub :: g_admin.c

	Main Admin functionality.

Created: 24. Mar / 2014
===========================================================================
*/
#include "g_admin.h"

/*
===========
Admin Log

Just a wrapper with syntax fixed..
===========
*/
void admLog(char *info) {
	
	if (!g_extendedLog.integer)
		return;

	logEntry(ADMACT, va("Time: %s\n%s%s", getTime(), info, LOGLINE));
}

/*
===========
Returns user Tag based upon user status..
===========
*/
char *usrTag(gentity_t *ent, qboolean inquiry) {
	char *tag = "";

	if (!ent->client->sess.admin) {
		if (inquiry)
			tag = "Player";
	}
	else if (ent->client->sess.admin == USER_REFEREE) {
		tag = va("%s", REFEREE);
	}
	else if (ent->client->sess.admin == ADMIN_1) {
		tag = va("%s^7", &a1_tag.string);
	}
	else if (ent->client->sess.admin == ADMIN_2) {
		tag = va("%s^7", &a2_tag.string);
	}
	else if (ent->client->sess.admin == ADMIN_3) {
		tag = va("%s^7", &a3_tag.string);
	}
	else if (ent->client->sess.admin == ADMIN_4) {
		tag = va("%s^7", &a4_tag.string);
	}
	else if (ent->client->sess.admin == ADMIN_5) {
		tag = va("%s^7", &a5_tag.string);
	}
	return tag;
}

/*
===========
Sort tag
===========
*/
char *sortTag(gentity_t *ent) {
	char n1[MAX_STRING_TOKENS];

	SanitizeString(usrTag(ent, qfalse), n1, qtrue);

	if (!Q_stricmp(n1, ""))	{
		Q_strncpyz(n1, "", sizeof(n1));
	}
	else {
		Q_CleanStr(n1);
	}
	return va("%s", n1);
}

/*
===========
Deals with ! & ?
===========
*/
void admCmds(const char *strCMD1, char *strCMD2, char *strCMD3, qboolean help){

	int i = 0, j = 0;
	int foundcolon = 0;

	while (strCMD1[i] != 0)
	{
		if (!foundcolon)
		{
			if (help) {
				if (strCMD1[i] == '?') {
					foundcolon = 1;
					strCMD2[i] = 0;
				}
				else
					strCMD2[i] = strCMD1[i];
				i++;
			}
			else {
				if (strCMD1[i] == '!') {
					foundcolon = 1;
					strCMD2[i] = 0;
				}
				else
					strCMD2[i] = strCMD1[i];
				i++;
			}
		}
		else
		{
			strCMD3[j++] = strCMD1[i++];
		}
	}
	if (!foundcolon)
		strCMD2[i] = 0;
	strCMD3[j] = 0;
}

/*
===========
Parse string (if I recall right this bit is from S4NDMoD)
===========
*/
void ParseAdmStr(const char *strInput, char *strCmd, char *strArgs)
{
	int i = 0, j = 0;
	int foundspace = 0;

	while (strInput[i] != 0){
		if (!foundspace){
			if (strInput[i] == ' '){
				foundspace = 1;
				strCmd[i] = 0;
			}
			else
				strCmd[i] = strInput[i];
			i++;
		}
		else{
			strArgs[j++] = strInput[i++];
		}
	}
	if (!foundspace)
		strCmd[i] = 0;

	strArgs[j] = 0;
}

/*
===========
Determine if admin level allows command
===========
*/
qboolean canUse(gentity_t *ent, qboolean dHelp) {
	char *list = "";
	char alt[128];
	char cmd[128];

	switch (ent->client->sess.admin) {
	case USER_REGULAR:
		return qfalse;
		break;
	case ADMIN_1:
		list = a1_cmds.string;
		break;
	case ADMIN_2:
		list = a2_cmds.string;
		break;
	case ADMIN_3:
		list = a3_cmds.string;
		break;
	case ADMIN_4:
		list = a4_cmds.string;
		break;
	case ADMIN_5:		
		list = a5_cmds.string; 
		break;
	}

	admCmds(ent->client->pers.cmd1, alt, cmd, qfalse);

	return Q_findToken(list, cmd);
}

/*
===========
A simple Flat level check..
===========
*/
qboolean isHigher(gentity_t *ent, gentity_t *target)
{
	if (ent->client->sess.admin <= target->client->sess.admin) {
		CP(va("print \"^3Denied^7: %s ^7has the same or higher level than you!\n^3Reason^7: You can only Administrate users with lower level than yours.\n\"", target->client->pers.netname));
		return qtrue;
	}
	return qfalse;
}

/*
===========
Admin structure
===========
*/
typedef struct {
	char *command;	
	void( *pCommand )(gentity_t *ent, qboolean fParam);
	qboolean fParam;
	qboolean dWarmup;
	const char *cUsage;
	const char *cHelp;	
} cmd_reference_t;

static const cmd_reference_t userCmd[] = {
	{ "help",			NULL,				qfalse,	qtrue,	"?command",								"Shows property and usage info of specified command." },	
	{ "incognito",		cmd_incognito,		qfalse,	qtrue,	"!incognito",							"Toggles your Admin status from visible to hidden."  },
	{ "ignore",			cmd_ignoreHandle,	qtrue,	qtrue,	"!ignore <unique part of name>",		"Takes ability to (v)chat or call votes from a targeted player." },
	{ "unignore",		cmd_ignoreHandle,	qfalse,	qtrue,	"!unignore <unique part of name>",		"Restores ability to (v)chat or call votes to a targeted player." },
	{ "allready",		cmd_readyHandle,	qfalse,	qtrue,	"!allready",							"Sets status of both teams as Ready and goes to countdown." },
	{ "unreadyall",		cmd_readyHandle,	qtrue,	qtrue,	"!unreadyall",							"Cancels countdown and returns match back in warmup state." },
	{ "speclock",		cmd_specHandle,		qtrue,	qtrue,	"!speclock <a|b|both>",					"Locks a player's team from spectators." },
	{ "specunlock",		cmd_specHandle,		qfalse,	qtrue,	"!specunlock <a|b|both>",				"Unlocks a player's team from spectators." },
	{ "pause",			cmd_pauseHandle,	qtrue,	qfalse,	"!pause",								"Pauses a match." },
	{ "unpause",		cmd_pauseHandle,	qfalse, qfalse,	"!unpause",								"Unpauses a match." },
	{ "kick",			cmd_kick,			qfalse, qtrue,	"!kick <unique part of name> <msg>",	"Removes player from a server."},
	{ "clientkick",		cmd_clientkick,		qfalse, qtrue,	"!clientkick <client slot> <msg>",		"Removes player from a server." },
	{ "rename",			cmd_rename,			qfalse,	qtrue,	"!rename <client slot> <new name>"		"Renames a players."},
	{ "slap",			cmd_slap,			qfalse, qtrue,	"!slap <client slot>",					"Does 20 hp damage to a player."},
	{ "kill",			cmd_kill,			qfalse, qtrue,	"!kill <client slot>",					"Kills a player." },
	{ "axis",			cmd_forceToTeam,	qfalse, qtrue,	"!axis <unique part of name>",			"Forces a player to Axis team." },
	{ "allies",			cmd_forceToTeam,	qfalse, qtrue,	"!allies <unique part of name>",		"Forces a player to Allied team."},
	{ "specs",			cmd_forceToTeam,	qtrue,	qtrue,	"!specs <unique part of name>",			"Forces a player to Spectator team."},
	{ "exec",			cmd_exec,			qfalse, qtrue,	"!exec <config file>",					"Executes a config file - assuming it exists on a server.." },
	{ "nextmap",		cmd_nextmap,		qfalse, qtrue,	"!nextmap",								"Loads the nextmap.." },
	{ "map",			cmd_map,			qfalse, qtrue,	"!map <map name>",						"Loads the map." },
	{ "vstr",			cmd_vstr,			qfalse, qtrue,	"!vstr <rotation marker>",				"Loads a specific part of map rotation."  },
	{ "cpa",			cmd_cpa,			qfalse, qtrue,	"!cpa <message>",						"Center prints message to everyone on a server." },
	{ "cp",				cmd_cp,				qfalse, qtrue,	"!cp <client slot> <message>",			"Center prints a message to a player." },
	{ "warn",			cmd_warn,			qfalse, qtrue,	"!warn <message>",						"Prints on screen and chat a message to everyone." },
	{ "chat",			cmd_chat,			qfalse, qtrue,	"!chat <message>",						"Print in chat a message to everyone." },
	{ "cancelvote",		cmd_cancelvote,		qfalse, qtrue,	"!cancelvote",							"Cancels any vote in progress." },
	{ "passvote",		cmd_passvote,		qfalse, qtrue,	"!passvote",							"Passes any vote in progress." },
	{ "restart",		cmd_restart,		qfalse, qtrue,	"!restart",								"Issues a map_restart" },
	{ "reset",			cmd_resetmatch,		qfalse, qtrue,	"!reset",								"Resets a match."},
	{ "swap",			cmd_swap,			qfalse, qtrue,	"!swap",								"Swaps the teams." },
	{ "shuffle",		cmd_shuffle,		qfalse, qtrue,	"!shuffle <@>",							"Shuffles the teams. Use @ to shuffle without reseting a match.." },
	{ "specs999",		cmd_specs999,		qfalse, qtrue,	"!specs999",							"Moves all lagged out player to spectators." },
	{ "whereis",		cmd_revealCamper,	qfalse, qtrue,	"!whereis <client slot>",				"Reveals to all the current location of a player." },
	{ "lock",			cmd_handleTeamLock, qtrue, qtrue,	"!lock <a|b|both>",						"Lock the team(s) so players cannot join." },
	{ "unlock",			cmd_handleTeamLock, qfalse, qtrue,	"!unlock <a|b|both>",					"Unlocks the team(s) so players can join." },
	{ 0,				NULL,				qfalse,	qfalse,	0,									0 }
};

/*
===========
Commands Structure

NOTE: Commented it to so it's easier for ones reviewing it..
===========
*/
qboolean userCommands(gentity_t *ent, char *cmd, qboolean pHelp) {	
	unsigned int i, \
		uCmd = ARRAY_LEN(userCmd);
	const cmd_reference_t *uCM;
	qboolean wasUsed = qfalse;

	for (i = 0; i < uCmd; i++) {
		uCM = &userCmd[i];
		if (NULL != uCM->command && 0 == Q_stricmp(cmd, uCM->command)) {

			// Helpers have no function..but they exist for some
			// reason, so print usage info as chances are it's help..
			if (uCM->pCommand == NULL)
				pHelp = qtrue;

			// Player requested info about a command
			if (pHelp) {
				CP(va("print \"^3%s %s %s\n\"",
					va(uCM->cUsage ? "Help ^7:" : "Help^7:"),
					uCM->cHelp,
					va("%s", (uCM->cUsage ? va("\n^3Usage^7: %s\n", uCM->cUsage) : ""))));
			}
			// Player executed command..
			else {		
				if (!level.warmupTime || uCM->dWarmup && level.warmupTime)
					uCM->pCommand(ent, uCM->fParam);
				else
					CP(va("print \"^3Denied^7: %s command cannot be used during warmup!\n\"", uCM->command));
			}

			wasUsed = qtrue;
		}
	}
	return wasUsed;
}

/*
===========
Command's Entry Point

NOTE: Commented it to so it's easier for ones reviewing it..
===========
*/
qboolean cmds_admin(gentity_t *ent, qboolean dHelp) {
	char alt[128];
	char cmd[128];

	admCmds(ent->client->pers.cmd1, alt, cmd, dHelp);

	// Allow ?help at any time..
	if (Q_stricmp(cmd, "help") == 0) {
		return userCommands(ent, cmd, qtrue);
	}	
	// a5_allowAll enabled allows for client to execute 
	// any existing command while a5_cmds can then be used
	// for any server related commands (e.g. g_allowvote..)
	else if (ent->client->sess.admin == ADMIN_5	&& a5_allowAll.integer)
	{		
		// Admin feature is not meant to work as rcon (full access) so if it's not
		// in a5_cmds string and not in Admin structure, it's either a typo or not allowed. 
		if (!userCommands(ent, cmd, dHelp))
			CP(va("print \"^1Error^7: Command ^1%s ^7is either not found or not allowed for your level!\n\"", cmd));

		return qtrue;
	}
	// Command is allowed (found in a*_cmds string..)
	else if (canUse(ent, dHelp))
	{
		// Command was not in Admin structure
		if (!userCommands(ent, cmd, dHelp))
			// Chances are that it's a default (e.g. g_speed) 
			// server command as those can be set as well..
			// NOTE: simpleton function really..
			cmd_custom(ent);

		return qtrue;
	}
	else
	{
		// Was not found due typo, insufficient level..whatever just bail out
		CP(va("print \"^1Error^7: Command ^1%s ^7is either not found or not allowed for your level!\n\"", cmd));
	}
	return qfalse;
}