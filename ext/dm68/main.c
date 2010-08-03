/*
Quake III *.dm_6? Demo Specifications

Copyright (C) 2003 Andrey '[SkulleR]' Nazarov
Based on Argus and Quake II source code
Also contains some stuff from Q3A SDK

Argus is Copyright (C) 2000 Martin Otten
Quake II and Quake III are Copyright (C) 1997-2001 ID Software, Inc

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//
// main.c - example main program
//

#include "main.h"

demoFileState_t  demo;
demoState_t    ds;


char result[MAXRESULT];
/*
 * appends a string to the final result string
 *
 */
void append_result(const char *to_append, ...)
{
  va_list    argptr;
  char    buffer[MAXRESULT];

  va_start(argptr, to_append);
  vsprintf(buffer, to_append, argptr);
  va_end(argptr);

  Q_strcat(result, MAXRESULT, buffer);
}


/*
============
Com_AppendToGameState
============
*/
static void Com_AppendToGameState( gameState_t *gameState, int index, const char *configString ) {
  int len;

  if( !configString || !(len=strlen( configString )) ) {
    return;
  }

  if( gameState->dataCount + len + 2 >= MAX_GAMESTATE_CHARS ) {
    Com_Error( ERR_DROP, "Com_AppendToGameState: MAX_GAMESTATE_CHARS" );
  }

  gameState->stringOffsets[index] = gameState->dataCount + 1;
  strcpy( &gameState->stringData[gameState->dataCount + 1], configString );
  gameState->dataCount += len + 1;
}

/*
============
Com_InsertIntoGameState
============
*/
void Com_InsertIntoGameState( gameState_t *gameState, int index, const char *configString ) {
  char *strings[MAX_CONFIGSTRINGS];
  int ofs;
  int i;

  if( index < 0 || index >= MAX_CONFIGSTRINGS ) {
    Com_Error( ERR_DROP, "Com_InsertIntoGameState: bad index %i", index );
  }

  if( !gameState->stringOffsets[index] ) {
    // just append to the end of gameState
    Com_AppendToGameState( gameState, index, configString );
    return;
  }

  //
  // resort gameState
  //
  for( i=0 ; i<MAX_CONFIGSTRINGS ; i++ ) {
    ofs = gameState->stringOffsets[i];
    if( !ofs ) {
      strings[i] = NULL;
    } else if( i == index ) {
      strings[i] = CopyString( configString );
    } else {
      strings[i] = CopyString( &gameState->stringData[ofs] );
    }
  }

  memset( gameState, 0, sizeof( *gameState ) );

  for( i=0 ; i<MAX_CONFIGSTRINGS ; i++ ) {
    if( strings[i] ) {
      Com_AppendToGameState( gameState, i, strings[i] );
      free( strings[i] );
    }
  }

}

/*
============
Com_GetStringFromGameState
============
*/
char *Com_GetStringFromGameState( gameState_t *gameState, int index ) {
  int ofs;

  if( index < 0 || index >= MAX_CONFIGSTRINGS ) {
    Com_Error( ERR_DROP, "Com_GetStringFromGameState: bad index  %i", index  );
  }

  ofs = gameState->stringOffsets[index ];

  if( !ofs ) {
    return "";
  }

  return &gameState->stringData[ofs];
}

int main( int argc, char **argv ) {
  if( argc < 2 ) {
    Com_Error( ERR_FATAL, "Usage: dm68 [demofile]\n" );
  }
  if( !(demo.demofile=fopen( argv[1], "rb" )) ) {
    Com_Error( ERR_FATAL, "Couldn't open demofile" );
  }

  Huff_Init();

  Com_Printf("---\n");

  while( !demo.gameStatesParsed ) {
    if( !Parse_NextDemoMessage() ) {
      break;
    }
  }

  GameStateParsed();

  Com_Printf("prints:\n");

  while( 1 ) {
    if( !Parse_NextDemoMessage() ) {
      break;
    }
    NewFrameParsed();
  }

  fclose( demo.demofile );

  return EXIT_SUCCESS;
}

