#pragma once
#include <BWAPI/Unit.h>
#include <BWAPI.h>
#include <BWTA.h> //la till eftersom den av n�gon anledning inte f�ljde med BWAPI.h d�r den �ven inkluderas
#include <stack>

using namespace BWAPI;
using namespace std;

enum MarineStates
{
	FindChokePoint,
	OscarMike,  //on the move :)
	KeepWatch,
	StrikeAtBase,
	Combat,
	CoverAndMedic
};

enum MarineEvents
{
	Idle,
	ConquerTheEnemy
};

class MarineAI
{
private:
	BWTA::Region* home;			//to know what to guard
	BWTA::Region* enemy_base;	//and what to strike at
	TilePosition chokePoint;
	Unit target;

	Unit marinePtr;
	int currentState;
	int previousState;
	bool activeEvent;
	int currentEvent;
	bool inCombat;
	bool inMotion;
	//methods
	void findChokePoint();
	void lookForEnemies();
	void attack();
	void takeCover();
public:
	int getPreviousState() const;
	void setState(int state); //exists for initial purposes
	void setEvent(int event); //this event-switch only occurs at endgame to strike at base, Marines are Idle till then
	void update(); //this is where state machine logic is called
	int getCurrentState() const;
	//Put constructors and deconstructors at the bottom otherwise other public memebers might not appear (unknown bug)
	MarineAI() {}
	MarineAI(Unit unit, BWTA::Region* home, BWTA::Region* enemy_base);
	~MarineAI() {}

};
