/**
 *	markov.h
 *	Last Modified: 1/13/19
 *	Author: Matthew Yu
 **/
#ifndef _MARKOV_H
#define _MARKOV_H

#include <vector>
#include <string>
#include <iostream>
#include "colorQuant.h"
#include "ofMain.h"
using namespace std;

typedef struct DataPoint{
    //ofColor color;
    int count;
}DataPoint;


class Markov{
	private:
        DataPoint ***table = NULL;
        int dim = 0;

	public:
        /**
         *  @funct: creates a stochastic matrix of n dimensions
         *  @param: int dimSize - size of each dimension (how many values)
         *      set to 3 dimensions - 2d input and 1d output
         **/
		Markov(int dimSize);

        /**
         *  @funct: populates the stochastic matrix
         *  @param: vector<string>& files - set of images to build matrix from
         *  @param: vector<Bucket>& palette - colors img pixels are sorted into
         *  @return: bool - true if successful populate
         **/
        bool Populate(string dir, vector<string>& files, vector<Bucket>& palette);

        /**
         *  @funct: populates the stochastic matrix
         *  @param: Point left - input color of left adjacent pixel
         *  @param: Point above - input color of the above adjacent pixel
         *  @param: vector<Bucket>& palette - colors img pixels are sorted into
         *  @return: Point - output color of current pixel
         **/
        Point Step(Point left, Point above, vector<Bucket>& palette);

        ~Markov();
};


#endif
