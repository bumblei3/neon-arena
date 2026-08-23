// NeonArena wave-survival gametype logic (GT_NEONWAVE)
// Spawns escalating bot waves, tracks score + best-wave highscore.
#include "g_local.h"

#ifdef NEONARENA_MOD

#define NW_FIRST_WAVE_DELAY	5000	// ms after map start
#define NW_WAVE_BREAK		4000	// ms between waves
#define NW_MAX_WAVE			20
#define NW_BOSS_WAVE		10	// from here on, each wave gets one boss drone

// CS_NEONWAVE payload: "<wave> <event> <bossHp> <bossMax>"
// event: 0 = wave running, 1 = wave just cleared
#define NW_EV_RUNNING		0
#define NW_EV_CLEARED		1

static int nw_wave;				// current wave (1-based)
static int nw_aliveBots;
static int nw_nextSpawnTime;
static qboolean nw_started;
static int nw_botCounter;		// for unique bot names per wave

void NeonWave_Reset( void ) {
	nw_wave = 0;
	nw_aliveBots = 0;
	nw_nextSpawnTime = 0;
	nw_started = qfalse;
	nw_botCounter = 0;
}

static void NW_SpawnBot( int skill ) {
	// uniquely named bot so the killfeed stays readable
	trap_SendConsoleCommand( EXEC_APPEND,
		va("addbot sarge %i \"Drone W%d-%d\"\n", skill, nw_wave, ++nw_botCounter) );
}

/*
================
NW_SpawnBoss

Queue one boss drone (4x health via userinfo flag).
================
*/
static void NW_SpawnBoss( void ) {
	trap_Cvar_Set( "g_neonwave_nextboss", "1" );
	trap_SendConsoleCommand( EXEC_APPEND,
		va("addbot sarge 5 \"BOSS W%d\"\n", nw_wave) );
}

/*
================
NW_BossHealthPayload

Find the live boss and append its health to the configstring payload.
================
*/
static void NW_BossHealthPayload( char *buf, int size ) {
	gentity_t *ent;
	int i;

	buf[0] = '\0';
	for ( i = 0; i < level.maxclients; i++ ) {
		ent = &g_entities[i];
		if ( !ent->inuse || !ent->client ) continue;
		if ( !( ent->r.svFlags & SVF_BOT ) ) continue;
		if ( ent->health <= 0 ) continue;
		if ( !ent->client->pers.neonwaveBoss ) continue;
		Com_sprintf( buf, size, " %i %i", ent->health,
			ent->client->ps.stats[STAT_MAX_HEALTH] );
		return;
	}
}

/*
================
NW_SendStatus

Push "<wave> <event><bossHealth>" onto the NEONWAVE configstring.
================
*/
static void NW_SendStatus( int event ) {
	char bossPart[32];
	NW_BossHealthPayload( bossPart, sizeof(bossPart) );
	trap_SetConfigstring( CS_NEONWAVE, va( "%i %i%s", nw_wave, event, bossPart ) );
}

/*
================
NeonWave_DropReward

Spawn health/armor/ammo rewards at each living human's feet.
Called after a wave is cleared, before the next one starts.
================
*/
void NeonWave_DropReward( int clearedWave ) {
	gentity_t *ent;
	vec3_t origin, velocity = {0, 0, 20};
	gitem_t *mega, *armor, *ammo, *ra;
	int i;

	mega  = BG_FindItem( "Mega Health" );
	armor = BG_FindItem( "Heavy Armor" );
	ammo  = BG_FindItemForWeapon( WP_LIGHTNING );
	if (!mega)  mega  = BG_FindItem( "5 Health" );
	if (!armor) armor = BG_FindItem( "Armor Shard" );
	// rail slugs: keep the railgun fed (ammo economy)
	ra = BG_FindItemForWeapon( WP_RAILGUN );

	for ( i = 0; i < level.maxclients; i++ ) {
		ent = &g_entities[i];
		if ( !ent->inuse || !ent->client ) continue;
		if ( ent->r.svFlags & SVF_BOT ) continue;
		if ( ent->client->pers.connected != CON_CONNECTED ) continue;

		VectorCopy( ent->r.currentOrigin, origin );
		origin[2] += 24;
		if ( mega )  LaunchItem( mega,  origin, velocity );
		if ( armor ) LaunchItem( armor, origin, velocity );
		if ( ammo )  LaunchItem( ammo,  origin, velocity );
		if ( ra )    LaunchItem( ra,    origin, velocity );
	}
}

/*
================
NeonWave_UpdateHighscore

Persist best reached wave into an archived cvar so it survives restarts.
================
*/
static void NeonWave_UpdateHighscore( void ) {
	char bestBuf[16];
	int best;
	trap_Cvar_VariableStringBuffer( "g_neonwave_best", bestBuf, sizeof(bestBuf) );
	best = atoi(bestBuf);
	if ( nw_wave > best ) {
		trap_Cvar_Set( "g_neonwave_best", va("%i", nw_wave) );
		G_Printf( "NeonWave: NEW BEST wave %i (was %i)\n", nw_wave, best );
	}
}

void NeonWave_StartWave( int num ) {
	int i;
	int skill = 1 + num / 3;
	if ( skill > 5 ) skill = 5;
	nw_wave = num;
	nw_botCounter = 0;
	NW_SendStatus( NW_EV_RUNNING );
	G_Printf( "NeonWave: starting wave %i (%i bots, skill %i)%s\n", num, num + 1, skill,
		num >= NW_BOSS_WAVE ? " + BOSS" : "" );
	if ( num >= NW_BOSS_WAVE ) {
		NW_SpawnBoss();
	}
	for ( i = 0; i <= num && i < MAX_CLIENTS; i++ ) {
		NW_SpawnBot( skill );
	}
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
		NeonWave_UpdateHighscore();
		LogExit( "NeonWave over" );
		NeonWave_Reset();
		return;
	}

	// wave cleared -> next wave after break
	if ( nw_aliveBots == 0 && level.time > nw_nextSpawnTime ) {
		// announce clear event (cgame plays jingle) + grant an upgrade point
		NW_SendStatus( NW_EV_CLEARED );
		NeonWave_DropReward( nw_wave );

		{
			char ptsBuf[16];
			int pts;
			trap_Cvar_VariableStringBuffer( "g_neonwave_upgradepoints", ptsBuf, sizeof(ptsBuf) );
			pts = atoi(ptsBuf);
			pts += ( nw_wave >= NW_BOSS_WAVE ? 2 : 1 ); // boss waves grant 2
			trap_Cvar_Set( "g_neonwave_upgradepoints", va("%i", pts) );
			G_Printf( "NeonWave: upgrade point granted (%i banked)\n", pts );
		}

		if ( nw_wave >= NW_MAX_WAVE ) {
			G_Printf( "NeonWave: all %i waves survived!\n", NW_MAX_WAVE );
			NeonWave_UpdateHighscore();
			LogExit( "All waves cleared" );
			NeonWave_Reset();
			return;
		}
		NeonWave_StartWave( nw_wave + 1 );
		nw_nextSpawnTime = level.time + NW_WAVE_BREAK;
	} else if ( nw_aliveBots > 0 && nw_wave >= NW_BOSS_WAVE ) {
		// refresh boss health in the status string while a boss is alive
		static int lastRefresh = 0;
		if ( level.time > lastRefresh ) {
			lastRefresh = level.time + 250;
			NW_SendStatus( NW_EV_RUNNING );
		}
	}
}

#endif // NEONARENA_MOD
