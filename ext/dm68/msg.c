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
// msg.c - message i/o subsystem
//

#include "common.h"
#include "huff.h"
#include "msg.h"

// snapped vectors are packed in 13 bits instead of 32
#define SNAPPED_BITS    13
#define MAX_SNAPPED      (1<<SNAPPED_BITS)

/*
=======================================================================================

  OFFSET TABLES FOR MAIN GAME STRUCTURES

  If you want something from playerState_t or entityState structures to be
  transmitted on the network, just insert a field into one of the following tables.

  For network bandwidth saving, all fields are sorted in order from highest
  modification frequency (during active gameplay) to lowest.

=======================================================================================
*/

typedef struct {
  int    offset;
  int    bits;  // bits > 0  -->  unsigned integer
          // bits = 0  -->  float value
          // bits < 0  -->  signed integer
} field_t;

// field declarations
#define PS_FIELD(n,b)  { ((int)&(((playerState_t *)0)->n)), b }
#define ES_FIELD(n,b)  { ((int)&(((entityState_t *)0)->n)), b }


// field data accessing
#define FIELD_INTEGER(s)  (*(int   *)((byte *)(s)+field->offset))
#define FIELD_FLOAT(s)    (*(float *)((byte *)(s)+field->offset))

//
// playerState_t
//
static field_t psTable[] = {
  PS_FIELD( commandTime,      32 ),
  PS_FIELD( origin[0],       0 ),
  PS_FIELD( origin[1],       0 ),
  PS_FIELD( bobCycle,         8 ),
  PS_FIELD( velocity[0],       0 ),
  PS_FIELD( velocity[1],       0 ),
  PS_FIELD( viewangles[1],     0 ),
  PS_FIELD( viewangles[0],     0 ),
  PS_FIELD( weaponTime,       -16 ),
  PS_FIELD( origin[2],       0 ),
  PS_FIELD( velocity[2],       0 ),
  PS_FIELD( legsTimer,       8 ),
  PS_FIELD( pm_time,         -16 ),
  PS_FIELD( eventSequence,    16 ),
  PS_FIELD( torsoAnim,       8 ),
  PS_FIELD( movementDir,       4 ),
  PS_FIELD( events[0],       8 ),
  PS_FIELD( legsAnim,         8 ),
  PS_FIELD( events[1],       8 ),
  PS_FIELD( pm_flags,        16 ),
  PS_FIELD( groundEntityNum,    10 ),
  PS_FIELD( weaponstate,       4 ),
  PS_FIELD( eFlags,        16 ),
  PS_FIELD( externalEvent,    10 ),
  PS_FIELD( gravity,        16 ),
  PS_FIELD( speed,        16 ),
  PS_FIELD( delta_angles[1],    16 ),
  PS_FIELD( externalEventParm,   8 ),
  PS_FIELD( viewheight,      -8 ),
  PS_FIELD( damageEvent,       8 ),
  PS_FIELD( damageYaw,       8 ),
  PS_FIELD( damagePitch,       8 ),
  PS_FIELD( damageCount,       8 ),
  PS_FIELD( generic1,         8 ),
  PS_FIELD( pm_type,         8 ),
  PS_FIELD( delta_angles[0],    16 ),
  PS_FIELD( delta_angles[2],    16 ),
  PS_FIELD( torsoTimer,      12 ),
  PS_FIELD( eventParms[0],     8 ),
  PS_FIELD( eventParms[1],     8 ),
  PS_FIELD( clientNum,       8 ),
  PS_FIELD( weapon,         5 ),
  PS_FIELD( viewangles[2],     0 ),
  PS_FIELD( grapplePoint[0],     0 ),
  PS_FIELD( grapplePoint[1],     0 ),
  PS_FIELD( grapplePoint[2],     0 ),
  PS_FIELD( jumppad_ent,      10 ),
  PS_FIELD( loopSound,      16 )
};

