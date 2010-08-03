// Copyright (C) 1999-2000 Id Software, Inc.
//
// q_shared.c -- stateless support routines that are included in each code dll
#include "common.h"

/*
=============
Q_strncpyz

Safe strncpy that ensures a trailing zero
=============
*/
void Q_strncpyz( char *dest, const char *src, int destsize ) {
  // bk001129 - also NULL dest
  if ( !dest ) {
    Com_Error( ERR_FATAL, "Q_strncpyz: NULL dest" );
  }
  if ( !src ) {
    Com_Error( ERR_FATAL, "Q_strncpyz: NULL src" );
  }
  if ( destsize < 1 ) {
    Com_Error(ERR_FATAL,"Q_strncpyz: destsize < 1" );
  }

  strncpy( dest, src, destsize-1 );
  dest[destsize-1] = 0;
}

int Q_stricmpn (const char *s1, const char *s2, int n) {
  int    c1, c2;

  // bk001129 - moved in 1.17 fix not in id codebase
        if ( s1 == NULL ) {
           if ( s2 == NULL )
             return 0;
           else
             return -1;
        }
        else if ( s2==NULL )
          return 1;



  do {
    c1 = *s1++;
    c2 = *s2++;

    if (!n--) {
      return 0;    // strings are equal until end point
    }

    if (c1 != c2) {
      if (c1 >= 'a' && c1 <= 'z') {
        c1 -= ('a' - 'A');
      }
      if (c2 >= 'a' && c2 <= 'z') {
        c2 -= ('a' - 'A');
      }
      if (c1 != c2) {
        return c1 < c2 ? -1 : 1;
      }
    }
  } while (c1);

  return 0;    // strings are equal
}

int Q_strncmp (const char *s1, const char *s2, int n) {
  int    c1, c2;

  do {
    c1 = *s1++;
    c2 = *s2++;

    if (!n--) {
      return 0;    // strings are equal until end point
    }

    if (c1 != c2) {
      return c1 < c2 ? -1 : 1;
    }
  } while (c1);

  return 0;    // strings are equal
}

int Q_stricmp (const char *s1, const char *s2) {
  return (s1 && s2) ? Q_stricmpn (s1, s2, 99999) : -1;
}


// never goes past bounds or leaves without a terminating 0
void Q_strcat( char *dest, int size, const char *src ) {
  int    l1;

  l1 = strlen( dest );
  if ( l1 >= size ) {
    Com_Error( ERR_FATAL, "Q_strcat: already overflowed" );
  }
  Q_strncpyz( dest + l1, src, size - l1 );
}


void Com_sprintf( char *dest, int size, const char *fmt, ...) {
  int    len;
  va_list    argptr;
  char  bigbuffer[32000];  // big, but small enough to fit in PPC stack

  va_start (argptr,fmt);
  len = vsprintf (bigbuffer,fmt,argptr);
  va_end (argptr);
  if ( len >= sizeof( bigbuffer ) ) {
    Com_Error( ERR_FATAL, "Com_sprintf: overflowed bigbuffer" );
  }
  if (len >= size) {
    Com_Printf ("Com_sprintf: overflow of %i in %i\n", len, size);
#ifdef  _DEBUG
    __asm {
      int 3;
    }
#endif
  }
  Q_strncpyz (dest, bigbuffer, size );
}


/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char  *va( char *format, ... ) {
  va_list    argptr;
  static char    string[2][32000];  // in case va is called by nested functions
  static int    index = 0;
  char  *buf;

  buf = string[index & 1];
  index++;

  va_start (argptr, format);
  vsprintf (buf, format,argptr);
  va_end (argptr);

  return buf;
}


/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
FIXME: overflow check?
===============
*/
char *Info_ValueForKey( const char *s, const char *key ) {
  char  pkey[BIG_INFO_KEY];
  static  char value[2][BIG_INFO_VALUE];  // use two buffers so compares
                      // work without stomping on each other
  static  int  valueindex = 0;
  char  *o;

  if ( !s || !key ) {
    return "";
  }

  if ( strlen( s ) >= BIG_INFO_STRING ) {
    Com_Error( ERR_DROP, "Info_ValueForKey: oversize infostring" );
  }

  valueindex ^= 1;
  if (*s == '\\')
    s++;
  while (1)
  {
    o = pkey;
    while (*s != '\\')
    {
      if (!*s)
        return "";
      *o++ = *s++;
    }
    *o = 0;
    s++;

    o = value[valueindex];

    while (*s != '\\' && *s)
    {
      *o++ = *s++;
    }
    *o = 0;

    if (!Q_stricmp (key, pkey) )
      return value[valueindex];

    if (!*s)
      break;
    s++;
  }

  return "";
}


