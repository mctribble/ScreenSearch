#pragma once

#include <Windows.h>
#include <opencv2/imgcodecs/imgcodecs.hpp>

//this is a collection of OpenCV utility functions used elsewhere in ScreenSearch

//loads an image from the given file, finds countours of the image, (http://docs.opencv.org/3.1.0/df/d0d/tutorial_find_contours.html#gsc.tab=0)
//culls contours smaller than min_size, and then returns a Mat object that shows just the contours drawn in random colors
cv::Mat findCountoursFromFile(char* src, int threshold, double min_size);

//// <<this was postponed because it requires building the nonfree opencv-contrib.  Redirected efforts to tesseract OCR for now>>
////takes two images: the first is a sample image containing just the object to be located.  The second is a scene containing said object.
////this algorithm then finds the object in the first image in the second image and highlights it
////if showKeypoints is set, the resulting image contains both original images with lines drawn tos how the match
////based heavily on this tutorial: http://docs.opencv.org/3.1.0/d7/dff/tutorial_feature_homography.html#gsc.tab=0
////note that the test is performed in grayscale.
//cv::Mat findObjectInImage(cv::Mat objectSampleImage, cv::Mat imageToSearch, bool showKeypoints);