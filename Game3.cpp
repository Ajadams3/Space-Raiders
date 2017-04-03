#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>

using namespace std;

const int SW = 1000;
const int SH = 800;

const int LW = 4000;
const int LH = 4000;

const int TW = 20;
const int TH = 20;
const int TOTAL_TILES = (LW/TW) * (LH/TH);
const int TOTAL_TILE_SPRITES = 8;

int TILE_SPACE = 0;
int TILE_IRR_SPACE = 1;
int TILE_METAL_FLOOR = 2;
int TILE_IRR_METAL_FLOOR = 3;
int TILE_ASTEROID = 4;
int TILE_IRR_ASTEROID = 5;
int TILE_WALL = 6;
int TILE_IRR_WALL = 7;
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Rect gTileClips[TOTAL_TILE_SPRITES];


class LTexture
{
	public:
	
	LTexture(){
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
	~LTexture(){
		free();
	}
	
	bool LoadFromFile(string path){
		free();
		SDL_Texture* newTexture = NULL;
		
		SDL_Surface* loadedSurface = IMG_Load(path.c_str());
		if(loadedSurface == NULL){
			cout << "Unable to load image SDL_image Error: " << path.c_str() << IMG_GetError() << endl;
		}	
		else{
			SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
			
			newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
			if(newTexture ==NULL){
				cout << " Unable to create texture from SDL Error: " << path.c_str() << SDL_GetError() << endl;
			}
			else{
				mWidth = loadedSurface->w;
				mHeight = loadedSurface->h;
			}
			SDL_FreeSurface(loadedSurface);
		}
		mTexture = newTexture;
		return mTexture != NULL;
}
	
	#ifdef _SDL_TTF_H
	bool loadFromRenderedText(string textureText, SDL_Color textColor){
		free();
		
		SDL_Surface* textSurface = TTF_RenderText_Solid(gfont, textureText.c_str(), textColor);
		if(textSurface !=NULL){
			if(mTexture == NULL){
				cout << "Unable to create texture from rendered text! SDL Error: ", SDL_GetError() << endl;
			}			
		
			else{
				mWidth = textSurface->w;
				mHeight = textSurface->h;
				}
			}
		else{
			cout << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << endl;
		}
		return mTexture != NULL;
	}
	#endif
	
	void free(){
		if(mTexture != NULL){
			SDL_DestroyTexture(mTexture);
			mTexture = NULL;
			mWidth = 0;
			mHeight = 0;			
		}
	}
	
	void setColor(Uint8 red, Uint8 green, Uint8 blue){
		SDL_SetTextureColorMod(mTexture,red,green,blue);
	}
	
	void setBlendMode(SDL_BlendMode blending){
		SDL_SetTextureBlendMode(mTexture,blending);
	}
	
	void setAlpha(Uint8 alpha){
		SDL_SetTextureAlphaMod(mTexture,alpha);
	}
	
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE){
		SDL_Rect renderQuad = {x, y, mWidth, mHeight};
		if(clip !=NULL){
			renderQuad.w = clip->w;
			renderQuad.h = clip->h;
		}
		
		SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
	}
	
	int getWidth(){
		return mWidth;
	}
	int getHeight(){
		return mHeight;
	}
	
	private:
	
	SDL_Texture* mTexture;
	
	int mWidth;
	int mHeight;
	
	
};

LTexture gTileTexture;
LTexture gDotTexture;
LTexture BackgroundTexture;
bool checkCollision(SDL_Rect a, SDL_Rect b)
{
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    leftA = a.x;
    rightA = a.x + a.w;
    topA = a.y;
    bottomA = a.y + a.h;

    leftB = b.x;
    rightB = b.x + b.w;
    topB = b.y;
    bottomB = b.y + b.h;

    if( bottomA <= topB )
        return false;

    if( topA >= bottomB )
        return false;

    if( rightA <= leftB )
		return false;

    if( leftA >= rightB )
		return false;

    return true;
}

class Tile{
	
	private:
	
	SDL_Rect mBox;
	
	int mType;
	public:
	