/*
===================
Info_NextPair

Used to itterate through all the key/value pairs in an info string
===================
*/
void Info_NextPair( const char **head, char *key, char *value ) {
  char  *o;
  const char  *s;

  s = *head;

  if ( *s == '\\' ) {
    s++;
  }
  key[0] = 0;
  value[0] = 0;

  o = key;
  while ( *s != '\\' ) {
    if ( !*s ) {
      *o = 0;
      *head = s;
      return;
    }
    *o++ = *s++;
  }
  *o = 0;
  s++;

  o = value;
  while ( *s != '\\' && *s ) {
    *o++ = *s++;
  }
  *o = 0;

  *head = s;
}


/*
===================
Info_RemoveKey
===================
*/
void Info_RemoveKey( char *s, const char *key ) {
  char  *start;
  char  pkey[MAX_INFO_KEY];
  char  value[MAX_INFO_VALUE];
  char  *o;

  if ( strlen( s ) >= MAX_INFO_STRING ) {
    Com_Error( ERR_DROP, "Info_RemoveKey: oversize infostring" );
  }

  if (strchr (key, '\\')) {
    return;
  }

  while (1)
  {
    start = s;
    if (*s == '\\')
      s++;
    o = pkey;
    while (*s != '\\')
    {
      if (!*s)
        return;
      *o++ = *s++;
    }
    *o = 0;
    s++;

    o = value;
    while (*s != '\\' && *s)
    {
      if (!*s)
        return;
      *o++ = *s++;
    }
    *o = 0;

    if (!strcmp (key, pkey) )
    {
      strcpy (start, s);  // remove this part
      return;
    }

    if (!*s)
      return;
  }

}

/*
===================
Info_RemoveKey_Big
===================
*/
void Info_RemoveKey_Big( char *s, const char *key ) {
  char  *start;
  char  pkey[BIG_INFO_KEY];
  char  value[BIG_INFO_VALUE];
  char  *o;

  if ( strlen( s ) >= BIG_INFO_STRING ) {
    Com_Error( ERR_DROP, "Info_RemoveKey_Big: oversize infostring" );
  }

  if (strchr (key, '\\')) {
    return;
  }

  while (1)
  {
    start = s;
    if (*s == '\\')
      s++;
    o = pkey;
    while (*s != '\\')
    {
      if (!*s)
        return;
      *o++ = *s++;
    }
    *o = 0;
    s++;

    o = value;
    while (*s != '\\' && *s)
    {
      if (!*s)
        return;
      *o++ = *s++;
    }
    *o = 0;

    if (!strcmp (key, pkey) )
    {
      strcpy (start, s);  // remove this part
      return;
    }

    if (!*s)
      return;
  }

}




/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate( const char *s ) {
  if ( strchr( s, '\"' ) ) {
    return qfalse;
  }
  if ( strchr( s, ';' ) ) {
    return qfalse;
  }
  return qtrue;
}

/*
==================
Info_SetValueForKey

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey( char *s, const char *key, const char *value ) {
  char  newi[MAX_INFO_STRING];

  if ( strlen( s ) >= MAX_INFO_STRING ) {
    Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring" );
  }

  if (strchr (key, '\\') || strchr (value, '\\'))
  {
    Com_Printf ("Can't use keys or values with a \\\n");
    return;
  }

  if (strchr (key, ';') || strchr (value, ';'))
  {
    Com_Printf ("Can't use keys or values with a semicolon\n");
    return;
  }

  if (strchr (key, '\"') || strchr (value, '\"'))
  {
    Com_Printf ("Can't use keys or values with a \"\n");
    return;
  }

  Info_RemoveKey (s, key);
  if (!value || !strlen(value))
    return;

  Com_sprintf (newi, sizeof(newi), "\\%s\\%s", key, value);

  if (strlen(newi) + strlen(s) > MAX_INFO_STRING)
  {
    Com_Printf ("Info string length exceeded\n");
    return;
  }

  strcat (newi, s);
  strcpy (s, newi);
}

/*
==================
Info_SetValueForKey_Big

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey_Big( char *s, const char *key, const char *value ) {
  char  newi[BIG_INFO_STRING];

  if ( strlen( s ) >= BIG_INFO_STRING ) {
    Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring" );
  }

  if (strchr (key, '\\') || strchr (value, '\\'))
  {
    Com_Printf ("Can't use keys or values with a \\\n");
    return;
  }

  if (strchr (key, ';') || strchr (value, ';'))
  {
    Com_Printf ("Can't use keys or values with a semicolon\n");
    return;
  }

  if (strchr (key, '\"') || strchr (value, '\"'))
  {
    Com_Printf ("Can't use keys or values with a \"\n");
    return;
  }

  Info_RemoveKey_Big (s, key);
  if (!value || !strlen(value))
    return;

  Com_sprintf (newi, sizeof(newi), "\\%s\\%s", key, value);

  if (strlen(newi) + strlen(s) > BIG_INFO_STRING)
  {
    Com_Printf ("BIG Info string length exceeded\n");
    return;
  }

  strcat (s, newi);
}




//====================================================================

/*
============
Com_Error
============
*/
void Com_Error( errorParm_t level, const char *error, ... ) {
  va_list    argptr;
  char    buffer[MAXPRINTMSG];

  va_start( argptr, error );
  vsprintf( buffer, error, argptr );
  va_end( argptr );

  printf( "*********************\n"
      "ERROR: %s\n"
      "*********************\n", buffer );
  exit( EXIT_FAILURE );

}

