/**
 * $Id: gl_common.c,v 1.1 2007-02-11 10:09:32 nkeynes Exp $
 *
 * Common GL code that doesn't depend on a specific implementation
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

#include <GL/gl.h>
#include "dream.h"
#include "drivers/gl_common.h"

extern uint32_t video_width, video_height;

char *required_extensions[] = { "GL_EXT_framebuffer_object", NULL };

/**
 * Test if a specific extension is supported. From opengl.org
 * @param extension extension name to check for
 * @return TRUE if supported, otherwise FALSE.
 */
gboolean isGLExtensionSupported( const char *extension )
{
    const GLubyte *extensions = NULL;
    const GLubyte *start;
    GLubyte *where, *terminator;

    /* Extension names should not have spaces. */
    where = (GLubyte *) strchr(extension, ' ');
    if (where || *extension == '\0')
	return 0;
    extensions = glGetString(GL_EXTENSIONS);
    /* It takes a bit of care to be fool-proof about parsing the
       OpenGL extensions string. Don't be fooled by sub-strings,
       etc. */
    start = extensions;
    for (;;) {
	where = (GLubyte *) strstr((const char *) start, extension);
	if (!where)
	    break;
	terminator = where + strlen(extension);
	if (where == start || *(where - 1) == ' ')
	    if (*terminator == ' ' || *terminator == '\0')
		return TRUE;
	start = terminator;
    }
    return FALSE;
}

gboolean hasRequiredGLExtensions( ) 
{
    int i;
    gboolean isOK = TRUE;

    for( i=0; required_extensions[i] != NULL; i++ ) {
	if( !isGLExtensionSupported(required_extensions[i]) ) {
	    ERROR( "Required OpenGL extension not supported: %s", required_extensions[i] );
	    isOK = FALSE;
	}
    }
    return isOK;
}


gboolean gl_display_frame_buffer( frame_buffer_t frame )
{
    GLenum type = colour_formats[frame->colour_format].type;
    GLenum format = colour_formats[frame->colour_format].format;
    int bpp = colour_formats[frame->colour_format].bpp;
    GLint texid;

    glViewport( 0, 0, video_width, video_height );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, video_width, video_height, 0, 0, -65535 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    /* Disable everything */
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_ALPHA_TEST );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_SCISSOR_TEST );
    glDisable( GL_CULL_FACE );

    float scale = ((float)video_height) / frame->height;
    int rowstride = (frame->rowstride / bpp) - frame->width;
    
    
    /*
    glGenTextures( 1, &texid );
    glBindTexture( GL_TEXTURE_2D, texid );
    glTexImage2D( GL_TEXTURE_2D, 0, colour_formats[frame->colour_format].int_format,
		  frame->width, frame->height, 0, format, type, frame->data );
    glBegin( GL_QUADS );
    glTexCoord2i( 0, 1.0 );
    glVertex2f( 0.0, 0.0 );
    glTexCoord2i( 1.0, 1.0 );
    glVertex2f( frame->width, 0.0 );
    glTexCoord2i( 1.0, 0.0 );
    glVertex2f( frame->width, frame->height );
    glTexCoord2i( 0, 0.0 );
    glVertex2f( 0.0, frame->height );
    glEnd();
    glDeleteTextures( 1, &texid );
    */
    glRasterPos2i( 0, 0 );
    glPixelZoom( 1.0f, -scale );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, rowstride );
    glDrawPixels( frame->width, frame->height, format, type,
		  frame->data );
    
    glFlush();
    return TRUE;
}

gboolean gl_display_blank( uint32_t colour )
{
    glViewport( 0, 0, video_width, video_height );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, video_width, video_height, 0, 0, -65535 );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3b( (colour >> 16) & 0xFF, (colour >> 8) & 0xFF, colour & 0xFF );
    glRecti(0,0, video_width, video_height );
    glFlush();
    return TRUE;
}

/**
 * Generic GL read_render_buffer. This function assumes that the caller
 * has already set the appropriate glReadBuffer(); in other words, unless
 * there's only one buffer this needs to be wrapped.
 */
gboolean gl_read_render_buffer( render_buffer_t buffer, char *target ) 
{
    if( buffer->address == -1 )
	return FALSE;
    glFinish();
    GLenum type = colour_formats[buffer->colour_format].type;
    GLenum format = colour_formats[buffer->colour_format].format;
    int line_size = buffer->width * colour_formats[buffer->colour_format].bpp;
    int size = line_size * buffer->height;
    int rowstride = (buffer->rowstride / colour_formats[buffer->colour_format].bpp) - buffer->width;
    // glPixelStorei( GL_PACK_ROW_LENGTH, rowstride );
    
    glReadPixels( 0, 0, buffer->width, buffer->height, format, type, target );
    return TRUE;
}