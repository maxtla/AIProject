#pragma once


#include <BWTA.h>
#include <windows.h>
#include <vector>
#include "../WorkerAI.h"
#include <queue>

using namespace BWAPI;
using namespace BWTA;
using namespace std;

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
	createFirstDepot,
	createWorker,
	createRefinery,
	createSecondDepot,
	createFourWorkers, //these workers exclusively gather gas
	createBarracks,
	createTenMarines,
	createAcademy,
	createThreeMedics,
	createFactory,
	createMachineShop,
	createThreeTanks,
	createSiegeModeUpgrade,
	createCommandCenter,
	createFiveWorkers, //these workers are created from the new command center
	attackEnemyBase,
	Last
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
	vector<WorkerAI> workerAIContainer;
	queue<int> GameStates;

	//private methods
	void buildDepot();
	void createWorker();
	void buildRefinery();
};
