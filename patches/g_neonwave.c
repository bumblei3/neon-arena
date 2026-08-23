// NeonArena wave-survival gametype logic (GT_NEONWAVE)
// Spawns escalating bot waves, tracks score + best-wave highscore.
#include "g_local.h"

#ifdef NEONARENA_MOD

#define NW_FIRST_WAVE_DELAY	5000	// ms after map start
#define NW_WAVE_BREAK		8000	// ms between waves (upgrade window)
#define NW_MAX_WAVE			20
#define NW_BOSS_WAVE		10	// from here on, each wave gets one boss drone

// Test hooks (used by CI smoke test):
//   g_neonwave_autostart 1   -> waves start without a human player (headless test)
//   g_neonwave_startwave N   -> force-start wave N (polled in NeonWave_Frame)

// CS_NEONWAVE payload: "<wave> <event> <bossHp> <bossMax> <breakMs> <pts> <best> <modifier>"
// event: 0 running, 1 cleared/break, 2 failed, 3 victory
#define NW_EV_RUNNING		0
#define NW_EV_CLEARED		1
#define NW_EV_FAILED		2
#define NW_EV_VICTORY		3

// wave modifiers (from wave 5, one per wave, boss waves excluded)
#define NW_MOD_NONE			0
#define NW_MOD_GLASS		1	// all drones die to one hit, but +2 skill aggression
#define NW_MOD_SWARM		2	// double drone count, skill capped lower
#define NW_MOD_LOWGRAV		3	// g_gravity halved for the wave
#define NW_MOD_DOUBLEPTS	4	// wave clear grants x2 upgrade points

static int nw_wave;				// current wave (1-based)
static int nw_aliveBots;
static int nw_modifier = NW_MOD_NONE;
static int nw_runStartTime;		// run stats: level.time of first wave start
static qboolean nw_started;
static int nw_botCounter;
static qboolean nw_inBreak;
static int nw_breakEnd;
static qboolean nw_waveHadBots;	// true once at least one bot connected this wave
static qboolean nw_over;
static int nw_event;

void NeonWave_Reset( void ) {
	nw_wave = 0;
	nw_aliveBots = 0;
	nw_modifier = NW_MOD_NONE;
	nw_runStartTime = level.time;
	nw_started = qfalse;
	nw_botCounter = 0;
	nw_inBreak = qfalse;
	nw_breakEnd = 0;
	nw_waveHadBots = qfalse;
	nw_over = qfalse;
	nw_event = 0;
	trap_Cvar_Set( "g_neonwave_upgradepoints", "0" );
}

qboolean NeonWave_IsBreak( void ) {
	return ( nw_inBreak && !nw_over ) ? qtrue : qfalse;
}

// test hook: mark the wave loop as started (used by "nwstartwave" console cmd)
void NeonWave_ForceStarted( void ) {
	nw_started = qtrue;
	nw_over = qfalse;
}

// Sync upgrade state to the local player's HUD.
// Packs points+levels into ps.persistant[PERS_CAPTURES] (unused in GT_NEONWAVE):
// bits 0-7 points, 8-11 hp level, 12-15 dmg level, 16-19 speed level
static void NW_SyncUpgrades( void ) {
	char ptsBuf[16];
	int pts, i, val;
	gentity_t *ent;

	trap_Cvar_VariableStringBuffer( "g_neonwave_upgradepoints", ptsBuf, sizeof(ptsBuf) );
	pts = atoi( ptsBuf );
	if ( pts < 0 ) pts = 0;
	if ( pts > 255 ) pts = 255;

	for ( i = 0; i < level.maxclients; i++ ) {
		ent = &g_entities[i];
		if ( !ent->inuse || !ent->client ) continue;
		if ( ent->client->pers.connected != CON_CONNECTED ) continue;
		if ( ent->r.svFlags & SVF_BOT ) continue;
		val = pts
			| ( ( ent->client->pers.neonwaveUpHp   & 0xF ) << 8 )
			| ( ( ent->client->pers.neonwaveDmg    & 0xF ) << 12 )
			| ( ( ent->client->pers.neonwaveSpeed  & 0xF ) << 16 );
		ent->client->ps.persistant[PERS_CAPTURES] = val;
	}
}

