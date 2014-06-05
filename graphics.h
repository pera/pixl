/* 
 * Copyright (C) 2012 - Brian Gomes Bascoy
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <assert.h>
#include <GL/glew.h>
#include <GL/glxew.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <fontconfig/fontconfig.h>
#include <pango/pangocairo.h>
#include <pango/pangofc-fontmap.h>
#include <cairo/cairo.h>
#include "cairosdl.h"
#include <librsvg/rsvg.h>

#include "config.h"

typedef unsigned int uint;

/**
 * @brief Texture class
 */
class PIXL_Texture {
	public:
		PIXL_Texture(const GLvoid* d, const int w, const int h);
		~PIXL_Texture();
		GLuint getTarget() { return texture; }
		const GLvoid* getData() { return data; }
		int getWidth() { return width; }
		int getHeight() { return height; }
		void blit();
		void draw(int x,int y);
	private:
		GLuint texture;
		const GLvoid* data;
		uint width;
		uint height;
};


/**
 * @brief Framebuffer object
 *
 */
// TODO checkear esto https://www.opengl.org/wiki/Framebuffer_Objects
class PIXL_FBO {
	public:
		PIXL_FBO();
		virtual ~PIXL_FBO();
		void bind();
		void draw(PIXL_FBO* target=NULL);
		GLuint shader;
	private:
		//PIXL_Texture *texture;
		GLuint fbo;
		GLuint texture;
};


/**
 * @brief Cairo surface
 */
class PIXL_Layer {
	public:
		PIXL_Layer(int w, int h);
		~PIXL_Layer();
		int getWidth() { return width; }
		int getHeight() { return height; }
		cairo_t* getContext() { return context; }
		void* getBuffer() { return sdlsurf->pixels; }
		void draw();
		void clear();
	private:
		SDL_Surface *sdlsurf;
		cairo_surface_t *layer;
		cairo_t* context;
		PIXL_Texture* texture;
		int width;
		int height;
};


/**
 * @brief Image class for Layers (png and svg )
 */
class PIXL_Image {
	public:
		PIXL_Image(PIXL_Layer* l, const char* f);
		virtual ~PIXL_Image();
		void draw(int w, int h);
	private:
		cairo_surface_t* image;
		PIXL_Layer* layer;
};


/**
 * @brief Sprite class (load png and use GL quads)
 */
class PIXL_Sprite {
	public:
		PIXL_Sprite(const char* f);
		virtual ~PIXL_Sprite();
		void draw(int x, int y);
	private:
		SDL_Surface* image;
		GLuint texture;
};


/**
 * @brief Sprite animation class
 */
class PIXL_Animation {
	public:
		PIXL_Animation(const char* f, uint w, uint h, uint s);
		virtual ~PIXL_Animation();
		void draw(int x, int y);
		void setSpeed(uint s);
		void play(uint new_n, bool l);
		bool isPlaying() { return playing; }
	private:
		SDL_Surface* image;
		GLuint texture;
		uint sprite_w; // width of one single sprite
		uint sprite_h; // height of one single sprite
		uint m; // frame number, or column
		uint n; // animation number, or row
		float speed; // duration of each frame in ms
		uint start_time;
		bool playing;
		bool loop;
};


/**
 * @brief Text rendering class
 */
class PIXL_Text {
	public:
		PIXL_Text(PIXL_Layer* l, const char* f, uint s, int x, int y);
		virtual ~PIXL_Text();
		void setSize(uint s) { font_size=s; }
		void setPos(int x, int y) { pos_x=x; pos_y=y; };
		void print(const char* text);
	private:
		const FcChar8 *font_name;
		uint font_size;
		int pos_x;
		int pos_y;
		FcConfig *fc; //para checkear si existe fuente
		FcBlanks *blanks; //para errores de fuentes
		FcPattern *pattern;
		cairo_t* context; 
		PangoLayout *layout;
		PangoFontDescription *font_description;
		int count;
};