//
// entityState_t
//
static field_t esTable[] = {
  ES_FIELD( pos.trTime,      32 ),
  ES_FIELD( pos.trBase[0],     0 ),
  ES_FIELD( pos.trBase[1],     0 ),
  ES_FIELD( pos.trDelta[0],     0 ),
  ES_FIELD( pos.trDelta[1],     0 ),
  ES_FIELD( pos.trBase[2],     0 ),
  ES_FIELD( apos.trBase[1],     0 ),
  ES_FIELD( pos.trDelta[2],     0 ),
  ES_FIELD( apos.trBase[0],     0 ),
  ES_FIELD( event,        10 ),
  ES_FIELD( angles2[1],       0 ),
  ES_FIELD( eType,         8 ),
  ES_FIELD( torsoAnim,       8 ),
  ES_FIELD( eventParm,       8 ),
  ES_FIELD( legsAnim,         8 ),
  ES_FIELD( groundEntityNum,    10 ),
  ES_FIELD( pos.trType,       8 ),
  ES_FIELD( eFlags,        19 ),
  ES_FIELD( otherEntityNum,    10 ),
  ES_FIELD( weapon,         8 ),
  ES_FIELD( clientNum,       8 ),
  ES_FIELD( angles[1],       0 ),
  ES_FIELD( pos.trDuration,    32 ),
  ES_FIELD( apos.trType,       8 ),
  ES_FIELD( origin[0],       0 ),
  ES_FIELD( origin[1],       0 ),
  ES_FIELD( origin[2],       0 ),
  ES_FIELD( solid,        24 ),
  ES_FIELD( powerups,        16 ),
  ES_FIELD( modelindex,       8 ),
  ES_FIELD( otherEntityNum2,    10 ),
  ES_FIELD( loopSound,       8 ),
  ES_FIELD( generic1,         8 ),
  ES_FIELD( origin2[2],       0 ),
  ES_FIELD( origin2[0],       0 ),
  ES_FIELD( origin2[1],       0 ),
  ES_FIELD( modelindex2,       8 ),
  ES_FIELD( angles[0],       0 ),
  ES_FIELD( time,          32 ),
  ES_FIELD( apos.trTime,      32 ),
  ES_FIELD( apos.trDuration,    32 ),
  ES_FIELD( apos.trBase[2],     0 ),
  ES_FIELD( apos.trDelta[0],     0 ),
  ES_FIELD( apos.trDelta[1],     0 ),
  ES_FIELD( apos.trDelta[2],     0 ),
  ES_FIELD( time2,        32 ),
  ES_FIELD( angles[2],       0 ),
  ES_FIELD( angles2[0],       0 ),
  ES_FIELD( angles2[2],       0 ),
  ES_FIELD( constantLight,    32 ),
  ES_FIELD( frame,        16 )
};

static const int psTableSize = sizeof( psTable ) / sizeof( psTable[0] );
static const int esTableSize = sizeof( esTable ) / sizeof( esTable[0] );

static const entityState_t  nullEntityState;
static const playerState_t  nullPlayerState;

/*
=======================================================================================

  MISC UTILITY FUNCTIONS

=======================================================================================
*/

/*
============
MSG_GetSpace
============
*/
static void *MSG_GetSpace( sizebuf_t *msg, int length ) {
  void  *data;

  if( msg->cursize + length > msg->maxsize ) {
    if( !msg->allowoverflow ) {
      Com_Error( ERR_FATAL, "MSG_GetSpace: overflow without allowoverflow set" );
    }

    if( length > msg->maxsize ) {
      Com_Error( ERR_FATAL, "MSG_GetSpace: %i is > full buffer size", length );
    }

    MSG_Clear( msg );
    msg->overflowed = qtrue;
  }

  data = msg->data + msg->cursize;
  msg->cursize += length;
  msg->bit += length << 3;

  return data;
}

