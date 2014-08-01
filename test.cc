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
	std::vector<int> tile; // tile number starting from 1
	PIXL_T_size tile_size; // in pixels
	std::string tileset_file; // atlas file name
	PIXL_T_size tileset_size; // in tiles
	PIXL_T_size size; // in tiles
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
			map->tileset_size.w = atoi((char*)xmlTextReaderGetAttribute(reader,(xmlChar*)"width")) / map->tile_size.w;
			map->tileset_size.h = atoi((char*)xmlTextReaderGetAttribute(reader,(xmlChar*)"height")) / map->tile_size.h;
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
		struct {
			GLint x;
			GLint y;
		} myarray[(25+1)*(15+1)*4]; // vbo array, since indices is GLushort the max size should be sizeof(GLushort) which is at least 16 bits (gl2.1 specs, table 2.2), therefore the map should not be bigger than 256x256
		struct {
			GLfloat s;
			GLfloat t;
		} myarray2[(25+1)*(15+1)*4];
		PIXL_Texture *ttexture;
		GLuint vbo;
		GLuint vbo2;
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

	SDL_Surface* image = IMG_Load(map.tileset_file.c_str());
	ttexture = new PIXL_Texture(image->pixels, image->w, image->h);

	/****************/
	/* VERTEX ARRAY */
	/****************/
	for(int i=0; i<((map.size.w+1)*(map.size.h+1)*4); i+=4){
		myarray[i+0].x = ((i/4)%map.size.w)*map.tile_size.w +100;
		myarray[i+0].y = ((i/4)/map.size.w)*map.tile_size.h +100;

		myarray[i+1].x = myarray[i].x;
		myarray[i+1].y = myarray[i].y + map.tile_size.h;

		myarray[i+2].x = myarray[i].x + map.tile_size.w;
		myarray[i+2].y = myarray[i].y + map.tile_size.h;

		myarray[i+3].x = myarray[i].x + map.tile_size.w;
		myarray[i+3].y = myarray[i].y;
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myarray), myarray, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/*******************/
	/* TEXCOORDS ARRAY */
	/*******************/
	const float w = 1.f/map.tileset_size.w;
	const float h = 1.f/map.tileset_size.h;
	for(int i=0; i<(map.size.w*map.size.h*4); i+=4){
		int u = (map.tile[i/4]-1) % map.tileset_size.w;
		int v = (map.tile[i/4]-1) / map.tileset_size.w;

		myarray2[i+0].s=w*u;
		myarray2[i+0].t=h*v;

		myarray2[i+1].s=w*u;
		myarray2[i+1].t=h*v + h;

		myarray2[i+2].s=w*u + w;
		myarray2[i+2].t=h*v + h;

		myarray2[i+3].s=w*u + w;
		myarray2[i+3].t=h*v;
	}

	glGenBuffers(1, &vbo2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myarray2), myarray2, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_INT, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);
	ttexture->bind();
	glDrawArrays(GL_QUADS, 0, 25*15*4);
	ttexture->unbind();
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


int main(int argc, const char *argv[])
{
	Game* mygame = new Game();

	mygame->run();

	delete mygame;

	return 0;
}

