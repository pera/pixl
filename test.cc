#include "pixl.h"

///////////////////////////////////////////////////////////////////////////////
// Map loading sample functions ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <libxml/xmlreader.h>
#include <libxml/xmlstring.h>

typedef struct {
	unsigned int w;
	unsigned int h;
} PIXL_T_size;

typedef struct {
	std::vector<int> tile;
	PIXL_T_size tile_size;
	std::string tileset_file;
	PIXL_T_size tileset_size;
	PIXL_T_size size;
} PIXL_T_map;

inline bool isCurrentElementX(xmlTextReaderPtr reader, const char* elem)
{
	return 0 == xmlStrcmp(xmlTextReaderConstName(reader),(xmlChar*)elem);
}

inline bool isEndOfElementX(xmlTextReaderPtr reader, const char* elem)
{
	return isCurrentElementX(reader,elem) && 15 == xmlTextReaderNodeType(reader);
}

// TODO: error checking, using atoi
void loadMap(const char* filename, PIXL_T_map *map)
{
	map->tile_size.w = 16;
	map->tile_size.h = 16;
	
	xmlTextReaderPtr reader = xmlReaderForFile(filename, NULL, 0);
	if(reader==NULL){
		printf("Error: xmlReaderForFile() returned NULL when opening \"%s\"\n\n", filename);
		return;
	}

	while(xmlTextReaderRead(reader)==1 && !isEndOfElementX(reader,"map")){
		if(isCurrentElementX(reader,"image")){
			map->tileset_size.w = atoi((char*)xmlTextReaderGetAttribute(reader,(xmlChar*)"width"));
			map->tileset_size.h = atoi((char*)xmlTextReaderGetAttribute(reader,(xmlChar*)"height"));
			map->tileset_file = std::string( (char*)xmlTextReaderGetAttribute(reader,(xmlChar*)"source") );
		} else if(isCurrentElementX(reader,"layer")){
			map->size.w = atoi((char*)xmlTextReaderGetAttribute(reader,(xmlChar*)"width"));
			map->size.h = atoi((char*)xmlTextReaderGetAttribute(reader,(xmlChar*)"height"));
			while(xmlTextReaderRead(reader) && !isEndOfElementX(reader,"layer")){
				if(isCurrentElementX(reader,"tile")){
					map->tile.push_back(atoi((char*)xmlTextReaderGetAttribute(reader,(xmlChar*)"gid")));
				}
			}
		}
	}

	xmlFreeTextReader(reader);

	return;
}

///////////////////////////////////////////////////////////////////////////////
// Game engine main object ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class Game: public PIXL_App {
	public:
		Game();
		void update();
		void render();
	private:
		PIXL_Layer *mylayer;

		PIXL_Layer *mylayer2;
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

		/***************/
		/* LOADING MAP */
		/***************/
		int myarray[25*15*2];
		GLuint ttexture;
		GLuint vboId;
};

Game::Game()
{
	mylayer = new PIXL_Layer(*PIXL_config.w, *PIXL_config.h);

	mylayer2 = new PIXL_Layer(*PIXL_config.w, *PIXL_config.h);
	mytext = new PIXL_Text(mylayer2, "fonts/ProggyTiny.ttf", 12, 10, 10);

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

	/***************/
	/* LOADING MAP */
	/***************/
	LIBXML_TEST_VERSION; // this should be in the init
	PIXL_T_map map;
	loadMap("map.tmx", &map);

/////////////////////////////////___________________________________VBO

	SDL_Surface* image = IMG_Load("tile.png");
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &ttexture);
	glBindTexture(GL_TEXTURE_2D, ttexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
	glDisable(GL_TEXTURE_2D);

	for(int i=0; i<(25*15*2); i+=2){
		myarray[i]=((i/2)%25)*16 +100;
		myarray[i+1]=((i/2)/25)*16 +100;
	}

	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myarray), myarray, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

//	maplayer = new PIXL_Layer(map.size.w, map.size.h);
//	tileset = new PIXL_Sprite(map.tileset_file.c_str()); // DANGER ?!
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
	mylayer2->clear();

	for(int i=0; i<100; i++){
		myimage->draw(320+sin(sin(p)*4*M_PI*i/100)*i*2,240+cos(sin(p)*4*M_PI*i/100)*i*2);
	}

	mylayer->draw();

	mysprite->draw(*PIXL_config.w*0.5+(100*cos(p*2)),*PIXL_config.h*0.5+(100*sin(p*2)));

	myfbo->draw(myfbo2);
	myfbo2->draw();

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
	mylayer2->draw();

	myanimation->draw(50,50);



/////////////////////////////////___________________________________VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_INT, 0, 0);
	//texture
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_POINT_SPRITE);
	glBindTexture(GL_TEXTURE_2D, ttexture);
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glPointSize(16);
	//draw the vbo
	glDrawArrays(GL_POINTS, 0, 25*15);
	//deactivate
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
	glDisable(GL_POINT_SPRITE);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


int main(int argc, const char *argv[])
{
	Game* mygame = new Game();

	mygame->run();

	delete mygame;

	return 0;
}