/*
============
MSG_Init
============
*/
void MSG_Init( sizebuf_t *msg, byte *buffer, int size ) {
  memset( msg, 0, sizeof( *msg ) );
  msg->data = buffer;
  msg->maxsize = size;
}

/*
============
MSG_InitRaw
============
*/
void MSG_InitRaw( sizebuf_t *msg, byte *buffer, int size ) {
  memset( msg, 0, sizeof( *msg ) );
  msg->data = buffer;
  msg->maxsize = size;
  msg->uncompressed = qtrue;
}

/*
============
MSG_Clear
============
*/
void MSG_Clear( sizebuf_t *msg ) {
  msg->cursize = 0;
  msg->overflowed = qfalse;
  msg->uncompressed = qfalse;
  msg->bit = 0;
}

/*
============
MSG_SetBitstream
============
*/
void MSG_SetBitstream( sizebuf_t *msg ) {
  msg->uncompressed = qfalse;
}

/*
============
MSG_WriteRawData
============
*/
void MSG_WriteRawData( sizebuf_t *msg, const void *data, int length ) {
  if( length > 0 ) {
    memcpy( MSG_GetSpace( msg, length ), data, length );
  }
}

/*
=======================================================================================

  WRITING FUNCTIONS

=======================================================================================
*/

/*
============
MSG_BeginWriting
============
*/
void MSG_BeginWriting( sizebuf_t *msg ) {
  msg->uncompressed = qtrue;
  msg->overflowed = 0;
  msg->cursize = 0;
  msg->bit = 0;
}


/*
============
MSG_WriteBits
============
*/
void MSG_WriteBits( sizebuf_t *msg, int value, int bits ) {
  int        remaining;
  int        i;
  byte      *buf;

  if( msg->maxsize - msg->cursize < 4 ) {
    msg->overflowed = qtrue;
    return;
  }

  if( !bits || bits < -31 || bits > 32 ) {
    Com_Error( ERR_DROP, "MSG_WriteBits: bad bits %i", bits );
  }

  if( bits < 0 ) {
    bits = -bits;
  }

  if( msg->uncompressed ) {
    if( bits <= 8 ) {
      buf = MSG_GetSpace( msg, 1 );
      buf[0] = value;
    } else if( bits <= 16 ) {
      buf = MSG_GetSpace( msg, 2 );
      buf[0] = value & 0xFF;
      buf[1] = value >> 8;
    } else if( bits <= 32 ) {
      buf = MSG_GetSpace( msg, 4 );
      buf[0] = value & 0xFF;
      buf[1] = (value >> 8) & 0xFF;
      buf[2] = (value >> 16) & 0xFF;
      buf[3] = value >> 24;
    }
    return;
  }

  value &= 0xFFFFFFFFU >> (32 - bits);
  remaining = bits & 7;

  for( i=0; i<remaining ; i++ ) {
    if( !(msg->bit & 7) ) {
      msg->data[msg->bit >> 3] = 0;
    }
    msg->data[msg->bit >> 3] |= (value & 1) << (msg->bit & 7);
    msg->bit++;
    value >>= 1;
  }
  bits -= remaining;

  if( bits > 0 ) {
    for( i=0 ; i<(bits+7)>>3 ; i++ ) {
      Huff_EmitByte( value & 255, msg->data, &msg->bit );
      value >>= 8;
    }
  }

  msg->cursize = (msg->bit >> 3) + 1;
}


/*
============
MSG_WriteData
============
*/
void MSG_WriteData( sizebuf_t *msg, const void *data, int length ) {
  int i;

  for( i=0 ; i<length ; i++ ) {
    MSG_WriteByte( msg, ((byte *)data)[i] );
  }
}

