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
// parse.c - illustrates how to parse the demo file
//

#include "main.h"

/*
==================
Parse_GameState
==================
*/
static void Parse_GameState( sizebuf_t *msg ) {
  int    c;
  int    index;
  char  *configString;

  memset( &ds, 0, sizeof( ds ) );

  ds.lastServerCommandNum = MSG_ReadLong( msg );
  ds.currentServerCommandNum = ds.lastServerCommandNum;

  while( 1 ) {
    c = MSG_ReadByte( msg );

    if( c == -1 ) {
      Com_Error( ERR_DROP, "Parse_GameState: read past end of demo message" );
    }

    if( c == SVC_EOM ) {
      break;
    }


    switch( c ) {
    default:
      Com_Error( ERR_DROP, "Parse_GameState: bad command byte" );
      break;

    case SVC_CONFIGSTRING:
      index = MSG_ReadShort( msg );
      if( index < 0 || index >= MAX_CONFIGSTRINGS ) {
        Com_Error( ERR_DROP, "Parse_GameState: configString index %i out of range", index );
      }
      configString = MSG_ReadBigString( msg );
      Com_InsertIntoGameState( &ds.gameState, index, configString );
      break;

    case SVC_BASELINE:
      index = MSG_ReadBits( msg, GENTITYNUM_BITS );
      if( index < 0 || index >= MAX_GENTITIES ) {
        Com_Error( ERR_DROP, "Parse_GameState: baseline index %i out of range", index );
      }
      MSG_ReadDeltaEntity( msg, NULL, &ds.baselines[index], index );
      break;
    }
  }

  // FIXME: unknown stuff, not used in demos?
  MSG_ReadLong( msg );
  MSG_ReadLong( msg );

  demo.gameStatesParsed++;

}

/*
=====================================================================

ACTION MESSAGES

=====================================================================
*/

/*
=====================
Parse_ServerCommand
=====================
*/
static void Parse_ServerCommand( sizebuf_t *msg ) {
  int    number;
  char  *string;

  number = MSG_ReadLong( msg );
  string = MSG_ReadString( msg );

  if( number < ds.lastServerCommandNum ) {
    return; // we have already received this command
  }
  ds.lastServerCommandNum = number;

  // archive the command to be processed later
  Q_strncpyz( ds.serverCommands[number & SERVERCMD_MASK], string, sizeof( ds.serverCommands[0] ) );
}

/*
==================
Parse_DeltaEntity

Parses deltas from the given base and adds the resulting entity
to the current frame
==================
*/
static void Parse_DeltaEntity( sizebuf_t *msg, snapshot_t *frame, int newnum, entityState_t *old, qboolean unchanged ) {
  entityState_t *state;

  state = &ds.parseEntities[ds.firstParseEntity & PARSE_ENTITIES_MASK];

  if( unchanged ) {
    memcpy( state, old, sizeof( *state ) ); // don't read any bits
  } else {
    MSG_ReadDeltaEntity( msg, old, state, newnum );
    if( state->number == ENTITYNUM_NONE ) {
      // the entity present in oldframe is not in the current frame
      return;
    }
  }

  ds.firstParseEntity++;
  frame->numEntities++;
}

/*
==================
Parse_PacketEntities

An svc_packetentities has just been parsed, deal with the
rest of the data stream.
==================
*/
static void Parse_PacketEntities( sizebuf_t *msg, snapshot_t *oldframe, snapshot_t *newframe ) {
  int      newnum;
  entityState_t  *oldstate;
  int      oldindex, oldnum;

  newframe->firstEntity = ds.firstParseEntity;
  newframe->numEntities = 0;

  // delta from the entities present in oldframe
  oldindex = 0;
  if( !oldframe ) {
    oldnum = 99999;
  } else {
    if( oldindex >= oldframe->numEntities ) {
      oldnum = 99999;
    } else {
      oldstate = &ds.parseEntities[(oldframe->firstEntity + oldindex) & PARSE_ENTITIES_MASK];
      oldnum = oldstate->number;
    }
  }

  while( 1 ) {
    newnum = MSG_ReadBits( msg, GENTITYNUM_BITS );
    if( newnum < 0 || newnum >= MAX_GENTITIES ) {
      Com_Error( ERR_DROP, "Parse_PacketEntities: bad number %i", newnum );
    }

    if( msg->readcount > msg->cursize ) {
      Com_Error( ERR_DROP, "Parse_PacketEntities: end of message" );
    }

    if( newnum == ENTITYNUM_NONE ) {
      break; // end of packetentities
    }

    while( oldnum < newnum ) {
      // one or more entities from the old packet are unchanged
      Parse_DeltaEntity( msg, newframe, oldnum, oldstate, qtrue );

      oldindex++;

      if( oldindex >= oldframe->numEntities ) {
        oldnum = 99999;
      } else {
        oldstate = &ds.parseEntities[(oldframe->firstEntity + oldindex) & PARSE_ENTITIES_MASK];
        oldnum = oldstate->number;
      }
    }

    if( oldnum == newnum ) {
      // delta from previous state
      Parse_DeltaEntity( msg, newframe, newnum, oldstate, qfalse );

      oldindex++;

      if( oldindex >= oldframe->numEntities ) {
        oldnum = 99999;
      } else {
        oldstate = &ds.parseEntities[(oldframe->firstEntity + oldindex) & PARSE_ENTITIES_MASK];
        oldnum = oldstate->number;
      }
      continue;
    }

    if( oldnum > newnum ) {
      // delta from baseline
      Parse_DeltaEntity( msg, newframe, newnum, &ds.baselines[newnum], qfalse );
    }
  }

  // any remaining entities in the old frame are copied over
  while( oldnum != 99999 ) {
    // one or more entities from the old packet are unchanged
    Parse_DeltaEntity( msg, newframe, oldnum, oldstate, qtrue );

    oldindex++;

    if( oldindex >= oldframe->numEntities ) {
      oldnum = 99999;
    } else {
      oldstate = &ds.parseEntities[(oldframe->firstEntity + oldindex) & PARSE_ENTITIES_MASK];
      oldnum = oldstate->number;
    }
  }
}

