// Copyright (C) 1999-2000 Id Software, Inc.
//
// q_shared.c -- stateless support routines that are included in each code dll

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#define ID_INLINE

#ifdef WIN32
#  pragma warning(disable:4018)
#endif

#ifdef WIN32
#  undef ID_INLINE
#  define ID_INLINE __inline
#endif

#define LittleLong

typedef enum {qfalse, qtrue}  qboolean;
typedef unsigned char      byte;
typedef float          vec3_t[3];

#ifndef NULL
#define NULL ((void *)0)
#endif

#define MAXPRINTMSG      16284

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define  MAX_STRING_CHARS  1024  // max length of a string passed to Cmd_TokenizeString
#define  MAX_STRING_TOKENS  1024  // max tokens resulting from Cmd_TokenizeString
#define  MAX_TOKEN_CHARS    1024  // max length of an individual token

#define  MAX_INFO_STRING    1024
#define  MAX_INFO_KEY      1024
#define  MAX_INFO_VALUE    1024

#define  BIG_INFO_STRING    8192  // used for system info key only
#define  BIG_INFO_KEY      8192
#define  BIG_INFO_VALUE    8192


#define  MAX_QPATH      64    // max length of a quake game pathname
#ifdef PATH_MAX
#define MAX_OSPATH      PATH_MAX
#else
#define  MAX_OSPATH      256    // max length of a filesystem pathname
#endif

#define  MAX_MAP_AREA_BYTES    32    // bit vector of area visibility


#ifdef ERR_FATAL
#undef ERR_FATAL      // this is be defined in malloc.h
#endif

// parameters to the main Error routine
typedef enum {
  ERR_FATAL,          // exit the entire game with a popup window
  ERR_DROP,          // print to console and disconnect from game
  ERR_SERVERDISCONNECT,    // don't kill server
  ERR_DISCONNECT,        // client disconnected from the server
  ERR_NEED_CD          // pop up the need-cd dialog
} errorParm_t;

void Com_Error( errorParm_t level, const char *error, ... );
void Com_Printf( const char *text, ... );

void  Com_sprintf (char *dest, int size, const char *fmt, ...);
char  *va(char *format, ...);

// buffer size safe library replacements
void  Q_strncpyz( char *dest, const char *src, int destsize );
void  Q_strcat( char *dest, int size, const char *src );

int    Q_stricmpn (const char *s1, const char *s2, int n);
int    Q_strncmp (const char *s1, const char *s2, int n);
int    Q_stricmp (const char *s1, const char *s2);
void  Q_strcat( char *dest, int size, const char *src );