static void NW_SpawnBot( int skill ) {
	trap_SendConsoleCommand( EXEC_APPEND,
		va("addbot sarge %i \"Drone W%d-%d\"\n", skill, nw_wave, ++nw_botCounter) );
}

static void NW_SpawnBoss( void ) {
	trap_Cvar_Set( "g_neonwave_nextboss", "1" );
	trap_SendConsoleCommand( EXEC_APPEND,
		va("addbot sarge 5 \"BOSS W%d\"\n", nw_wave) );
}

static void NW_BossHealth( int *hp, int *maxhp ) {
	gentity_t *ent;
	int i;

	*hp = 0;
	*maxhp = 0;
	for ( i = 0; i < level.maxclients; i++ ) {
		ent = &g_entities[i];
		if ( !ent->inuse || !ent->client ) continue;
		if ( !( ent->r.svFlags & SVF_BOT ) ) continue;
		if ( ent->health <= 0 ) continue;
		if ( !ent->client->pers.neonwaveBoss ) continue;
		*hp = ent->health;
		*maxhp = ent->client->ps.stats[STAT_MAX_HEALTH];
		return;
	}
}

static int NW_Points( void ) {
	char buf[16];
	trap_Cvar_VariableStringBuffer( "g_neonwave_upgradepoints", buf, sizeof(buf) );
	return atoi( buf );
}

static int NW_Best( void ) {
	char buf[16];
	trap_Cvar_VariableStringBuffer( "g_neonwave_best", buf, sizeof(buf) );
	return atoi( buf );
}

/*
================
NW_SendStatus

"<wave> <event> <bossHp> <bossMax> <breakMs> <pts> <best>"
================
*/
// ---- run statistics (aggregated over all humans) ----
// (nw_runStartTime declared with the other statics at top)

static int NW_RunKills( void ) {
	int i, kills = 0;
	gentity_t *ent;
	for ( i = 0; i < level.maxclients; i++ ) {
		ent = &g_entities[i];
		if ( !ent->inuse || !ent->client ) continue;
		if ( ent->r.svFlags & SVF_BOT ) continue;
		if ( ent->client->pers.connected != CON_CONNECTED ) continue;
		kills += ent->client->pers.nwKills;
	}
	return kills;
}

static int NW_RunBestCombo( void ) {
	int i, best = 0;
	gentity_t *ent;
	for ( i = 0; i < level.maxclients; i++ ) {
		ent = &g_entities[i];
		if ( !ent->inuse || !ent->client ) continue;
		if ( ent->r.svFlags & SVF_BOT ) continue;
		if ( ent->client->pers.connected != CON_CONNECTED ) continue;
		if ( ent->client->pers.nwBestCombo > best ) best = ent->client->pers.nwBestCombo;
	}
	return best;
}

static void NW_SendStatus( int event ) {
	int bossHp, bossMax, breakMs;

	nw_event = event;
	NW_BossHealth( &bossHp, &bossMax );
	breakMs = 0;
	if ( nw_inBreak && nw_breakEnd > level.time ) {
		breakMs = nw_breakEnd - level.time;
	}
	// append run stats: "<wave> <ev> <bhp> <bmax> <brk> <pts> <best> <mod> <kills> <bestcombo> <runsec>"
	trap_SetConfigstring( CS_NEONWAVE, va( "%i %i %i %i %i %i %i %i %i %i %i",
		nw_wave, event, bossHp, bossMax, breakMs, NW_Points(), NW_Best(), nw_modifier,
		NW_RunKills(), NW_RunBestCombo(), ( level.time - nw_runStartTime ) / 1000 ) );
}

void NeonWave_RefreshStatus( void ) {
	NW_SendStatus( nw_event );
}

