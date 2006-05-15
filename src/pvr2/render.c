/**
 * $Id: render.c,v 1.8 2006-05-15 08:28:52 nkeynes Exp $
 *
 * PVR2 Renderer support. This is where the real work happens.
 *
 * Copyright (c) 2005 Nathan Keynes.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "pvr2/pvr2.h"
#include "asic.h"


#define POLY_COLOUR_PACKED 0x00000000
#define POLY_COLOUR_FLOAT 0x00000010
#define POLY_COLOUR_INTENSITY 0x00000020
#define POLY_COLOUR_INTENSITY_PREV 0x00000030

static int pvr2_poly_vertexes[4] = { 3, 4, 6, 8 };
static int pvr2_poly_type[4] = { GL_TRIANGLES, GL_QUADS, GL_TRIANGLE_STRIP, GL_TRIANGLE_STRIP };
static int pvr2_poly_depthmode[8] = { GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL,
				      GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, 
				      GL_ALWAYS };
static int pvr2_poly_srcblend[8] = { 
    GL_ZERO, GL_ONE, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, 
    GL_ONE_MINUS_DST_ALPHA };
static int pvr2_poly_dstblend[8] = {
    GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA };
static int pvr2_poly_texblend[4] = {
    GL_REPLACE, GL_BLEND, GL_DECAL, GL_MODULATE };
static int pvr2_render_colour_format[8] = {
    COLFMT_ARGB1555, COLFMT_RGB565, COLFMT_ARGB4444, COLFMT_ARGB1555,
    COLFMT_RGB888, COLFMT_ARGB8888, COLFMT_ARGB8888, COLFMT_ARGB4444 };

#define POLY_STRIP_TYPE(poly) ( pvr2_poly_type[((poly->command)>>18)&0x03] )
#define POLY_STRIP_VERTEXES(poly) ( pvr2_poly_vertexes[((poly->command)>>18)&0x03] )
#define POLY_DEPTH_MODE(poly) ( pvr2_poly_depthmode[poly->poly_cfg>>29] )
#define POLY_DEPTH_WRITE(poly) ((poly->poly_cfg&0x04000000) == 0 )
#define POLY_TEX_WIDTH(poly) ( 1<< (((poly->poly_mode >> 3) & 0x07 ) + 3) )
#define POLY_TEX_HEIGHT(poly) ( 1<< (((poly->poly_mode) & 0x07 ) + 3) )
#define POLY_BLEND_SRC(poly) ( pvr2_poly_srcblend[(poly->poly_mode) >> 29] )
#define POLY_BLEND_DEST(poly) ( pvr2_poly_dstblend[((poly->poly_mode)>>26)&0x07] )
#define POLY_TEX_BLEND(poly) ( pvr2_poly_texblend[((poly->poly_mode) >> 6)&0x03] )
#define POLY_COLOUR_TYPE(poly) ( poly->command & 0x00000030 )

/**
 * Describes a rendering buffer that's actually held in GL, for when we need
 * to fetch the bits back to vram.
 */
typedef struct pvr2_render_buffer {
    uint32_t render_addr; /* The actual address rendered to in pvr ram */
    int width, height;
    int colour_format;
} *pvr2_render_buffer_t;

struct pvr2_render_buffer front_buffer;
struct pvr2_render_buffer back_buffer;

struct tile_descriptor {
    uint32_t header[6];
    struct tile_pointers {
	uint32_t tile_id;
	uint32_t opaque_ptr;
	uint32_t opaque_mod_ptr;
	uint32_t trans_ptr;
	uint32_t trans_mod_ptr;
	uint32_t punchout_ptr;
    } tile[0];
};

/* Textured polygon */
struct pvr2_poly {
    uint32_t command;
    uint32_t poly_cfg; /* Bitmask */
    uint32_t poly_mode; /* texture/blending mask */
    uint32_t texture; /* texture data */
    float alpha;
    float red;
    float green;
    float blue;
};

struct pvr2_specular_highlight {
    float base_alpha;
    float base_red;
    float base_green;
    float base_blue;
    float offset_alpha;
    float offset_red;
    float offset_green;
    float offset_blue;
};
				     

struct pvr2_vertex_packed {
    uint32_t command;
    float x, y, z;
    float s,t;
    uint32_t colour;
    float f;
};

struct pvr2_vertex_float {
    uint32_t command;
    float x,y,z;
    float a, r, g, b;
};

union pvr2_vertex {
    struct pvr2_vertex_packed pack;
    struct pvr2_vertex_float flt;
};