	Tile(int x, int y, int tileType){
	mBox.x = x;
    mBox.y = y;
    mBox.w = TW;
    mBox.h = TH;

    mType = tileType;
	}
	
	void render(SDL_Rect& camera){
		if(checkCollision(camera,mBox)){
			gTileTexture.render(mBox.x - camera.x, mBox.y - camera.y, &gTileClips[mType]);
		}
	}
	
	int getType(){
		return mType;
	}
	
	SDL_Rect getBox(){
		return mBox;
	}
};
	bool setTiles(Tile *tiles[]){
		bool tilesLoaded = true;
		int x = 0; int y = 0;
		
		ifstream map("tiles.map");
			for(int i =0; i < TOTAL_TILES; ++i)
			{
				int tileType = -1;
				map >> tileType;
				if(map.fail()){
					cout << "Error Loading map" << endl;
					tilesLoaded = false;
					break;
				}
				if((tileType >= 0) && (tileType < TOTAL_TILE_SPRITES))
					tiles[i] = new Tile(x,y,tileType); 
				else{
					cout << "Error Loading map" << endl;
					tilesLoaded = false;
					break;
				}
				x+= TW;
				if(x>= LW){
					x = 0;
					y+=TH;				
				}
			}
			if(tilesLoaded)
			{
				gTileClips[ TILE_SPACE ].x = 0;
				gTileClips[ TILE_SPACE ].y = 0;
				gTileClips[ TILE_SPACE ].w = TW;
				gTileClips[ TILE_SPACE].h = TH;
				
				gTileClips[ TILE_IRR_SPACE].x = 20;
				gTileClips[ TILE_IRR_SPACE].y = 0;
				gTileClips[ TILE_IRR_SPACE].w = TW;
				gTileClips[ TILE_IRR_SPACE].h = TH;
				
				gTileClips[ TILE_METAL_FLOOR].x = 40;
				gTileClips[ TILE_METAL_FLOOR].y = 0;
				gTileClips[ TILE_METAL_FLOOR].w = TW;
				gTileClips[ TILE_METAL_FLOOR].h = TH;
				
				gTileClips[ TILE_IRR_METAL_FLOOR ].x = 60;
				gTileClips[ TILE_IRR_METAL_FLOOR ].y = 0;
				gTileClips[ TILE_IRR_METAL_FLOOR ].w = TW;
				gTileClips[ TILE_IRR_METAL_FLOOR].h = TH;
				
				gTileClips[ TILE_ASTEROID ].x = 80;
				gTileClips[ TILE_ASTEROID ].y = 0;
				gTileClips[ TILE_ASTEROID ].w = TW;
				gTileClips[ TILE_ASTEROID].h = TH;
				
				gTileClips[ TILE_IRR_ASTEROID ].x = 100;
				gTileClips[ TILE_IRR_ASTEROID ].y = 0;
				gTileClips[ TILE_IRR_ASTEROID ].w = TW;
				gTileClips[ TILE_IRR_ASTEROID].h = TH;
				
				gTileClips[ TILE_WALL ].x = 120;
				gTileClips[ TILE_WALL ].y = 0;
				gTileClips[ TILE_WALL ].w = TW;
				gTileClips[ TILE_WALL].h = TH;
				
				gTileClips[ TILE_IRR_WALL ].x = 140;
				gTileClips[ TILE_IRR_WALL ].y = 0;
				gTileClips[ TILE_IRR_WALL ].w = TW;
				gTileClips[ TILE_IRR_WALL].h = TH;
				}
			
		map.close();
		
		return tilesLoaded;
		
		
	}

	
bool touchesWall(SDL_Rect box, Tile* tiles[]){
	 //Go through the tiles
    for( int i = 0; i < TOTAL_TILES; ++i )
    {
		if( ( tiles[ i ]->getType() >= TILE_ASTEROID ) && ( tiles[ i ]->getType() <= TILE_WALL ) )
            if( checkCollision( box, tiles[ i ]->getBox()))
                return true;
    }

 return false;
}

class Entities{
	static const int DOT_WIDTH = 15;
	static const int DOT_HEIGHT = 15;
	static const int DOT_VEL = 1;
	