void NeonWave_DropReward( int clearedWave ) {
	gentity_t *ent;
	vec3_t origin, velocity = {0, 0, 20};
	gitem_t *mega, *armor, *ammo, *ra;
	int i;

	(void)clearedWave;
	mega  = BG_FindItem( "Mega Health" );
	armor = BG_FindItem( "Heavy Armor" );
	ammo  = BG_FindItemForWeapon( WP_LIGHTNING );
	if (!mega)  mega  = BG_FindItem( "5 Health" );
	if (!armor) armor = BG_FindItem( "Armor Shard" );
	ra = BG_FindItemForWeapon( WP_RAILGUN );

	for ( i = 0; i < level.maxclients; i++ ) {
		ent = &g_entities[i];
		if ( !ent->inuse || !ent->client ) continue;
		if ( ent->r.svFlags & SVF_BOT ) continue;
		if ( ent->client->pers.connected != CON_CONNECTED ) continue;
		if ( ent->health <= 0 ) continue;

		VectorCopy( ent->r.currentOrigin, origin );
		origin[2] += 24;
		if ( mega )  LaunchItem( mega,  origin, velocity );
		if ( armor ) LaunchItem( armor, origin, velocity );
		if ( ammo )  LaunchItem( ammo,  origin, velocity );
		if ( ra )    LaunchItem( ra,    origin, velocity );
	}
}

static void NeonWave_UpdateHighscore( void ) {
	int best = NW_Best();
	if ( nw_wave > best ) {
		trap_Cvar_Set( "g_neonwave_best", va("%i", nw_wave) );
		G_Printf( "NeonWave: NEW BEST wave %i (was %i)\n", nw_wave, best );
	}
}

static void NW_PickModifier( int num ) {
	static const int pool[4] = { NW_MOD_GLASS, NW_MOD_SWARM, NW_MOD_LOWGRAV, NW_MOD_DOUBLEPTS };
	int idx;
	char mbBuf[8];

	nw_modifier = NW_MOD_NONE;
	if ( num < 5 || num >= NW_BOSS_WAVE || num == NW_MAX_WAVE ) {
		return;
	}
	// test hook: g_neonwave_modifier N forces modifier 1-4
	trap_Cvar_VariableStringBuffer( "g_neonwave_modifier", mbBuf, sizeof(mbBuf) );
	if ( atoi( mbBuf ) >= NW_MOD_GLASS && atoi( mbBuf ) <= NW_MOD_DOUBLEPTS ) {
		nw_modifier = atoi( mbBuf );
		return;
	}
	// deterministic-ish variety: rotate through the pool by wave number
	idx = ( num / 2 + num % 3 ) % 4;
	nw_modifier = pool[idx];
}

static const char *NW_ModifierName( int mod ) {
	switch ( mod ) {
	case NW_MOD_GLASS:		return "GLASS DRONES";
	case NW_MOD_SWARM:		return "SWARM";
	case NW_MOD_LOWGRAV:	return "LOW GRAVITY";
	case NW_MOD_DOUBLEPTS:	return "DOUBLE POINTS";
	default:				return "";
	}
}

void NeonWave_StartWave( int num ) {
	int i;
	int skill = 1 + num / 3;
	int botCount;

	NW_PickModifier( num );
	if ( skill > 5 ) skill = 5;
	nw_wave = num;
	nw_botCounter = 0;
	nw_inBreak = qfalse;
	nw_waveHadBots = qfalse;

	// apply modifier side effects
	if ( nw_modifier == NW_MOD_LOWGRAV ) {
		trap_Cvar_Set( "g_gravity", "400" ); // half of default 800
	} else {
		trap_Cvar_Set( "g_gravity", "800" );
	}
	if ( nw_modifier == NW_MOD_GLASS && skill < 4 ) {
		skill += 1; // glass drones are fast/aggressive
	}
	botCount = num + 1;
	if ( nw_modifier == NW_MOD_SWARM ) {
		botCount *= 2;
	}

	NW_SendStatus( NW_EV_RUNNING );
	if ( nw_modifier != NW_MOD_NONE ) {
		trap_SendServerCommand( -1, va( "cp \"WAVE %i: %s\\n\"", num, NW_ModifierName( nw_modifier ) ) );
	} else {
		trap_SendServerCommand( -1, va( "cp \"WAVE %i\\n\"", num ) );
	}
	G_Printf( "NeonWave: starting wave %i (%i bots, skill %i)%s%s\n", num, botCount, skill,
		num >= NW_BOSS_WAVE ? " + BOSS" : "",
		nw_modifier != NW_MOD_NONE ? va(" [%s]", NW_ModifierName( nw_modifier )) : "" );
	if ( num >= NW_BOSS_WAVE ) {
		NW_SpawnBoss();
	}
	for ( i = 0; i < botCount && i < MAX_CLIENTS - 2; i++ ) {
		NW_SpawnBot( skill );
	}
}