typedef struct pvr2_bgplane_packed {
        uint32_t        poly_cfg, poly_mode;
        uint32_t        texture_mode;
        float           x1, y1, z1;
        uint32_t          colour1;
        float           x2, y2, z2;
        uint32_t          colour2;
        float           x3, y3, z3;
        uint32_t          colour3;
} *pvr2_bgplane_packed_t;



void pvr2_render_copy_to_sh4( pvr2_render_buffer_t buffer, 
			      gboolean backBuffer );

int pvr2_render_font_list = -1;
int pvr2_render_trace = 0;

int glPrintf( int x, int y, const char *fmt, ... )
{
    va_list ap;     /* our argument pointer */
    char buf[256];
    int len;
    if (fmt == NULL)    /* if there is no string to draw do nothing */
        return;
    va_start(ap, fmt); 
    len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);


    glPushAttrib(GL_LIST_BIT);
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_ALPHA_TEST );
    glListBase(pvr2_render_font_list - 32);
    glColor3f( 1.0, 1.0, 1.0 );
    glRasterPos2i( x, y );
    glCallLists(len, GL_UNSIGNED_BYTE, buf);
    glPopAttrib();

    return len;
}


gboolean pvr2_render_init( void )
{
    front_buffer.render_addr = -1;
    back_buffer.render_addr = -1;
}

/**
 * Display a rendered frame if one is available.
 * @param address An address in PVR ram (0500000 range).
 * @return TRUE if a frame was available to be displayed, otherwise false.
 */
gboolean pvr2_render_display_frame( uint32_t address )
{
    if( front_buffer.render_addr == address ) {
	/* Current front buffer is already displayed, so do nothing
	 * and tell the caller that all is well.
	 */
	return TRUE;
    }
    if( back_buffer.render_addr == address ) {
	/* The more useful case - back buffer is to be displayed. Swap
	 * the buffers 
	 */
	display_driver->display_back_buffer();
	front_buffer = back_buffer;
	back_buffer.render_addr = -1;
	return TRUE;
    }
    return FALSE;
}	

/**
 * Prepare the OpenGL context to receive instructions for a new frame.
 */
