#include "OpenCVUtils.h"
#include <iostream>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <cmath>

const double PI = 3.1415926535897932384626433832795;
const double PI_2 = 1.5707963267948966192313216916398;

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

	//run edge detection on it using the "canny" algorithm (http://docs.opencv.org/3.1.0/dd/d1a/group__imgproc__feature.html#ga04723e007ed888ddf11d9ba04e2232de&gsc.tab=0)
	Mat edgeDetectionOutput;
	Canny(sourceMat, edgeDetectionOutput, threshold, threshold * 2);

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

	//two different keypoint algorithms.  
	//KAZE is more accurate, but ORB is much faster, especially for larger images with a lot of keypoints
	//WARNING: KAZE also uses significantly more memory, and may cause crashes with large images on some machines
	//choose algorithm based on image size, since KAZE works much better but has trouble with large images
	Ptr<Feature2D> keypointDetector;
	//if the combined area of both images is larger than this, use ORB
	//this value is based on my dev machine, which has only 4GB memory.
	//depending on your specs, you may be able to increase this figure considerably.
	const unsigned int MAX_TOTAL_AREA = 15000000; 
	unsigned int totalArea = (objectSampleImage.cols * objectSampleImage.rows) + (sceneToSearch.cols * sceneToSearch.rows);
	if ( totalArea > MAX_TOTAL_AREA )
	{
		wcout << "*"; //show an indication that we reverted to ORB
		keypointDetector = ORB::create(); //ORB keypoint detector.  (http://docs.opencv.org/3.1.0/db/d95/classcv_1_1ORB.html#gsc.tab=0)
	}
	else
	{
		keypointDetector = KAZE::create();	//KAZE keypoint detector.  (http://docs.opencv.org/3.1.0/d3/d61/classcv_1_1KAZE.html#gsc.tab=0)
	}


	//keypoint detection and description data
	vector<KeyPoint>	allObjectKeypoints;	//keypoints found in the object sample image
	vector<KeyPoint>	allSceneKeypoints;	//keypoints found in the image being searched
	Mat					objectDescriptors;	//describes keypoints found in the object sample image
	Mat					sceneDescriptors;	//describes keypoints found in the image being searched

	//perform keypoint detection
	try
	{
		keypointDetector->detectAndCompute(objectSampleImage, Mat(), allObjectKeypoints, objectDescriptors);
		wcout << L"."; //add a . to show progress
		keypointDetector->detectAndCompute(sceneToSearch, Mat(), allSceneKeypoints, sceneDescriptors);
		wcout << L"."; //add a . to show progress
	}
	catch (const cv::Exception e)
	{
		//we had an exception, proably from running out of memory.  print the area and wait for user input.
		//we dont need to print the error message because opencv does this internally anyway.
		wcerr << L"(Area: " << totalArea << L")" << endl;
		cin.clear();
		wcout << L"Press enter to conintue." << endl;
		cin.get();
		return Mat();
	}
	

	//error detection: no keypoints
	if (allObjectKeypoints.empty() || allSceneKeypoints.empty())
	{
		wcout << L"no keypoints";
		return Mat();
	}

	//ensure descriptors are in CV_32F format, which is what FLANN requires
	if(objectDescriptors.type() != CV_32F) 
		objectDescriptors.convertTo(objectDescriptors, CV_32F);
	if(sceneDescriptors.type() != CV_32F)
		sceneDescriptors.convertTo(sceneDescriptors, CV_32F);

	//find keypoint matches between the two images 
	//FlannBasedMatcher keypointMatcher;	//using FLANN (faster, less accurate) (http://docs.opencv.org/3.1.0/dc/de2/classcv_1_1FlannBasedMatcher.html#gsc.tab=0)
	BFMatcher keypointMatcher;		//using brute force (slower, more accurate) (http://docs.opencv.org/3.1.0/d3/da1/classcv_1_1BFMatcher.html#gsc.tab=0)
	vector<DMatch> allKeypointMatches;
	keypointMatcher.match(objectDescriptors, sceneDescriptors, allKeypointMatches);

	//find the minimum distance between any two matched keypoints
	double minMatchDistance = 100; //it is important this is not DBL_MAX to specify a minimum precision for "close" matches (see below)
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

	//error detection: not enough matches
	size_t MIN_MATCH_COUNT = 15;
	size_t matchCount = closeKeypointMatches.size();
	if (matchCount < MIN_MATCH_COUNT)
	{
		wcout << L"not enough matches (" << matchCount << "/" << MIN_MATCH_COUNT << L")";
		return Mat();
	}
		

	//find the transformation between keypoints on the object and their matches in the scene
	Mat homography = findHomography(closeObjectKeypoints, closeSceneKeypoints, RANSAC);

	//error detection: no transform
	if (homography.empty())
	{
		wcout << L"couldn't find transform";
		return Mat();
	}
		

	//take the corners of the object image and transform them to the scene image to locate the object
	vector<Point2f> objectCorners(4);
	vector<Point2f> sceneCorners(4);
	objectCorners[0] = cvPoint(0, 0);
	objectCorners[1] = cvPoint(objectSampleImage.cols, 0);
	objectCorners[2] = cvPoint(objectSampleImage.cols, objectSampleImage.rows);
	objectCorners[3] = cvPoint(0, objectSampleImage.rows);
	perspectiveTransform(objectCorners, sceneCorners, homography);

	//error detection: the detected region is not actually inside the scene
	//(small buffer to account for floating point error)
	if ((sceneCorners[0].inside(Rect(-1, -1, sceneToSearch.cols + 2, sceneToSearch.rows + 2)) == false) &&
		(sceneCorners[1].inside(Rect(-1, -1, sceneToSearch.cols + 2, sceneToSearch.rows + 2)) == false) &&
		(sceneCorners[2].inside(Rect(-1, -1, sceneToSearch.cols + 2, sceneToSearch.rows + 2)) == false) &&
		(sceneCorners[3].inside(Rect(-1, -1, sceneToSearch.cols + 2, sceneToSearch.rows + 2)) == false))
	{
		wcout << L"outside the scene";
		return Mat();
	}	

	//error detection: detected region is very small
	const double MIN_AREA = 750.0;
	double area = contourArea(sceneCorners);
	if (area < MIN_AREA)
	{
		wcout << L"region too small (" << area << L"/" << MIN_AREA << L")";
		return Mat();
	}

	//error detection: two corners are very close together (usually from regions that are deformed or very small)
	//testing based on distance squared for performance reasons
	const double MIN_ALLOWED_DIST_SQUARED = 150;
	double curDistSquared[6];
	curDistSquared[0] = Point2fDistanceSquared(sceneCorners[0], sceneCorners[1]); 
	curDistSquared[1] = Point2fDistanceSquared(sceneCorners[0], sceneCorners[2]); 
	curDistSquared[2] = Point2fDistanceSquared(sceneCorners[0], sceneCorners[3]); 
	curDistSquared[3] = Point2fDistanceSquared(sceneCorners[1], sceneCorners[2]); 
	curDistSquared[4] = Point2fDistanceSquared(sceneCorners[1], sceneCorners[3]); 
	curDistSquared[5] = Point2fDistanceSquared(sceneCorners[2], sceneCorners[3]); 
	for (int i = 0; i < 6; i++)
	{
		if (curDistSquared[i] < MIN_ALLOWED_DIST_SQUARED)
		{
			wcout << "corners too close (" << curDistSquared[i] << " < " << MIN_ALLOWED_DIST_SQUARED << ")";
			return Mat();
		}
	}

	//error detection: since the source image is always rectangular, detected regions with very small or very large angles are likely false positives
	//angles are in radians
	vector<double> cornerAngles;
	polyAngles(&sceneCorners, &cornerAngles);
	const double MIN_ANGLE = PI / 64;
	const double MAX_ANGLE = PI;
	for (int i = 0; i < 4; i++)
	{
		if (cornerAngles[i] < MIN_ANGLE)
		{
			wcout << L"Corner angle too small (" << cornerAngles[i] << " < " << MIN_ANGLE << ")";
			return Mat();
		}
		else if (cornerAngles[i] > MAX_ANGLE)
		{
			wcout << L"Corner angle too large (" << cornerAngles[i] << " > " << MAX_ANGLE << ")";
			return Mat();
		}
	}

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

//returns distance squared between two points
inline float Point2fDistanceSquared(Point2f a, Point2f b)
{
	return (((a.x - b.x)*(a.x - b.x)) + ((a.y - b.y)*(a.y - b.y)));
}

//takes a set of polygon corners and returns the interior angle of each in radians
//assumes clockwise winding order
void polyAngles(std::vector<cv::Point2f>* polyCorners, std::vector<double>* polyAngles)
{
	size_t cornerCount = polyCorners->size();
	polyAngles->resize(cornerCount);
	for (size_t i = 0; i < cornerCount; i++)
	{
		//get vectors of adjacent segments
		Vec2f a = Vec2f(polyCorners->at((i + 1)               % cornerCount) - polyCorners->at(i));
		Vec2f b = Vec2f(polyCorners->at((i - 1 + cornerCount) % cornerCount) - polyCorners->at(i));

		//special case: dot product is 0
		if (a.ddot(b) == 0)
		{
			polyAngles->at(i) = PI_2;
		}

		//cosine formula
		double cosine = a.ddot(b) / (norm(a) * norm(b));
		
		//calculate angle
		polyAngles->at(i) = acos(cosine);
	}
	return;
}