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

#ifndef _PIXL_CONFIG_H_
#define _PIXL_CONFIG_H_

typedef unsigned int uint;
 
/**
 * @brief General configuration object
 */
class PIXL_Config {
	public:
		PIXL_Config(uint initial_width = 640, uint initial_height = 480): w(&width), h(&height), width(initial_width), height(initial_height) {}
		//void setResolution(uint new_width, uint new_height) { width=new_width; height=new_height; } // Pandora's Box
		const uint *const w;
		const uint *const h;
	private:
		uint width;
		uint height;
};

extern PIXL_Config PIXL_config;

#endif // _PIXL_CONFIG_H_

