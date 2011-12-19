#ifndef SEARCHSTRUCTURES_H_
#define SEARCHSTRUCTURES_H_

#include <vector>

#include "Location.h"

struct FoodSearchItem
{
    int cPathLength, noFoodSquares, battleSlack;
    Location cLoc, tLoc;

    //constructers
    FoodSearchItem()
    {
        cPathLength = noFoodSquares = 0;
        battleSlack = 9999;
    };

    FoodSearchItem(const Location &TLoc, int NoFoodSquares, const Location &CLoc, int CPathLength, int BattleSlack)
    {
        tLoc = TLoc;
        noFoodSquares = NoFoodSquares;
        cLoc = CLoc;
        cPathLength = CPathLength;
        battleSlack = BattleSlack;
    };
};

inline bool operator>(const FoodSearchItem &item1, const FoodSearchItem &item2)
{
    //prioritises shortest current paths
    if(item1.cPathLength > item2.cPathLength)
        return 1;
    else if(item1.cPathLength < item2.cPathLength)
        return 0;

    //prioritises paths that collect the most food
    if(item1.noFoodSquares < item2.noFoodSquares)
        return 1;
    else if(item1.noFoodSquares > item2.noFoodSquares)
        return 0;

    return 0;
};


struct ExploreSearchItem
{
    int cPathLength, battleSlack;
    Location cLoc, uLoc;

    //constructers
    ExploreSearchItem()
    {
        cPathLength = battleSlack = 9999;
    };

    ExploreSearchItem(const Location &CLoc, int CPathLength, const Location &ULoc, int BattleSlack)
    {
        cLoc = CLoc;
        cPathLength = CPathLength;
        uLoc = ULoc;
        battleSlack = BattleSlack;
    };
};

struct BorderSearchItem
{
    bool pathBlocked;
    int cPathLength;
    Location cLoc, tLoc;

    //constructers
    BorderSearchItem()
    {
        cPathLength = 9999;
    };

    BorderSearchItem(const Location &CLoc, int CPathLength, bool PathBlocked, const Location &TLoc)
    {
        cLoc = CLoc;
        cPathLength = CPathLength;
        tLoc = TLoc;
        pathBlocked = PathBlocked;
    };
};

inline bool operator>(const BorderSearchItem &item1, const BorderSearchItem &item2)
{
    //prioritises locations closer to the enemy
    if(item1.cPathLength > item2.cPathLength)
        return 1;
    else if(item1.cPathLength < item2.cPathLength)
        return 0;

    //prioritises locations closer to the enemies shortest path
    if(item1.pathBlocked && !item2.pathBlocked)
        return 1;
    else if(!item1.pathBlocked && item2.pathBlocked)
        return 0;

    return 0;
};

struct InvadeSearchItem
{
    int cPathLength, battleSlack;
    Location cLoc;

    //constructers
    InvadeSearchItem()
    {
        cPathLength = battleSlack = 9999;
    };

    InvadeSearchItem(const Location &CLoc, int CPathLength, int BattleSlack)
    {
        cLoc = CLoc;
        cPathLength = CPathLength;
        battleSlack = BattleSlack;
    };
};

struct EnemySearchItem
{
    int cPathLength, closestHill;
    Location cLoc;

    //constructers
    EnemySearchItem()
    {
        cPathLength = closestHill = 9999;
    };

    EnemySearchItem(const Location &CLoc, int CPathLength, int ClosestHill)
    {
        cLoc = CLoc;
        cPathLength = CPathLength;
        closestHill = ClosestHill;
    };
};

inline bool operator>(const EnemySearchItem &item1, const EnemySearchItem &item2)
{
    //prioritises locations closer to the enemy
    if(item1.cPathLength > item2.cPathLength)
        return 1;
    else if(item1.cPathLength < item2.cPathLength)
        return 0;

    //prioritises locations closer to the enemies shortest path
    if(item1.closestHill > item2.closestHill)
        return 1;
    else if(item1.closestHill < item2.closestHill)
        return 0;

    return 0;
};


#endif //SEARCHSTRUCTURES_H_