/*
============
MSG_WriteString
============
*/
void MSG_WriteString( sizebuf_t *msg, const char *string ) {
  char  buffer[MAX_STRING_CHARS];
  int    i;
  int    len;

  if( !string ) {
    MSG_WriteByte( msg, 0 );
    return;
  }

  len = strlen( string );

  if( len >= sizeof( buffer ) ) {
    Com_Printf( "MSG_WriteString: MAX_STRING_CHARS\n" );
    MSG_WriteByte( msg, 0 );
    return;
  }

  Q_strncpyz( buffer, string, sizeof( buffer ) );

  for( i=0 ; i<len ; i++ ) {
    if( buffer[i] > 127 ) {
      buffer[i] = '.';
    }
  }

  for( i=0 ; i<=len ; i++ ) {
    MSG_WriteByte( msg, buffer[i] );
  }
}

/*
============
MSG_WriteString
============
*/
void MSG_WriteBigString( sizebuf_t *msg, const char *string ) {
  char  buffer[BIG_INFO_STRING];
  int    i;
  int    len;

  if( !string ) {
    MSG_WriteByte( msg, 0 );
    return;
  }

  len = strlen( string );

  if( len >= sizeof( buffer ) ) {
    Com_Printf( "MSG_WriteString: BIG_INFO_STRING\n" );
    MSG_WriteByte( msg, 0 );
    return;
  }

  Q_strncpyz( buffer, string, sizeof( buffer ) );

  for( i=0 ; i<len ; i++ ) {
    if( buffer[i] > 127 ) {
      buffer[i] = '.';
    }
  }

  for( i=0 ; i<=len ; i++ ) {
    MSG_WriteByte( msg, buffer[i] );
  }
}

/*
============
MSG_WriteDeltaEntity

  If 'force' parm is false, this won't result any bits
  emitted if entity didn't changed at all

  'from' == NULL  -->  nodelta update
  'to'   == NULL  -->  entity removed
============
*/
void MSG_WriteDeltaEntity( sizebuf_t *msg, const entityState_t *from, const entityState_t *to, qboolean force ) {
  field_t  *field;
  int        to_value;
  int        to_integer;
  float      to_float;
  int        maxFieldNum;
  int        i;

  if( !to ) {
    if( from ) {
      MSG_WriteBits( msg, from->number, GENTITYNUM_BITS );
      MSG_WriteBits( msg, 1, 1 );
    }
    return; // removed
  }

  if( to->number < 0 || to->number > MAX_GENTITIES ) {
    Com_Error( ERR_DROP, "MSG_WriteDeltaEntity: Bad entity number: %i", to->number );
  }

  if( !from ) {
    from = &nullEntityState; // nodelta update
  }

  //
  // find last modified field in table
  //
  maxFieldNum = 0;
  for( i=0, field=esTable ; i<esTableSize ; i++, field++ ) {
    if( FIELD_INTEGER( from ) != FIELD_INTEGER( to ) ) {
      maxFieldNum = i + 1;
    }
  }

  if( !maxFieldNum ) {
    if( !force ) {
      return; // don't emit any bits at all
    }

    MSG_WriteBits( msg, to->number, GENTITYNUM_BITS );
    MSG_WriteBits( msg, 0, 1 );
    MSG_WriteBits( msg, 0, 1 );
    return; // unchanged
  }

  MSG_WriteBits( msg, to->number, GENTITYNUM_BITS );
  MSG_WriteBits( msg, 0, 1 );
  MSG_WriteBits( msg, 1, 1 );
  MSG_WriteByte( msg, maxFieldNum );

  //
  // write all modified fields
  //
  for( i=0, field=esTable ; i<maxFieldNum ; i++, field++ ) {
    to_value = FIELD_INTEGER( to );

    if( FIELD_INTEGER( from ) == to_value ) {
      MSG_WriteBits( msg, 0, 1 );
      continue; // field unchanged
    }
    MSG_WriteBits( msg, 1, 1 );

    if( !to_value ) {
      MSG_WriteBits( msg, 0, 1 );
      continue; // field set to zero
    }
    MSG_WriteBits( msg, 1, 1 );

    if( field->bits ) {
      MSG_WriteBits( msg, to_value, field->bits );
      continue; // integer value
    }

    //
    // figure out how to pack float value
    //
    to_float = FIELD_FLOAT( to );
    to_integer = (int)to_float;

    if( (float)to_integer == to_float
      && to_integer + MAX_SNAPPED/2 >= 0
      && to_integer + MAX_SNAPPED/2 < MAX_SNAPPED )
    {
      MSG_WriteBits( msg, 0, 1 ); // pack in 13 bits
      MSG_WriteBits( msg, to_integer + MAX_SNAPPED/2, SNAPPED_BITS );
    } else {
      MSG_WriteBits( msg, 1, 1 ); // pack in 32 bits
      MSG_WriteLong( msg, to_value );
    }
  }

}

