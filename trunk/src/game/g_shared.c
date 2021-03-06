/*/
===========================================================================
	OSPx :: g_shared.c

	Game logic shared functionality

Created: 29 Apr / 2014
===========================================================================
*/
#include "g_local.h"

/*
==================
Ported from et: NQ
DecolorString

Remove color characters
==================
*/
void Q_decolorString(char *in, char *out)
{
	while (*in) {
		if (*in == 27 || *in == '^') {
			in++;		// skip color code
			if (*in) in++;
			continue;
		}
		*out++ = *in++;
	}
	*out = 0;
}

/*
==================
Time

Returns current time.
==================
*/
extern int trap_RealTime(qtime_t * qtime);
const char *months[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
// Returns current time
char *getTime( void )
{
	qtime_t		ct;
	trap_RealTime(&ct);

	return va("%02d:%02d:%02d/%02d %s %d",
		ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday,
		months[ct.tm_mon], 1900 + ct.tm_year);
}
/*
==================
Splits string into tokens
==================
*/
void Q_tokenize(char *str, char **splitstr, char *delim) {
	char *p;
	int i = 0;

	p = strtok(str, delim);
	while (p != NULL)
	{
		printf("%s", p);

		splitstr[i] = G_Alloc(strlen(p) + 1);

		if (splitstr[i])
			strcpy(splitstr[i], p);
		i++;

		p = strtok(NULL, delim);
	}
}

/*
==================
TokenList

See if there's a match
==================
*/
qboolean Q_findToken(char *haystack, char *needle) {

	if (strlen(haystack) && strlen(needle)) {
		char *token;
		
		while (1)
		{
			token = COM_Parse(&haystack);
			if (!token || !token[0])
				break;

			if (!Q_stricmp(needle, token))
				return qtrue;
		}
	}
	return qfalse;
}

/*
===================
Str replacer

Ported from etPub
===================
*/
char *Q_strReplace(char *haystack, char *needle, char *newp)
{
	static char final[MAX_STRING_CHARS] = { "" };
	char dest[MAX_STRING_CHARS] = { "" };
	char newStr[MAX_STRING_CHARS] = { "" };
	char *destp;
	int needle_len = 0;
	int new_len = 0;

	if (!*haystack) {
		return final;
	}
	if (!*needle) {
		Q_strncpyz(final, haystack, sizeof(final));
		return final;
	}
	if (*newp) {
		Q_strncpyz(newStr, newp, sizeof(newStr));
	}

	dest[0] = '\0';
	needle_len = strlen(needle);
	new_len = strlen(newStr);
	destp = &dest[0];
	while (*haystack) {
		if (!Q_stricmpn(haystack, needle, needle_len)) {
			Q_strcat(dest, sizeof(dest), newStr);
			haystack += needle_len;
			destp += new_len;
			continue;
		}
		if (MAX_STRING_CHARS > (strlen(dest) + 1)) {
			*destp = *haystack;
			*++destp = '\0';
		}
		haystack++;
	}
	// tjw: don't work with final return value in case haystack 
	//      was pointing at it.
	Q_strncpyz(final, dest, sizeof(final));

	return final;
}

/*
===========
Global sound
===========
*/
void APSound(char *sound) {
	gentity_t *ent;
	gentity_t *te;

	ent = g_entities;

	te = G_TempEntity(ent->s.pos.trBase, EV_GLOBAL_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
	te->r.svFlags |= SVF_BROADCAST;
}

/*
===========
Global sound - Hooked under cg_announced .. 
===========
*/
void AAPSound(char *sound) {
	gentity_t *ent;
	gentity_t *te;

	ent = g_entities;

	te = G_TempEntity(ent->s.pos.trBase, EV_ANNOUNCER_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
	te->r.svFlags |= SVF_BROADCAST;
}

/*
===========
Client sound
===========
*/
void CPSound(gentity_t *ent, char *sound) {
	gentity_t *te;

	te = G_TempEntity(ent->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
	te->s.teamNum = ent->s.clientNum;
}

/*
===========
Global sound with limited range
===========
*/
void APRSound(gentity_t *ent, char *sound) {
	gentity_t   *te;

	te = G_TempEntity(ent->r.currentOrigin, EV_GENERAL_SOUND);
	te->s.eventParm = G_SoundIndex(sound);
}
