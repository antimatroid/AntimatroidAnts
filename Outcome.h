#ifndef OUTCOME_H_
#define OUTCOME_H_

#include <iostream>
#include <cstdlib>
#include <cmath>

struct Outcome
{
    bool defensive, enemyRazedFriendHill, enemyRazedEnemyHill, friendRazedEnemyHill;
    double friendDeaths, enemyDeaths;

    Outcome()
    {
        defensive = 1;
        enemyRazedFriendHill = enemyRazedEnemyHill = friendRazedEnemyHill = 0;
        friendDeaths = enemyDeaths = 0;
    };

    void setBest()
    {
        friendRazedEnemyHill = 1;
        enemyRazedFriendHill = enemyRazedEnemyHill = 0;
        friendDeaths = 0;
        enemyDeaths = 9999;
    };

    void setWorst()
    {
        friendRazedEnemyHill = 0;
        enemyRazedFriendHill = enemyRazedEnemyHill = 1;
        friendDeaths = 9999;
        enemyDeaths = 0;
    };

};

//order operation
inline bool operator<(const Outcome &outcome1, const Outcome &outcome2)
{
    if(outcome1.enemyRazedFriendHill && !outcome2.enemyRazedFriendHill)
        return 1;
    else if(!outcome1.enemyRazedFriendHill && outcome2.enemyRazedFriendHill)
        return 0;

    if(!outcome1.friendRazedEnemyHill && outcome2.friendRazedEnemyHill)
        return 1;
    else if(outcome1.friendRazedEnemyHill && !outcome2.friendRazedEnemyHill)
        return 0;

    if(outcome1.enemyRazedEnemyHill && !outcome2.enemyRazedEnemyHill)
        return 1;
    else if(!outcome1.enemyRazedEnemyHill && outcome2.enemyRazedEnemyHill)
        return 0;

    if(outcome1.enemyDeaths-outcome1.friendDeaths < outcome2.enemyDeaths-outcome2.friendDeaths)
        return 1;
    else if(outcome1.enemyDeaths-outcome1.friendDeaths > outcome2.enemyDeaths-outcome2.friendDeaths)
        return 0;

    if(outcome1.defensive)
    {
        if(outcome1.friendDeaths > outcome2.friendDeaths)
            return 1;
        else if(outcome1.friendDeaths < outcome2.friendDeaths)
            return 0;
    }
    else
    {
        if(outcome1.enemyDeaths < outcome2.enemyDeaths)
            return 1;
        else if(outcome1.enemyDeaths > outcome2.enemyDeaths)
            return 0;
    }

    return 0;
}

inline std::ostream& operator<<(std::ostream &os, const Outcome &outcome)
{
    os << "friendDeaths: " << outcome.friendDeaths << " enemyDeaths: " << outcome.enemyDeaths << std::endl;

    return os;
};

#endif //OUTCOME_H_

