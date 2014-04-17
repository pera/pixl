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
#include <GL/glew.h>
#include <GL/glxew.h>
#include <SDL/SDL.h>
#include <fontconfig/fontconfig.h>
#include <pango/pangocairo.h>
#include <pango/pangofc-fontmap.h>
#include <cairo/cairo.h>
#include "cairosdl.h"
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>


/**
 * @brief Timer class
 */
class PIXL_Timer {
	public:
		PIXL_Timer(unsigned int t);
		void set(unsigned int t);
		void start();
		void stop();
		bool status();
	private:
		unsigned int time;
		unsigned int start_time;
		bool status_flag;
};

/**
 * @brief Timer constructor
 *
 * @param t interval time (ms)
 */
PIXL_Timer::PIXL_Timer(unsigned int t)
{
	set(t);
	status_flag = false;
}

/**
 * @brief Change the current interval
 *
 * @param t new interval time
 */
void PIXL_Timer::set(unsigned int t)
{
	time = t;
	status_flag = false;
}

/**
 * @brief Stop the timer
 */
void PIXL_Timer::stop()
{
	status_flag = false;
}

/**
 * @brief Start the timer
 */
void PIXL_Timer::start()
{
	start_time = SDL_GetTicks();
	status_flag = true;
}

/**
 * @brief Get the current timer status
 *
 * @return 1 if the timer have finished and 0 if not
 */
bool PIXL_Timer::status()
{
	if( status_flag && ((start_time+time) <= SDL_GetTicks()) )
		return true;

	return false;
}


/**
 * @brief Generic object class
 */
class PIXL_Object {
	public:
	private:
};


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
		unsigned int width;
		unsigned int height;
};

/**
 * @brief Texture class constructor
 *
 * @param d pixel data
 * @param w width
 * @param h height
 */
PIXL_Texture::PIXL_Texture(const GLvoid* d, const int w, const int h):data(d)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA,
				 GL_UNSIGNED_INT_8_8_8_8_REV, data);

	width = w;
	height = h;
}

/**
 * @brief Render the texture object
 *
 * @param x x-position
 * @param y y-position
 */
void PIXL_Texture::draw(int x=0, int y=0)
{
		glColor4ub(255,255,255,255);
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


/**
 * @brief Cairo surface
 */
class PIXL_Canvas {
	public:
		PIXL_Canvas(int w, int h);
		~PIXL_Canvas();
		int getWidth() { return width; }
		int getHeight() { return height; }
		cairo_t* getContext() { return context; }
		void blit();
		void clear();
		void flush() { cairosdl_surface_flush(canvas); }
		void* getBuffer() { return sdlsurf->pixels; }
	private:
		SDL_Surface *sdlsurf;
		cairo_surface_t *canvas;
		cairo_t* context;
		int width;
		int height;
};

/**
 * @brief Canvas class constructor
 */
PIXL_Canvas::PIXL_Canvas(int w, int h): width(w), height(h)
{
	sdlsurf = SDL_CreateRGBSurface( 0, width, height, 32,
									CAIROSDL_RMASK,
									CAIROSDL_GMASK,
									CAIROSDL_BMASK,
									CAIROSDL_AMASK );

	canvas = cairosdl_surface_create(sdlsurf);

	context = cairo_create(canvas);
}

void PIXL_Canvas::clear()
{
	cairo_save(context);
	cairo_set_operator(context,CAIRO_OPERATOR_CLEAR);
	cairo_paint(context);
	cairo_restore(context);
}


/**
 * @brief Layer class (sdl surface)
 */
class PIXL_Layer {
	public:
	private:
};


/**
 * @brief Sprite class (load png and svg)
 */
class PIXL_Sprite {
	public:
	private:
};


/**
 * @brief Text rendering class
 */
class PIXL_Text {
	public:
		PIXL_Text(PIXL_Canvas* c, const char* f, unsigned int s);
		virtual ~PIXL_Text ();
		void setSize(unsigned int s) { font_size=s; }
		void setPos(unsigned int x, unsigned int y) { cairo_move_to(context, x, y); };
		void print(const char* text);
	private:
		const FcChar8 *font_name;
		unsigned int font_size;
		FcConfig *fc; //para checkear si existe fuente
		FcBlanks *blanks; //para errores de fuentes
		FcPattern *pattern;
		cairo_t* context; 
		PangoLayout *layout;
		PangoFontDescription *font_description;
		int count;
};

PIXL_Text::PIXL_Text(PIXL_Canvas* c, const char* f, unsigned int s): context(c->getContext()), font_name((const FcChar8*)f), font_size(s)
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
	cairo_set_source_rgba(context, 1, 1, 1, 1);
	pango_cairo_show_layout(context, layout);

	pango_layout_set_text(layout, text, -1);
	cairo_fill(context);
}


/**
 * @brief todo lo que requiera ser iniciado
 *
 * @return 
 */
