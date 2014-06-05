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

#include "app.h"

/*
 * Instantiating the ugly global...
 *
 */
PIXL_Config PIXL_config;


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


GLuint PIXL_loadShader(const char* filename)
{
	GLuint FragmentShader;
	GLint linked;
	GLuint program;
	GLint compiled;

	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	program = glCreateProgram();

	const GLchar *fragmentSrc = PIXL_LoadTextFile(filename);

	glShaderSource(FragmentShader, 1, &fragmentSrc, NULL);
	glCompileShader(FragmentShader);

// TODO comparar con https://www.opengl.org/wiki/GLSL#Error_Checking
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &compiled);

	if (!compiled) {
		GLint length;
		GLchar* log;
		glGetShaderiv(FragmentShader, GL_INFO_LOG_LENGTH, &length);

		log = (GLchar*) malloc(length);
		glGetShaderInfoLog(FragmentShader, length, &length, log);
		fprintf(stderr, "\nCompile log\n-----------\n%s\n", log);
	}

	glAttachShader(program, FragmentShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	if(linked){
		glUseProgram(program);
	} else {
		GLint length;
		GLchar* log;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

		log = (GLchar*) malloc(length);
		glGetProgramInfoLog(program, length, &length, log);
		fprintf(stderr, "Linking %s\n", log);
	}

	GLint texLoc = glGetUniformLocation(program, "sampler0");
	glUniform1i(texLoc, 0);

	GLfloat width = glGetUniformLocation(program, "w");
	GLfloat height = glGetUniformLocation(program, "h");
	glUniform1f(width, *PIXL_config.w);
	glUniform1f(height, *PIXL_config.h);

	glActiveTexture(GL_TEXTURE0);

	GLenum errCode = glGetError();
	const GLubyte *errString;
	if(errCode != GL_NO_ERROR) {
		errString = gluErrorString(errCode);
		fprintf(stderr, "OpenGL Error: %s\n", errString);
	}

	glUseProgram(0);

	return program;
}


PIXL_App::PIXL_App()
{
	/*
	 * SDL INITIALIZATION
	 * 
	 */

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK))
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		//return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

	if(!(screen = SDL_SetVideoMode(*PIXL_config.w, *PIXL_config.h, 32, SDL_OPENGL))){
		printf("Unable to set video mode: %s\n", SDL_GetError());
		//return 1;
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
		//return 1;
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

	glViewport(0, 0, *PIXL_config.w, *PIXL_config.h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, *PIXL_config.w, *PIXL_config.h, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

