/**
 *	colorQuant.cpp
 *  for color quantization of images,
 *      using 'buckets' (bounding boxes) and median cut.
 *	Last Modified: 1/28/19
 *      1/20 - updated find kth largest, added swap, integrated median cut with
 *          consistently improved results
 *      1/23 - updated insert, getIdx (for remove) to build and maintain ordered
 *          lists of points with binary searches - 91% speed improvement.
 *      1/28 - updated getColor, added setColor to optimize returning mean color
 *          of buckets - >99% speed improvement.
 *	Author: Matthew Yu
 **/
#include "colorQuant.h"
using namespace std;
#include <iostream>

bool Point::operator ==(const Point& rhs) const{
    if(x == rhs.x && y == rhs.y && z == rhs.z)
        return true;
    else
        return false;
}

bool Point::operator >(const Point& rhs) const{
    if(x > rhs.x)
        return true;
    if(x == rhs.x){
        if(y > rhs.y)
            return true;
        if(y == rhs.y){
            if(z > rhs.z)
                return true;
        }
    }
    return false;
}

bool Point::operator <(const Point& rhs) const{
    return !((*this) == rhs || (*this) > rhs);
}

int& Point::operator[](const int idx){
    switch(idx) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw "something";
    }
}

void Point::printPoint(){
    cout << x << "," << y << "," << z << endl;
}

/*----------------------------------------------------------------------------*/
BoundBox::BoundBox(){
    pos = {0, 0, 0};
    dims = {0, 0, 0};
}

BoundBox::BoundBox(Point position, Point dimensions){
    pos = position;
    dims = dimensions;
}

bool BoundBox::contains(Point p){
    return
       (p.x >= pos.x && p.x < pos.x + dims.x &&
        p.y >= pos.y && p.y < pos.y + dims.y &&
        p.z >= pos.z && p.z < pos.z + dims.z );
}

bool BoundBox::intersects(BoundBox b){
    return
      !(b.pos.x > pos.x + dims.x    ||
        b.pos.x + b.dims.x < pos.x  ||
        b.pos.y > pos.y + dims.y    ||
        b.pos.y + b.dims.y < pos.y  ||
        b.pos.z > pos.z + dims.z    ||
        b.pos.z + b.dims.z < pos.z  );
}

void BoundBox::printBoundBox(){
    cout << pos.x << "," << pos.y << "," << pos.z << "|" <<
        dims.x << "," << dims.y << "," << dims.z << endl;
}
/*----------------------------------------------------------------------------*/
//private
int Bucket::getIdx(Point pt){
    //binary search implementation (iterative)
        //look at subset of pts[low <-> high]
    int l = 0, r = pts.size()-1;
    int size = r - l + 1;
    int mI = (r + l)/2;

    // if size == 1, no more search
    while(size >= 1){
        //compare pt with pts[mI]
        if(pt < pts[mI]){           //if less, high > mI (high = mI + 1)
            r = mI - 1;
        }else if(pt == pts[mI]){    //if equal, return found idx
            return mI;
        }else{                      //if greater, low < mI (low = mI - 1)
            l = mI + 1;
        }

        size = r - l + 1;
        mI = (r + l)/2;
    }
    return -1;
}

void Bucket::swap(vector<int>& nums, int l, int r){
    int temp = nums[l];
    nums[l] = nums[r];
    nums[r] = temp;
}

/*Reginald Frank, A&M Jan 2019*/
int Bucket::findKthLargest(vector<int>& nums, int k, int cIdx) {
    k = nums.size()-k; //where the element would appear in a sorted array
    int L,R,l,r;
    L = 0;
    R = nums.size()-1;
    while(L < R){
        int p = rand()%(R-L+1)+L;
        swap(nums, p, L);
        l = L;
        r = R;
        while(l<r){
            while(pts[nums[r]][cIdx] > pts[nums[L]][cIdx])
                r--;
            if(l == r)
                break;
            while(pts[nums[l]][cIdx] <= pts[nums[L]][cIdx] && l < R)
                l++;
            if(l < r)
                swap(nums, l, r--);
            else if(r != R)
                l--;
        }
        swap(nums, l, L);
        if(k == l)
            return nums[l];
        if(k < l)
            R = l-1;
        else L = l+1;
    }
    return nums[L];
}

