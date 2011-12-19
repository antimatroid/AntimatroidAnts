#ifndef MATRIX_H_
#define MATRIX_H_

#include <vector>
#include <iostream>

#include "Location.h"

template <class T>
struct Matrix
{
    /*
        Variables
    */
    int rows, cols;
    std::vector<T> matrix;

    /*
        Functions
    */
    //constructers
    Matrix<T>()
    {
        rows = cols = 0;
    };

    Matrix<T>(int Rows, int Cols)
    {
        rows = Rows;
        cols = Cols;
        matrix = std::vector<T>(rows*cols, T());
    };

    Matrix<T>(int Rows, int Cols, const T &startValue)
    {
        rows = Rows;
        cols = Cols;
        matrix = std::vector<T>(rows*cols, startValue);
    };

    void reset()
    {
        matrix = std::vector<T>(rows*cols, T());
    };

    void reset(const T &startValue)
    {
        matrix = std::vector<T>(rows*cols, startValue);
    };

    //index functions
    inline T& operator()(int r, int c) const
    {
        return (T&) matrix[r*cols + c];
    };

    /*inline T* operator[](int r)
    {
        return (T*) &matrix[r*cols];
    };*/

    inline T& operator[](const Location &loc) const
    {
        return (T&) matrix[loc.row*cols + loc.col];
    };
};

inline std::ostream& operator<<(std::ostream &os, const Matrix<uint8_t> &matrix)
{
    for(int r=0; r<matrix.rows; r++)
    {
        for(int c=0; c<matrix.cols; c++)
        {
            if(matrix(r,c) < 10)
                os << " ";
            os << std::min(int(matrix(r, c)), 99) << " ";
        }
        os << std::endl;
    }

    return os;
};

inline std::ostream& operator<<(std::ostream &os, const Matrix<int> &matrix)
{
    for(int r=0; r<matrix.rows; r++)
    {
        for(int c=0; c<matrix.cols; c++)
        {
            if(matrix(r,c) < 10)
                os << " ";
            os << std::min(matrix(r, c), 99) << " ";
        }
        os << std::endl;
    }

    return os;
};

template <class T>
std::ostream& operator<<(std::ostream &os, const Matrix<T> &matrix)
{
    for(int r=0; r<matrix.rows; r++)
    {
        for(int c=0; c<matrix.cols; c++)
            os << matrix(r, c);
        os << std::endl;
    }

    return os;
};

#endif //MATRIX_H_
