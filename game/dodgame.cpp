#include "core/util.h"
#include "core/graphics.h"
#include "core/Shader.h"
#include "core/BadMesh.h"
#include "core/FontRender.h"
#include "core/core.h"

#include "GameTextures.h"
#include "GameMeshes.h"

#include "GameConsts.h"

#include <list>

// asset handles
BadMesh *cube, *dude;
BadMesh *smallertile;

void UpdateLogic( double delta );
void DrawHUD();
void DrawWorld();

void GameUpdate() {
	static double previousTime = glfwGetTime();
	double drawStart = glfwGetTime();
	double delta = drawStart - previousTime;
	previousTime = drawStart;
	if( delta > 0.2f ) delta = 0.2f;

	UpdateLogic( delta );
	DrawHUD();
	DrawWorld();
}
void GameInit() {
	GameTextures::Init();
	GameMeshes::Init();

	smallertile = new BadMesh();
	*smallertile = *GameMeshes::Get( "floor" );
	Mat44 scaleDown = gIdentityMat;
	scaleDown.x.x = 0.9f;
	scaleDown.z.z = 0.9f;
	smallertile->ApplyTransform( scaleDown );

	dude = GameMeshes::Get( "quadpeep" );

	cube = new BadMesh();
	cube->SetAsCube();
	cube->UVsFromBB();

	SetGameTitle( "DOD game" );
}
void GameShutdown() {
}

// game state
int gTileState[3*3];
typedef std::pair<int,float> Growing;
typedef std::list<Growing> GrowingList;
GrowingList gGrowingList;
Vec2 gDudePos;
Vec2 gDudeFacing(0.0f,-1.0f);
Vec2 gDudeDest;
int haveSeeds = 5;

void UpdateLogic( double delta ) {

	Vec2 d = gDudeDest - gDudePos;
	bool moving = d != Vec2(0.0f,0.0f);
	if( moving ) {
		bool aboutTurn = dot( gDudeFacing, d ) < 0.0f;
		float offBy = cross( gDudeFacing, d );
		bool turning = aboutTurn || offBy != 0.0f;
		if( turning ) {
			Mat22 turn;
			if( offBy > 0.0f ) {
				turn.Rot( TURN_SPEED * delta );
			} else {
				turn.Rot( -TURN_SPEED * delta );
			}
			Vec2 newFace = turn * gDudeFacing;
			float newOff = cross( newFace, d );
			if( !aboutTurn && newOff * offBy <= 0.0f ) {
				gDudeFacing = d.normalized();	
			} else {
				gDudeFacing = newFace;
			}
		} else {
			const float dudeSpeed = 1.5f * delta;
			d.x = clamp( d.x, -dudeSpeed, dudeSpeed );
			d.y = clamp( d.y, -dudeSpeed, dudeSpeed );
			gDudePos += d;
		}
	} else {
		float mx=0.0f,my=0.0f;
		if ( glfwGetKey( 'W' ) == GLFW_PRESS ) my += 1.0f;
		if ( glfwGetKey( 'S' ) == GLFW_PRESS ) my -= 1.0f;
		if ( glfwGetKey( 'A' ) == GLFW_PRESS ) mx += 1.0f;
		if ( glfwGetKey( 'D' ) == GLFW_PRESS ) mx -= 1.0f;
		if ( glfwGetKey( GLFW_KEY_UP ) == GLFW_PRESS ) my += 1.0f;
		if ( glfwGetKey( GLFW_KEY_DOWN ) == GLFW_PRESS ) my -= 1.0f;
		if ( glfwGetKey( GLFW_KEY_LEFT ) == GLFW_PRESS ) mx += 1.0f;
		if ( glfwGetKey( GLFW_KEY_RIGHT ) == GLFW_PRESS ) mx -= 1.0f;

		if( mx != 0.0f || my != 0.0f ) {
			if( mx != 0.0f && my != 0.0f ) {
			} else {
				if( gDudePos.x < 1.0f && mx > 0.0f ) { gDudeDest = gDudePos + Vec2(1.0f,0.0f); }
				if( gDudePos.x > -1.0f && mx < 0.0f ) { gDudeDest = gDudePos - Vec2(1.0f,0.0f); }
				if( gDudePos.y < 1.0f && my > 0.0f ) { gDudeDest = gDudePos + Vec2(0.0f,1.0f); }
				if( gDudePos.y > -1.0f && my < 0.0f ) { gDudeDest = gDudePos - Vec2(0.0f,1.0f); }
			}
		}
	}

	bool action = false;
	if ( glfwGetKey( GLFW_KEY_SPACE ) == GLFW_PRESS ) action = true;

	static bool actionLast = false;
	if( actionLast ) {
		actionLast = action;
		action = false;
	} else {
		actionLast = action;
	}

	if( action ) {
		Vec3 gDudeRelative = gDudePos - Vec2( -1.0f, -1.0f );
		float x = floorf( gDudeRelative.x + 0.5f );
		float y = floorf( gDudeRelative.y + 0.5f );
		if( x >= 0 && x < 3 && y >= 0 && y < 3 ) {
			int cell = (int)x + 3 * (int)y;
			if( gTileState[cell] == 0 ) {
				gTileState[cell] = 1;
				Log( 1, "ploughed the land at %i (%.2f,%.2f)\n", cell, x, y );
			} else if( gTileState[cell] == 1 && haveSeeds ) {
				gTileState[cell] = 2;
				gGrowingList.push_back( Growing( cell, 0.0f ) );
				haveSeeds -= 1;
				Log( 1, "planted an owl at %i (%.2f,%.2f)\n", cell, x, y );
			} else if( gTileState[cell] == 3 ) {
				gTileState[cell] = 0;
				haveSeeds += 3;
				Log( 1, "harvested an owl to get three owl seeds.\n" );
			}
		}
	}

	for( GrowingList::iterator i = gGrowingList.begin(); i != gGrowingList.end(); ) {
		i->second += GROWTH_RATE * delta;
		if( i->second >= 1.0f ) {
			gTileState[i->first] = 3;
			i = gGrowingList.erase( i );
		} else {
			++i;
		}
	}
}