/*
============
MSG_WriteDeltaPlayerstate

  'from' == NULL  -->  nodelta update
  'to'   == NULL  -->  do nothing
============
*/
void MSG_WriteDeltaPlayerstate( sizebuf_t *msg, const playerState_t *from, const playerState_t *to ) {
  field_t  *field;
  int        to_value;
  float      to_float;
  int        to_integer;
  int        maxFieldNum;
  int        statsMask;
  int        persistantMask;
  int        ammoMask;
  int        powerupsMask;
  int        i;

  if( !to ) {
    return;
  }

  if( !from ) {
    from = &nullPlayerState; // nodelta update
  }

  //
  // find last modified field in table
  //
  maxFieldNum = 0;
  for( i=0, field=psTable ; i<psTableSize ; i++, field++ ) {
    if( FIELD_INTEGER( from ) != FIELD_INTEGER( to ) ) {
      maxFieldNum = i + 1;
    }
  }

  MSG_WriteByte( msg, maxFieldNum );

  //
  // write all modified fields
  //
  for( i=0, field=psTable ; i<maxFieldNum ; i++, field++ ) {
    to_value = FIELD_INTEGER( to );

    if( FIELD_INTEGER( from ) == to_value ) {
      MSG_WriteBits( msg, 0, 1 );
      continue; // field unchanged
    }
    MSG_WriteBits( msg, 1, 1 );

    if( field->bits ) {
      MSG_WriteBits( msg, to_value, field->bits );
      continue; // integer value
    }

    //
    // figure out how to pack float value
    //
    to_float = FIELD_FLOAT( to );
    to_integer = (int)to_float;

    if( (float)to_integer == to_float
      && to_integer + MAX_SNAPPED/2 >= 0
      && to_integer + MAX_SNAPPED/2 < MAX_SNAPPED )
    {
      MSG_WriteBits( msg, 0, 1 ); // pack in 13 bits
      MSG_WriteBits( msg, to_integer + MAX_SNAPPED/2, SNAPPED_BITS );
    } else {
      MSG_WriteBits( msg, 1, 1 ); // pack in 32 bits
      MSG_WriteLong( msg, to_value );
    }
  }

  //
  // find modified arrays
  //
  statsMask = 0;
  for( i=0 ; i<MAX_STATS ; i++ ) {
    if( from->stats[i] != to->stats[i] ) {
      statsMask |= (1 << i);
    }
  }

  persistantMask = 0;
  for( i=0 ; i<MAX_PERSISTANT ; i++ ) {
    if( from->persistant[i] != to->persistant[i] ) {
      persistantMask |= (1 << i);
    }
  }

  ammoMask = 0;
  for( i=0 ; i<MAX_WEAPONS ; i++ ) {
    if( from->ammo[i] != to->ammo[i] ) {
      ammoMask |= (1 << i);
    }
  }

  powerupsMask = 0;
  for( i=0 ; i<MAX_POWERUPS ; i++ ) {
    if( from->powerups[i] != to->powerups[i] ) {
      powerupsMask |= (1 << i);
    }
  }

  if( !statsMask && !persistantMask && !ammoMask && !powerupsMask ) {
    MSG_WriteBits( msg, 0, 1 );
    return; // no arrays modified
  }

  //
  // write all modified arrays
  //
  MSG_WriteBits( msg, 1, 1 );

  // PS_STATS
  if( statsMask ) {
    MSG_WriteBits( msg, 1, 1 );
    MSG_WriteShort( msg, statsMask );
    for( i=0 ; i<MAX_STATS ; i++ ) {
      if( statsMask & (1 << i) ) {
        MSG_WriteSignedShort( msg, to->stats[i] );
      }
    }
  } else {
    MSG_WriteBits( msg, 0, 1 ); // unchanged
  }

  // PS_PERSISTANT
  if( persistantMask ) {
    MSG_WriteBits( msg, 1, 1 );
    MSG_WriteShort( msg, persistantMask );
    for( i=0 ; i<MAX_PERSISTANT ; i++ ) {
      if( persistantMask & (1 << i) ) {
        MSG_WriteSignedShort( msg, to->persistant[i] );
      }
    }
  } else {
    MSG_WriteBits( msg, 0, 1 ); // unchanged
  }


  // PS_AMMO
  if( ammoMask ) {
    MSG_WriteBits( msg, 1, 1 );
    MSG_WriteShort( msg, ammoMask );
    for( i=0 ; i<MAX_WEAPONS ; i++ ) {
      if( ammoMask & (1 << i) ) {
        MSG_WriteShort( msg, to->ammo[i] );
      }
    }
  } else {
    MSG_WriteBits( msg, 0, 1 ); // unchanged
  }

  // PS_POWERUPS
  if( powerupsMask ) {
    MSG_WriteBits( msg, 1, 1 );
    MSG_WriteShort( msg, powerupsMask );
    for( i=0 ; i<MAX_POWERUPS ; i++ ) {
      if( powerupsMask & (1 << i) ) {
        MSG_WriteLong( msg, to->powerups[i] ); // WARNING: powerups use 32 bits, not 16
      }
    }
  } else {
    MSG_WriteBits( msg, 0, 1 ); // unchanged
  }

}


