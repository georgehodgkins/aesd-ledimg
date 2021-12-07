#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <cassert>
#include <random>
#include <cstdlib>
#include <unistd.h>
#include <optional>
#include <signal.h>
#include "ledgrid.h"

using namespace cv;
using namespace std;

// lengths are in units of pixels
#define MIN_CIRCLE_DIST 100 // minimum distance between centers of circles
#define INITIAL_MIN_RAD 75 // starting minimum circle radius
#define MIN_RAD_STEP 25 // decrement when no circles are found
#define MIN_MIN_RAD 25 // radius after which to abort search
#define MAX_RAD (ROWHEIGHT*2) 
#define HOUGH_CANNY_THRESH 100 // threshold for the canny edge detector used by HoughCircles
#define HOUGH_ACC_THRESH 100 // Hough accumulator threshold, smaller = more sensitive

static Mat greyscale;
int COLWIDTH, ROWHEIGHT; // input image resolution, set when camera is initialized
static optional<Vec3f> locateCircle (const Mat& image) {
	
	// convert image to greyscale and blur noise
	cvtColor(image, greyscale, COLOR_BGR2GRAY);
	blur(greyscale, greyscale, Size(3, 3));

	// find largest circle
	vector<Vec3f> circles;
	int minrad = INITIAL_MIN_RAD;
	while (circles.size() == 0 && minrad >= MIN_MIN_RAD) {
		HoughCircles(greyscale, circles, HOUGH_GRADIENT, 1,
				MIN_CIRCLE_DIST, HOUGH_CANNY_THRESH, HOUGH_ACC_THRESH, minrad, MAX_RAD);
		minrad -= MIN_RAD_STEP;
	}
	if (circles.size() == 0) return nullopt;

	Vec3f ret;
	if (circles.size() > 1) { 
		auto cit = max_element(circles.begin(), circles.end(),
			[](Vec3f a, Vec3f b){ return a[2] < b[2]; }); // compare by radius
		ret = *cit;
	} else {
		ret = circles[0];
	}

	return make_optional(ret);
}

#define IMG_CAP_SOURCE "/dev/video0"
bool loop = true; // loop exit flag

void exitHandler(int sig) {
	loop = false;
}

#define GRID_THICKNESS 4
const Scalar GRID_COLOR (0, 0, 0); // black
const Scalar BOX_COLOR (255, 0, 0); // red

static void setGridPoint (const optional<Vec3f>& Po, Mat& image) {
	// draw reference grid
	for (int c = 1; c < LEDGRID_COLS; ++c) {
		Point2i top (COLWIDTH*c, 0);
		Point2i bot (COLWIDTH*c, image.rows-1);
		line(image, top, bot, GRID_COLOR, GRID_THICKNESS);
	}
	for (int r = 1; r < LEDGRID_ROWS; ++r) {
		Point2i left (0, ROWHEIGHT*r);
		Point2i rght (image.cols-1, ROWHEIGHT*r);
		line(image, left, rght, GRID_COLOR, GRID_THICKNESS);
	}

	if (Po) {
		const Vec3f& Pv = *Po;
		Point2f P (Pv[0]/COLWIDTH, Pv[1]/ROWHEIGHT);
		unsigned r = (unsigned) P.y & 0x7;
		unsigned c = (unsigned) P.x & 0x7;
		unsigned addr = (r << 2) + c;
		cout << "\"Selected\" coord (" << ((addr & 0xc) >> 2) << ", " << (addr & 0x3) << ") [raw " << addr << "]\n";
		assert(addr >= 0 && addr < 16);

#ifndef DEVHOST
		// select LED
		int s = grid_select(addr);
		assert(s == 0 && "Error selecting LED!");
#endif
		// select box
		Point2i topleft (c*COLWIDTH, r*ROWHEIGHT);
		Point2i botrght ((c+1)*COLWIDTH-1, (r+1)*ROWHEIGHT-1);
		rectangle(image, topleft, botrght, BOX_COLOR, GRID_THICKNESS);
		circle(image, Point2f(Pv[0], Pv[1]), Pv[2], BOX_COLOR);
	}
}	


#define WIN_REFIMG "Reference image"
int main (int argc, char** argv) {
	if (argc != 2) {
		cout << "Usage: " << argv[0] << "</path/to/cam>" << '\n';
		return 1;
	}

	// open camera stream
	VideoCapture cam (argv[1], CAP_V4L2);
	if (!cam.isOpened()) {
		cout << "Could not open camera at " << argv[1] << " with V4L2 backend.\n";
		return 1;
	}
	COLWIDTH = cam.get(CAP_PROP_FRAME_WIDTH) / LEDGRID_COLS;
	ROWHEIGHT = cam.get(CAP_PROP_FRAME_HEIGHT) / LEDGRID_ROWS;

#ifndef DEVHOST	
	// open LED grid
	int s = grid_init();
	if (s) {
		cout << "Could not initialize LED grid.\n";
		return 1;
	}
#endif
	
	struct sigaction act;
	act.sa_handler = exitHandler;
	act.sa_flags = 0;
	int ss = sigaction(SIGINT, &act, NULL);
	assert(ss == 0);

	Mat image;
	while (loop) {
		// get image
		bool f = cam.read(image);
		if (!f || image.empty()) {
			cout << "No image from camera, exiting\n";
			break;
		}
		// find circle + light appropriate LEDs + write reference annotations to image
		setGridPoint(locateCircle(image), image);
		// display image
#ifdef DEVHOST
		imshow(WIN_REFIMG, image);
#endif
		waitKey(1);
	}
	
#ifndef DEVHOST
	grid_free();
#endif
	return 0;
}
