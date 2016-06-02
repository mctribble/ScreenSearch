#pragma once

#include <Windows.h>
#include <opencv2/imgcodecs/imgcodecs.hpp>

//this is a collection of OpenCV utility functions used elsewhere in ScreenSearch

//loads an image from the given file, finds countours of the image, (http://docs.opencv.org/3.1.0/df/d0d/tutorial_find_contours.html#gsc.tab=0)
//culls contours smaller than min_size, and then returns a Mat object that shows just the contours drawn in random colors
//the threshold is used for edge detection.  smaller values find more edges and result in finding more contours
cv::Mat findCountoursFromFile(cv::Mat sourceMat, int threshold, double min_size);

//takes two images: the first is a sample image containing just the object to be located.  The second is a scene containing said object.
//this algorithm then finds the object in the first image in the second image and highlights it
//if showKeypoints is set, the resulting image contains both original images with lines drawn tos how the match
//based heavily on this tutorial: http://docs.opencv.org/3.1.0/d7/dff/tutorial_feature_homography.html#gsc.tab=0
//note that the test is performed in grayscale, so input images must be grayscale
cv::Mat findObjectInImage(cv::Mat objectSampleImage, cv::Mat imageToSearch, bool showKeypoints);

//saves the given Mat to a file of the given name.  if showPrompt is true, ask the user if they want
//to see the file after it is saved.  If yes, it is opened with the system default application
bool matToFile(cv::Mat src, LPCSTR dest, bool showPrompt = false);