/*
=======================================================================================

  READING FUNCTIONS

=======================================================================================
*/

/*
============
MSG_BeginReading
============
*/
void MSG_BeginReading( sizebuf_t *msg ) {
  msg->readcount = 0;
  msg->bit = 0;
  msg->uncompressed = qtrue;
}



/*
============
MSG_ReadBits
============
*/
int MSG_ReadBits( sizebuf_t *msg, int bits ) {
  int i;
  int val;
  int bitmask = 0;
  int remaining;
  qboolean extend = qfalse;

  if( !bits || bits < -31 || bits > 32 ) {
    Com_Error( ERR_DROP, "MSG_ReadBits: bad bits %i", bits );
  }

  if( bits < 0 ) {
    bits = -bits;
    extend = qtrue;
  }

  if( msg->uncompressed ) {
    if( bits <= 8 ) {
      bitmask = (unsigned char)msg->data[msg->readcount];
      msg->readcount++;
      msg->bit += 8;
    } else if( bits <= 16 ) {
      bitmask = (unsigned short)(msg->data[msg->readcount]
        + (msg->data[msg->readcount+1] << 8));
      msg->readcount += 2;
      msg->bit += 16;
    } else if( bits <= 32 ) {
      bitmask = msg->data[msg->readcount]
        + (msg->data[msg->readcount+1] << 8)
        + (msg->data[msg->readcount+2] << 16)
        + (msg->data[msg->readcount+3] << 24);
      msg->readcount += 4;
      msg->bit += 32;
    }
  } else {
    remaining = bits & 7;

    for( i=0 ; i<remaining ; i++ ) {
      val = msg->data[msg->bit >> 3] >> (msg->bit & 7);
      msg->bit++;
      bitmask |= (val & 1) << i;
    }

    for( i=0 ; i<bits-remaining ; i+=8 ) {
      val = Huff_GetByte( msg->data, &msg->bit );
      bitmask |= val << (i + remaining);
    }

    msg->readcount = (msg->bit >> 3) + 1;
  }

  if( extend ) {
    if( bitmask & (1 << (bits - 1)) ) {
      bitmask |= ~((1 << bits) - 1);
    }
  }

  return bitmask;
}

