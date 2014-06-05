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

#define VERSION "0.1"

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
#include "filesystem.h"

typedef unsigned int uint;

/**
 * @brief Scene manager for poors
 */
class PIXL_State {
	public:
		PIXL_State(){state = 0;}
		enum {play, quit};
		void set(char s) {state = s;}
		uint get() {return state;}
	private:
		uint state;
};


/**
 * @brief Timer class
 */
class PIXL_Timer {
	public:
		PIXL_Timer(uint t);
		void set(uint t);
		void start();
		void stop();
		bool status();
	private:
		uint time;
		uint start_time;
		bool status_flag;
};


/**
 * @brief Application abstract class
 */
class PIXL_App {
	public:
		PIXL_App();
		~PIXL_App() { SDL_Quit(); }
		//virtual ~PIXL_App();
		//void run();
		virtual void update() = 0;
		virtual void render() = 0;
	private:
		SDL_Surface *screen;
		SDL_Event event;
};


/**
 * @brief Helper function to load a fragment shader from a file
 *
 * @note Includes "w" (width), "h" (height) and "sampler0" uniforms
 */
GLuint PIXL_loadShader(const char* filename);


/**
 * @brief Simple bounding box collision detection
 *
 * @param b1 the first box
 * @param b2 the second box
 *
 * @return if there is a collision it returns true, otherwise false
 */
bool PIXL_bbc(SDL_Rect b1, SDL_Rect b2);