/*
============
Com_Printf
============
*/
void Com_Printf( const char *text, ... ) {
  va_list    argptr;
  char    buffer[MAXPRINTMSG];

  va_start( argptr, text );
  vsprintf( buffer, text, argptr );
  va_end( argptr );

  printf( "%s", buffer );
}

/*
=================
CopyString
=================
*/
char *CopyString( const char *in ) {
  char  *out;

  if( !in ) {
    return NULL;
  }

  out = malloc( strlen( in ) + 1 );
  strcpy( out, in );

  return out;
}


/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char *COM_Parse (char **data_p)
{
  int    c;
  int    len;
  char  *data;
  static char  com_token[MAX_TOKEN_CHARS];

  data = *data_p;
  len = 0;
  com_token[0] = 0;

  if (!data)
  {
    *data_p = NULL;
    return "";
  }

// skip whitespace
skipwhite:
  while ( (c = *data) <= ' ')
  {
    if (c == 0)
    {
      *data_p = NULL;
      return "";
    }
    data++;
  }

// skip // comments
  if (c=='/' && data[1] == '/')
  {
    while (*data && *data != '\n')
      data++;
    goto skipwhite;
  }

// handle quoted strings specially
  if (c == '\"')
  {
    data++;
    while (1)
    {
      c = *data++;
      if (c=='\"' || !c)
      {
        com_token[len] = 0;
        *data_p = data;
        return com_token;
      }
      if (len < MAX_TOKEN_CHARS)
      {
        com_token[len] = c;
        len++;
      }
    }
  }

// parse a regular word
  do
  {
    if (len < MAX_TOKEN_CHARS)
    {
      com_token[len] = c;
      len++;
    }
    data++;
    c = *data;
  } while (c>32);

  if (len == MAX_TOKEN_CHARS)
  {
//    Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
    len = 0;
  }
  com_token[len] = 0;

  *data_p = data;
  return com_token;
}

static  int      cmd_argc;
static  char    *cmd_argv[MAX_STRING_TOKENS];
static  char    *cmd_null_string = "";
static  char    cmd_args[MAX_STRING_CHARS];

/*
============
Cmd_Argc
============
*/
int    Cmd_Argc (void)
{
  return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
char  *Cmd_Argv (int arg)
{
  if ( (unsigned)arg >= cmd_argc )
    return cmd_null_string;
  return cmd_argv[arg];
}

/*
============
Cmd_Args

Returns a single string containing argv(1) to argv(argc()-1)
============
*/
char    *Cmd_Args (void)
{
  return cmd_args;
}


/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
$Cvars will be expanded unless they are in a quoted token
============
*/
void Cmd_TokenizeString (char *text)
{
  int    i;
  char  *com_token;

// clear the args from the last string
  for (i=0 ; i<cmd_argc ; i++)
    free (cmd_argv[i]);

  cmd_argc = 0;
  cmd_args[0] = 0;

  if (!text)
    return;

  while (1)
  {
// skip whitespace up to a /n
    while (*text && *text <= ' ' && *text != '\n')
    {
      text++;
    }

    if (*text == '\n')
    {  // a newline seperates commands in the buffer
      text++;
      break;
    }

    if (!*text)
      return;

    // set cmd_args to everything after the first arg
    if (cmd_argc == 1)
    {
      int    l;

      strcpy (cmd_args, text);

      // strip off any trailing whitespace
      l = strlen(cmd_args) - 1;
      for ( ; l >= 0 ; l--)
        if (cmd_args[l] <= ' ')
          cmd_args[l] = 0;
        else
          break;
    }

    com_token = COM_Parse (&text);
    if (!text)
      return;

    if (cmd_argc < MAX_STRING_TOKENS)
    {
      cmd_argv[cmd_argc] = malloc (strlen(com_token)+1);
      strcpy (cmd_argv[cmd_argc], com_token);
      cmd_argc++;
    }
  }

}


/*
============
Info_Print
============
*/
void Info_Print( const char *s ) {
  char  key[MAXPRINTMSG];
  char  value[MAXPRINTMSG];
  char  *o;
  int    l;

  if( *s == '\\' ) {
    s++;
  }

  while( *s )
  {
    o = key;
    while( *s && *s != '\\' ) // && o - key < MAXPRINTMSG )
    {
      *o++ = *s++;
    }

    l = o - key;
    if( l < 20 ) {
      memset( o, 0, 20-l );
      key[20] = 0;
    } else {
      *o = 0;
    }
    append_result("  %s: ", key);

    if( !*s ) {
      Com_Printf( "MISSING VALUE\n" );
      return;
    }

    o = value;
    s++;
    while( *s && *s != '\\' ) // && o - value < MAXPRINTMSG )
    {
      *o++ = *s++;
    }
    *o = 0;

    if( *s ) {
      s++;
    }
    append_result("\"%s\"\n", value);
  }
}

