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
#include <SDL/SDL_image.h>
#include <fontconfig/fontconfig.h>
#include <pango/pangocairo.h>
#include <pango/pangofc-fontmap.h>
#include <cairo/cairo.h>
#include "cairosdl.h"
#include <librsvg/rsvg.h>
#include <yaml.h>

typedef unsigned int uint;

/**
 * @brief General configuration object
 */
class PIXL_Config {
	public:
		PIXL_Config(uint initial_width = 640, uint initial_height = 480): w(&width), h(&height), width(initial_width), height(initial_height) {}
		void setResolution(uint new_width, uint new_height) { width=new_width; height=new_height; }
		const uint *const w;
		const uint *const h;
	private:
		uint width;
		uint height;
} PIXL_Config;


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
 * @brief Timer constructor
 *
 * @param t interval time (ms)
 */
PIXL_Timer::PIXL_Timer(uint t)
{
	set(t);
	status_flag = false;
}

/**
 * @brief Change the current interval
 *
 * @param t new interval time
 */
void PIXL_Timer::set(uint t)
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
 * @brief Texture class constructor
 *
 * @param d pixel data
 * @param w width
 * @param h height
 */
PIXL_Texture::PIXL_Texture(const GLvoid* d, const int w, const int h):data(d),width(w),height(h)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
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


/**
 * @brief Sprite class (load png and svg)
 */
class PIXL_Sprite {
	public:
		PIXL_Sprite(PIXL_Layer* l, const char* f);
		virtual ~PIXL_Sprite();
		void draw(int w, int h);
	private:
		cairo_surface_t* image;
		PIXL_Layer* layer;
};

PIXL_Sprite::PIXL_Sprite(PIXL_Layer* l, const char* f)
{
	layer = l;
	image = cairo_image_surface_create_from_png(f);
}

void PIXL_Sprite::draw(int w, int h)
{
	cairo_set_source_surface(layer->getContext(), image, w, h);
	cairo_paint(layer->getContext());
}

PIXL_Sprite::~PIXL_Sprite()
{
	cairo_surface_destroy(image);
}


/**
 * @brief Text rendering class
 */
class PIXL_Text {
	public:
		PIXL_Text(PIXL_Layer* l, const char* f, uint s);
		virtual ~PIXL_Text();
		void setSize(uint s) { font_size=s; }
		void setPos(uint x, uint y) { cairo_move_to(context, x, y); };
		void print(const char* text);
	private:
		const FcChar8 *font_name;
		uint font_size;
		FcConfig *fc; //para checkear si existe fuente
		FcBlanks *blanks; //para errores de fuentes
		FcPattern *pattern;
		cairo_t* context; 
		PangoLayout *layout;
		PangoFontDescription *font_description;
		int count;
};