static void pvr2_render_prepare_context( sh4addr_t render_addr, 
					 uint32_t width, uint32_t height,
					 uint32_t colour_format, 
					 float bgplanez,
					 gboolean texture_target )
{
    /* Select and initialize the render context */
    display_driver->set_render_format( width, height, colour_format, texture_target );

    if( pvr2_render_font_list == -1 ) {
	pvr2_render_font_list = video_glx_load_font( "-*-helvetica-*-r-normal--16-*-*-*-p-*-iso8859-1");
    }

    if( back_buffer.render_addr != -1 && 
	back_buffer.render_addr != render_addr ) {
	/* There's a current back buffer, and we're rendering somewhere else -
	 * flush the back buffer back to vram and start a new back buffer
	 */
	pvr2_render_copy_to_sh4( &back_buffer, TRUE );
    }

    if( front_buffer.render_addr == render_addr ) {
	/* In case we've been asked to render to the current front buffer -
	 * invalidate the front buffer and render to the back buffer, ensuring
	 * we swap at the next frame display.
	 */
	front_buffer.render_addr = -1;
    }
    back_buffer.render_addr = render_addr;
    back_buffer.width = width;
    back_buffer.height = height;
    back_buffer.colour_format = colour_format;

    /* Setup the display model */
    glDrawBuffer(GL_BACK);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glViewport( 0, 0, width, height );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, width, height, 0, bgplanez, -1 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glCullFace( GL_BACK );

    /* Clear out the buffers */
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(bgplanez);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

static void pvr2_dump_display_list( uint32_t * display_list, uint32_t length )
{
    uint32_t i;
    gboolean vertex = FALSE;
    for( i =0; i<length>>2; i++ ) {
	if( (i % 8) == 0 ) {
	    if( i != 0 )
		fprintf( stderr, "\n" );
	    fprintf( stderr, "%08X:", i*4 );
	    if( display_list[i] == 0xE0000000 ||
		display_list[i] == 0xF0000000 ) 
		vertex = TRUE;
	    else vertex = FALSE;
	}
	if( vertex && (i%8) > 0 && (i%8) < 4 )
	    fprintf( stderr, " %f", ((float *)display_list)[i] );
	else
	    fprintf( stderr, " %08X", display_list[i] );
    }
    fprintf( stderr, "\n" );
}

static void pvr2_render_display_list( uint32_t *display_list, uint32_t length )
{
    uint32_t *cmd_ptr = display_list;
    int strip_length = 0, vertex_count = 0;
    int colour_type;
    gboolean textured = FALSE;
    gboolean shaded = FALSE;
    struct pvr2_poly *poly;
    if( pvr2_render_trace ) {
	fprintf( stderr, "-------- %d\n", pvr2_get_frame_count() );
	pvr2_dump_display_list( display_list, length );
    }
    while( cmd_ptr < display_list+(length>>2) ) {
	unsigned int cmd = *cmd_ptr >> 24;
	switch( cmd ) {
	case PVR2_CMD_POLY_OPAQUE:
	case PVR2_CMD_POLY_TRANS:
	case PVR2_CMD_POLY_PUNCHOUT:
	    poly = (struct pvr2_poly *)cmd_ptr;
	    if( poly->command & PVR2_POLY_TEXTURED ) {
		uint32_t addr = PVR2_TEX_ADDR(poly->texture);
		int width = POLY_TEX_WIDTH(poly);
		int height = POLY_TEX_HEIGHT(poly);
		glEnable( GL_TEXTURE_2D );
		texcache_get_texture( addr, width, height, poly->texture );
		textured = TRUE;
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, POLY_TEX_BLEND(poly) );
	    } else {
		textured = FALSE;
		glDisable( GL_TEXTURE_2D );
	    }
	    glBlendFunc( POLY_BLEND_SRC(poly), POLY_BLEND_DEST(poly) );
	    if( poly->command & PVR2_POLY_SPECULAR ) {
		/* Second block expected */
	    }
	    if( POLY_DEPTH_WRITE(poly) ) {
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( POLY_DEPTH_MODE(poly) );
	    } else {
		glDisable( GL_DEPTH_TEST );
	    }

	    switch( (poly->poly_cfg >> 27) & 0x03 ) {
	    case 0:
	    case 1:
		glDisable( GL_CULL_FACE );
		break;
	    case 2:
		glEnable( GL_CULL_FACE );
		glFrontFace( GL_CW );
		break;
	    case 3:
		glEnable( GL_CULL_FACE );
		glFrontFace( GL_CCW );
	    }
	    strip_length = POLY_STRIP_VERTEXES( poly );
	    colour_type = POLY_COLOUR_TYPE( poly );
	    vertex_count = 0;
	    if( poly->command & PVR2_POLY_SHADED ) {
		shaded = TRUE;
	    } else {
		shaded = FALSE;
	    }
	    if( poly->poly_mode & PVR2_POLY_MODE_TEXALPHA ) {
		glDisable( GL_BLEND );
	    } else {
		glEnable( GL_BLEND );
	    }

	    break;
	case PVR2_CMD_MOD_OPAQUE:
	case PVR2_CMD_MOD_TRANS:
	    /* TODO */
	    break;
	case PVR2_CMD_END_OF_LIST:
	    break;
	case PVR2_CMD_VERTEX_LAST:
	case PVR2_CMD_VERTEX:
	    if( vertex_count == 0 ) {
		glBegin( GL_TRIANGLE_STRIP );
	    }
	    vertex_count++;

	    struct pvr2_vertex_packed *vertex = (struct pvr2_vertex_packed *)cmd_ptr;
	    if( textured ) {
		glTexCoord2f( vertex->s, vertex->t );

		if( shaded || vertex_count == 1) {
		    switch( colour_type ) {
		    case POLY_COLOUR_PACKED:
			glColor4ub( vertex->colour >> 16, vertex->colour >> 8,
				    vertex->colour, vertex->colour >> 24 );
			break;
		    }
		}
	    } else {
		if( shaded || vertex_count == 1 ) {
		    switch( colour_type ) {
		    case POLY_COLOUR_PACKED:
			glColor4ub( vertex->colour >> 16, vertex->colour >> 8,
				    vertex->colour, vertex->colour >> 24 );
			break;
		    case POLY_COLOUR_FLOAT: 
			{
			    struct pvr2_vertex_float *v = (struct pvr2_vertex_float *)cmd_ptr;
			    glColor4f( v->r, v->g, v->b, v->a );
			}
			break;
		    }
		}
	    }

	    glVertex3f( vertex->x, vertex->y, vertex->z );
	    
	    if( cmd == PVR2_CMD_VERTEX_LAST ) {
		glEnd();
		vertex_count = 0;
	    }
	    break;
	default:
	    ERROR( "Unhandled command %08X in display list", *cmd_ptr );
	    pvr2_dump_display_list( display_list, length );
	    return;
	}
	cmd_ptr += 8; /* Next record */
    }
}

