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

#include "graphics.h"

/**
 * @brief Texture class constructor
 *
 * @param d pixel data
 * @param w width
 * @param h height
 */
PIXL_Texture::PIXL_Texture(const GLvoid* d, const int w, const int h):data(d),width(w),height(h)
{
	glGenTextures(1, &texture);
	//GLuint boundTexture = 0;
	//glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*) &boundTexture);
	glBindTexture(GL_TEXTURE_2D, texture);
	//glTexParameteri(GL_TEXTURE_2D, pname, param);
	//glBindTexture(GL_TEXTURE_2D, boundTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA,
				 GL_UNSIGNED_INT_8_8_8_8_REV, data);
}

/**
 * @brief Render the texture object
 *
 * @param x x-position
 * @param y y-position
 */
void PIXL_Texture::draw(int x=0, int y=0)
{
	glColor4f(1.f,1.f,1.f,1.f);

	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, texture );
	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2i(0, 0);
		glTexCoord2f(0.0f, 1.0f); glVertex2i(0, height);
		glTexCoord2f(1.0f, 1.0f); glVertex2i(width, height);
		glTexCoord2f(1.0f, 0.0f); glVertex2i(width, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}


PIXL_FBO::PIXL_FBO()
{
	shader = 0;

	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &texture);

	glBindTexture( GL_TEXTURE_2D, texture);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, *PIXL_config.w, *PIXL_config.h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		puts("FBO error");
}

PIXL_FBO::~PIXL_FBO()
{
}

void PIXL_FBO::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
}

/**
 * @brief RTT to the whole screen
 *
 * @param You may draw to another FBO (eg for multipass shaders like gaussian blur)
 */
void PIXL_FBO::draw(PIXL_FBO* target)
{
	if(shader)
		glUseProgram(shader);

	if(target)
		target->bind();
	else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
		glTexCoord2f(0.0f, 0.0f); glVertex2i(0, *PIXL_config.h);
		glTexCoord2f(1.0f, 0.0f); glVertex2i(*PIXL_config.w, *PIXL_config.h);
		glTexCoord2f(1.0f, 1.0f); glVertex2i(*PIXL_config.w, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	if(shader)
		glUseProgram(0);
}


/**
 * @brief Layer class constructor
 */
PIXL_Layer::PIXL_Layer(int w, int h): width(w), height(h)
{
	sdlsurf = SDL_CreateRGBSurface( 0, width, height, 32,
									CAIROSDL_RMASK,
									CAIROSDL_GMASK,
									CAIROSDL_BMASK,
									CAIROSDL_AMASK );

	layer = cairosdl_surface_create(sdlsurf);

	context = cairo_create(layer);

	texture = new PIXL_Texture(getBuffer(), width, height);
}

void PIXL_Layer::draw()
{
	cairosdl_surface_flush(layer); 
	texture->draw();
}

void PIXL_Layer::clear()
{
	cairo_save(context);
	cairo_set_operator(context,CAIRO_OPERATOR_CLEAR);
	cairo_paint(context);
	cairo_restore(context);
}


PIXL_Image::PIXL_Image(PIXL_Layer* l, const char* f)
{
	layer = l;
	image = cairo_image_surface_create_from_png(f);
}

PIXL_Image::~PIXL_Image()
{
	cairo_surface_destroy(image);
}

void PIXL_Image::draw(int w=0, int h=0)
{
	cairo_set_source_surface(layer->getContext(), image, w, h);
	cairo_paint(layer->getContext());
}


PIXL_Sprite::PIXL_Sprite(const char* f)
{
		image = IMG_Load(f);
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// This could be better...
		switch(image->format->BitsPerPixel){
			case 24:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->w, image->h, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
				break;
			case 32:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, image->pixels);
				break;
		}
		glDisable(GL_TEXTURE_2D);
}

PIXL_Sprite::~PIXL_Sprite()
{
	SDL_FreeSurface(image);
	glDeleteTextures(1, &texture);
}

