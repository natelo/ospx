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
				getTime(), ent->client->pers.netname, clientIP(ent, qtrue), sortTag(ent), LOGLINE);

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
Deals with customm commands

NOTE: Only called directly from cmds_admin..
===========
*/
void cmd_custom(gentity_t *ent) {
	char *tag, *log;

	tag = sortTag(ent);

	if (!strcmp(ent->client->pers.cmd2, "")) {
		CP(va("print \"Command ^1%s ^7must have a value^1!\n\"", ent->client->pers.cmd1));
		return;
	}
	else {
		// Rconpasswords or sensitve commands can be changed without public print..
		if (!strcmp(ent->client->pers.cmd3, "@"))
			CP(va("print \"Info: ^2%s ^7was silently changed to ^2%s^7!\n\"", ent->client->pers.cmd1, ent->client->pers.cmd2));
		else
			AP(va("chat \"console: %s ^7changed ^3%s ^7to ^3%s %s\n\"", tag, ent->client->pers.cmd1, ent->client->pers.cmd2, ent->client->pers.cmd3));
		// Change the stuff
		trap_SendConsoleCommand(EXEC_APPEND, va("%s %s %s", ent->client->pers.cmd1, ent->client->pers.cmd2, ent->client->pers.cmd3));
		// Log it
		log = va("Player %s (IP: %s) has changed %s to %s %s.",
			ent->client->pers.netname, clientIP(ent, qtrue), ent->client->pers.cmd1, ent->client->pers.cmd2, ent->client->pers.cmd3);
		admLog(log);

		return;
	}
}
