// NeonArena look pack — dark sky, cyan weapons, energy 'blood', oa_shine neon floors.

// ---------------------------------------------------------------------------
// Sky: replace oa_shine ice box with a dark starfield (same env path).
// ---------------------------------------------------------------------------
textures/anoice1/anoice1
{
	qer_editorimage textures/neonarena/night.jpg
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm sky
	q3map_globaltexture
	skyparms env/anoice1/anoice1 512 -
	{
		map textures/neonarena/night.jpg
		tcMod scale 3 2
		tcMod scroll 0.004 0.008
		rgbGen identity
	}
	{
		map textures/neonarena/grid.tga
		blendfunc add
		rgbGen const ( 0.06 0.22 0.26 )
		tcMod scale 6 6
		tcMod scroll 0.01 0
	}
}

skies/env_starfield
{
	surfaceparm sky
	surfaceparm noimpact
	surfaceparm nolightmap
	skyparms - 512 -
	{
		map textures/neonarena/night.jpg
		tcMod scroll 0.006 0.02
		rgbGen identity
	}
}

// ---------------------------------------------------------------------------
// oa_shine floors: original albedo + additive neon grid
// ---------------------------------------------------------------------------
textures/mc-oa-dm04/ano_techfloor1_dark
{
	{
		map textures/mc-oa-dm04/ano_techfloor1_dark.jpg
		rgbGen const ( 0.55 0.62 0.70 )
	}
	{
		map $lightmap
		blendfunc filter
	}
	{
		map textures/neonarena/grid.tga
		blendfunc add
		rgbGen const ( 0.10 0.38 0.42 )
		tcMod scale 2 2
	}
}

textures/mc-oa-dm04/ano_floor1_128
{
	{
		map textures/mc-oa-dm04/ano_floor1_128.jpg
		rgbGen const ( 0.55 0.62 0.70 )
	}
	{
		map $lightmap
		blendfunc filter
	}
	{
		map textures/neonarena/grid.tga
		blendfunc add
		rgbGen const ( 0.09 0.34 0.38 )
		tcMod scale 2 2
	}
}

textures/evil6_floors/e6c_floor
{
	{
		map textures/evil6_floors/e6c_floor.jpg
		rgbGen const ( 0.52 0.58 0.66 )
	}
	{
		map $lightmap
		blendfunc filter
	}
	{
		map textures/neonarena/grid.tga
		blendfunc add
		rgbGen const ( 0.11 0.40 0.44 )
		tcMod scale 3 3
	}
}

textures/evil6_floors/e6c_floor_b
{
	{
		map textures/evil6_floors/e6c_floor_b.jpg
		rgbGen const ( 0.52 0.58 0.66 )
	}
	{
		map $lightmap
		blendfunc filter
	}
	{
		map textures/neonarena/grid.tga
		blendfunc add
		rgbGen const ( 0.10 0.36 0.40 )
		tcMod scale 3 3
	}
}

// walls: slightly cooler / darker so neon pops
textures/mc-oa-dm04/anodm2_grey0
{
	{
		map textures/mc-oa-dm04/anodm2_grey0.jpg
		rgbGen const ( 0.62 0.70 0.78 )
	}
	{
		map $lightmap
		blendfunc filter
	}
}

textures/mc-oa-dm04/anodm2_grey1
{
	{
		map textures/mc-oa-dm04/anodm2_grey1.jpg
		rgbGen const ( 0.62 0.70 0.78 )
	}
	{
		map $lightmap
		blendfunc filter
	}
}

textures/mc-oa-dm04/anodm2_grey2
{
	{
		map textures/mc-oa-dm04/anodm2_grey2.jpg
		rgbGen const ( 0.62 0.70 0.78 )
	}
	{
		map $lightmap
		blendfunc filter
	}
}

textures/mc-oa-dm04/anodm2_grey3
{
	{
		map textures/mc-oa-dm04/anodm2_grey3.jpg
		rgbGen const ( 0.62 0.70 0.78 )
	}
	{
		map $lightmap
		blendfunc filter
	}
}