int Bucket::findMedian(int cIdx){
    if(cIdx == 0)
        return pts.size()/2;
    //median of medians for subsets

    int kthLargest = pts.size()/2;
    if(pts.size()%2 == 1)   //odd
        kthLargest++;

    //build array
    vector<int> idxArr;
    for(unsigned int i = 0; i < pts.size(); i++){
        idxArr.push_back(i);
    }

    return findKthLargest(idxArr, kthLargest, cIdx);
}

//public
Bucket::Bucket(BoundBox b, int c){
    boundingArea = b;
    BUCKET_CAPACITY = c;
    color.x = 0;
    color.y = 0;
    color.z = 0;
}

bool Bucket::insert(Point pt){
    if(boundingArea.contains(pt) && (int) pts.size() < BUCKET_CAPACITY){
        //base case size 0
        if(pts.size() == 0){
            pts.push_back(pt);
            return true;
        }

        //binary insertion sort implementation (iterative)
            //look at subset of pts[low <-> high]
        int l = 0, r = pts.size()-1;
        int size = r - l + 1;
        int mI = (r + l)/2;

        // if size == 1, no more sort
        while(size > 1){
            //compare pt with pts[mI]
            if(pt < pts[mI]){           //if less, high > mI (high = mI + 1)
                r = mI - 1;
            }else if(pt == pts[mI]){    //if equal, mI is duplicate, do nothing
                return true;
            }else{                      //if greater, low < mI (low = mI - 1)
                l = mI + 1;
            }

            size = r - l + 1;
            mI = (r + l)/2;
        }
        //compare to single index
        if(pt > pts[mI])    //insert behind
            pts.insert(pts.begin() + mI + 1, pt);
        else if(pt == pts[mI]){
            return true;
        }else   //insert in front
            pts.insert(pts.begin() + mI, pt);
        return true;
    }
    return false;
}

bool Bucket::remove(Point pt){
    int j = getIdx(pt);
    if(j != -1){
        //shift all points down (n op)
        pts.erase(pts.begin() + j);
        return true;
    }
    return false;
}

int Bucket::getNumPts(){
    return pts.size();
}

int Bucket::getCapacity(){
    return BUCKET_CAPACITY;
}

void Bucket::setCapacity(int capacity){
    BUCKET_CAPACITY = capacity;
}

BoundBox Bucket::getBoundingArea(){
    return boundingArea;
}

void Bucket::setBoundingArea(BoundBox b){
    boundingArea = b;
}

Point Bucket::getPoint(int idx){
    if(idx >= 0 && idx < BUCKET_CAPACITY)
        return pts[idx];
    return pts[0];
}

bool Bucket::split(Bucket &otherBucket){
    //don't run if other bucket has stuff (to be safe)
    if(otherBucket.getNumPts() == 0){
        int i;
        // find the longest axis of Cube;
        for(i = 0; i < 3; i++){
            if(boundingArea.getDims()[i%3] >= boundingArea.getDims()[(i+1)%3] &&
                boundingArea.getDims()[i%3] >= boundingArea.getDims()[(i+2)%3]){
                //largest dim is found
                break;
            }
        }

        // find the median along this axis;
        // cut Cube at median to form CubeA, CubeB;
        Point pos;
        Point dim;
        for(int j = 0; j < 3; j++){
            if(j == i){
                //if along the cut axis, shift pos/cut dim
                int medIdx = findMedian(j);
                dim[j] = pts[medIdx][j] -  boundingArea.getPos()[j];
                pos[j] = boundingArea.getPos()[j] + dim[j];
            }else{
                pos[j] = boundingArea.getPos()[j];
                dim[j] = boundingArea.getDims()[j];
            }
        }
        Point otherDim = boundingArea.getDims(), selfDim = dim;
        otherDim[i] -= selfDim[i];
        BoundBox b = BoundBox(pos, otherDim);
        otherBucket.setBoundingArea(b);
        boundingArea.setDims(dim);  //start pos doesn't change

        // move points into other bucket
        for(i = 0; i < getNumPts(); i++){
            Point pt = pts[i];
            if(!boundingArea.contains(pt)){
                if(!remove(pt)){
                    cout << "bad delete" << endl;
                    while(1){
                        int b = 0;
                    }
                }
                if(!otherBucket.insert(pt)){
                    cout << "bad insert (splitting)" << endl;
                    while(1){
                        int b = 0;
                    }
                }
                i--;
            }
        }
        return true;
    }
    return false;
}

