//
// dump.c - ugly stuff, don't look!
//

#include "main.h"

typedef struct {
  entityState_t  currentState;  // zeiger auf ringbuff pso
  qboolean    currentValid;  // true if cg.frame holds this entity
  int        previousEvent;
} centity_t;

static char clientNames[MAX_CLIENTS][36];
static centity_t  centities[MAX_GENTITIES];

static void UpdateClientInfo( int clientNum, const char *info ) {
  if( !info || !info[0] ) {
    Q_strncpyz( clientNames[clientNum], "BADNAME", sizeof( clientNames[0] ) );
  } else {
    Q_strncpyz( clientNames[clientNum], Info_ValueForKey( info, "n" ), sizeof( clientNames[0] ) );
  }
}

static void CG_CheckEvents( centity_t *cent ) {
  entityState_t *es = &cent->currentState;
  int event;
  int  mod;
  int  target, attacker;
  char *targetName, *attackerName;
  char      *message;
  char      *message2 = "";

  // check for event-only entities
  if( es->eType > ET_EVENTS ) {
    if( cent->previousEvent ) {
      return;  // already fired
    }
    cent->previousEvent = 1;

    es->event = es->eType - ET_EVENTS;
  } else {
    // check for events riding with another entity
    if ( es->event == cent->previousEvent ) {
      return;
    }
    cent->previousEvent = es->event;
    if( ( es->event & ~EV_EVENT_BITS ) == 0 ) {
      return;
    }
  }

  event = es->event & ~EV_EVENT_BITS;
  if( event != EV_OBITUARY ) {
    return;
  }

  target = es->otherEntityNum;
  attacker = es->otherEntityNum2;
  mod = es->eventParm;

  if( target < 0 || target >= MAX_CLIENTS ) {
    return;
  }

  if( attacker < 0 || attacker >= MAX_CLIENTS ) {
    attacker = target;
  }

  targetName = clientNames[target];
  attackerName = clientNames[attacker];

  if (attacker == target)  // selbstmord oder "unfall"
  {
    switch (mod)
    {
      case MOD_GRENADE_SPLASH:message = "tripped on own grenade.";  break;
      case MOD_ROCKET_SPLASH:  message = "blew up by own rocket.";break;
      case MOD_PLASMA_SPLASH:  message = "melted by own plasma.";  break;
      case MOD_BFG_SPLASH:  message = "should have used a smaller gun.";  break;
      case MOD_SUICIDE:    message = "suicides";break;
      case MOD_FALLING:    message = "cratered";break;
      case MOD_CRUSH:      message = "was squished";break;
      case MOD_WATER:      message = "sank like a rock";break;
      case MOD_SLIME:      message = "melted";  break;
      case MOD_LAVA:      message = "does a back flip into the lava";  break;
      case MOD_TARGET_LASER:  message = "saw the light";break;
      case MOD_TRIGGER_HURT:  message = "was in the wrong place";  break;
      default:        message = "committed suicid."; break;
    }
    Com_Printf( "%s %s.\n", targetName, message );
    return;
  }

  switch (mod)
  {
    case MOD_GRAPPLE:  message = "was caught by";  break;
    case MOD_GAUNTLET:    message = "was pummeled by";break;
    case MOD_MACHINEGUN:  message = "was machinegunned by";break;
    case MOD_SHOTGUN:    message = "was gunned down by";  break;
    case MOD_GRENADE:    message = "ate";message2 = "'s grenade";break;
    case MOD_GRENADE_SPLASH:message = "was shredded by";message2 = "'s shrapnel";break;
    case MOD_ROCKET:    message = "ate";message2 = "'s rocket";break;
    case MOD_ROCKET_SPLASH:  message = "almost dodged";message2 = "'s rocket";break;
    case MOD_PLASMA:    message = "was melted by";message2 = "'s plasmagun";break;
    case MOD_PLASMA_SPLASH:  message = "was melted by";message2 = "'s plasmagun";break;
    case MOD_RAILGUN:    message = "was railed by";break;
    case MOD_LIGHTNING:    message = "was electrocuted by";break;
    case MOD_BFG:
    case MOD_BFG_SPLASH:  message = "was blasted by";message2 = "'s BFG";  break;
    case MOD_TELEFRAG:    message = "tried to invade";message2 = "'s personal space";  break;
    default:        message = "was killed by";break;
  }
  Com_Printf( "%s %s %s%s.\n", targetName, message, attackerName, message2);
  return;


  // we don't know what it was
  Com_Printf( "%s died.\n", targetName );
}

void GameStateParsed( void ) {
  int i;
  char *configString;

  for( i=0 ; i<MAX_CONFIGSTRINGS ; i++ ) {
    configString = Com_GetStringFromGameState( &ds.gameState, i );
    if( configString[0] ) {
      if( i < RESERVED_CONFIGSTRINGS ) {
        append_result( "%s_info:\n", (i == CS_SERVERINFO) ? "server" : "system" );
        Info_Print( configString );
        append_result( "\n" );
      }/* else {
        Com_Printf( "configString %i: \"%s\"\n", i, configString );
      }*/
    }
  }

  for( i=0 ; i<MAX_CLIENTS ; i++ ) {
    configString = Com_GetStringFromGameState( &ds.gameState, CS_PLAYERS + i );
    UpdateClientInfo( i, configString );
  }
}

static void UpdateConfigString( int index, char *string ) {
  Com_InsertIntoGameState( &ds.gameState, index, string );

  if( index > CS_PLAYERS && index < CS_PLAYERS + MAX_CLIENTS ) {
    UpdateClientInfo( index, string );
  }
}

void NewFrameParsed( void ) {
  int i;
  entityState_t *es;
  char *serverCommand;
  char *args;

  for( i=ds.currentServerCommandNum+1 ; i<=ds.lastServerCommandNum ; i++ ) {
    serverCommand = ds.serverCommands[i & SERVERCMD_MASK];
    Cmd_TokenizeString( serverCommand );

    if( !Q_stricmp( Cmd_Argv( 0 ), "print" ) || !Q_stricmp( Cmd_Argv( 0 ), "cp" ) ) {
      args = Cmd_Args();
      args[strlen(args)-2] = 0;
      args++;

      append_result("  - \"%s\"\n", args);
    } else if( !Q_stricmp( Cmd_Argv( 0 ), "cs" ) ) {
      UpdateConfigString( atoi( Cmd_Argv( 1 ) ), Cmd_Argv( 2 ) );
    }

//    Com_Printf( "serverCommand: %i \"%s\"\n", i, Com_TranslateLinefeeds(  ) );
  }
  ds.currentServerCommandNum = ds.lastServerCommandNum;

  if( !ds.snap || !ds.snap->valid ) {
    return;
  }

  for( i=0 ; i<MAX_GENTITIES ; i++ ) {
    centities[i].currentValid = qfalse;
  }

  for( i=0 ; i<ds.snap->numEntities ; i++ ) {
    es = &ds.parseEntities[(ds.snap->firstEntity+i) & PARSE_ENTITIES_MASK];
    memcpy( &centities[es->number].currentState, es, sizeof( entityState_t ) );
    centities[es->number].currentValid = qtrue;
    CG_CheckEvents( &centities[es->number] );
  }

  for( i=0 ; i<MAX_GENTITIES ; i++ ) {
    if( !centities[i].currentValid ) {
      centities[i].previousEvent = 0;
    }
  }
}
