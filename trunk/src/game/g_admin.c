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

// Admin Activity


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
		tag = "^3Referee^7";
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
Login
===========
*/
void cmd_doLogin(gentity_t *ent, qboolean silent) {
	char str[MAX_TOKEN_CHARS];
	qboolean error;
	char *log;

	error = qfalse;
	trap_Argv(1, str, sizeof(str));

	// Make sure user is not already logged in.
	if (ent->client->sess.admin != USER_REGULAR) {
		CP("print \"You are already logged in^1!\n\"");
		return;
	}

	// Prevent bogus logins	
	if ((!Q_stricmp(str, "\0"))
		|| (!Q_stricmp(str, ""))
		|| (!Q_stricmp(str, "\""))
		|| (!Q_stricmp(str, "none")))
	{
		CP("print \"Incorrect password^1!\n\"");
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
			CP("print \"Error has occured while trying to log you in^1!\n\"");
			return;
		}

		// We came so far so go with it..
		if (silent) {
			CP("print \"Silent Login successful^2!\n\"");
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
			AP(va("chat \"^7console: %s ^7has logged in as %s^7!\n\"", ent->client->pers.netname, sortTag(ent)));

			// Log it
			log = va("Time: %s\nPlayer %s (IP: %s) has logged in as %s.%s",
				getTime(),ent->client->pers.netname, clientIP(ent, qtrue), sortTag(ent), LOGLINE);

			if (g_extendedLog.integer)
				logEntry(ADMLOG, log);
		}
		return;
	}
	else
	{
		CP("print \"Incorrect password^1!\n\"");

		// Log it
		log = va("Time: %s\nPlayer %s (IP: %s) has tried to login using password: %s%s",
			getTime(), ent->client->pers.netname, clientIP(ent, qtrue), str, LOGLINE );

		if (g_extendedLog.integer)
			logEntry(PASSLOG, log);

		return;
	}
}

/*
===========
Logout
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
			CP("print \"You have successfully logged out^3!\n\"");
		else
			AP(va("chat \"console: %s ^7has logged out^3!\n\"", ent->client->pers.netname));

		// Log them out now
		ent->client->sess.admin = USER_REGULAR;

		// Set incognito to visible..
		ent->client->sess.incognito = 0;

		return;
	}
}

/*
===========
Deals with ! & ?
===========
*/
void admCmds(const char *strCMD1, char *strCMD2, char *strCMD3, qboolean cmd){

	int i = 0, j = 0;
	int foundcolon = 0;

	while (strCMD1[i] != 0)
	{
		if (!foundcolon)
		{
			if (cmd) {
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
Can't use command msg..
===========
*/
void cantUse(gentity_t *ent) {
	char alt[128];
	char cmd[128];

	admCmds(ent->client->pers.cmd1, alt, cmd, qfalse);

	CP(va("print \"Command ^1%s ^7is not allowed for your level^1!\n\"", cmd));
	return;
}

/*
===========
Determine if admin level allows command
===========
*/
qboolean canUse(gentity_t *ent, qboolean isCmd) {
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
		if (a5_allowAll.integer && isCmd) 
			return qtrue;
		else
			list = a5_cmds.string; 
		break;
	}

	admCmds(ent->client->pers.cmd1, alt, cmd, qfalse);

	return Q_findToken(list, cmd);
}

/*
===========
List commands
===========
*/
void cmd_listCmds(gentity_t *ent) {
	char *cmds;

	if (!adm_help.integer) {
		CP("print \"Admin commands list is disabled on this server^1!\n\"");
		return;
	}

	cmds = "";

	if (ent->client->sess.admin == ADMIN_1)
		CP(va("print \"^3Available commands are:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", a1_cmds.string));
	else if (ent->client->sess.admin == ADMIN_2)
		CP(va("print \"^3Available commands are:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", a2_cmds.string));
	else if (ent->client->sess.admin == ADMIN_3)
		CP(va("print \"^3Available commands are:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", a3_cmds.string));
	else if (ent->client->sess.admin == ADMIN_4)
		CP(va("print \"^3Available commands are:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", a4_cmds.string));
	else if (ent->client->sess.admin == ADMIN_5 && !a5_allowAll.integer)
		CP(va("print \"^3Available commands are:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", a5_cmds.string));
	else if (ent->client->sess.admin == ADMIN_5 && a5_allowAll.integer)
		CP(va("print \"^3Available commands are:^7\n%s\n^5Additional server commands:^7\n%s\n^3Use ? for help with command. E.g. ?incognito.\n\"", cmds, a5_cmds.string));

	return;
}

/*
===========
Admin commands
===========
*/
qboolean do_cmds(gentity_t *ent) {
	char alt[128];
	char cmd[128];

	admCmds(ent->client->pers.cmd1, alt, cmd, qfalse);
	
	// Any other command
	if (canUse(ent, qfalse))			{ cmd_custom(ent, cmd); return qtrue; }

	// It failed on all checks..
	else { CP(va("print \"Command ^1%s ^7was not found^1!\n\"", cmd)); return qfalse; }

}

/*
===========
Admin help
===========
*/
typedef struct {
	char *command;
	char *help;
	char *usage;
} helpCmd_reference_t;

#define _HELP(x,y,z) {x, y, z},
/**
* Fairly straight forward approach _HELP(COMMAND, DESCRIPTION, USAGE)
* Alternatively, usage can be empty.
* Add new as needed..
*/
static const helpCmd_reference_t helpInfo[] = {
	_HELP("help", "Prints help about specific command.", "?COMMAND")	
	// --> Add new ones after this line..

	{
		NULL, NULL, NULL
	}
};

qboolean do_help(gentity_t *ent) {
	char alt[128];
	char cmd[128];
	unsigned int i, \
		aHelp = ARRAY_LEN(helpInfo);
	const helpCmd_reference_t *hCM;
	qboolean wasUsed = qfalse;

	admCmds(ent->client->pers.cmd1, alt, cmd, qtrue);

	for (i = 0; i < aHelp; i++) {
		hCM = &helpInfo[i];
		if (NULL != hCM->command && 0 == Q_stricmp(cmd, hCM->command)) {
			CP(va("print \"^3%s %s %s\n\"",
				va(hCM->usage ? "Help ^7:" : "Help^7:"),
				hCM->help,
				va("%s", (hCM->usage ? va("\n^3Usage^7: %s\n", hCM->usage) : ""))));
			wasUsed = qtrue;
		}
	}
	return wasUsed;
}

/*
===========
Commands
===========
*/
qboolean cmds_admin(char cmd[MAX_TOKEN_CHARS], gentity_t *ent) {

	// We're dealing with command
	if (Q_stricmp(cmd, "!") == 0) {
		return do_cmds(ent);
	}
	// We're dealing with help
	else if (Q_stricmp(cmd, "?") == 0) {
		return do_help(ent);
	}
	return qfalse;
}