int NeonWave_GetWave( void ) {
	return nw_wave;
}

static void NW_GrantUpgradePoints( void ) {
	int pts = NW_Points();
	int gain = ( nw_wave >= NW_BOSS_WAVE ? 2 : 1 );
	int combo;

	if ( nw_modifier == NW_MOD_DOUBLEPTS ) {
		gain *= 2;
	}
	// combo bonus: +1 point for streaks of 5+ kills
	combo = NW_RunBestCombo();
	if ( combo >= 5 ) {
		gain += combo / 5;
		G_Printf( "NeonWave: combo bonus +%i (best streak %i)\n", combo / 5, combo );
	}
	pts += gain;
	trap_Cvar_Set( "g_neonwave_upgradepoints", va("%i", pts) );
	G_Printf( "NeonWave: upgrade point granted (%i banked)\n", pts );
}

static void NW_EnterBreak( void ) {
	nw_inBreak = qtrue;
	// restore gravity after low-grav wave
	trap_Cvar_Set( "g_gravity", "800" );
	nw_breakEnd = level.time + NW_WAVE_BREAK;
	// test hook: shorten break window when g_neonwave_fastbreak is set
	{
		char fbBuf[8];
		trap_Cvar_VariableStringBuffer( "g_neonwave_fastbreak", fbBuf, sizeof(fbBuf) );
		if ( atoi( fbBuf ) == 1 ) {
			nw_breakEnd = level.time + 500;
		}
	}
	NW_GrantUpgradePoints();
	NeonWave_DropReward( nw_wave );
	NW_SendStatus( NW_EV_CLEARED );
	trap_SendServerCommand( -1, va( "cp \"WAVE %i CLEARED\nF1 HP  F2 DMG  F3 SPEED\"", nw_wave ) );
	G_Printf( "NeonWave: wave %i cleared, break %i ms\n", nw_wave, NW_WAVE_BREAK );
}

static void NW_KickBots( void ) {
	int i;
	gentity_t *ent;
	for ( i = 0; i < level.maxclients; i++ ) {
		ent = &g_entities[i];
		if ( !ent->inuse || !ent->client ) continue;
		if ( !( ent->r.svFlags & SVF_BOT ) ) continue;
		trap_DropClient( i, "eliminated" );
	}
}

static void NW_GameOver( int event, const char *why ) {
	if ( nw_over ) {
		return;
	}
	nw_over = qtrue;
	nw_inBreak = qfalse;
	// restore gravity in case we ended during a low-grav wave
	trap_Cvar_Set( "g_gravity", "800" );
	NeonWave_UpdateHighscore();
	NW_SendStatus( event );
	G_Printf( "NeonWave: RUN STATS kills=%i bestCombo=%i time=%is\n",
		NW_RunKills(), NW_RunBestCombo(), ( level.time - nw_runStartTime ) / 1000 );
	NW_KickBots();
	G_Printf( "NeonWave: %s (wave %i)\n", why, nw_wave );
	LogExit( why );
}