#define MIN3( a,b,c ) ((a) < (b) ? ( (a) < (c) ? (a) : (c) ) : ((b) < (c) ? (b) : (c)) )
#define MAX3( a,b,c ) ((a) > (b) ? ( (a) > (c) ? (a) : (c) ) : ((b) > (c) ? (b) : (c)) )

/**
 * Render the background plane as best we can. Unfortunately information
 * is a little scant, to say the least.
 */
void pvr2_render_draw_backplane( uint32_t mode, uint32_t *poly )
{
    if( (mode >> 24) == 0x01 ) {
	/* Packed colour. I think */
	pvr2_bgplane_packed_t bg = (pvr2_bgplane_packed_t)poly;
	if( bg->colour1 != bg->colour2 || bg->colour2 != bg->colour3 ) {
	    WARN( "Multiple background colours specified. Confused" );
	}
	float x1 = MIN3( bg->x1, bg->x2, bg->x3 );
	float y1 = MIN3( bg->y1, bg->y2, bg->y3 );
	float x2 = MAX3( bg->x1, bg->x2, bg->x3 );
	float y2 = MAX3( bg->y1, bg->y2, bg->y3 );
	float z = MIN3( bg->z1, bg->z2, bg->z3 );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_DEPTH_TEST );
	glColor3ub( (uint8_t)(bg->colour1 >> 16), (uint8_t)(bg->colour1 >> 8), 
		    (uint8_t)bg->colour1 );
	glBegin( GL_QUADS );
	glVertex3f( x1, y1, z );
	glVertex3f( x2, y1, z );
	glVertex3f( x2, y2, z );
	glVertex3f( x1, y2, z );
	glEnd();
    } else {
	WARN( "Unknown bgplane mode: %08X", mode );
	fwrite_dump( poly, 48, stderr );
    }
}

/**
 * Render a complete scene into the OpenGL back buffer.
 * Note: this will probably need to be broken up eventually once timings are
 * determined.
 */
void pvr2_render_scene( )
{
    struct tile_descriptor *tile_desc =
	(struct tile_descriptor *)mem_get_region(PVR2_RAM_BASE + MMIO_READ( PVR2, TILEBASE ));

    uint32_t render_addr = MMIO_READ( PVR2, RENDADDR1 );
    gboolean render_to_tex;
    if( render_addr & 0x01000000 ) {
	render_addr = (render_addr & 0x00FFFFFF) + PVR2_RAM_BASE_INT;
	/* Heuristic - if we're rendering to the interlaced region we're
	 * probably creating a texture rather than rendering actual output.
	 * We can optimise for this case a little
	 */
	render_to_tex = TRUE;
	WARN( "Render to texture not supported properly yet" );
    } else {
	render_addr = (render_addr & 0x00FFFFFF) + PVR2_RAM_BASE;
	render_to_tex = FALSE;
    }
    
    float bgplanez = MMIO_READF( PVR2, BGPLANEZ );
    uint32_t render_mode = MMIO_READ( PVR2, RENDMODE );
    int width = 640; /* FIXME - get this from the tile buffer */
    int height = 480;
    int colour_format = pvr2_render_colour_format[render_mode&0x07];
    pvr2_render_prepare_context( render_addr, width, height, colour_format, 
				 bgplanez, render_to_tex );

    uint32_t *display_list = 
	(uint32_t *)mem_get_region(PVR2_RAM_BASE + MMIO_READ( PVR2, OBJBASE ));

    uint32_t display_length = *display_list++;

    int clip_x = MMIO_READ( PVR2, HCLIP ) & 0x03FF;
    int clip_y = MMIO_READ( PVR2, VCLIP ) & 0x03FF;
    int clip_width = ((MMIO_READ( PVR2, HCLIP ) >> 16) & 0x03FF) - clip_x + 1;
    int clip_height= ((MMIO_READ( PVR2, VCLIP ) >> 16) & 0x03FF) - clip_y + 1;

    if( clip_x == 0 && clip_y == 0 && clip_width == width && clip_height == height ) {
	glDisable( GL_SCISSOR_TEST );
    } else {
	glEnable( GL_SCISSOR_TEST );
	glScissor( clip_x, clip_y, clip_width, clip_height );
    }

    /* Fog setup goes here */

    /* Render the background plane */
    uint32_t bgplane_mode = MMIO_READ(PVR2, BGPLANE);
    uint32_t *bgplane = display_list + (((bgplane_mode & 0x00FFFFFF)) >> 3) - 1;
    pvr2_render_draw_backplane( bgplane_mode, bgplane );

    /* Render the display list */
    pvr2_render_display_list( display_list, display_length );

    /* Post-render cleanup and update */

    /* Add frame, fps, etc data */
    glPrintf( 4, 16, "Frame %d", pvr2_get_frame_count() );
    
    /* Generate end of render event */
    asic_event( EVENT_PVR_RENDER_DONE );
    DEBUG( "Rendered frame %d", pvr2_get_frame_count() );
}