PIXL_Text::PIXL_Text(PIXL_Layer* l, const char* f, uint s): context(l->getContext()), font_name((const FcChar8*)f), font_size(s)
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

	if(!(screen = SDL_SetVideoMode(*PIXL_Config.w, *PIXL_Config.h, 32, SDL_OPENGL))){
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
class PIXL_State {
	public:
		PIXL_State(){state = 0;}
		enum {play, quit};
		void set(char s) {state = s;}
		uint get() {return state;}
	private:
		uint state;
} state;

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

int main(int argc, const char *argv[])
{
	PIXL_Init();

	SDL_Event event;

/**************************demo stuff*****************************************/
/*****************************************************************************/
PIXL_Layer *mylayer = new PIXL_Layer(*PIXL_Config.w, *PIXL_Config.h);

PIXL_Text *mytext = new PIXL_Text(mylayer, "fonts/ProggyTiny.ttf", 12);

PIXL_Sprite *mysprite = new PIXL_Sprite(mylayer, "bullet.png");
std::stringstream mystring;
int frame_count=0;
int fps=0;
int mytime=SDL_GetTicks();

double p; //pi phase
	
	double t = 0.f;
	const double dt = 1.f / 100.f;

	double currentTime = SDL_GetTicks(); //GetTicks es uint32
	double accumulator = 0.f;

	int loops=0;

		SDL_Surface* image = IMG_Load("test.png");
		glEnable(GL_TEXTURE_2D);
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 15, 15, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, image->pixels);
		glDisable(GL_TEXTURE_2D);

int myarray[200000];

	while(state.get() != state.quit)
	{
		double newTime = SDL_GetTicks();
		double frameTime = newTime - currentTime;
		currentTime = newTime;

		accumulator += frameTime;

		loops=0;
		while(accumulator >= dt)
		{
			//pi phase
			p+=M_PI/200000.f;
			if(p>2*M_PI)
				p=p-2*M_PI;

			accumulator -= dt;
			t += dt;

			loops++;
		}

		glClear( GL_COLOR_BUFFER_BIT );
		mylayer->clear();

		//for(int i=0; i<10000; i++){
			//mysprite->draw(320+sin(sin(p)*4*M_PI*i/100)*i*0.02,240+cos(sin(p)*4*M_PI*i/100)*i*0.02);
		//}

		//glEnable(GL_TEXTURE_2D);
		//glBindTexture(GL_TEXTURE_2D, texture);
		//glBegin(GL_QUADS);
			//for(int i=0; i<100000; i++){
				//int x=320+sin(sin(p)*4*M_PI*i/100)*i*0.002;
				//int y=240+cos(sin(p)*4*M_PI*i/100)*i*0.002;
				//glTexCoord2f(0.0f, 0.0f); glVertex2i(x, y);
				//glTexCoord2f(0.0f, 1.0f); glVertex2i(x, y+15);
				//glTexCoord2f(1.0f, 1.0f); glVertex2i(x+15, y+15);
				//glTexCoord2f(1.0f, 0.0f); glVertex2i(x+15, y);
			//}
		//glEnd();
		//glDisable(GL_TEXTURE_2D);

		//glEnable(GL_TEXTURE_2D);
		//glEnable(GL_POINT_SPRITE);
		//glBindTexture(GL_TEXTURE_2D, texture);
		//glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		//glPointSize(15);
		//glBegin(GL_POINTS);
			//for(int i=0; i<100000; i++){
				//int x=320+sin(sin(p)*4*M_PI*i/100)*i*0.002;
				//int y=240+cos(sin(p)*4*M_PI*i/100)*i*0.002;
				//glVertex2i(x,y);
			//}
		//glEnd();
		//glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
		//glDisable(GL_POINT_SPRITE);
		//glDisable(GL_TEXTURE_2D);

		for(int i=0; i<10000; i++){
			int x=320+sin(sin(p)*4*M_PI*i/100)*i*0.02;
			int y=240+cos(sin(p)*4*M_PI*i/100)*i*0.02;
			myarray[i*2]=x;
			myarray[i*2+1]=y;
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_INT, 0, myarray);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_POINT_SPRITE);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		glPointSize(15);
		glDrawArrays(GL_POINTS, 0, 10000);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_VERTEX_ARRAY);



		frame_count++;
		if(frame_count==20){
			fps=1000/((SDL_GetTicks()-mytime)/frame_count);
			mytime=SDL_GetTicks();
			frame_count=0;
		}
		mystring.str("");
		mystring << "FPS: " << fps;
		mystring << "\n" << SDL_GetTicks()/1000.f;
		mystring << "\nloops: " << loops;

		mytext->setPos(10,10);
		mytext->print(mystring.str().c_str());

		mylayer->draw();


		SDL_GL_SwapBuffers();

		///////////////////////////////
		SDL_PollEvent(&event);
		if(event.type == SDL_KEYDOWN)
		{
			switch(event.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					state.set(state.quit);
					break;
			}
		}
	}
/*****************************************************************************/
/*****************************************************************************/

	PIXL_End();

	return 0;
}