	public:
	Entities(){
		DesiredX = Box.x = 300;
		DesiredY = Box.y = 300;
		Box.w = DOT_WIDTH;
		Box.h = DOT_HEIGHT;
		VelX = 0;
		VelY = 0;
		Cam.x = 0;
		Cam.y = 0;
		CVelX = CVelY = 0;
		
	}
	void pathfinder(int x, int y)
	{		
		{
			
			cout << x - Box.x << "," << y - Box.y << endl;
			 if(Box.x > x && Box.y > y)
			{
				VelY=-DOT_VEL;
				VelX=-DOT_VEL; 
			}
			 if(Box.x < x && Box.y > y)
			{
				VelY=-DOT_VEL;
				VelX=DOT_VEL; 
			}
			if(Box.x > x && Box.y < y)
			{
				VelY=DOT_VEL;
				VelX=-DOT_VEL; 
			}
			if(Box.x < x && Box.y < y)
			{
				VelY=DOT_VEL;
				VelX=DOT_VEL; 
			}
			
			if(Box.x > x){
					VelX=-DOT_VEL;
				}
			if(Box.x < x){
					VelX=DOT_VEL;
				}
			if(Box.y > y){
					VelY=-DOT_VEL;
				}
			if(Box.y < y){
					VelY=DOT_VEL;
				}
		}
	}
	void handleEvent(SDL_Event& e){
		if (e.type == SDL_KEYDOWN && e.key.repeat == 0){
			switch(e.key.keysym.sym){
				case SDLK_w: CVelY = -DOT_VEL; break;
				case SDLK_s: CVelY = DOT_VEL; break;
				case SDLK_a: CVelX = -DOT_VEL; break;
				case SDLK_d: CVelX = DOT_VEL; break;
			}
		}
		else if( e.type == SDL_KEYUP && e.key.repeat == 0 ){
            switch( e.key.keysym.sym ){
				case SDLK_w: CVelY += DOT_VEL; break;
				case SDLK_s: CVelY -= DOT_VEL; break;
				case SDLK_a: CVelX += DOT_VEL; break;
				case SDLK_d: CVelX -= DOT_VEL; break;
			}
		}
		if (e.type == SDL_MOUSEBUTTONDOWN){
			if(e.button.button == SDL_BUTTON_LEFT)
			{
				SDL_GetMouseState(&DesiredX,&DesiredY);
				CamNX = DesiredX + Cam.x;
				CamNY = DesiredY + Cam.y;
				cout << CamNX << "," << CamNY << endl;
				pathfinder(CamNX,CamNY);
				cout << "5" << endl;
			}
		}
	}
	
	void move(Tile *tiles[]){
	if((abs((CamNX-Box.x)) >= 2) || (abs((CamNY-Box.y)) >= 2)){
			Box.x += VelX;
			pathfinder(CamNX,CamNY);
			if( ( Box.x < 0 ) || ( Box.x + DOT_WIDTH > LW ) || touchesWall( Box, tiles ) ){
				Box.x -= VelX;
				cout << "6" << endl;
				pathfinder(CamNX,CamNY);
			}
			Box.y += VelY;
			pathfinder(CamNX,CamNY);	
			if( (Box.y < 0) || (Box.y + DOT_HEIGHT > LH) || touchesWall(Box, tiles)){
				Box.y -= VelY;
				cout << "2" << endl;
				pathfinder(CamNX,CamNY);
			}
		}
			Cam.x += CVelX;
			if( ( Cam.x < 0 ) || ( Cam.x > LW )){
				Cam.x -= CVelX;
				
			}
			Cam.y += CVelY;
			if( (Cam.y < 0) || (Cam.y > LH)){
				Cam.y -= CVelY;	
			}
}
	void setCamera(SDL_Rect& camera){
	camera.x = (Cam.x);
	camera.y = (Cam.y);
	if( camera.x < 0 ){ 
		camera.x = 0;
		Cam.x = 0;
	}
	if( camera.y < 0 ){
		camera.y = 0;
		Cam.y = 0;
	}
	if( camera.x > LW - camera.w ){
		camera.x = LW - camera.w;
		Cam.x = LW - camera.w;
	}
	if( camera.y > LH - camera.h ){
		camera.y = LH - camera.h;
		Cam.y = LH - camera.h;
	}
}
	void render(SDL_Rect& camera){
		gDotTexture.render( Box.x - camera.x, Box.y - camera.y );
	}
	
