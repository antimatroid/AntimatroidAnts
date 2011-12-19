#ifndef BOT_H_
#define BOT_H_

#include <functional>
#include <map>

#include "State.h"
#include "SearchStructures.h"

struct Bot
{
    State state;

    Bot();

    void playGame();    //plays a single game of Ants

    void makeMoves();   //makes moves for a single turn

    void performBattle(const std::vector<Location> &ourAnts, const std::vector<Location> &theirAnts);
    void battleAnts(double maxTimeProportion);

    void collectEnemyAroundHill(const Location &eLoc, double maxTimeProportion);
    void protectFriendHills(double maxTimeProportion);
    void saveHills(double maxTimeProportion);
    void invadeEnemyHills(double maxTimeProportion);

    void collectFood(double maxTimeProportion);

    void exploreTheMap(double maxTimeProportion);

    void collectPossibleEnemiesBorder(double maxTimeProportion);
    void collectBattleBorder(double maxTimeProportion);
    void collectNotRecentlySeenBorder(double maxTimeProportion);

    void exploreRemainingAnts(double maxTimeProportion);

    void endTurn();     //indicates to the engine that it has made its moves
};

#endif //BOT_H_