/*
=====================
ParseSnapshot
=====================
*/
static void Parse_Snapshot( sizebuf_t *msg ) {
  snapshot_t  *oldsnap;
  int        delta;
  int        len;

  // save the frame off in the backup array for later delta comparisons
  ds.snap = &ds.snapshots[demo.demoMessageSequence & SNAPSHOT_MASK];
  memset( ds.snap, 0, sizeof( *ds.snap ) );

  ds.snap->seq = demo.demoMessageSequence;
  ds.snap->serverTime = MSG_ReadLong( msg );

  // If the frame is delta compressed from data that we
  // no longer have available, we must suck up the rest of
  // the frame, but not use it, then ask for a non-compressed
  // message
  delta = MSG_ReadByte( msg );
  if( delta ) {
    ds.snap->deltaSeq = demo.demoMessageSequence - delta;
    oldsnap = &ds.snapshots[ds.snap->deltaSeq & SNAPSHOT_MASK];

    if( !oldsnap->valid ) {
      // should never happen
      Com_Printf( "Delta from invalid frame (not supposed to happen!).\n" );
    } else if( oldsnap->seq != ds.snap->deltaSeq ) {
      // The frame that the server did the delta from
      // is too old, so we can't reconstruct it properly.
      Com_Printf( "Delta frame too old.\n" );
    } else if( ds.firstParseEntity - oldsnap->firstEntity >
      MAX_PARSE_ENTITIES - MAX_ENTITIES_IN_SNAPSHOT )
    {
      Com_Printf( "Delta parse_entities too old.\n" );
    } else {
      ds.snap->valid = qtrue; // valid delta parse
    }
  } else {
    oldsnap = NULL;
    ds.snap->deltaSeq = -1;
    ds.snap->valid = qtrue; // uncompressed frame
  }

  // read snapFlags
  ds.snap->snapFlags = MSG_ReadByte( msg );

  // read areabits
  len = MSG_ReadByte( msg );
  MSG_ReadData( msg, ds.snap->areamask, len );

  // read playerinfo
  MSG_ReadDeltaPlayerstate( msg, oldsnap ? &oldsnap->ps : NULL, &ds.snap->ps );

  // read packet entities
  Parse_PacketEntities( msg, oldsnap, ds.snap );
}

/*
=====================
Parse_DemoMessage
=====================
*/
static void Parse_DemoMessage( sizebuf_t *msg ) {
  int      cmd;

  // remaining data is Huffman compressed
  MSG_SetBitstream( msg );

  MSG_ReadLong( msg ); // FIXME: not used in demos


//
// parse the message
//
  while( 1 ) {
    if( msg->readcount > msg->cursize ) {
      Com_Error( ERR_DROP, "Parse_DemoMessage: read past end of demo message" );
      break;
    }

    cmd = MSG_ReadByte( msg );

    if( cmd == SVC_EOM ) {
      break;
    }

  // other commands
    switch( cmd ) {
    default:
    case SVC_BASELINE:
    case SVC_CONFIGSTRING:
    case SVC_DOWNLOAD:
      Com_Error( ERR_DROP, "Parse_DemoMessage: illegible demo message" );
      break;
    case SVC_NOP:      /* do nothing */        break;
    case SVC_GAMESTATE:    Parse_GameState( msg );    break;
    case SVC_SERVERCOMMAND:  Parse_ServerCommand( msg );  break;
    case SVC_SNAPSHOT:    Parse_Snapshot( msg );    break;
    }
  }

}

/*
=====================
Parse_NextDemoMessage

  Read next message from demo file and parse it
  Return qfalse if demo EOF reached or error occured
=====================
*/
qboolean Parse_NextDemoMessage( void ) {
  sizebuf_t  msg;
  byte    buffer[MAX_MSGLEN];
  int      len;
  int      seq;

  if( fread( &seq, 1, 4, demo.demofile ) != 4 ) {
    Com_Printf( "Demo file was truncated\n" );
    return qfalse;
  }

  if( fread( &len, 1, 4, demo.demofile ) != 4 ) {
    Com_Printf( "Demo file was truncated\n" );
    return qfalse;
  }

  if( seq == -1 || len == -1 ) {
    return qfalse; // demo EOF reached
  }

  MSG_Init( &msg, buffer, sizeof( buffer ) );

  demo.demoMessageSequence = LittleLong( seq );
  msg.cursize = LittleLong( len );

  if( msg.cursize <= 0 || msg.cursize >= msg.maxsize ) {
    Com_Error( ERR_DROP, "Illegal demo message length" );
  }

  if( fread( msg.data, 1, msg.cursize, demo.demofile ) != msg.cursize ) {
    Com_Printf( "Demo file was truncated\n" );
    return qfalse;
  }

  Parse_DemoMessage( &msg ); // parse the message

  return qtrue;
}