bool Bucket::merge(Bucket &otherBucket){
    if(getNumPts() + otherBucket.getNumPts() < BUCKET_CAPACITY){
        //check if they intersect
        BoundBox other = otherBucket.getBoundingArea();
        if(boundingArea.intersects(other)){
            //adjust boundingArea of bucket
            Point pos, dim;
            for(int i = 0; i < 3; i++){
                if(boundingArea.getPos()[i] > other.getPos()[i]){
                    dim[i] = boundingArea.getPos()[i] + boundingArea.getDims()[i] -
                        other.getPos()[i];
                    pos[i] = other.getPos()[i];
                }else{
                    dim[i] = other.getPos()[i] + other.getDims()[i] -
                        boundingArea.getPos()[i];
                    pos[i] = boundingArea.getPos()[i];
                }
            }
            boundingArea.setPos(pos);
            boundingArea.setDims(dim);

            //move points into bucket
            for(int i = 0; i < otherBucket.getNumPts(); i++){
                Point p = otherBucket.getPoint(i);
                if(!insert(p)){
                    cout << "bad insert" << endl;
                    while(1){
                        int b = 0;
                    }
                }
                // if(!otherBucket.remove(p)){
                //     cout << "bad delete" << endl;
                //     while(1){
                //         int b = 0;
                //     }
                // }
            }
            //deallocating or using .erase() on otherBucket post process automatically destroys the memory
            return true;
        }
        cout << "no intersection" << endl;
        return false;
    }
    cout << "overfilled bucket" << endl;
    return false;
}

bool Bucket::matches(Bucket &otherBucket){
    BoundBox self = getBoundingArea();
    BoundBox other = otherBucket.getBoundingArea();
    int equal = 0;
    bool aligned = false;
    for(int i = 0; i < 3; i++){
        //compare each dim, check if equal
        if(self.getDims()[i] == other.getDims()[i] &&
            self.getPos()[i] == other.getPos()[i])
            equal++;
        else{
            if(self.getPos()[i] + self.getDims()[i] == other.getPos()[i] ||
                other.getPos()[i] + other.getDims()[i] == self.getPos()[i])
                aligned = true;
        }
    }
    if(equal == 2 && aligned)
        return true;
    return false;
}

void Bucket::getBucketContent(){
    for(int i = 0; i < getNumPts(); i++){
        pts[i].printPoint();
    }
}

Point Bucket::getColor(){
    return color;
}

void Bucket::setColor(){
    Point p = {0, 0, 0};
    for(int i = 0; i < getNumPts(); i++){
        for(int j = 0; j < 3; j++){
            p[j] += pts[i][j];

        }
    }
    for(int j = 0; j < 3; j++){
        p[j] /= getNumPts();
    }
    color = p;
}

//0, 0, 0
//0, 0, 1
//0, 0, 2
//0, 1, 0
//0, 1, 1
//0, 1, 2
//0, 2, 0
//0, 2, 1
//0, 2, 2

//1, 0, 0
//1, 0, 1
//1, 0, 2
//1, 1, 0
//1, 1, 1   //median for x axis
//1, 1, 2
//1, 2, 0
//1, 2, 1
//1, 2, 2

//2, 0, 0
//2, 0, 1
//2, 0, 2
//2, 1, 0
//2, 1, 1
//2, 1, 2
//2, 2, 0
//2, 2, 1
//2, 2, 2
