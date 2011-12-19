#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <unordered_set>

#include "Timer.h"
#include "Grid.h"
#include "Location.h"
#include "Move.h"
#include "Outcome.h"

//State struct
struct State
{
    /*
        Variables
    */
    int noVisibleSquares;
    bool offensive;

    int rows, cols,
        turn, turns,
        attackradius, harvestradius, viewradius;

    double loadtime, turntime;

    Grid grid;
    int ourNoAnts;
    std::unordered_set<Location, LocationIndex> unmovedAnts, moveableAnts,
                       uncollectedEnemies, enemyAnts,
                       myHills, oldMyHills,
                       enemyHills, oldEnemyHills,
                       uncollectedFood, food, oldFood,
                       unseenBorder, possibleEnemyBorder, notVisibleBorder, battleBorder, enemyHillBorder;

    bool closestFriendHillInfoChanged, closestEnemyHillInfoChanged;
    Matrix<int> closestHill, closestFriendHill, closestEnemyHill;
    Matrix<uint8_t> adjUnmovedFriends, adjUnmovedEnemies;

    Timer timer;

    /*
        Functions
    */
    State();
    ~State();

    void setup();
    void reset();

    bool checkTime(double maxTimeProportion);

    void makeMove(const Move &move);
    void makeMoves(const std::vector<Move> &moves);


    Location getLocation(const Location &startLoc, int direction);
    Location getAbsLocation(const Location &startLoc, int direction);
    Location getTranslateLocation(const Location &startLoc, const Location &translateAmount);

    std::vector<Move> getOurDefensiveMovesMinimisingBattleEnemies(const std::vector<Location> &ourAnts);
    std::vector<Move> getOurNeutralMovesMinimisingPositiveBattleEnemies(const std::vector<Location> &ourAnts);
    std::vector<Move> getOurNeutralMovesMaximisingBattleEnemies(const std::vector<Location> &ourAnts);
    std::vector<Move> getOurSpecialOffensiveMoves(const std::vector<Location> &ourAnts);
    std::vector<Move> getOurOffensiveMovesMinimisingPositiveBattleEnemies(const std::vector<Location> &ourAnts);
    std::vector<Move> getOurOffensiveMovesMaximisingBattleEnemies(const std::vector<Location> &ourAnts);

    std::vector<Move> getTheirOffensiveMovesMinimisingPositiveBattleEnemies(const std::vector<Location> &theirAnts, const std::vector<Move> &ourMoves);
    std::vector<Move> getTheirOffensiveMovesMaximisingBattleEnemies(const std::vector<Location> &theirAnts, const std::vector<Move> &ourMoves);
    std::vector<Move> getTheirNeutralMovesMinimisingPositiveBattleEnemies(const std::vector<Location> &theirAnts, const std::vector<Move> &ourMoves);
    std::vector<Move> getTheirNeutralMovesMaximisingBattleEnemies(const std::vector<Location> &theirAnts, const std::vector<Move> &ourMoves);

    std::vector<Move> getTheirDefensiveMoves(const std::vector<Location> &theirAnts);

    void updateVisionInformation();
    void processOldInformation();
    void collectFoodLocation(const Location &fLoc);
    void updateAdjFoodInformation(const Location &fLoc);
    void updateAdjFoodInformation();
    void updateClosestHillInformation();
    void updateClosestFriendAndEnemyInfo();

    void updatePossibleEnemyBorder();
    void updateBattleBorder();
    void updateNotVisibleBorder();
    void updateEnemyHillBorder();

    void updatePossibleEnemyInfo();
    void updateAdjToEnemyInfo();

    void calculatePossibleBattleInfo();

    void calculateSoonestBattleInfo();
    void updateAdjUnmovedAntInfo();

    void bombVisionInformation(const Location &cLoc, std::unordered_set<Location, LocationIndex> &uBorder);

    Outcome evaluate(std::vector<Move> &ourMoves, std::vector<Move> &theirMoves);

};

std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);

#endif //STATE_H_
