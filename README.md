## markovGeneratorV3
##### Jan 2019
Matthew Yu
C++ Program that generates images from Markov Chains.

![Sunflower with a palette size of 10.](https://github.com/dimembermatt/Markov-Chain-Generator/blob/master/output/sunflower007_10bins.png)

### Instructions (ubuntu/linux):
1. With the binary markovGeneratorV3 and a /data folder with images in it (suggestion to use [Caltech's image datasets](http://www.vision.caltech.edu/Image_Datasets/Caltech101/))
2. run ./markovGeneratorV3
3. type a file in /data to use as the base (i.e. 'launch.png')  
4. enter in a palette size (i.e. '10')
5. enter image scaling (>= 1, rec is 1|2)
6. Markov Chain picture outputs (time varies based on palette size, dataset size, and results may vary based on input image)!

### OPTIONAL COMMANDS
After generating the picture, OfApp accepts the following commands by keyboard:
- _'s'_: saves the image under 'Markov_Chain_[i].jpg' (colors are inverted though for some reason)
- _'r'_: reseeds the Markov Chain (but does not redo palette size)
- _'c'_: returns the credits and exits the program (Program can also be exited by ctrl+c in the console or esc in the window).


If you're interested at looking at the OF code for markovGeneratorV3 to make this program, please contact me.


##### Procedure:
1. Build a bucket as the start of a palette.
2. Given a directory of images, for each image:
    - extract each image.
    - for each pixel, build upon the palette.
        - if a bucket size is exceeded, split the bucket.
        - ignore duplicate colors.
3. Reduce the number of buckets by determining aligned buckets and merging them.
    - results in __n__ buckets.
4. Given the directory of images, for each image:
    - extract each image.
    - for each pixel and its adjacent left and top pixel, determine the bucket that contains the color.
        - the former pixels acts as the indexes for the current pixel.
        - increment the counter at this location (and possibly update the color for this location if needed)
5. Given a seed line (the first row and column for the image, possibly taken from a source image used), generate each successive pixel in row major order using the sums in the stochastic matrix to build a probability model. Repeat until all pixels are drawn.

##### Error Log:
- __FIXED:__ ofApp::compressPalette() - sometimes causes merge error and segfault.
    - passes Bucket::matches() (2|3 dimensions align) but fails BoundBox::intersects() - (not within)
    - removing intersect test leads to bad insert/deletes
    - _Solution:_ adjusting matching check to see if position of left|right bounding box matches position of other box adding the dim for dimensions and positions not equivalent. Flipping the points looked at in the insert/delete portion of the merge function led to all points properly allocated into the left bin.
- __FIXED:__ markov::Populate() - segfaults.
    - _Update:_ crashes at i = 0.
    - commenting out Bucket::remove() in markov::Populate gives a 5 second speed up and otherBucket memory is still deallocated through use of .erase() since otherBucket is part of a vector<Bucket> palette
    - _Solution:_ initialization discrepancies between Markov::dim and palette.size() caused invalid index access. Solved through re-initialization through ofApp::IO();

##### Adjustments:
- Need to speed up the the palette generation process with large sets of images
    - look at similar hashing program with text files from EE312_Cheaters
- ofApp::buildPalette()
    - for file in files:
        - get image pixel data
        - for pixel in image pixel data:
            - get color (pixel)
            - for bucket in palette:
                - if color is contained in bucket
                    - if color fits in bucket:
                        - insert - _optimized_
                        - break
                    - else
                        - split the bucket - _optimized, possibly can cut point checks for moving points_
                        - insert into one of the new buckets
- ofApp::compressPalette()
    - while palette size > wanted palette size:
        - for bucket in palette:
            - if bucket matches otherBucket:
                - set bucket size to both buckets' size
                - merge bucket and otherBucket
                - erase otherBucket - _optimized_
                - break
    - for bucket in palette:
        - set internal mean color
- ofApp::buildSMatrix()
    - Markov::Populate()
        - for image in files:
            - get image pixel data, width, height
            - for pixel in image:
                - get color data for left, above, and center current pixel
                - get bucket index in palette for each pixel
                - increment counter at position table[left][top][center]
- ofApp::buildBuffer()
    - get image pixel data, width, height
    - for pixel in image that is along the first row|column:
        - for bucket in palette:
            - if palette contains the pixel color:
                - get color (bucket) - _optimized_
                - put color into buffer location
- ofApp::update()
    - for x|y index before the end of image:
        - Markov::step()
            - get bucket index in palette for left and above point
            - for bucket in palette:
                - generate totalCt of rolls
            - roll a random number between 1, totalCt
            - for bucket in palette:
                - if roll > 0:
                    - subtract the roll by the table[left][top][center].count
            - get color (bucket) from bucket at palette[idx] - _optimized_
            - return color

##### OPTIMIZATIONS:
- Insertion/Deletion optimization:
    - Bucket::insert() - replaced O(1) insert with vector::push_back() with O(log n) binary search insertion sort.
    - Bucket::getIdx() - replaced O(n) linear probing with O(log n) binary search.
    - Point::<(), Point::>() - created more comparatory methods for Point class for binary search.
    - Pre/Post Mod ofApp::buildPalette() (85 images, 1.2MiB, palette size 3, scaling 1): 90s/6s, 93% faster.

- getColor optimization:
    - Bucket::getColor()|setColor() - set an internal Point color variable that is set by Bucket::setColor() and can be retrieved by Bucket::getColor().
    - set after Bucket::compressPalette(), reduces the amount of O(n) calculations per call of Bucket::getColor() to O(1).
    - Pre/Post Mod of ofApp::buildBuffer, ofApp::update() (85 images, 1.2MiB, palette size 3, scaling 1): 100ms/1ms, ~99% faster; 7.7s/3ms, >99% faster.

- findMedian optimization:
    - __WORKING__ Bucket::findMedian() - using the list of points in the bucket of which maintains the ordered property, attempt to find the median along the dimension given the size of each subset.
        - for x|r dim, median = pts.size()/2
        - for y|g dim, build a list of medians where each number subset of x|r = ###, gets a local median of subset.size()/2
            - median of medians is medians.size()/2
        - for z|b dim, build a list of medians where each number subset of y|g = ###, gets a local median of subset.size()/2
        - time complexity: x|O(1), y|O(k(x)), z|O(k(y)) where k(dim) is the average number of unique values for that dim. Assume k(y) == k(x^2).
        - big priority, ofApp:compressPalette() takes 25 - ~1 a with increasing palette size, a large priority for managing small palette sizes.

- Markov::Populate() - big time drain is in for loop for each pixel, looking at each bucket to get palette index
    - __SUGGESTED__ ofApp::sortPalette() - (after ofApp:buildPalette())
        - reduces O(n) bucket check for Markov::Populate() to O(log n) using a binary search
        - reduces O(n) bucket check for ofApp::compressPalette() to O(log n) using a binary search
        - can replace O(1) push_back for bucket in splitting in ofApp:buildPalette() and O(log n) quicksort/mergesort with binary search insert O(log n) based on Bucket.BoundBox.pos
        - overall, Markov::Populate() takes 5 - ~40 ms with increasing palette size, not much of a priority.

##### Data structure update for Buckets:
- data structure for a 3 dimensional Point (color r,g,b) is based on X > Y > Z priority; sort by X then by Y then by Z
- instead of Bucket::insert() just using vector::push_back() and input order
- use binary search insertion sort for Bucket::insert().
- each bucket's points are now in order.
- simplify Bucket::findMedian() by removing Bucket::findKthLargest(), return pt[middle][cIdx] (the sorted middle point for that dim).
    - the middle is based on cIdx
        - if cIdx == 0 (X), get pts[pts.size()/2][cIdx]
        - if cIdx == 1 (Y), get median of medians if each point in medians is pts[pts[i].size()/2] where pts[i].size() is the length of the array subset where i is the value of the first dim of the point.
        - if cIdx == 2 (Z), get median of medians if each point in medians is pts[pts[i][j].size()/2] where pts[i][j].size() is the length of the array subset where i is the value of the first dim of the point and j is the second dim of the point.
