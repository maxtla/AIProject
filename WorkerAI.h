#pragma once
#include <BWAPI/Unit.h>
#include <BWAPI.h>
#include <stack>

using namespace BWAPI;
using namespace std;

enum WorkerStates
{
	LookForMinerals,
	LookForGas,
	GatheringMinerals,
	GatheringGas,
	Building
};

enum BuildEvents
{
	buildSupplyDepot,
	buildBarracks,
	buildRefinery,
	buildAcademy,
	buildFactory,
	buildCommandCenter
};

class WorkerAI
{
private:
	Unit workerPtr;
	int currentState;
	int previousState;
	bool activeEvent;
	int currentEvent;
	bool isBuilding;
	//methods
	void findNearestMineral();
	void findNearestRefinery();
	void build();
	void _buildDepot();
	void _buildRefinery();
	void _buildBarracks();
	//smaller-task methods/ help methods
	TilePosition findBuildLocation(UnitType type, TilePosition t_pos); //New //also added some to the algorithm, see method for more
public:
	int getPreviousState() const;
	void setState(int state); //exists for initial purposes
	void setEvent(int event); //this event must be a build event
	void update(); //this is where state machine logic is called
	int getCurrentState() const;
	//Put constructors and deconstructors at the bottom otherwise other public memebers might not appear (unknown bug)
	WorkerAI() {}
	WorkerAI(Unit unit);
	~WorkerAI() {}

};