#include "pixl.h"

class Game: PIXL_App {
	public:
		Game();
		void run();
		void update();
		void render();
	private:
		SDL_Event event;
		PIXL_State state;
		/////////////////////////////////

		PIXL_Layer *mylayer;

		PIXL_Text *mytext;

		PIXL_Image *myimage;
		std::stringstream mystring;
		int frame_count;
		int fps;
		int mytime;

		PIXL_Sprite *mysprite;

		PIXL_Animation *myanimation;

		double p; //pi phase

		PIXL_FBO *myfbo;
		PIXL_FBO *myfbo2;
};

Game::Game()
{
	mylayer = new PIXL_Layer(*PIXL_config.w, *PIXL_config.h);

	mytext = new PIXL_Text(mylayer, "fonts/ProggyTiny.ttf", 12, 10, 10);

	myimage = new PIXL_Image(mylayer, "bullet.png");
	frame_count=0;
	fps=0;
	mytime=SDL_GetTicks();

	mysprite = new PIXL_Sprite("test.png");

	myanimation = new PIXL_Animation("cats.png", 23, 23, 100);
	myanimation->play(3,true);

	myfbo = new PIXL_FBO();
	myfbo2 = new PIXL_FBO();

	myfbo->shader = PIXL_loadShader("gbh.glsl");
	myfbo2->shader = PIXL_loadShader("gbv.glsl");
}

void Game::run()
{
	/*** game time stuff HACK ***/
	double t = 0.f;
	const double dt = 1.f / 100.0f;

	double currentTime = SDL_GetTicks(); //GetTicks es uint32
	double accumulator = 0.f;

	/*** MAIN LOOP ***/
	while(state.get() != state.quit)
	{
		double newTime = SDL_GetTicks();
		double frameTime = newTime - currentTime;
		currentTime = newTime;

		accumulator += frameTime;

		/*** UPDATE ***/
		while(accumulator >= dt)
		{
			update();

			accumulator -= dt;
			t += dt;
		}

		/*** RENDER ***/
		render();
		SDL_GL_SwapBuffers();

		/*** INPUT HANDLING ***/
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
}

void Game::update()
{
	//pi phase
	p+=M_PI/200000.f;
	if(p>2*M_PI)
		p=p-2*M_PI;
}

void Game::render()
{
	myfbo->bind();

	glClear( GL_COLOR_BUFFER_BIT );
	mylayer->clear();

	for(int i=0; i<100; i++){
		myimage->draw(320+sin(sin(p)*4*M_PI*i/100)*i*2,240+cos(sin(p)*4*M_PI*i/100)*i*2);
	}

	mysprite->draw(*PIXL_config.w*0.5+(100*cos(p*2)),*PIXL_config.h*0.5+(100*sin(p*2)));

	frame_count++;
	if(frame_count==20){
		fps=1000/((SDL_GetTicks()-mytime)/frame_count);
		mytime=SDL_GetTicks();
		frame_count=0;
	}
	mystring.str("");
	mystring << "FPS: " << fps;
	mystring << "\n" << SDL_GetTicks()/1000.f;

	mytext->print(mystring.str().c_str());

	mylayer->draw();

	myfbo->draw(myfbo2);
	myfbo2->draw();


	myanimation->draw(500,50);
}


int main(int argc, const char *argv[])
{
	Game* mygame = new Game();

	mygame->run();

	delete mygame;

	return 0;
}