	private:
	SDL_Rect Cam;
	SDL_Rect Box;
	int CVelX,CVelY;
	int VelX, VelY;
	int CamNX, CamNY;
	int DesiredX, DesiredY;
};

void loadMedia(Tile* tiles[]){
	if(!gDotTexture.LoadFromFile("dot.bmp")){
		cout << "Failed to load dot texture! " << endl;
	}
	if(!BackgroundTexture.LoadFromFile("dot.bmp")){
		cout << "Failed to load dot texture! " << endl;
	}
	if(!gTileTexture.LoadFromFile("tiles.png")){
		cout << "Failed to load tile set texture! "  << endl;
	}
	if(!setTiles(tiles)){
		cout << "Failed to load tile set!\n" << endl;
	}
	
}

class Game {
	protected:
	
	public:
	virtual void init(const char *gameName){
		if(SDL_Init(SDL_INIT_VIDEO) < 0) cout << "SDL_Initialization error: " << SDL_GetError() << endl;
		else
		{
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
			{
				cout <<  "Warning: Linear texture filtering not enabled!" << endl;
			}
		
		
		
		gWindow = SDL_CreateWindow( gameName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SW, SH, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
			{
				cout << "Window could not be created! SDL Error: " << SDL_GetError() <<endl;
			}
		else
			{
				gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
				if( gRenderer == NULL )
				{
					cout <<  "Renderer could not be created! SDL Error: " << SDL_GetError() << endl;
				}
				else
				{
					SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
					int imgFlags = IMG_INIT_PNG;
					if( !( IMG_Init( imgFlags ) & imgFlags ) )
					{
						cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError()<<endl;
					}
				}
			}
		}
	}

	virtual void close(Tile* tiles[]){
		for( int i = 0; i < TOTAL_TILES; ++i )
		{
 			 if( tiles[ i ] == NULL )
			 {
				delete tiles[ i ];
				tiles[ i ] = NULL;
			 }
		}
		gDotTexture.free();
		gTileTexture.free();
		
		SDL_DestroyRenderer( gRenderer );
		SDL_DestroyWindow( gWindow );
		gWindow = NULL;
		gRenderer = NULL;

		IMG_Quit();
		SDL_Quit();
	}

	void run(){
		
		bool quit = false;
		
		
		while (!quit){
			SDL_Event e;
			while(SDL_PollEvent(&e) !=0){
				
				if(e.type == SDL_QUIT)
					quit = true;
				if(!quit)
					handleEvent(e);
			}
			
			
			
			SDL_RenderClear(gRenderer);
			
			show();
			
		
			SDL_RenderPresent(gRenderer);
		}
	}
	
	virtual void show() = 0;
	virtual void handleEvent(SDL_Event &e) = 0;
};
SDL_Rect camera = {0,0,SW,SH};
class ourGame: public Game{
	Entities CH1;
	Tile* tileSet[TOTAL_TILES];
	
	
	public:
	void init(const char *gameName = "Space Raid"){
		Game::init(gameName);
		loadMedia(tileSet);
		
		
	}
	
	void show(){
		CH1.move(tileSet);
		CH1.setCamera(camera);
		for (int i = 0; i < TOTAL_TILES; ++i)
		{
			tileSet[i]->render(camera);
		}
		
		CH1.render(camera);
		SDL_SetRenderDrawColor(gRenderer,0xFF,0xFF,0xFF,0xFF);
	}
	
	void handleEvent(SDL_Event &e){
		CH1.handleEvent(e);
	}
	
	void close(){
		Game::close(tileSet);
	}
};

int main( int argc, char *argv[] ){
	ourGame g;
	g.init();
	g.run();
	g.close();
	return 0;
}