/*
============
MSG_ReadData
============
*/
void MSG_ReadData( sizebuf_t *msg, void *data, int len ) {
  int    i;
  int    c;

  for( i=0 ; i<len ; i++ ) {
    c = MSG_ReadByte( msg );
    if( c == -1 ) {
      break;
    }
    if( data ) {
      ((byte *)data)[i] = c;
    }
  }
}

/*
============
MSG_ReadString
============
*/
char *MSG_ReadString( sizebuf_t *msg ) {
  static char  string[MAX_STRING_CHARS];
  int      i;
  int      c;

  for( i=0 ; i<sizeof( string )-1; i++ ) {
    c = MSG_ReadByte( msg );
    if( c == -1 || c == 0 ) {
      break;
    }
    if( c == '%' || c > 127 ) {
      c = '.';
    }
    string[i] = c;
  }

  string[i] = 0;

  return string;
}

/*
============
MSG_ReadStringLine
============
*/
char *MSG_ReadStringLine( sizebuf_t *msg ) {
  static char  string[MAX_STRING_CHARS];
  int      i;
  int      c;

  for( i=0 ; i<sizeof( string )-1 ; i++ ) {
    c = MSG_ReadByte( msg );
    if( c == -1 || c == 0 || c == '\n' ) {
      break;
    }
    if( c == '%' || c > 127 ) {
      c = '.';
    }
    string[i] = c;
  }

  string[i] = 0;

  return string;
}

/*
============
MSG_ReadBigString
============
*/
char *MSG_ReadBigString( sizebuf_t *msg ) {
  static char  string[BIG_INFO_STRING];
  int      i;
  int      c;

  for( i=0 ; i<sizeof( string )-1 ; i++ ) {
    c = MSG_ReadByte( msg );
    if( c == -1 || c == 0 ) {
      break;
    }
    if( c == '%' || c > 127 ) {
      c = '.';
    }
    string[i] = c;
  }

  string[i] = 0;

  return string;
}

/*
============
MSG_ReadDeltaEntity

  'from' == NULL  -->  nodelta update
  'to'   == NULL  -->  do nothing
============
*/
void MSG_ReadDeltaEntity( sizebuf_t *msg, const entityState_t *from, entityState_t *to, int number ) {
  field_t  *field;
  int    maxFieldNum;
  int    i;

  if( number < 0 || number >= MAX_GENTITIES ) {
    Com_Error( ERR_DROP, "MSG_ReadDeltaEntity: Bad delta entity number: %i\n", number );
  }

  if( !to ) {
    return;
  }

  if( MSG_ReadBits( msg, 1 ) ) {
    memset( to, 0, sizeof( *to ) );
    to->number = ENTITYNUM_NONE;
    return;  // removed
  }

  if( !from ) {
    memset( to, 0, sizeof( *to ) ); // nodelta update
  } else {
    memcpy( to, from, sizeof( *to ) );
  }
  to->number = number;

  if( !MSG_ReadBits( msg, 1 ) ) {
    return; // unchanged
  }

  maxFieldNum = MSG_ReadByte( msg );
  if( maxFieldNum > esTableSize ) {
    Com_Error( ERR_DROP, "MSG_ReadDeltaEntity: maxFieldNum > esTableSize" );
  }

  //
  // read all modified fields
  //
  for( i=0, field=esTable ; i<maxFieldNum ; i++, field++ ) {
    if( !MSG_ReadBits( msg, 1 ) ) {
      continue; // field unchanged
    }
    if( !MSG_ReadBits( msg, 1 ) ) {
      FIELD_INTEGER( to ) = 0;
      continue; // field set to zero
    }

    if( field->bits ) {
      FIELD_INTEGER( to ) = MSG_ReadBits( msg, field->bits );
      continue;  // integer value
    }

    //
    // read packed float value
    //
    if( !MSG_ReadBits( msg, 1 ) ) {
      FIELD_FLOAT( to ) = (float)(MSG_ReadBits( msg, SNAPPED_BITS ) - MAX_SNAPPED/2);
    } else {
      FIELD_INTEGER( to ) = MSG_ReadLong( msg );
    }
  }
}


