/**
 *  markov.cpp
 *  VERSION 3
 *  Last Modified: 1/28/19
 *  Author: Matthew Yu
 */
#include "markov.h"

Markov::Markov(int dimSize){
    dim = dimSize;
    //table of dim rows, which have dim cols, which have dim layers
    table = new DataPoint**[dim];
    for(int i = 0; i < dim; i++){
        table[i] = new DataPoint*[dim];
        for(int j = 0; j < dim; j++){
            table[i][j] = new DataPoint[dim];
            table[i][j][0].count = 1;
            for(int k = 1; k < dim; k++)
                table[i][j][k].count = 0;
        }
    }
}

bool Markov::Populate(string dir, vector<string>& files, vector<Bucket>& palette){
    string filePath;
    for(unsigned int i = 0; i < files.size(); i++){
        filePath = dir + '/' + files[i];
        ofImage img;
        img.load(filePath);
        img.setImageType(OF_IMAGE_COLOR);
        ofPixels pixelData = img.getPixels();

        int width = img.getWidth();
        int height = img.getHeight();

        int x = 1, y = 1;   //2nd col of 2nd row

        Point left, above, center;
        int idxR = 0, idxC = 0, idxL = 0;   //row, column, layer

        //get next pixel from img
        while(x < width && y < height){
            left[0] = pixelData[x - 3 + y*width];
            left[1] = pixelData[x - 2 + y*width];
            left[2] = pixelData[x - 1 + y*width];
            above[0] = pixelData[x + (y-1)*width];
            above[1] = pixelData[x + (y-1)*width + 1];
            above[2] = pixelData[x + (y-1)*width + 2];
            center[0] = pixelData[x + y*width];
            center[1] = pixelData[x + y*width + 1];
            center[2] = pixelData[x + y*width + 2];

            //get indexes for each pixel's color
            //index is the bucketIdx of the bucket that contains the point
            bool boolR = false, boolC = false, boolL = false;
            for(int bucketIdx = 0; bucketIdx < dim; bucketIdx++){
                if(palette[bucketIdx].getBoundingArea().contains(left)){
                    idxR = bucketIdx;
                    boolR = true;
                }
                if(palette[bucketIdx].getBoundingArea().contains(above)){
                    idxC = bucketIdx;
                    boolC = true;
                }
                if(palette[bucketIdx].getBoundingArea().contains(center)){
                    idxL = bucketIdx;
                    boolL = true;
                }
                if(boolR && boolC && boolL)
                    break;
            }
            table[idxR][idxC][idxL].count++;
            x++;
            if(x == width){
                x = 0;
                y++;
            }
        }

        // cout << "l|t|c:[oIdx]" << endl;
        // for(int i = 0; i < dim; i++){
        //     for(int j = 0; j < dim; j++){
        //         for(int k = 0; k < dim; k++){
        //             cout << i << "|" << j << "|" << k << ":[" <<
        //                 table[i][j][k].count << "]" << "\t";
        //         }
        //         cout << endl;
        //     }
        //     cout << endl;
        // }
        return true;
    }
    return false;
}

Point Markov::Step(Point left, Point above, vector<Bucket>& palette){
    unsigned int totalCt = 0;

    //given two points, determine idxR, idxC
    int idxR = 0, idxC = 0;
    bool boolR = false, boolC = false;
    for(unsigned int bucketIdx = 0; bucketIdx < palette.size(); bucketIdx++){
        if(palette[bucketIdx].getBoundingArea().contains(left)){
            idxR = bucketIdx;
            boolR = true;
        }
        if(palette[bucketIdx].getBoundingArea().contains(above)){
            idxC = bucketIdx;
            boolC = true;
        }
        if(boolR && boolC)
            break;
    }

    //calculate maximum roll from possible odds
    for(unsigned int i = 0; i < palette.size(); i++){
        totalCt += table[idxR][idxC][i].count;
    }

    //prng pick a child idx as next state
    int roll = round(ofRandom(1, totalCt));

    unsigned int idx = 0;
    while(roll > 0 && idx < palette.size()){
        roll -= table[idxR][idxC][idx].count;
        idx++;
    }
    idx--;

    //output color of next state
    Point output = palette[idx].getColor();
    return output;
}

Markov::~Markov(){
    for(int i = 0; i < dim; i++){
        for(int j = 0; j < dim; j++)
            delete[] table[i][j];
        delete[] table[i];
    }
    delete[] table;
}