int PIXL_Init()
{
	SDL_Surface *screen;
	SDL_Event event;
	cairo_surface_t* surf = NULL;


/*
 * SDL INITIALIZATION
 * 
 */

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK))
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

	if(!(screen = SDL_SetVideoMode(640, 480, 32, SDL_OPENGL))){
		printf("Unable to set video mode: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	glEnable(GL_MULTISAMPLE);

	SDL_WM_SetCaption("PIXL v" VERSION, NULL);

	SDL_ShowCursor(SDL_DISABLE);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	printf("OpenGL %s\n",(const char *) glGetString(GL_VERSION));
	printf("GLEW %s\n", glewGetString(GLEW_VERSION));
	//printf("Available extensions: %s\n",(const char *) glGetString(GL_EXTENSIONS));
	printf("GLSL %s\n",(const char *) glGetString(GL_SHADING_LANGUAGE_VERSION));

	if(!GLEW_EXT_framebuffer_object)
		puts("FBO unsupported");
	if(!GLEW_ARB_pixel_buffer_object)
		puts("PBO unsupported");

	SDL_Joystick *joystick1 = NULL;
	if(SDL_NumJoysticks()){
		printf("Joysticks found:\n");
		for(int i=0; i < SDL_NumJoysticks(); i++ ) 
		{
			printf(" %s (%i)\n", SDL_JoystickName(i), i);
			joystick1 = SDL_JoystickOpen(i);
		}
		SDL_JoystickEventState(SDL_ENABLE);
	}



/*
 * OPENGL SETTINGS
 *
 */

	glClearColor(0, 0, 0, 1);

	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, screen->w, screen->h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, screen->w, screen->h, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// liberar recursos (despues del main loop)
int PIXL_End(){
	SDL_Quit();
}

// esta funcion controla el game logic
void PIXL_Update(){}

// todo lo que sea dibujado
void PIXL_Render(){}

/**
 * @brief Scene manager for poors
 */
class PIXL_Stage {
	public:
		PIXL_Stage(){stage = 0;}
		enum {play, quit};
		void set(char s) {stage = s;}
		unsigned int get() {return stage;}
	private:
		unsigned int stage;
} stage;

/**
 * @brief Plain text loading function (faster than C++ rutines)
 *
 * @param file_name the path to the file
 *
 * @return a pointer to the text
 */
const char* PIXL_LoadTextFile(const char *file_name)
{
	FILE *fp;
	struct stat buf;
	char *source = NULL;

	if((stat(file_name, &buf)) < 0){
		fprintf(stderr, "Error: stat %s\n", file_name);
		exit(EXIT_FAILURE);
	}

	if(!(fp = fopen(file_name, "r"))){
		fprintf(stderr, "Error: open %s\n", file_name);
		exit(EXIT_FAILURE);
	}

	// Le sumamos 1 al tamaño del string para el null-terminated
	if(!(source = (char*) malloc ((sizeof(char) * buf.st_size) + 1))){
		fprintf(stderr, "Error: malloc\n");
		exit(EXIT_FAILURE);
	}

	fread(source, sizeof(char), buf.st_size, fp);
	source[buf.st_size] = '\0'; // Esto nos asegura que el final del string sea null-terminated, sino podriamos obtener un ETB antes

	if(fclose(fp)){
		fprintf(stderr, "Error: close %s\n", file_name);
		exit(EXIT_FAILURE);
	}

	return source;
}

/**
 * @brief Simple bounding box collision detection
 *
 * @param b1 the first box
 * @param b2 the second box
 *
 * @return if there is a collision it returns true, otherwise false
 */
bool PIXL_bbc(SDL_Rect b1, SDL_Rect b2)
{
    if ((b1.x > b2.x + b2.w - 1) ||
        (b1.y > b2.y + b2.h - 1) ||
        (b2.x > b1.x + b1.w - 1) ||
        (b2.y > b1.y + b1.h - 1))  
    {
        // no collision
        return false;
    }
 
    // collision
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int main(int argc, const char *argv[])
{
	PIXL_Init();
	
	SDL_Event event;

/*****************************************************************************/
/*****************************************************************************/
PIXL_Canvas *mycanvas = new PIXL_Canvas(640, 480);
cairo_t *cr = mycanvas->getContext();

PIXL_Text *mytext = new PIXL_Text(mycanvas, "fonts/ProggyTiny.ttf", 12);

PIXL_Texture *mytexture = new PIXL_Texture(mycanvas->getBuffer(), mycanvas->getWidth(), mycanvas->getHeight());
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
std::stringstream mystring;
int frame_count=0;
int fps=0;
int t=SDL_GetTicks();

double p; //pi phase
	while(stage.get() != stage.quit)
	{
		glClear( GL_COLOR_BUFFER_BIT );

		//pi phase
		p+=M_PI/200.f;
		if(p>2*M_PI)
			p=p-2*M_PI;
		glPointSize(1.f);
		glColor4f(1.f,1.f,1.f,1.f);
		glBegin(GL_POINTS);
			for(int i=0; i<1000; i++){
				//glVertex2i(320+sin(sin(p)*4*M_PI*i/100)*i,240+cos(sin(p)*4*M_PI*i/100)*i);
				glVertex2i(320+tan(p-2*M_PI*i/1000.f)*100,240+sin(p+2*M_PI*i/100.f)*100*sin(p));
				//glVertex2i(100+2*(2+sin(p))*i,240+cos(p)*i+0.5*i*sin(p+i));
			}
		glEnd();

		frame_count++;
		if(frame_count==20){
			fps=1000/((SDL_GetTicks()-t)/frame_count);
			t=SDL_GetTicks();
			frame_count=0;
		}
		mystring.str("");
		mystring << "FPS: " << fps;

		mycanvas->clear();

		mytext->setPos(10,10);
		mytext->print(mystring.str().c_str());

		mycanvas->flush();
		mytexture->draw();


		SDL_GL_SwapBuffers();

		///////////////////////////////
		SDL_PollEvent(&event);
		if(event.type == SDL_KEYDOWN)
		{
			switch(event.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					stage.set(stage.quit);
					break;
			}
		}
	}
/*****************************************************************************/
/*****************************************************************************/

	PIXL_End();

	return 0;
}