/*
============
MSG_ReadDeltaPlayerstate

  'from' == NULL  -->  nodelta update
  'to'   == NULL  -->  do nothing
============
*/
void MSG_ReadDeltaPlayerstate( sizebuf_t *msg, const playerState_t *from, playerState_t *to ) {
  field_t  *field;
  int    maxFieldNum;
  int    bitmask;
  int    i;

  if( !to ) {
    return;
  }

  if( !from ) {
    memset( to, 0, sizeof( *to ) ); // nodelta update
  } else {
    memcpy( to, from, sizeof( *to ) );
  }

  maxFieldNum = MSG_ReadByte( msg );
  if( maxFieldNum > psTableSize ) {
    Com_Error( ERR_DROP, "MSG_ReadDeltaPlayerstate: maxFieldNum > psTableSize" );
  }

  //
  // read all modified fields
  //
  for( i=0, field=psTable ; i<maxFieldNum ; i++, field++ ) {
    if( !MSG_ReadBits( msg, 1 ) ) {
      continue; // field unchanged
    }

    if( field->bits ) {
      FIELD_INTEGER( to ) = MSG_ReadBits( msg, field->bits );
      continue;  // integer value
    }

    //
    // read packed float value
    //
    if( !MSG_ReadBits( msg, 1 ) ) {
      FIELD_FLOAT( to ) = (float)(MSG_ReadBits( msg, SNAPPED_BITS ) - MAX_SNAPPED/2);
    } else {
      FIELD_INTEGER( to ) = MSG_ReadLong( msg );
    }
  }

  //
  // read all modified arrays
  //
  if( !MSG_ReadBits( msg, 1 ) ) {
    return; // no arrays modified
  }

  // PS_STATS
  if( MSG_ReadBits( msg, 1 ) ) {
    bitmask = MSG_ReadShort( msg );
    for( i=0 ; i<MAX_STATS ; i++ ) {
      if( bitmask & (1 << i) ) {
        to->stats[i] = MSG_ReadSignedShort( msg ); // PS_STATS can be negative
      }
    }
  }

  // PS_PERSISTANT
  if( MSG_ReadBits( msg, 1 ) ) {
    bitmask = MSG_ReadShort( msg );
    for( i=0 ; i<MAX_PERSISTANT ; i++ ) {
      if( bitmask & (1 << i) ) {
        to->persistant[i] = MSG_ReadSignedShort( msg ); // PS_PERSISTANT can be negative
      }
    }
  }

  // PS_AMMO
  if( MSG_ReadBits( msg, 1 ) ) {
    bitmask = MSG_ReadShort( msg );
    for( i=0 ; i<MAX_WEAPONS ; i++ ) {
      if( bitmask & (1 << i) ) {
        to->ammo[i] = MSG_ReadShort( msg );
      }
    }
  }

  // PS_POWERUPS
  if( MSG_ReadBits( msg, 1 ) ) {
    bitmask = MSG_ReadShort( msg );
    for( i=0 ; i<MAX_POWERUPS ; i++ ) {
      if( bitmask & (1 << i) ) {
        to->powerups[i] = MSG_ReadLong( msg ); // WARNING: powerups use 32 bits, not 16
      }
    }
  }
}




