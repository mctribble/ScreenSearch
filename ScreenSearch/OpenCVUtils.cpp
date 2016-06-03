#include "OpenCVUtils.h"
#include <stdio.h>
#include <iostream>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
//#include "opencv2/xfeatures2d.hpp"

using namespace cv;
using namespace std;

RNG rng(GetTickCount()); //declared global to make sure it only gets seeded once

//loads an image from the given file, finds countours of the image, (http://docs.opencv.org/3.1.0/df/d0d/tutorial_find_contours.html#gsc.tab=0)
//culls contours smaller than min_size, and then returns a Mat object that shows just the contours drawn in random colors
::Mat findCountoursFromFile(Mat sourceMat, int threshold, double min_size)
{
	//validate input
	if (sourceMat.empty())
	{
		wcerr << L"input image is empty!." << endl;
		return sourceMat;
	}

	//convert it to grayscale.  required for later algorithms that dont use the color data anyway.  Also run a blur filter on it to reduce noise
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

	//create a blank image and draw each contour onto it using a random color
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

//takes two images: the first is a sample image containing just the object to be located.  The second is a scene containing said object.
//this algorithm then finds the object in the first image in the second image and highlights it
//if showMatchData is set, the resulting image contains both original images with lines drawn to show the match
//based heavily on this tutorial: http://docs.opencv.org/3.1.0/d7/dff/tutorial_feature_homography.html#gsc.tab=0
//the test is performed in grayscale, so his function expects the parameters to be grayscale
cv::Mat findObjectInImage(cv::Mat objectSampleImage, cv::Mat sceneToSearch, bool showMatchData)
{
	//validate input
	if (objectSampleImage.empty() || sceneToSearch.empty())
	{
		wcerr << L"input image is empty!." << endl;
		return Mat();
	}

	//two different keypoint algorithms.  KAZE is more accurate, but ORB is much faster, especially for larger images with a lot of keypoints
	Ptr<KAZE>			keypointDetector = KAZE::create();	//KAZE keypoint detector.  (http://docs.opencv.org/3.1.0/d3/d61/classcv_1_1KAZE.html#gsc.tab=0)
	//Ptr<ORB>			keypointDetector = ORB::create();	//ORB keypoint detector.  (http://docs.opencv.org/3.1.0/db/d95/classcv_1_1ORB.html#gsc.tab=0)

	//keypoint detection and description data
	vector<KeyPoint>	allObjectKeypoints;	//keypoints found in the object sample image
	vector<KeyPoint>	allSceneKeypoints;	//keypoints found in the image being searched
	Mat					objectDescriptors;	//describes keypoints found in the object sample image
	Mat					sceneDescriptors;	//describes keypoints found in the image being searched

	//perform keypoint detection
	keypointDetector->detectAndCompute(objectSampleImage, Mat(), allObjectKeypoints, objectDescriptors);
	keypointDetector->detectAndCompute(sceneToSearch, Mat(), allSceneKeypoints, sceneDescriptors);

	//if either list of keypoints is empty, bail now and return empty image
	if (allObjectKeypoints.empty() || allSceneKeypoints.empty())
		return Mat();

	//ensure descriptors are in CV_32F format, which is what FLANN requires
	if(objectDescriptors.type() != CV_32F) 
		objectDescriptors.convertTo(objectDescriptors, CV_32F);
	if(sceneDescriptors.type() != CV_32F)
		sceneDescriptors.convertTo(sceneDescriptors, CV_32F);

	//find keypoint matches between the two images using FLANN (http://docs.opencv.org/3.1.0/dc/de2/classcv_1_1FlannBasedMatcher.html#gsc.tab=0)
	FlannBasedMatcher keypointMatcher;
	vector<DMatch> allKeypointMatches;
	keypointMatcher.match(objectDescriptors, sceneDescriptors, allKeypointMatches);

	//find the minimum distance between any two matched keypoints
	double minMatchDistance = 100; //it is important this is 100-ish and not DBL_MAX to specify a minimum precision for "close" matches (see below)
	for (int i = 0; i < objectDescriptors.rows; i++)
	{
		double curDist = allKeypointMatches[i].distance;
		if (curDist < minMatchDistance)
			minMatchDistance = curDist;
	}

	//reduce errors by only working with matches that are relatively close to the minimum match distance (minMatchDistance*3)
	vector<DMatch> closeKeypointMatches;	//matches themselves
	vector<Point2f> closeObjectKeypoints, closeSceneKeypoints;	//points from said matches
	for (int i = 0; i < objectDescriptors.rows; i++)
	{
		if (allKeypointMatches[i].distance <= (minMatchDistance * 3.0))
		{
			closeKeypointMatches.push_back(allKeypointMatches[i]);
			closeObjectKeypoints.push_back(allObjectKeypoints[allKeypointMatches[i].queryIdx].pt);
			closeSceneKeypoints.push_back(allSceneKeypoints[allKeypointMatches[i].trainIdx].pt);
		}
	}

	//find the transformation between keypoints on the object and their matches in the scene
	Mat homography = findHomography(closeObjectKeypoints, closeSceneKeypoints, RANSAC);

	//if there is no homography matrix, bail out now and return an empty image
	if (homography.empty())
		return Mat();

	//take the corners of the object image and transform them to the scene image to locate the object
	vector<Point2f> objectCorners(4);
	vector<Point2f> sceneCorners(4);
	objectCorners[0] = cvPoint(0, 0);
	objectCorners[1] = cvPoint(objectSampleImage.cols, 0);
	objectCorners[2] = cvPoint(objectSampleImage.cols, objectSampleImage.rows);
	objectCorners[3] = cvPoint(0, objectSampleImage.rows);
	perspectiveTransform(objectCorners, sceneCorners, homography);
	
	//if none of the scene corners are actually in the scene, then the object was not found.  return an empty image.
	if ((sceneCorners[0].inside(Rect(0, 0, sceneToSearch.cols, sceneToSearch.rows)) == false) &&
		(sceneCorners[1].inside(Rect(0, 0, sceneToSearch.cols, sceneToSearch.rows)) == false) &&
		(sceneCorners[2].inside(Rect(0, 0, sceneToSearch.cols, sceneToSearch.rows)) == false) &&
		(sceneCorners[3].inside(Rect(0, 0, sceneToSearch.cols, sceneToSearch.rows)) == false))
		return Mat();

	//create a new image to use as our output that supports color
	Mat result = Mat(sceneToSearch.size(), CV_8UC3);
	cvtColor(sceneToSearch, result, CV_GRAY2BGR); //uses BGR instead of RGB because that is the default in openCV

	//regardless of showPrompt, we want the object to be highlighted in the scene.  We draw this outline first:
	line(result, sceneCorners[0], sceneCorners[1], Scalar(0, 255, 0), 4);
	line(result, sceneCorners[1], sceneCorners[2], Scalar(0, 255, 0), 4);
	line(result, sceneCorners[2], sceneCorners[3], Scalar(0, 255, 0), 4);
	line(result, sceneCorners[3], sceneCorners[0], Scalar(0, 255, 0), 4);
	
	//now that the object is highlighted, we have finished our main job.  Return now if we don't need to show the matches
	if (showMatchData == false)
		return result;

	//we want the match data also.  Call drawMatches to do the heavy lifting.
	Mat resultWithMatchData;
	drawMatches(objectSampleImage, allObjectKeypoints,		//object data
				result, allSceneKeypoints,					//scene data
				closeKeypointMatches,						//match data
				resultWithMatchData,						//where to put it
				Scalar::all(-1), Scalar::all(-1),			//use random colors
				std::vector<char>(),						//empty mask
				DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);	//hide points that didnt match anything

	//and return it
	return resultWithMatchData;
}

//saves the given Mat to a file of the given name.  if showPrompt is true, ask the user if they want
//to see the file after it is saved.  If yes, it is opened with the system default application
bool matToFile(cv::Mat src, LPCSTR fileName, bool showPrompt)
{
	//save result to file
	if (cv::imwrite(fileName, src) == false)
		return false; //file save failed

	//offer to show result to the user
	//ask user until they respond in a valid way or the input stream closes
	if (showPrompt)
	{
		char response = '0';
		do
		{
			wcout << "Saved file " << fileName << " to disk.  Would you like to open it? [y/n]" << endl;
			cin >> response;
		} while (!cin.fail() && response != 'y' && response != 'Y' && response != 'n' && response != 'N');

		//if yes, open file with the default program
		if (response == 'y' || response == 'Y')
			ShellExecuteA(0, 0, fileName, 0, 0, SW_SHOW);
	}

	return true;
}
