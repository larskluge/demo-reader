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
// msg.h - message i/o subsystem API
//

#define  MAX_MSGLEN    0x4000  // max length of demo message

typedef struct sizebuf_s {
  qboolean  allowoverflow;  // if false, do a Com_Error
  qboolean  overflowed;     // set to true if the buffer size failed
  qboolean  uncompressed;  // don't do Huffman encoding, write raw bytes
  byte    *data;  // pointer to message buffer, set by MSG_Init
  int    maxsize;  // size in bytes of message buffer, set by MSG_Init
  int    cursize;  // number of bytes written to the buffer, set by MSG_WriteBits
  int    readcount;  // number of bytes read from the buffer, set by MSG_ReadBits
  int    bit;    // number of bits written to or read from the buffer
} sizebuf_t;


void  MSG_Init( sizebuf_t *msg, byte *buffer, int size );
void  MSG_InitRaw( sizebuf_t *msg, byte *buffer, int size );
void  MSG_Clear( sizebuf_t *msg );
void  MSG_SetBitstream( sizebuf_t *msg );
void  MSG_WriteRawData( sizebuf_t *msg, const void *data, int length );

void  MSG_BeginWriting( sizebuf_t *msg );
void  MSG_WriteBits( sizebuf_t *msg, int value, int bits );
void  MSG_WriteData( sizebuf_t *msg, const void *data, int length );
void  MSG_WriteString( sizebuf_t *msg, const char *string );
void  MSG_WriteBigString( sizebuf_t *msg, const char *string );
void  MSG_WriteDeltaEntity( sizebuf_t *msg, const entityState_t *from, const entityState_t *to, qboolean force );
void  MSG_WriteDeltaPlayerstate( sizebuf_t *msg, const playerState_t *from, const playerState_t *to );

void  MSG_BeginReading( sizebuf_t *msg );
int    MSG_ReadBits( sizebuf_t *msg, int bits );
void  MSG_ReadData( sizebuf_t *msg, void *data, int len );
char *  MSG_ReadString( sizebuf_t *msg );
char *  MSG_ReadStringLine( sizebuf_t *msg );
char *  MSG_ReadBigString( sizebuf_t *msg );
void  MSG_ReadDeltaEntity( sizebuf_t *msg, const entityState_t *from, entityState_t *to, int number );
void  MSG_ReadDeltaPlayerstate( sizebuf_t *msg, const playerState_t *from, playerState_t *to );

#define  MSG_WriteByte(msg,c)      MSG_WriteBits(msg,c,8)
#define  MSG_WriteShort(msg,c)      MSG_WriteBits(msg,c,16)
#define  MSG_WriteSignedShort(msg,c)    MSG_WriteBits(msg,c,-16)
#define  MSG_WriteLong(msg,c)      MSG_WriteBits(msg,c,32)

static ID_INLINE int MSG_ReadByte( sizebuf_t *msg ) {
  int c = MSG_ReadBits( msg, 8 ) & 0xFF;

  if( msg->readcount > msg->cursize ) {
    return -1;
  }

  return c;
}

static ID_INLINE int MSG_ReadShort( sizebuf_t *msg ) {
  int c = MSG_ReadBits( msg, 16 );

  if( msg->readcount > msg->cursize ) {
    return -1;
  }

  return c;
}

static ID_INLINE int MSG_ReadSignedShort( sizebuf_t *msg ) {
  int c = MSG_ReadBits( msg, -16 );

  if( msg->readcount > msg->cursize ) {
    return -1;
  }

  return c;
}

static ID_INLINE int MSG_ReadLong( sizebuf_t *msg ) {
  int c = MSG_ReadBits( msg, 32 );

  if( msg->readcount > msg->cursize ) {
    return -1;
  }

  return c;
}