void PIXL_Sprite::draw(int x, int y)
{
	glColor4f(1.f,1.f,1.f,1.f);

	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, texture );
	//glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, image->w, image->h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2i(x+0, y+0);
		glTexCoord2f(0.0f, 1.0f); glVertex2i(x+0, y+image->h);
		glTexCoord2f(1.0f, 1.0f); glVertex2i(x+image->w, y+image->h);
		glTexCoord2f(1.0f, 0.0f); glVertex2i(x+image->w, y+0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}


PIXL_Animation::PIXL_Animation(const char* f, uint w, uint h, uint s): sprite_w(w), sprite_h(h), speed(s)
{
	image = IMG_Load(f);
	m=0; // we start with the first frame
	n=0; // and the first animation (just in case we try to draw without play() first)
	assert(speed!=0); // speed can't be 0 because we divide by speed
	start_time = SDL_GetTicks();
	playing=false;
	loop=false;

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, image->pixels);
	glDisable(GL_TEXTURE_2D);
}

PIXL_Animation::~PIXL_Animation()
{
	SDL_FreeSurface(image);
	glDeleteTextures(1, &texture);
}

void PIXL_Animation::draw(int x, int y)
{
	if(playing) {
		int frames = image->w/(int)sprite_w; // amount of frames in 
		if(loop) {
			m = ((SDL_GetTicks()-start_time)/(int)speed) % frames; // the modulo is not necessary since textures already repeat, but we want to have m and n
		} else {
			m = (SDL_GetTicks()-start_time)/(int)speed;
			if(m>=frames) {
				// we reached the end...
				m=frames-1;
				playing = false;
			}
		}
	}

	GLfloat vx = sprite_w/(float)image->w;
	GLfloat vy = sprite_h/(float)image->h;

	glColor4f(1.f,1.f,1.f,1.f);

	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, texture );
	glBegin(GL_QUADS);
		glTexCoord2f(m*vx, n*vy); glVertex2i(x+0, y+0);
		glTexCoord2f(m*vx, (n+1)*vy); glVertex2i(x+0, y+sprite_h);
		glTexCoord2f((m+1)*vx, (n+1)*vy); glVertex2i(x+sprite_w, y+sprite_h);
		glTexCoord2f((m+1)*vx, n*vy); glVertex2i(x+sprite_w, y+0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void PIXL_Animation::setSpeed(uint s)
{
	assert(s!=0);
	speed = s;
}

void PIXL_Animation::play(uint new_n, bool l=false)
{
	// TODO check n
	n = new_n;
	playing = true;
	loop = l;
	start_time = SDL_GetTicks();
}


PIXL_Text::PIXL_Text(PIXL_Layer* l, const char* f, uint s, int x=0, int y=0): context(l->getContext()), font_name((const FcChar8*)f), font_size(s), pos_x(x), pos_y(y)
{
	fc = FcConfigGetCurrent(); //para checkear si existe fuente
	blanks = FcBlanksCreate(); //para errores de fuentes
	count = 0;
	if(!FcConfigAppFontAddFile(fc, font_name)) {
		printf("ERROR FontConfig!\n");
	}
	pattern = FcFreeTypeQuery(font_name , 0, blanks, &count); //obtengo el pattern

	layout = pango_cairo_create_layout(context); //creo layout de pango para el texto
	font_description = pango_fc_font_description_from_pattern(pattern, 0); //le paso el pattern a pango

	pango_font_description_set_size(font_description, font_size*PANGO_SCALE); //seteo tamanio de la fuente
	pango_layout_set_font_description(layout, font_description); //seteo la fuente a usar en el layout
}

PIXL_Text::~PIXL_Text(){
	g_object_unref(layout);
	pango_font_description_free(font_description);
}

void PIXL_Text::print(const char* text){
	cairo_move_to(context, pos_x, pos_y);

	cairo_set_source_rgba(context, 1, 1, 1, 1);
	pango_cairo_show_layout(context, layout);

	pango_layout_set_text(layout, text, -1);
	cairo_fill(context);
}

