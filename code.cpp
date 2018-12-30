#include "opencv2\opencv.hpp"
#include "opencv\cv.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ctime>
#include <tuple>
#include <cmath>
#include <vector>
#include <queue>
#include <Windows.h>
#include <opencv2/highgui.hpp>
#include "opencv2\nonfree\nonfree.hpp"
#include "opencv2\features2d.hpp"
using namespace cv;
using namespace std;
Mat storeBK[30];
Mat StoreHBK[30];
Mat StoreAHBK[30];
size_t bkWidth[30];
Mat Harris_s, src;
vector<Vec4i> pos;
pair<double, int> val(1000, 0);
pair<int, int> mPoint;
Mat descriptor_s;
Mat result;
vector<KeyPoint> keypoint_s;
int segIdx = 0;
char segTempname[30][4] = { "b1","b2","b3","b4","b5","b6","b7","b8","b9","b10","b11","b12","b13","b14","b15" };
void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		int idx = -1;
		for (int curX = 0; curX <= x; curX++) {
			if (result.at<Vec3b>(y, curX)[2] == 255 && result.at<Vec3b>(y, curX)[0] == 0 && result.at<Vec3b>(y, curX)[1] == 0) {
				idx++;
			}
		}
		imshow(segTempname[segIdx++], storeBK[idx]);
	}
}
void CornerHarris(Mat &Image, Mat &ret)
{
	IplImage* gray_lpl1 = &IplImage(Image);
	IplImage* out1 = cvCreateImage(cvGetSize(gray_lpl1), IPL_DEPTH_32F, 1);

	cvCornerHarris(gray_lpl1, out1, 2, 11, 0.00000001);
	ret = cvarrToMat(out1);
	ret.convertTo(ret, CV_8UC3);
	Image.convertTo(Image, CV_8UC3);
	ret = Image*0.2 + ~ret*0.4;
}
void Surf(Mat &harris, Mat &descriptors, vector <KeyPoint> &Keypoints, int features)
{
	//특징 기술자 저장
	SurfFeatureDetector detector(700);//hessian threshold
	detector.detect(harris, Keypoints);
	cout << "keypoints size = " << Keypoints.size() << endl;
	KeyPointsFilter::retainBest(Keypoints, features);

	//특징 추출자 저장을 위한 extractor 객체 생성
	SurfDescriptorExtractor extractor;
	extractor.compute(harris, Keypoints, descriptors);
}
int surf_BFmatch(int idx, Mat descriptor_s, vector <KeyPoint> &Keypoint_s,
	Mat descriptor_t, vector <KeyPoint> &Keypoint_t)
{
	vector<DMatch>matches;
	BFMatcher matcher(NORM_L2);
	matcher.match(descriptor_t, descriptor_s, matches);
	if (matches.size() == 0) return 0;
	cout << "matches.size()=" << matches.size() << endl;
	//if (matches.size() < 4)
	//	return 0;

	double minDist, maxDist;
	minDist = maxDist = matches[0].distance;
	for (int i = 1; i < matches.size(); i++) {
		double dist = matches[i].distance;
		if (dist < minDist) minDist = dist;
		if (dist > maxDist) maxDist = dist;
	}
	cout << "minDist=" << minDist << endl;
	cout << "maxDist=" << maxDist << endl;

	vector<DMatch>goodMatches;
	double fTh = 4 * minDist;
	for (int i = 0; i < matches.size(); i++) {
		if (matches[i].distance <= max(fTh, 0.4))
			goodMatches.push_back(matches[i]);
	}
	cout << "goodMatches.size()=" << goodMatches.size() << endl;
	if (goodMatches.size() < 4)
	{
		imshow("error1", StoreHBK[idx]);
		return 0;
	}
	Mat imgMatches;
	drawMatches(storeBK[idx], Keypoint_t, src, Keypoint_s,
		goodMatches, imgMatches, Scalar::all(-1), Scalar::all(-1),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	vector<Point2f> obj;
	vector<Point2f> scene;
	for (int i = 0; i < goodMatches.size(); i++) {
		obj.push_back(Keypoint_t[goodMatches[i].queryIdx].pt);
		scene.push_back(Keypoint_s[goodMatches[i].trainIdx].pt);
	}
	Mat H = findHomography(obj, scene, CV_RANSAC);

	vector<Point2f> objP(4);
	objP[0] = cvPoint(0, 0);
	objP[1] = cvPoint(storeBK[idx].cols, 0);
	objP[2] = cvPoint(storeBK[idx].cols, storeBK[idx].rows);
	objP[3] = cvPoint(0, storeBK[idx].rows);

	vector<Point2f>sceneP(4);
	perspectiveTransform(objP, sceneP, H);

	for (int i = 0; i < 4; i++)
		sceneP[i] += Point2f(storeBK[idx].cols, 0);
	for (int i = 0; i < 4; i++)
		line(imgMatches, sceneP[i], sceneP[(i + 1) % 4], Scalar(255, 0, 0), 4);
	imshow("imgMatches", imgMatches);
	//waitKey();
	return 0;
}

pair<int, int> sortAndPosY(vector<Vec4i> &lines, vector<pair<int, int>> &set)
{
	priority_queue<int> pq;
	//sorting matching indexing
	for (int i = 0; i < lines.size(); i++)
	{
		set[i].first = lines[i][0];
		set[i].second = i;

		// y insert
		pq.push(lines[i][1]);
		pq.push(lines[i][3]);
	}

	int cnt = 0;
	int topY, bottomY;
	int intVal = 4; // top, bottom 4번째 
	while (!pq.empty()) {
		cnt++;
		if (cnt == intVal) topY = pq.top();
		if (pq.size() == intVal) { bottomY = pq.top(); break; }
		pq.pop();
	}
	sort(set.begin(), set.end());
	return make_pair(bottomY, topY);
}
void pruning(vector<Vec4i> &lines, vector<pair<int, int>> &indexing)
{
	//remove blank
	//coding........


	//remove short x
	int tempUpper, templower;
	tempUpper = templower = -20;
	int upperX, upperY, lowerX, lowerY;
	for (int i = 0; i < lines.size(); i++)
	{
		int idx = indexing[i].second;
		upperX = lines[idx][0];
		upperY = lines[idx][1];
		lowerX = lines[idx][2];
		lowerY = lines[idx][3];
		if (upperY > lowerY) {
			swap(upperX, lowerX);
			swap(upperY, lowerY);
		}
		if (abs(upperX - tempUpper) < 17 || abs(lowerX - templower) < 15) continue;
		tempUpper = upperX;
		templower = lowerX;
		pos.push_back(Vec4i(upperY, upperX, lowerY, lowerX));
	}
}
void DrawallP(vector<Vec4i> &lines, Mat &result)
{
	for (int i = 0; i < lines.size(); i++)
	{
		rectangle(result, Rect(lines[i][0], lines[i][1], 5, 5), Scalar(255, 0, 0), 2);
		rectangle(result, Rect(lines[i][2], lines[i][3], 5, 5), Scalar(255, 0, 0), 2);
	}
	return;
}
void DrawallPAfterPruning(Mat &result)
{
	for (int i = 0; i < pos.size(); i++)
	{
		rectangle(result, Rect(pos[i][1], pos[i][0], 5, 5), Scalar(255, 0, 0), 2);
		rectangle(result, Rect(pos[i][3], pos[i][2], 5, 5), Scalar(255, 0, 0), 2);
	}
	return;
}
int main()
{
	char name[30][8] = { "b1.jpg", "b2.jpg", "b3.jpg","b4.jpg" ,"b5.jpg" ,"b6.jpg" ,"b7.jpg" ,"b8.jpg" ,"b9.jpg" ,
		"b10.jpg" ,"b11.jpg" ,"b12.jpg" ,"b13.jpg","b14.jpg", "b15.jpg","b16.jpg","b17.jpg","b18.jpg","b19.jpg"
		,"b20.jpg","b21.jpg","b22.jpg" ,"b33.jpg" };
	src = imread("test8.jpg", 0);

	// init storeBK size 
	for (int i = 0; i < 30; i++)
		storeBK[i] = Mat(Size(150, src.rows), CV_8UC1);

	IplImage* gray_lpl1 = &IplImage(src);
	IplImage* out1 = cvCreateImage(cvGetSize(gray_lpl1), IPL_DEPTH_32F, 1);
	cvCornerHarris(gray_lpl1, out1, 3, 9, 0.05); // 코너 해리스 

	Mat temp = cvarrToMat(out1);
	imshow("aa", src);
	imshow("reversed src", ~src);
	threshold(temp, temp, 1, 255, THRESH_BINARY);
	temp.convertTo(temp, CV_8UC3);
	imshow("temp", temp);
	//vertical filter
	Mat hough;
	adaptiveThreshold(~src, hough, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 13, -2);

	imshow("adaptive threshold", hough);

	// remove short edge1 : morphology 이용
	Mat Mor_Rect(hough.rows / 80, 1, CV_8U, Scalar(1));
	Mat erodeMat(25, 1, CV_8U);
	Mat dialationMat(1, 5, CV_8U);
	Mat erodeMat2(3, 1, CV_8U);
	morphologyEx(hough, hough, MORPH_OPEN, Mor_Rect);
	hough.convertTo(hough, CV_8UC3);
	imshow("middle", hough);
	morphologyEx(hough, hough, MORPH_CLOSE, dialationMat);
	morphologyEx(hough, hough, MORPH_OPEN, erodeMat, Point(-1, -1), 1);
	morphologyEx(hough, hough, MORPH_OPEN, erodeMat2, Point(-1, -1), 10);
	imshow("end", hough);
	// Remove short edge2 : corner harris 이용
	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++)
			if (temp.at<uchar>(i, j) > 10) line(hough, Point(j, i), Point(j, i + 5), 0, 1, 8);

	imshow("aaa", hough);
	result = Mat(src.size(), CV_8UC3);
	cvtColor(src, result, COLOR_GRAY2BGR);
	imshow("before hough", hough);
	//short-line 증가 long line 증가
	//#@! hough 각도 적용 범위 여부
	vector<Vec4i> lines;
	HoughLinesP(hough, lines, 1, CV_PI / 180, 190, 100, 250);

	//sorting line 
	vector<pair<int, int>> indexing(lines.size()); // (x1's value, index);
	pair<int, int> posY = sortAndPosY(lines, indexing);

	//pruning
	pruning(lines, indexing);


	//DrawallP(lines, result);
	//DrawallPAfterPruning(result);

	for (int i = 0; i < pos.size(); i++) {

		// vertical line expansion
		float intervalX = pos[i][3] - pos[i][1];
		float intervalY = pos[i][2] - pos[i][0];
		float rate = intervalX / intervalY;
		//
		if (posY.first < pos[i][0]) {
			pos[i][1] -= rate*(pos[i][0] - posY.first);
			pos[i][1] = pos[i][1] < 0 ? 0 : pos[i][1]; // first Line exception
			pos[i][0] = posY.first;
		}
		if (posY.second > pos[i][2]) {
			pos[i][3] -= rate*(posY.second - pos[i][2]);
			pos[i][3] = pos[i][3] < 0 ? 0 : pos[i][3]; // first Line exception
			pos[i][2] = posY.second;
		}
		line(result, Point(pos[i][1], pos[i][0]), Point(pos[i][3], pos[i][2]), Scalar(0, 0, 255), 1, 8, 0);
	}

	imshow("result", result);
	//segmentation
	for (int i = posY.first; i <= posY.second; i++) {
		bool firstLine = false;
		int widthX = 0;
		int idx = -1;
		for (int j = 0; j < result.cols; j++) {
			if (result.at<Vec3b>(i, j)[2] == 255 && result.at<Vec3b>(i, j)[0] == 0 && result.at<Vec3b>(i, j)[1] == 0) {
				// for storeBK resizing
				if (idx >= 0)
					bkWidth[idx] = bkWidth[idx] > j - widthX ? bkWidth[idx] : j - widthX;

				// increment
				firstLine = true;
				idx++;
				widthX = j;
			}
			else if (firstLine) {
				storeBK[idx].at<uchar>(i, j - widthX) = src.at<uchar>(i, j);
			}
		}
	}

	//storeBK resizing
	for (int i = 0; i < pos.size() - 1; i++) {
		storeBK[i] = storeBK[i](Rect(0, 0, bkWidth[i], storeBK[i].rows));
		imshow(name[i], storeBK[i]);
		waitKey();
	}
	// 마우스 포인터
	//setMouseCallback("result", CallBackFunc, NULL);
	//waitKey();


	Mat descriptor_t;
	vector<KeyPoint> keypoint_t;
	CornerHarris(src, Harris_s);
	Surf(Harris_s, descriptor_s, keypoint_s, 4000);

	for (int i = 0; i < pos.size() - 1; i++)
	{
	cout << i << endl;
	CornerHarris(storeBK[i], StoreHBK[i]);
	Surf(StoreHBK[i], descriptor_t, keypoint_t, 300);
	surf_BFmatch(i, descriptor_s, keypoint_s, descriptor_t, keypoint_t);
	waitKey();
	}
	return 0;
	waitKey();
}