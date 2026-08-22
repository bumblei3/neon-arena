// NeonArena look overrides — loaded from neonarena/neon-look.pk3
// Forces dark sky + strong fog on stock maps for a Tron-style mood.

// generic: darken skies (many OA maps use env/ or skies/ textures)
skies/env_starfield
{
	qer_editorimage textures/skies/env_starfield.tga
	surfaceparm sky
	surfaceparm noimpact
	surfaceparm nolightmap
	skyparms - 512 -
	{
		map textures/skies/topclouds.jpg
		tcMod scroll 0.008 0.03
		rgbgen identityLighting
	}
}

// neon glow on teleporter surfaces
textures/neonarena/glow_cyan
{
	qer_editorimage textures/effects/tjumpfront.tga
	surfaceparm nomarks
	surfaceparm trans
	cull none
	{
		map textures/effects/tjumpfront.tga
		blendFunc GL_ONE GL_ONE
		rgbGen identity
		tcMod turb 0 0.15 0 0.3
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
	}
}

sprites/neonarena/spark
{
	nomipmaps
	nopicmip
	{
		clampMap sprites/flare1.tga
		blendFunc GL_ONE GL_ONE
		rgbGen vertex
	}
}
