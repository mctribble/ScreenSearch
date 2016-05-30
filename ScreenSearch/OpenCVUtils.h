#pragma once

#include <Windows.h>
#include <opencv2/imgcodecs/imgcodecs.hpp>

//this is a collection of OpenCV utility functions used elsewhere in ScreenSearch

//loads an image from the given file, finds countours of the image, (http://docs.opencv.org/3.1.0/df/d0d/tutorial_find_contours.html#gsc.tab=0)
//culls contours smaller than min_size, and then returns a Mat object that shows just the contours drawn in random colors
cv::Mat findCountoursFromFile(char* src, int threshold, double min_size);