void DrawHUD() {
	DefaultOrtho();
	SetCamera(gIdentityMat);
	SetModel(gIdentityMat);
	glDepthFunc(GL_LEQUAL);
	ClearScreen( 0.3f, 0.3f, 0.3f );
	SetTexture( "sword", 0 );
	DrawSquare( 16, 16, 32, 32, 0xFFFFFFFF );


	DefaultOrtho();
	SetCamera( gIdentityMat );
	SetModel( gIdentityMat );
	glDepthFunc(GL_LEQUAL);

	Mat44 modelMat;
	modelMat = Translation(Vec3( 0.0f,0.0f,0.0f ));
	modelMat.Scale(1.0f);
	char buffer[128];
	sprintf( buffer, "Your Farm .. Seeds: %i", haveSeeds );
	FontPrint( modelMat, buffer);
}

void DrawWorld() {
	Mat44 modelMat;
	Vec3 from, to;
	DefaultProjection();
	extern float g_fGameTime;
	glUniform1f(GLShader::Current()->timeLocation, g_fGameTime);
	from = Vec3( 0.1f, 4.0f, -10.0f );
	to = Vec3( 0.0f, 0.0f, 0.0f );

	Mat44 look = Mat44LookAt( from, to, gYVec4 );
	SetCamera(look);

	int tile = 0;
	for( float tz = -2.0f; tz <= 2.0f; tz += 2.0f ) {
		for( float tx = -2.0f; tx <= 2.0f; tx += 2.0f ) {
			modelMat = Translation(Vec3( tx, 0.0, tz ));
			SetModel( modelMat );
			switch(gTileState[tile]) {
				case 0: SetTexture( "earth", 0 ); break;
				case 1:
				case 2:
				case 3: SetTexture( "pick", 0 ); break;
			}
			smallertile->DrawTriangles();
			if(gTileState[tile]==3) {
				modelMat = Translation(Vec3( tx, 0.0, tz ));
				modelMat.Scale( 0.5f );
				SetModel( modelMat );
				SetTexture( "owl", 0 );
				dude->DrawTriangles();
			}
			tile += 1;
		}
	}

	for( GrowingList::iterator i = gGrowingList.begin(); i != gGrowingList.end(); ++i ) {
		float tx = (i->first%3) * 2.0f - 2.0f;
		float tz = (i->first/3) * 2.0f - 2.0f;
		modelMat = Translation(Vec3( tx, 0.0, tz ));
		modelMat.Scale( i->second * 0.5f );
		SetModel( modelMat );
		SetTexture( "owl", 0 );
		dude->DrawTriangles();
	}

	SetTexture( "guy", 0 );
	modelMat = Translation( Vec3( gDudePos.x * 2.0f, 0.0f, gDudePos.y * 2.0f ) );
	Vec2 aim( -gDudeFacing.y, gDudeFacing.x );
	modelMat.x.x = aim.x;
	modelMat.x.z = aim.y;
	modelMat.z.x = -aim.y;
	modelMat.z.z = aim.x;
	SetModel( modelMat );
	dude->DrawTriangles();
}