void NeonWave_Frame( void ) {
	int humans, bots, i;
	gentity_t *ent;
	static int lastRefresh;

	if ( g_gametype.integer != GT_NEONWAVE ) {
		return;
	}
	if ( level.intermissiontime || level.intermissionQueued ) {
		return;
	}
	if ( nw_over ) {
		return;
	}

	humans = 0;
	bots = 0;
	for ( i = 0; i < level.maxclients; i++ ) {
		ent = &g_entities[i];
		if ( !ent->inuse || !ent->client ) continue;
		if ( ent->client->pers.connected != CON_CONNECTED ) continue;
		if ( ent->health <= 0 ) continue;
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) continue;
		if ( ent->r.svFlags & SVF_BOT ) bots++;
		else humans++;
	}
	nw_aliveBots = bots;
	if ( bots > 0 ) {
		nw_waveHadBots = qtrue;
		// test hook: g_neonwave_autokill 1 -> kill all drones each frame
		// so a headless run plays through waves up to victory automatically
		{
			char akBuf[8];
			trap_Cvar_VariableStringBuffer( "g_neonwave_autokill", akBuf, sizeof(akBuf) );
			if ( atoi( akBuf ) == 1 ) {
				for ( i = 0; i < level.maxclients; i++ ) {
					ent = &g_entities[i];
					if ( !ent->inuse || !ent->client ) continue;
					if ( !( ent->r.svFlags & SVF_BOT ) ) continue;
					if ( ent->health <= 0 ) continue;
					ent->health = 0;
					ent->client->ps.stats[STAT_HEALTH] = 0;
				}
			}
		}
	}

	// test hook: g_neonwave_startwave N forces wave N (polled every frame,
	// works headless regardless of when the cvar is set)
	if ( !nw_over ) {
		char swBuf[8];
		int sw;
		trap_Cvar_VariableStringBuffer( "g_neonwave_startwave", swBuf, sizeof(swBuf) );
		sw = atoi( swBuf );
		if ( sw > 0 && sw != nw_wave ) {
			NeonWave_ForceStarted();
			nw_inBreak = qfalse;
			trap_Cvar_Set( "g_neonwave_startwave", "0" ); // consume (fire once)
			NeonWave_StartWave( sw );
			return;
		}
	}

	if ( !nw_started ) {
		qboolean autostart = qfalse;
		{
			char asBuf[8];
			trap_Cvar_VariableStringBuffer( "g_neonwave_autostart", asBuf, sizeof(asBuf) );
			autostart = ( atoi( asBuf ) != 0 ) ? qtrue : qfalse;
		}
		if ( ( humans > 0 || autostart ) && level.time > NW_FIRST_WAVE_DELAY ) {
			nw_started = qtrue;
			NeonWave_StartWave( 1 );
		}
		return;
	}

	if ( humans == 0 && !trap_Cvar_VariableValue( "g_neonwave_autostart" ) ) {
		NW_GameOver( NW_EV_FAILED, "NeonWave over" );
		return;
	}

	// test hook: g_neonwave_failrun 1 -> trigger failed game over once (tests
	// the FAILED path + RUN STATS + end screen without a real player death)
	{
		static qboolean failFired = qfalse;
		char frBuf[8];
		trap_Cvar_VariableStringBuffer( "g_neonwave_failrun", frBuf, sizeof(frBuf) );
		if ( !failFired && atoi( frBuf ) == 1 && nw_started && nw_wave > 0 ) {
			failFired = qtrue;
			NW_GameOver( NW_EV_FAILED, "NeonWave over" );
			return;
		}
	}

	if ( nw_inBreak ) {
		if ( level.time >= nw_breakEnd ) {
			nw_inBreak = qfalse;
			NeonWave_StartWave( nw_wave + 1 );
			return;
		}
		if ( level.time > lastRefresh ) {
			lastRefresh = level.time + 200;
			NW_SyncUpgrades();
			NW_SendStatus( NW_EV_CLEARED );
		}
		return;
	}

	if ( nw_waveHadBots && bots == 0 ) {
		if ( nw_wave >= NW_MAX_WAVE ) {
			NW_GrantUpgradePoints();
			NeonWave_DropReward( nw_wave );
			NW_GameOver( NW_EV_VICTORY, "All waves cleared" );
			return;
		}
		NW_EnterBreak();
		return;
	}

	if ( bots > 0 && nw_wave >= NW_BOSS_WAVE ) {
		if ( level.time > lastRefresh ) {
			lastRefresh = level.time + 250;
			NW_SyncUpgrades();
			NW_SendStatus( NW_EV_RUNNING );
		}
	}
}

#endif // NEONARENA_MOD
