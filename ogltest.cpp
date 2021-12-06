#include <opencv2/core/opengl.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace cv;

int main () {
	std::cout << getBuildInformation() << std::endl;

	Mat sample = imread(argv[1]);
	assert(!sample.empty());

	setOpenGlContext("Sample");
	cuda::setGlDevice();

	ogl::Texture2D conv;
	ogl::convertToGLTexture2D(sample, conv);
	ogl::render(conv);
	waitKey(0);

	return 0;
}