/**
 * Flush the indicated render buffer back to PVR. Caller is responsible for
 * tracking whether there is actually anything in the buffer.
 *
 * @param buffer A render buffer indicating the address to store to, and the
 * format the data needs to be in.
 * @param backBuffer TRUE to flush the back buffer, FALSE for 
 * the front buffer.
 */
void pvr2_render_copy_to_sh4( pvr2_render_buffer_t buffer, 
			      gboolean backBuffer )
{
    if( buffer->render_addr == -1 )
	return;
    GLenum type, format = GL_RGBA;
    int size = buffer->width * buffer->height;

    switch( buffer->colour_format ) {
    case COLFMT_RGB565: 
	type = GL_UNSIGNED_SHORT_5_6_5; 
	format = GL_RGB; 
	size <<= 1;
	break;
    case COLFMT_RGB888: 
	type = GL_UNSIGNED_INT; 
	format = GL_RGB;
	size = (size<<1)+size;
	break;
    case COLFMT_ARGB1555: 
	type = GL_UNSIGNED_SHORT_5_5_5_1; 
	size <<= 1;
	break;
    case COLFMT_ARGB4444: 
	type = GL_UNSIGNED_SHORT_4_4_4_4; 
	size <<= 1;
	break;
    case COLFMT_ARGB8888: 
	type = GL_UNSIGNED_INT_8_8_8_8; 
	size <<= 2;
	break;
    }
    
    if( backBuffer ) {
	glFinish();
	glReadBuffer( GL_BACK );
    } else {
	glReadBuffer( GL_FRONT );
    }

    if( buffer->render_addr & 0xFF000000 == 0x04000000 ) {
	/* Interlaced buffer. Go the double copy... :( */
	char target[size];
	glReadPixels( 0, 0, buffer->width, buffer->height, format, type, target );
	pvr2_vram64_write( buffer->render_addr, target, size );
    } else {
	/* Regular buffer - go direct */
	char *target = mem_get_region( buffer->render_addr );
	glReadPixels( 0, 0, buffer->width, buffer->height, format, type, target );
    }
}


/**
 * Copy data from PVR ram into the GL render buffer. 
 *
 * @param buffer A render buffer indicating the address to read from, and the
 * format the data is in.
 * @param backBuffer TRUE to write the back buffer, FALSE for 
 * the front buffer.
 */
void pvr2_render_copy_from_sh4( pvr2_render_buffer_t buffer, 
				gboolean backBuffer )
{
    if( buffer->render_addr == -1 )
	return;
    GLenum type, format = GL_RGBA;
    int size = buffer->width * buffer->height;

    switch( buffer->colour_format ) {
    case COLFMT_RGB565: 
	type = GL_UNSIGNED_SHORT_5_6_5; 
	format = GL_RGB; 
	size <<= 1;
	break;
    case COLFMT_RGB888: 
	type = GL_UNSIGNED_INT; 
	format = GL_RGB;
	size = (size<<1)+size;
	break;
    case COLFMT_ARGB1555: 
	type = GL_UNSIGNED_SHORT_5_5_5_1; 
	size <<= 1;
	break;
    case COLFMT_ARGB4444: 
	type = GL_UNSIGNED_SHORT_4_4_4_4; 
	size <<= 1;
	break;
    case COLFMT_ARGB8888: 
	type = GL_UNSIGNED_INT_8_8_8_8; 
	size <<= 2;
	break;
    }
    
    if( backBuffer ) {
	glDrawBuffer( GL_BACK );
    } else {
	glDrawBuffer( GL_FRONT );
    }

    glRasterPos2i( 0, 0 );
    if( buffer->render_addr & 0xFF000000 == 0x04000000 ) {
	/* Interlaced buffer. Go the double copy... :( */
	char target[size];
	pvr2_vram64_read( target, buffer->render_addr, size );
	glDrawPixels( buffer->width, buffer->height, 
		      format, type, target );
    } else {
	/* Regular buffer - go direct */
	char *target = mem_get_region( buffer->render_addr );
	glDrawPixels( buffer->width, buffer->height, 
		      format, type, target );
    }
}
