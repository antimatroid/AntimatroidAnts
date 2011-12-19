#ifndef SQUARE_H_
#define SQUARE_H_

#include <vector>
#include "Location.h"

struct Square
{
    bool isWater, isFood, taken, possibleEnemy, adjToEnemy;

    int ant, available, hill, lastSeen, adjFood,
        closestFriend, closestEnemy,
        soonestFriendBattle, soonestEnemyBattle;

    std::vector<Location> landNeighbours, battleNeighbours, possibleNeighbours;
    std::vector<char> landDirections, oppositeLandDirections;

    int possibleBattleEnemies, possibleBattleFriends;
    int possibleBattleEnemyLocations, possibleBattleFriendLocations;

    Square()
    {
        isWater = isFood = taken = adjToEnemy = 0;
        possibleEnemy = 1;
        ant = hill = available = lastSeen = -1;
        adjFood = 0;

        closestFriend = closestEnemy = 9999;
        soonestEnemyBattle = soonestFriendBattle = 9999;
        possibleBattleEnemies = possibleBattleFriends = 0;
        possibleBattleEnemyLocations = possibleBattleFriendLocations = 0;

        landDirections.push_back('N');
        landDirections.push_back('E');
        landDirections.push_back('S');
        landDirections.push_back('W');

        oppositeLandDirections.push_back('S');
        oppositeLandDirections.push_back('W');
        oppositeLandDirections.push_back('N');
        oppositeLandDirections.push_back('E');
    };

    //resets the information for the square except water information
    void reset()
    {
        isFood = taken = adjToEnemy = 0;
        ant = available = -1;
        adjFood = 0;

        closestFriend = closestEnemy = 9999;
        soonestEnemyBattle = soonestFriendBattle = 9999;
        possibleBattleEnemies = possibleBattleFriends = 0;
        possibleBattleEnemyLocations = possibleBattleFriendLocations = 0;
    };
};

#endif //SQUARE_H_
