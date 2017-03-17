#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

const int SCW = 1520;
const int SCH = 800;

int LW = 10000;
int LH = 10000;

int TW = 20;
int TH = 20;
int TOTAL_TILES = (LW/TW) * (LH/TH);
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
LTexture StartTexture;
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

bool touchesWall(SDL_Rect box, Tile* tiles[]){
    for( int i = 0; i < TOTAL_TILES; ++i )
    {
		if( ( tiles[ i ]->getType() >= TILE_ASTEROID ) && ( tiles[ i ]->getType() <= TILE_WALL ) )
            if( checkCollision( box, tiles[ i ]->getBox()))
                return true;
    }

 return false;
}
bool setTiles(Tile *tiles[]){
	bool tilesLoaded = true;
	int x = 0; int y = 0;
	
	ifstream map("tiles.map");
	
	if(map == NULL){
		cout << " Unable to load map file " << endl;
		tilesLoaded = false;
	}
	else{
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
		
	}
	map.close();
	
	return tilesLoaded;
	
	
}
class Characters{
	static const int DOT_WIDTH = 15;
	static const int DOT_HEIGHT = 15;
	
	static const int DOT_VEL = 20;
	public:
	Characters(){
		mBox.x = 0;
		mBox.y = 0;
		mBox.w = DOT_WIDTH;
		mBox.h = DOT_HEIGHT;
		mVelX = 0;
		mVelY = 0;
	}
	
	void handleEvent(SDL_Event& e){
		if (e.type == SDL_KEYDOWN && e.key.repeat == 0){
			switch(e.key.keysym.sym){
				case SDLK_w: mVelY -= DOT_VEL; break;
				case SDLK_s: mVelY += DOT_VEL; break;
				case SDLK_a: mVelX -= DOT_VEL; break;
				case SDLK_d: mVelX += DOT_VEL; break;
			}
		}
		else if( e.type == SDL_KEYUP && e.key.repeat == 0 ){
            switch( e.key.keysym.sym ){
				case SDLK_w: mVelY += DOT_VEL; break;
				case SDLK_s: mVelY -= DOT_VEL; break;
				case SDLK_a: mVelX += DOT_VEL; break;
				case SDLK_d: mVelX -= DOT_VEL; break;
			}
		}
	}
	
	void move(Tile *tiles[]){
		mBox.x += mVelX;
		
	if( ( mBox.x < 0 ) || ( mBox.x + DOT_WIDTH > LW ) || touchesWall( mBox, tiles ) ){
        mBox.x -= mVelX;
	}
		mBox.y += mVelY;
	if( (mBox.y < 0) || (mBox.y + DOT_HEIGHT > LH) || touchesWall(mBox, tiles)){
		mBox.y -= mVelY;	
	}
}
	void setCamera(SDL_Rect& camera){
	camera.x = ( mBox.x + DOT_WIDTH / 2 ) - SCW / 2;
	camera.y = ( mBox.y + DOT_HEIGHT / 2 ) - SCH / 2;
	if( camera.x < 0 ){ 
		camera.x = 0;
	}
	if( camera.y < 0 ){
		camera.y = 0;
	}
	if( camera.x > LW - camera.w ){
		camera.x = LW - camera.w;
	}
	if( camera.y > LH - camera.h ){
		camera.y = LH - camera.h;
	}
}
	void render(SDL_Rect& camera){
		gDotTexture.render( mBox.x - camera.x, mBox.y - camera.y );
		StartTexture.render(camera.x, camera.y);
	}
	
	private:
	SDL_Rect mBox;
	int mVelX, mVelY;
};
void init(){
	if(SDL_Init(SDL_INIT_VIDEO) < 0) cout << "SDL_Initialization error: " << SDL_GetError() << endl;
	else
	{
	if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			cout <<  "Warning: Linear texture filtering not enabled!" << endl;
		}
	
	
	
	gWindow = SDL_CreateWindow( "Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCW, SCH, SDL_WINDOW_SHOWN );
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
void loadMedia(Tile* tiles[]){	
	if(!gDotTexture.LoadFromFile("dot.bmp")){
		cout << "Failed to load dot texture! " << endl;
	}
	if(!gTileTexture.LoadFromFile("tiles.png")){
		cout << "Failed to load tile set texture! "  << endl;
	}
	if(!setTiles(tiles)){
		cout << "Failed to load tile set!\n" << endl;
	}	
}
void close(Tile* tiles[]){
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
void gamestart()
{
	Tile* tileSet[TOTAL_TILES];
	init();
	loadMedia(tileSet);
	bool quit = false;
	SDL_Event e;
	Characters CH1;
	SDL_Rect camera = {0,0,SCW,SCH};
	
	while (!quit){
		while(SDL_PollEvent(&e) !=0){
			if(e.type == SDL_QUIT)
				quit = true;
				if(e.type == SDL_KEYDOWN)
					StartTexture.free();
			CH1.handleEvent(e);
		}
		CH1.move(tileSet);
		CH1.setCamera(camera);
		
		SDL_SetRenderDrawColor(gRenderer,0xFF,0xFF,0xFF,0xFF);
		SDL_RenderClear(gRenderer);
		
		for (int i = 0; i < TOTAL_TILES; ++i)
		{
			tileSet[i]->render(camera);
		}
		
		CH1.render(camera);
		SDL_RenderPresent(gRenderer);
	}
	close(tileSet);
}

int main( int argc, char *argv[] ){
	gamestart();
	return 0;
}
