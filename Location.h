#ifndef LOCATION_H_
#define LOCATION_H_

#include <iostream>

#define MAX_COLS 500

struct Location
{
    int row, col;

    Location()
    {
        row = col = 0;
    };

    Location(int r, int c)
    {
        row = r;
        col = c;
    };
};

struct LocationIndex
{
    int operator()(const Location &loc) const
    {
        return loc.row * MAX_COLS + loc.col;
    };
};

inline Location operator-(const Location &loc1, const Location &loc2)
{
    return Location(loc1.row-loc2.row, loc1.col-loc2.col);
};

inline bool operator==(const Location &loc1, const Location &loc2)
{
    if(loc1.row != loc2.row || loc1.col != loc2.col)
        return 0;
    return 1;
};

inline bool operator!=(const Location &loc1, const Location &loc2)
{
    if(loc1.row != loc2.row || loc1.col != loc2.col)
        return 1;
    return 0;
};

inline bool operator<(const Location &loc1, const Location &loc2)
{
    if(loc1.row < loc2.row)
        return 1;
    else if(loc1.row > loc2.row)
        return 0;

    if(loc1.col < loc2.col)
        return 1;
    else if(loc1.col > loc2.col)
        return 0;

    return 0;
};


inline std::ostream& operator<<(std::ostream &os, const Location &loc)
{
    os << loc.row << " " << loc.col;
    return os;
};

#endif //LOCATION_H_