textures/mc-oa-dm04/ano_steelplate
{
	{
		map textures/mc-oa-dm04/ano_steelplate.jpg
		rgbGen const ( 0.60 0.68 0.78 )
	}
	{
		map $lightmap
		blendfunc filter
	}
}

// lamps: extra cyan wash
textures/mc-oa-dm04/b_lamp_s_2k
{
	qer_editorimage textures/mc-oa-dm04/b_lamp.tga
	{
		map textures/mc-oa-dm04/b_lamp.tga
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
	{
		map textures/neonarena/flare.tga
		blendfunc add
		rgbGen const ( 0.15 0.55 0.65 )
		tcMod rotate 8
	}
}

textures/base_light/ceil1_34
{
	qer_editorimage textures/base_light/ceil1_34.tga
	surfaceparm metalsteps
	{
		map textures/base_light/ceil1_34.tga
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		tcGen lightmap
	}
	{
		map textures/base_light/ceil1_34.blend.tga
		blendfunc add
	}
	{
		map textures/neonarena/flare.tga
		blendfunc add
		rgbGen const ( 0.10 0.45 0.55 )
	}
}

textures/mc-oa-dm04/ano-trim2
{
	qer_editorimage textures/mc-oa-dm04/ano-trim2.tga
	surfaceparm nomarks
	{
		map textures/mc-oa-dm04/ano-trim2.tga
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		tcGen lightmap
	}
	{
		map textures/mc-oa-dm04/ano-trim2.blend.tga
		blendfunc add
		rgbGen const ( 0.45 0.85 1.0 )
	}
}

// ---------------------------------------------------------------------------
// Weapons
// ---------------------------------------------------------------------------
railCore
{
	sort nearest
	cull disable
	{
		map textures/neonarena/railcore.tga
		blendfunc add
		rgbGen const ( 0.35 1.0 1.0 )
		tcMod scroll -1.6 0
	}
	{
		map textures/neonarena/railcore.tga
		blendfunc add
		rgbGen const ( 0.15 0.55 0.70 )
		tcMod scroll -0.7 0
		tcMod scale 0.5 1
	}
}

railDisc
{
	cull disable
	{
		clampmap textures/neonarena/flare.tga
		blendfunc add
		rgbGen vertex
		tcMod rotate 130
	}
}

lightningBoltNew
{
	cull disable
	{
		animmap 30 textures/oafx/lbeam3.tga textures/oafx/lbeam4.tga textures/oafx/lbeam5.tga textures/oafx/lbeam6.tga textures/oafx/lbeam7.tga textures/oafx/lbeam8.tga textures/oafx/lbeam5.tga textures/oafx/lbeam7.tga
		blendfunc add
		rgbGen const ( 0.35 1.0 1.0 )
		tcMod scale 0.5 1
		tcMod scroll -1 0
	}
	{
		animmap 40 textures/oafx/lbeam8.tga textures/oafx/lbeam7.tga textures/oafx/lbeam4.tga textures/oafx/lbeam5.tga textures/oafx/lbeam6.tga textures/oafx/lbeam3.tga textures/oafx/lbeam7.tga textures/oafx/lbeam4.tga
		blendfunc add
		rgbGen const ( 0.20 0.70 1.0 )
		tcMod scale 0.2 1
		tcMod scroll -0.3 0
	}
	{
		map textures/neonarena/railcore.tga
		blendfunc add
		rgbGen const ( 0.15 0.55 0.70 )
		tcMod scroll -2.4 0
	}
}

lightningBolt
{
	cull disable
	{
		animmap 30 textures/oafx/lbeam3.tga textures/oafx/lbeam4.tga textures/oafx/lbeam5.tga textures/oafx/lbeam6.tga textures/oafx/lbeam7.tga textures/oafx/lbeam8.tga textures/oafx/lbeam5.tga textures/oafx/lbeam7.tga
		blendfunc add
		rgbGen const ( 0.35 1.0 1.0 )
		tcMod scale 0.5 1
		tcMod scroll -1 0
	}
	{
		map textures/neonarena/railcore.tga
		blendfunc add
		rgbGen const ( 0.15 0.55 0.70 )
		tcMod scroll -2.4 0
	}
}

lightningBoltnew
{
	cull disable
	{
		animmap 30 textures/oafx/lbeam3.tga textures/oafx/lbeam4.tga textures/oafx/lbeam5.tga textures/oafx/lbeam6.tga textures/oafx/lbeam7.tga textures/oafx/lbeam8.tga textures/oafx/lbeam5.tga textures/oafx/lbeam7.tga
		blendfunc add
		rgbGen const ( 0.35 1.0 1.0 )
		tcMod scroll -1 0
	}
}

lightningExplosion
{
	cull disable
	deformVertexes wave 9 sin 0 1 0 9
	{
		map models/weaphits/elecscroll.tga
		blendfunc add
		rgbGen const ( 0.3 1.0 1.0 )
		tcMod scroll -8 0
	}
}

// ---------------------------------------------------------------------------
// Energy instead of blood
// ---------------------------------------------------------------------------
viewBloodBlend
{
	sort nearest
	{
		map gfx/damage/blood_screen.tga
		blendFunc GL_SRC_ALPHA GL_ONE
		rgbGen identityLighting
		alphaGen vertex
	}
}

bloodMark
{
	nopicmip
	polygonOffset
	{
		clampmap gfx/damage/blood_stain.tga
		blendFunc GL_ONE GL_ONE
		rgbGen identityLighting
		alphaGen vertex
	}
}

bloodTrail
{
	nopicmip
	entityMergable
	{
		clampmap gfx/damage/blood_spurt.tga
		blendFunc GL_ONE GL_ONE
		rgbGen vertex
		alphaGen vertex
	}
}

bloodExplosion
{
	{
		clampmap textures/neonarena/flare.tga
		blendfunc add
		rgbGen const ( 0.3 1.0 1.0 )
		tcMod rotate 77
		tcMod stretch sin 0 2 0 0.4
	}
	{
		clampmap textures/neonarena/spark.tga
		blendfunc add
		rgbGen const ( 0.8 0.4 1.0 )
		tcMod rotate -43
		tcMod stretch sin 0 1.7 0 0.4
	}
}

// ---------------------------------------------------------------------------
// Player shells + HUD sprites
// ---------------------------------------------------------------------------
neonarena/droneShell
{
	deformVertexes wave 100 sin 1.8 0 0 0
	{
		map textures/neonarena/grid.tga
		blendfunc add
		rgbGen const ( 0.35 1.60 1.90 )
		tcGen environment
		tcMod scroll 0.15 0.08
	}
}

neonarena/bossShell
{
	deformVertexes wave 100 sin 2.4 0 0 0
	{
		map textures/neonarena/grid.tga
		blendfunc add
		rgbGen const ( 2.20 0.30 2.20 )
		tcGen environment
		tcMod scroll 0.22 0.10
		tcMod scale 2 2
	}
}

neonarena/flare
{
	nopicmip
	cull disable
	{
		clampmap textures/neonarena/flare.tga
		blendfunc add
		rgbGen vertex
	}
}

gfx/2d/neon_vignette
{
	nopicmip
	nomipmaps
	{
		map gfx/2d/neon_vignette.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
		alphaGen vertex
	}
}

gfx/2d/neon_bar
{
	nopicmip
	nomipmaps
	{
		map gfx/2d/neon_bar.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
		alphaGen vertex
	}
}

sprites/neonarena/spark
{
	nomipmaps
	nopicmip
	{
		clampmap textures/neonarena/spark.tga
		blendFunc GL_ONE GL_ONE
		rgbGen vertex
	}
}

textures/neonarena/glow_cyan
{
	qer_editorimage textures/neonarena/flare.tga
	surfaceparm nomarks
	surfaceparm trans
	cull none
	{
		map textures/neonarena/flare.tga
		blendFunc GL_ONE GL_ONE
		rgbGen identity
		tcMod turb 0 0.15 0 0.3
	}
}
