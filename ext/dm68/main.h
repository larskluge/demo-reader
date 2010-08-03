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

#include "common.h"
#include "huff.h"
#include "msg.h"

// result
//

#define MAXRESULT 32000
extern char result[MAXRESULT];
void append_result(const char *to_append, ...);


//
// sizes of misc circular buffers in client and server system
//

// for delta compression of snapshots
#define MAX_SNAPSHOTS    32
#define SNAPSHOT_MASK    (MAX_SNAPSHOTS-1)

// for keeping reliable text commands not acknowledged by receiver yet
#define MAX_SERVERCMDS    64
#define SERVERCMD_MASK    (MAX_SERVERCMDS-1)

// for keeping all entityStates for delta encoding
#define MAX_PARSE_ENTITIES  (MAX_GENTITIES*2)
#define PARSE_ENTITIES_MASK  (MAX_PARSE_ENTITIES-1)

//
// max number of entityState_t present in a single update
//
#define MAX_ENTITIES_IN_SNAPSHOT  256

//
// possible server to client commands
//
typedef enum svc_ops_e {
  SVC_BAD,
  SVC_NOP,
  SVC_GAMESTATE,
  SVC_CONFIGSTRING,  // only inside game
  SVC_BASELINE,
  SVC_SERVERCOMMAND,
  SVC_DOWNLOAD,    // not used in demos
  SVC_SNAPSHOT,
  SVC_EOM
} svc_ops_t;

typedef struct {
  qboolean    valid;

  int        seq;        // die seqeunc number des snapshots
  int        deltaSeq;
  int        snapFlags;      // SNAPFLAG_RATE_DELAYED, etc


  int        serverTime;    // server time the message is valid for (in msec)

  byte      areamask[MAX_MAP_AREA_BYTES];    // portalarea visibility bits

  playerState_t  ps;            // complete information about the current player at this time

  int        numEntities;      // all of the entities that need to be presented
  int        firstEntity;      // ab dieser Position sind im Ringbuffer die numEntities viele Entities des Snapshots
                      // ersetzt entities[Max_entities_in snapshot]
} snapshot_t;

typedef struct {
  int    lastServerCommandNum;
  int    currentServerCommandNum;
  char  serverCommands[MAX_SERVERCMDS][MAX_STRING_CHARS];

  gameState_t    gameState;
  entityState_t  baselines[MAX_GENTITIES];

  entityState_t  parseEntities[MAX_PARSE_ENTITIES];
  int        firstParseEntity;

  snapshot_t    snapshots[MAX_SNAPSHOTS];
  snapshot_t    *snap;
} demoState_t;

typedef struct {
  FILE    *demofile;

  int demoMessageSequence;
  int gameStatesParsed;
} demoFileState_t;

extern demoFileState_t  demo;
extern demoState_t    ds;

qboolean  Parse_NextDemoMessage( void );

void GameStateParsed( void );
void NewFrameParsed( void );

void  Com_InsertIntoGameState( gameState_t *gameState, int index, const char *configString );
char *  Com_GetStringFromGameState( gameState_t *gameState, int index );

