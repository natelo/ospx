/*/
===========================================================================
	OSPx :: g_files.c

	Deals with File handling

Created: 29 Apr / 2014
===========================================================================
*/
#include "g_admin.h"

char *TempBannedMessage;

/*
===========
Log Admin related stuff
===========
*/
void logEntry(char *filename, char *info) {
	fileHandle_t	f;
	char *varLine;

	strcat(info, "\r");
	trap_FS_FOpenFile(filename, &f, FS_APPEND);

	varLine = va("%s\n", info);

	trap_FS_Write(varLine, strlen(varLine), f);
	trap_FS_FCloseFile(f);
	return;
}
