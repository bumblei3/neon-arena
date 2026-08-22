// NeonArena wave-survival gametype logic (GT_NEONWAVE)
// Spawns escalating bot waves, ends game when all humans die.
// Applied via patches/040+050; this file is copied to code/game/ by CI.
#include "g_local.h"

#define NW_FIRST_WAVE_DELAY	5000	// ms after map start
#define NW_WAVE_BREAK		4000	// ms between waves
#define NW_MAX_WAVE			20

static int nw_wave;				// current wave (1-based)
static int nw_aliveBots;
static int nw_nextSpawnTime;
static qboolean nw_started;

void NeonWave_Reset( void ) {
	nw_wave = 0;
	nw_aliveBots = 0;
	nw_nextSpawnTime = 0;
	nw_started = qfalse;
}

static void NW_SpawnBot( void ) {
	// queue a bot via the server console command (skill 3)
	trap_SendConsoleCommand( EXEC_APPEND, "addbot sarge 3\n" );
}

void NeonWave_StartWave( int num ) {
	int i;
	nw_wave = num;
	trap_SetConfigstring( CS_NEONWAVE, va( "%i %i", num, 0 ) );
	G_Printf( "NeonWave: starting wave %i (%i bots)\n", num, num + 1 );
	for ( i = 0; i <= num && i < MAX_CLIENTS; i++ ) {
		NW_SpawnBot();
	}
	nw_aliveBots += num + 1;
}

int NeonWave_GetWave( void ) {
	return nw_wave;
}

// called from G_RunFrame every server frame
void NeonWave_Frame( void ) {
	int humans, bots, i;
	gentity_t *ent;

	if ( g_gametype.integer != GT_NEONWAVE ) {
		return;
	}

	// count players
	humans = 0; bots = 0;
	for ( i = 0; i < level.maxclients; i++ ) {
		ent = &g_entities[i];
		if ( !ent->inuse || !ent->client ) continue;
		if ( ent->r.svFlags & SVF_BOT ) bots++;
		else if ( ent->client->pers.connected == CON_CONNECTED ) humans++;
	}
	nw_aliveBots = bots;

	// wait for a human to be in game
	if ( !nw_started ) {
		if ( humans > 0 && level.time > NW_FIRST_WAVE_DELAY ) {
			nw_started = qtrue;
			NeonWave_StartWave( 1 );
		}
		return;
	}

	// player died -> game over
	if ( humans == 0 ) {
		G_Printf( "NeonWave: all humans dead - game over at wave %i\n", nw_wave );
		LogExit( "NeonWave over" );
		NeonWave_Reset();
		return;
	}

	// wave cleared -> next wave after break
	if ( nw_aliveBots == 0 && level.time > nw_nextSpawnTime ) {
		if ( nw_wave >= NW_MAX_WAVE ) {
			G_Printf( "NeonWave: all %i waves survived!\n", NW_MAX_WAVE );
			LogExit( "All waves cleared" );
			NeonWave_Reset();
			return;
		}
		NeonWave_StartWave( nw_wave + 1 );
		nw_nextSpawnTime = level.time + NW_WAVE_BREAK;
	}
}