//
// key / value info strings
//
char *Info_ValueForKey( const char *s, const char *key );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_big( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
qboolean Info_Validate( const char *s );
void Info_NextPair( const char **s, char *key, char *value );


/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/

#define  ANGLE2SHORT(x)  ((int)((x)*65536/360) & 65535)
#define  SHORT2ANGLE(x)  ((x)*(360.0/65536))

#define  SNAPFLAG_RATE_DELAYED  1
#define  SNAPFLAG_NOT_ACTIVE    2  // snapshot used during connection and for zombies
#define SNAPFLAG_SERVERCOUNT  4  // toggled every map_restart so transitions can be detected

//
// per-level limits
//
#define  MAX_CLIENTS      64    // absolute limit
#define MAX_LOCATIONS    64

#define  GENTITYNUM_BITS    10    // don't need to send any more
#define  MAX_GENTITIES    (1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
#define  ENTITYNUM_NONE    (MAX_GENTITIES-1)
#define  ENTITYNUM_WORLD    (MAX_GENTITIES-2)
#define  ENTITYNUM_MAX_NORMAL  (MAX_GENTITIES-2)


#define  MAX_MODELS      256    // these are sent over the net as 8 bits
#define  MAX_SOUNDS      256    // so they cannot be blindly increased


#define  MAX_CONFIGSTRINGS  1024

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
#define  CS_SERVERINFO    0    // an info string with all the serverinfo cvars
#define  CS_SYSTEMINFO    1    // an info string for server system to client system configuration (timescale, etc)

#define  RESERVED_CONFIGSTRINGS  2  // game can't modify below this, only the system can

#define  MAX_GAMESTATE_CHARS  16000
typedef struct {
  int      stringOffsets[MAX_CONFIGSTRINGS];
  char    stringData[MAX_GAMESTATE_CHARS];
  int      dataCount;
} gameState_t;

//=========================================================

// bit field limits
#define  MAX_STATS        16
#define  MAX_PERSISTANT      16
#define  MAX_POWERUPS      16
#define  MAX_WEAPONS        16

#define  MAX_PS_EVENTS      2

#define PS_PMOVEFRAMECOUNTBITS  6

// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.
typedef struct playerState_s {
  int      commandTime;  // cmd->serverTime of last executed command
  int      pm_type;
  int      bobCycle;    // for view bobbing and footstep generation
  int      pm_flags;    // ducked, jump_held, etc
  int      pm_time;

  vec3_t    origin;
  vec3_t    velocity;
  int      weaponTime;
  int      gravity;
  int      speed;
  int      delta_angles[3];  // add to command angles to get view direction
                  // changed by spawns, rotating objects, and teleporters

  int      groundEntityNum;// ENTITYNUM_NONE = in air

  int      legsTimer;    // don't change low priority animations until this runs out
  int      legsAnim;    // mask off ANIM_TOGGLEBIT

  int      torsoTimer;    // don't change low priority animations until this runs out
  int      torsoAnim;    // mask off ANIM_TOGGLEBIT

  int      movementDir;  // a number 0 to 7 that represents the reletive angle
                // of movement to the view angle (axial and diagonals)
                // when at rest, the value will remain unchanged
                // used to twist the legs during strafing

  vec3_t    grapplePoint;  // location of grapple to pull towards if PMF_GRAPPLE_PULL

  int      eFlags;      // copied to entityState_t->eFlags

  int      eventSequence;  // pmove generated events
  int      events[MAX_PS_EVENTS];
  int      eventParms[MAX_PS_EVENTS];

  int      externalEvent;  // events set on player from another source
  int      externalEventParm;
  int      externalEventTime;

  int      clientNum;    // ranges from 0 to MAX_CLIENTS-1
  int      weapon;      // copied to entityState_t->weapon
  int      weaponstate;

  vec3_t    viewangles;    // for fixed views
  int      viewheight;

  // damage feedback
  int      damageEvent;  // when it changes, latch the other parms
  int      damageYaw;
  int      damagePitch;
  int      damageCount;

  int      stats[MAX_STATS];
  int      persistant[MAX_PERSISTANT];  // stats that aren't cleared on death
  int      powerups[MAX_POWERUPS];  // level.time that the powerup runs out
  int      ammo[MAX_WEAPONS];

  int      generic1;
  int      loopSound;
  int      jumppad_ent;  // jumppad entity hit this frame

  // not communicated over the net at all
  int      ping;      // server to game info for scoreboard
  int      pmove_framecount;  // FIXME: don't transmit over the network
  int      jumppad_frame;
  int      entityEventSequence;
} playerState_t;

//===================================================================

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
#define  SOLID_BMODEL  0xffffff

typedef enum {
  TR_STATIONARY,
  TR_INTERPOLATE,        // non-parametric, but interpolate between snapshots
  TR_LINEAR,
  TR_LINEAR_STOP,
  TR_SINE,          // value = base + sin( time / duration ) * delta
  TR_GRAVITY
} trType_t;

typedef struct {
  trType_t  trType;
  int    trTime;
  int    trDuration;      // if non 0, trTime + trDuration = stop time
  vec3_t  trBase;
  vec3_t  trDelta;      // velocity, etc
} trajectory_t;

// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

typedef struct entityState_s {
  int    number;      // entity index
  int    eType;      // entityType_t
  int    eFlags;

  trajectory_t  pos;  // for calculating position
  trajectory_t  apos;  // for calculating angles

  int    time;
  int    time2;

  vec3_t  origin;
  vec3_t  origin2;

  vec3_t  angles;
  vec3_t  angles2;

  int    otherEntityNum;  // shotgun sources, etc
  int    otherEntityNum2;

  int    groundEntityNum;  // -1 = in air

  int    constantLight;  // r + (g<<8) + (b<<16) + (intensity<<24)
  int    loopSound;    // constantly loop this sound

  int    modelindex;
  int    modelindex2;
  int    clientNum;    // 0 to (MAX_CLIENTS - 1), for players and corpses
  int    frame;

  int    solid;      // for client side prediction, trap_linkentity sets this properly

  int    event;      // impulse events -- muzzle flashes, footsteps, etc
  int    eventParm;

  // for players
  int    powerups;    // bit flags
  int    weapon;      // determines weapon and flash model, etc
  int    legsAnim;    // mask off ANIM_TOGGLEBIT
  int    torsoAnim;    // mask off ANIM_TOGGLEBIT

  int    generic1;
} entityState_t;

// ========================================================================================

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
#define  CS_MUSIC        2
#define  CS_MESSAGE        3    // from the map worldspawn's message field
#define  CS_MOTD          4    // g_motd string for server message of the day
#define  CS_WARMUP        5    // server time when the match will be restarted
#define  CS_SCORES1        6
#define  CS_SCORES2        7
#define CS_VOTE_TIME      8
#define CS_VOTE_STRING      9
#define  CS_VOTE_YES        10
#define  CS_VOTE_NO        11

#define CS_TEAMVOTE_TIME    12
#define CS_TEAMVOTE_STRING    14
#define  CS_TEAMVOTE_YES      16
#define  CS_TEAMVOTE_NO      18

#define  CS_GAME_VERSION      20
#define  CS_LEVEL_START_TIME    21    // so the timer only shows the current level
#define  CS_INTERMISSION      22    // when 1, fraglimit/timelimit has been hit and intermission will start in a second or two
#define CS_FLAGSTATUS      23    // string indicating flag status in CTF
#define CS_SHADERSTATE      24
#define CS_BOTINFO        25

#define  CS_ITEMS        27    // string of 0's and 1's that tell which items are present

#define  CS_MODELS        32
#define  CS_SOUNDS        (CS_MODELS+MAX_MODELS)
#define  CS_PLAYERS        (CS_SOUNDS+MAX_SOUNDS)
#define CS_LOCATIONS      (CS_PLAYERS+MAX_CLIENTS)
#define CS_PARTICLES      (CS_LOCATIONS+MAX_LOCATIONS)

#define CS_MAX          (CS_PARTICLES+MAX_LOCATIONS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

typedef enum {
  GT_FFA,        // free for all
  GT_TOURNAMENT,    // one on one tournament
  GT_SINGLE_PLAYER,  // single player ffa

  //-- team games go after this --

  GT_TEAM,      // team deathmatch
  GT_CTF,        // capture the flag
  GT_1FCTF,
  GT_OBELISK,
  GT_HARVESTER,
  GT_MAX_GAME_TYPE
} gametype_t;

typedef enum { GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER } gender_t;

// entityState_t->event values
// entity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field
// will be incremented with each change in the event so
// that an identical event started twice in a row can
// be distinguished.  And off the value with ~EV_EVENT_BITS
// to retrieve the actual event number
#define  EV_EVENT_BIT1    0x00000100
#define  EV_EVENT_BIT2    0x00000200
#define  EV_EVENT_BITS    (EV_EVENT_BIT1|EV_EVENT_BIT2)

#define  EVENT_VALID_MSEC  300

typedef enum {
  EV_NONE,

  EV_FOOTSTEP,
  EV_FOOTSTEP_METAL,
  EV_FOOTSPLASH,
  EV_FOOTWADE,
  EV_SWIM,

  EV_STEP_4,
  EV_STEP_8,
  EV_STEP_12,
  EV_STEP_16,

  EV_FALL_SHORT,
  EV_FALL_MEDIUM,
  EV_FALL_FAR,

  EV_JUMP_PAD,      // boing sound at origin, jump sound on player

  EV_JUMP,
  EV_WATER_TOUCH,  // foot touches
  EV_WATER_LEAVE,  // foot leaves
  EV_WATER_UNDER,  // head touches
  EV_WATER_CLEAR,  // head leaves

  EV_ITEM_PICKUP,      // normal item pickups are predictable
  EV_GLOBAL_ITEM_PICKUP,  // powerup / team sounds are broadcast to everyone

  EV_NOAMMO,
  EV_CHANGE_WEAPON,
  EV_FIRE_WEAPON,

  EV_USE_ITEM0,
  EV_USE_ITEM1,
  EV_USE_ITEM2,
  EV_USE_ITEM3,
  EV_USE_ITEM4,
  EV_USE_ITEM5,
  EV_USE_ITEM6,
  EV_USE_ITEM7,
  EV_USE_ITEM8,
  EV_USE_ITEM9,
  EV_USE_ITEM10,
  EV_USE_ITEM11,
  EV_USE_ITEM12,
  EV_USE_ITEM13,
  EV_USE_ITEM14,
  EV_USE_ITEM15,

  EV_ITEM_RESPAWN,
  EV_ITEM_POP,
  EV_PLAYER_TELEPORT_IN,
  EV_PLAYER_TELEPORT_OUT,

  EV_GRENADE_BOUNCE,    // eventParm will be the soundindex

  EV_GENERAL_SOUND,
  EV_GLOBAL_SOUND,    // no attenuation
  EV_GLOBAL_TEAM_SOUND,

  EV_BULLET_HIT_FLESH,
  EV_BULLET_HIT_WALL,

  EV_MISSILE_HIT,
  EV_MISSILE_MISS,
  EV_MISSILE_MISS_METAL,
  EV_RAILTRAIL,
  EV_SHOTGUN,
  EV_BULLET,        // otherEntity is the shooter

  EV_PAIN,
  EV_DEATH1,
  EV_DEATH2,
  EV_DEATH3,
  EV_OBITUARY,

  EV_POWERUP_QUAD,
  EV_POWERUP_BATTLESUIT,
  EV_POWERUP_REGEN,

  EV_GIB_PLAYER,      // gib a previously living player
  EV_SCOREPLUM,      // score plum

//#ifdef MISSIONPACK
  EV_PROXIMITY_MINE_STICK,
  EV_PROXIMITY_MINE_TRIGGER,
  EV_KAMIKAZE,      // kamikaze explodes
  EV_OBELISKEXPLODE,    // obelisk explodes
  EV_OBELISKPAIN,      // obelisk is in pain
  EV_INVUL_IMPACT,    // invulnerability sphere impact
  EV_JUICED,        // invulnerability juiced effect
  EV_LIGHTNINGBOLT,    // lightning bolt bounced of invulnerability sphere
//#endif

  EV_DEBUG_LINE,
  EV_STOPLOOPINGSOUND,
  EV_TAUNT,
  EV_TAUNT_YES,
  EV_TAUNT_NO,
  EV_TAUNT_FOLLOWME,
  EV_TAUNT_GETFLAG,
  EV_TAUNT_GUARDBASE,
  EV_TAUNT_PATROL

} entity_event_t;

// means of death
typedef enum {
  MOD_UNKNOWN,
  MOD_SHOTGUN,
  MOD_GAUNTLET,
  MOD_MACHINEGUN,
  MOD_GRENADE,
  MOD_GRENADE_SPLASH,
  MOD_ROCKET,
  MOD_ROCKET_SPLASH,
  MOD_PLASMA,
  MOD_PLASMA_SPLASH,
  MOD_RAILGUN,
  MOD_LIGHTNING,
  MOD_BFG,
  MOD_BFG_SPLASH,
  MOD_WATER,
  MOD_SLIME,
  MOD_LAVA,
  MOD_CRUSH,
  MOD_TELEFRAG,
  MOD_FALLING,
  MOD_SUICIDE,
  MOD_TARGET_LASER,
  MOD_TRIGGER_HURT,
#ifdef MISSIONPACK
  MOD_NAIL,
  MOD_CHAINGUN,
  MOD_PROXIMITY_MINE,
  MOD_KAMIKAZE,
  MOD_JUICED,
#endif
  MOD_GRAPPLE
} meansOfDeath_t;

//
// entityState_t->eType
//
typedef enum {
  ET_GENERAL,
  ET_PLAYER,
  ET_ITEM,
  ET_MISSILE,
  ET_MOVER,
  ET_BEAM,
  ET_PORTAL,
  ET_SPEAKER,
  ET_PUSH_TRIGGER,
  ET_TELEPORT_TRIGGER,
  ET_INVISIBLE,
  ET_GRAPPLE,        // grapple hooked on wall
  ET_TEAM,

  ET_EVENTS        // any of the EV_* events can be added freestanding
              // by setting eType to ET_EVENTS + eventNum
              // this avoids having to set eFlags and eventNum
} entityType_t;

char *  COM_Parse( char **data_p );

char *  CopyString( const char *in );

int    Cmd_Argc( void );
char *  Cmd_Argv( int arg );
char *  Cmd_Args( void );
void  Cmd_TokenizeString( char *text );

void  Info_Print( const char *s );

