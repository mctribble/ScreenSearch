#include "OpenCVUtils.h"
#include <stdio.h>
#include <iostream>
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

RNG rng(GetTickCount()); //declared global to make sure it only gets seeded once

//loads an image from the given file, finds countours of the image, (http://docs.opencv.org/3.1.0/df/d0d/tutorial_find_contours.html#gsc.tab=0)
//culls contours smaller than min_size, and then returns a Mat object that shows just the contours drawn in random colors
::Mat findCountoursFromFile(char* src, int threshold, double min_size)
{
	//load the source image
	Mat sourceMat = imread(src);
	if (sourceMat.empty())
	{
		wcerr << L"image load failed." << endl;
		return sourceMat;
	}

	//convert it to grayscale.  required for later algorithms that dont use the color data anyway.  Also run a filter on it to reduce noise
	Mat sourceMatGray;
	cvtColor(sourceMat, sourceMatGray, COLOR_BGR2GRAY);
	blur(sourceMatGray, sourceMatGray, Size(3, 3));

	//run edge detection on it using the "canny" algorithm (http://docs.opencv.org/3.1.0/dd/d1a/group__imgproc__feature.html#ga04723e007ed888ddf11d9ba04e2232de&gsc.tab=0)
	Mat edgeDetectionOutput;
	Canny(sourceMatGray, edgeDetectionOutput, threshold, threshold * 2);

	//run the countour detection (http://docs.opencv.org/3.1.0/d3/dc0/group__imgproc__shape.html#ga17ed9f5d79ae97bd4c7cf18403e1689a&gsc.tab=0)
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(edgeDetectionOutput, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	//create a blank image adn draw each contour onto it using a random color
	Mat finalResult = Mat::zeros(edgeDetectionOutput.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		double countourSize = contourArea(contours[i]); //calculates area of the contour
		if (countourSize >= min_size) //prune any that are too small
		{
			Scalar randomColor = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)); //returns, at random, 0-255 in each color channel
			drawContours(finalResult, contours, i, randomColor, 2, 8, hierarchy, 0, Point()); //(see http://docs.opencv.org/3.1.0/d6/d6e/group__imgproc__draw.html#ga746c0625f1781f1ffc9056259103edbc)
		}
	}

	return finalResult;
}
