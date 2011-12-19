#ifndef MOVE_H_
#define MOVE_H_

#include "Location.h"

//Move struct
struct Move
{
    bool reuseAnt;
    int pathLength;
    Location aLoc, mLoc, cLoc;
    char direction;

    Move()
    {
        pathLength = -1;
    };

    Move(const Location &ALoc, const Location &MLoc, char Direction)
    {
        reuseAnt = 0;
        pathLength = 0;

        aLoc = ALoc;
        mLoc = MLoc;
        cLoc = MLoc;

        direction = Direction;
    };

    Move(const Location &ALoc, const Location &MLoc, char Direction, const Location &CLoc, int PathLength, bool ReuseAnt)
    {
        reuseAnt = ReuseAnt;
        pathLength = PathLength;

        aLoc = ALoc;
        mLoc = MLoc;
        cLoc = CLoc;

        direction = Direction;
    };
};

inline std::ostream& operator<<(std::ostream &os, const Move &move)
{
    os << "Move: " << move.aLoc << ", " << move.mLoc << ", " << move.direction << std::endl;
    return os;
};

#endif //MOVE_H_
