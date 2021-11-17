#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <cassert>
#include <random>
#include <cstdlib>

using namespace cv;
using namespace std;
Mat greyscale;

#define CAN_THRESH 100
#define POLY_TOLERANCE 0.1

int main () {
	Mat image = imread("sample.jpeg", IMREAD_UNCHANGED);
	assert(!image.empty() && "No image data!");

	// convert image to greyscale and blur noise
	cvtColor(image, greyscale, COLOR_BGR2GRAY);
	blur(greyscale, greyscale, Size(3, 3));

	// find edges
	Mat edges;
	Canny(greyscale, edges, CAN_THRESH, CAN_THRESH*2);

	// find contours (connected edges)
	vector<vector<Point> > contours;
	vector<Vec4i> tree;
	findContours(edges, contours, tree, RETR_TREE, CHAIN_APPROX_NONE);
	
	// find bounding boxes, circles, and centroids for polygon-ish contours
	vector<vector<Point> > poly_contours (contours.size());
	vector<Rect> boxes (contours.size());
	vector<Point2f> centers (contours.size());
	vector<float> radii (contours.size());

	for (size_t i = 0; i < contours.size(); ++i) {
		approxPolyDP(contours[i], poly_contours[i], POLY_TOLERANCE, true);
		boxes[i] = boundingRect(poly_contours[i]);
		minEnclosingCircle( poly_contours[i], centers[i], radii[i]);
	}

	// draw boxes and contours
	Mat bounding = Mat::zeros(edges.size(), CV_8UC3);
	Mat cont = Mat::zeros(edges.size(), CV_8UC3);
	for (size_t i = 0; i < contours.size(); ++i) {
		Scalar color = Scalar(rand() % 256, rand() % 256, rand() % 256);
		drawContours(cont, poly_contours, (int) i, color);
		rectangle(bounding, boxes[i].tl(), boxes[i].br(), color, 2);
		circle(bounding, centers[i], (int) radii[i], color, 2);
	}

	imshow("Boxes", bounding);
	imshow("Contours", cont);
	imshow("Image", image);

	waitKey(0);
	return 0;
}
