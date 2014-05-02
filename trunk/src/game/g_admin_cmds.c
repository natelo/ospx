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
Deals with customm commands
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
