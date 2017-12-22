#pragma once


#include <BWTA.h>
#include <windows.h>
#include <vector>
#include "../WorkerManagerAI.h"
#include "../GroundTroopsManagerAI.h"
#include "../MarineAI.h"
#include <queue>

#define UNIT_TIME_OUT_MS 10000 //10 sec

using namespace BWAPI;
using namespace BWTA;
using namespace std;

extern int frameUpdateTime;
extern bool buildProcess;
extern bool eventProcessing;
extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

//this enumerator defines the games strategy states, by design the lowest value on a state has highest priority
//if you wish to change the strategy you have to change the order of the states, do not assign it a value
//we need to iterate through the enum too fill the min heap, I want to avoid long switch statement
enum GameStrategy
{
	createSupplyDepot,
	createBarracks,
	createMarine,
	createWorker,
	createRefinery,
	createAcademy,
	createMedic,
	createFactory,
	createMachineShop,
	createSiegeTank,
	researchSiegeMode,
	attack
};



class ExampleAIModule : public BWAPI::AIModule
{
public:
	//Methods inherited from BWAPI:AIModule
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player player, std::string text);
	virtual void onPlayerLeft(BWAPI::Player player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit unit);
	virtual void onUnitEvade(BWAPI::Unit unit);
	virtual void onUnitShow(BWAPI::Unit unit);
	virtual void onUnitHide(BWAPI::Unit unit);
	virtual void onUnitCreate(BWAPI::Unit unit);
	virtual void onUnitDestroy(BWAPI::Unit unit);
	virtual void onUnitMorph(BWAPI::Unit unit);
	virtual void onUnitRenegade(BWAPI::Unit unit);
	virtual void onSaveGame(std::string gameName);
	virtual void onUnitComplete(BWAPI::Unit unit);

	//Own methods
	void drawStats();
	void drawTerrainData();
	void showPlayers();
	void showForces();
	Position findGuardPoint();

	//Own private members, AI containers
private:
	WorkerManagerAI *wManager;
	GroundTroopsManagerAI *aManager; //a for army
	vector<MarineAI> marineAIContainer;
	int currentState;
	//private methods
	int counter;
	bool createWorker();
	bool createMarine();
	bool createMedic();
	bool createMachineShop();
	bool researchSiegeMode();
	bool createSiegeTank();
	void update();
};