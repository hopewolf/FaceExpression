#include"lbp.h"
#include"LBF.h"
#include"LBFRegressor.h"
#include "opencv2/opencv.hpp"
#include "facedetect-dll.h"
#pragma comment(lib,"libfacedetect.lib")
using namespace std;
using namespace cv;
void PredictFromCam()
{
	LBFRegressor regressor;
	regressor.Load(modelPath + "LBF.model");
	string inputName;
	CvCapture* capture = 0;
	Mat frame, frameCopy, image;


	capture = cvCaptureFromCAM(0);
	if (!capture) {
		cout << "Capture from CAM " << 0 << " didn't work" << endl;
	}

	bool tryflip = false;
	double scale = 1.3;
	Mat face[5];
	int fnum = 0;
	bool OK = false;
	cout << "In capture ..." << endl;
	while (1) {
		IplImage* iplImg = cvQueryFrame(capture);
		frame = cvarrToMat(iplImg);
		if (frame.empty())
			break;
		if (iplImg->origin == IPL_ORIGIN_TL)
			frame.copyTo(frameCopy);
		else
			flip(frame, frameCopy, 0);

		detectAndDraw(frameCopy, regressor, scale, tryflip).copyTo(face[fnum]);
		if (face[fnum].size().width <= 0 || face[fnum].size().height <= 0)
			continue;
		fnum++;
		fnum %= 5;
		cout << fnum << endl;
		if (fnum == 0) OK = true;
		if (OK) {

			GetLbp(face, fnum);
		}
			cout <<" ? ? "<<endl;
			/*if (waitKey(10) >= 0)
				goto _cleanup_;*/
		}

		waitKey(0);

	_cleanup_:
		cvReleaseCapture(&capture);
}