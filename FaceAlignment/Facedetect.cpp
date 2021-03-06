//
//  Facedetect.cpp
//  myopencv
//
//  Created by lequan on 1/24/15.
//  Copyright (c) 2015 lequan. All rights reserved.
//
#include "opencv2/opencv.hpp"
#include "facedetect-dll.h"
#pragma comment(lib,"libfacedetect.lib")
#include "LBFRegressor.h"
#include "windows.h"
#include <stdlib.h> 
using namespace std;
using namespace cv;

//extern LBFRegressor myregressor;
//void cutFace(char imagefile[MAXNUMIMAGE][MAX_PATH], int len, char *lpath)
//{
//	LBFRegressor myregressor;
//	myregressor.Load(modelPath + "LBF.model");
//	FacesDetectionAndAlignment(imagefile, len, lpath, myregressor);
//}
void cutFace(char imagefile[MAXNUMIMAGE][MAX_PATH], int len, char *lpath,LBFRegressor& regressor)
{
	FacesDetectionAndAlignment(imagefile, len, lpath, regressor);
}

int FacesDetectionAndAlignment(const char inputname[100][260],int num,char *lpath, LBFRegressor& regressor)
{
	string inputName;
	bool tryflip = false;
	double scale = 1.3;
	Mat frame, frameCopy, image;

	Mat face;
	char cmd[264];
	for (int i = 0; i < num; i++)
	{
		strcpy( cmd, "del ");
		strcat(cmd, inputname[i]);
		cout << cmd << endl;
		//cout << inputname[i] << endl;
		inputName.assign(inputname[i]);
		if (inputName.size()) {
			if (inputName.find(".jpg") != string::npos || inputName.find(".png") != string::npos
				|| inputName.find(".bmp") != string::npos) {
				image = imread(inputName, 1);
				if (image.empty()) {
					cout << "Read Image fail" << endl;
					return -1;
				}
			}
		}
		if (!image.empty()) {
			cout << "In image read" << endl;
			face=detectAndDraw(image, regressor, scale, tryflip);
			system(cmd);
			if(!face.empty())
			imwrite(inputname[i], face);
			//waitKey(0);
		}	
	}

//cvDestroyWindow("result");


}



int FaceDetectionAndAlignment(const char* inputname){
    extern string cascadeName;
    string inputName;
    CvCapture* capture = 0;
    Mat frame, frameCopy, image;
    bool tryflip = false;
    double scale  = 1.3;
    
    if (inputname!=NULL){
        inputName.assign(inputname);
    }
    
    // name is empty or a number
    if( inputName.empty() || (isdigit(inputName.c_str()[0]) && inputName.c_str()[1] == '\0') ){
        capture = cvCaptureFromCAM( inputName.empty() ? 0 : inputName.c_str()[0] - '0' );
        int c = inputName.empty() ? 0 : inputName.c_str()[0] - '0' ;
        if(!capture){
            cout << "Capture from CAM " <<  c << " didn't work" << endl;
            return -1;
        }
    }
    // name is not empty
    else if( inputName.size() ){
        if (inputName.find(".jpg")!=string::npos||inputName.find(".png")!=string::npos
            ||inputName.find(".bmp")!=string::npos){
            image = imread( inputName, 1 );
            if (image.empty()){
                cout << "Read Image fail" << endl;
                return -1;
            }
        }
        else if(inputName.find(".mp4")!=string::npos||inputName.find(".avi")!=string::npos
                ||inputName.find(".wmv")!=string::npos){
            capture = cvCaptureFromAVI( inputName.c_str() );
            if(!capture) cout << "Capture from AVI didn't work" << endl;
            return -1;
        }
    }
    // -- 0. Load LBF model
    LBFRegressor regressor;
    regressor.Load(modelPath+"LBF.model");

    if( capture ){
        cout << "In capture ..." << endl;
        for(;;){
            IplImage* iplImg = cvQueryFrame( capture );
            frame = cvarrToMat(iplImg);
            if( frame.empty() )
                break;
            if( iplImg->origin == IPL_ORIGIN_TL )
                frame.copyTo( frameCopy );
            else
                flip( frame, frameCopy, 0 );

            detectAndDraw(frameCopy,regressor, scale, tryflip );

           if( waitKey( 10 ) >= 0 )
             goto _cleanup_;
        }

       waitKey(0);

_cleanup_:
        cvReleaseCapture( &capture );
    }
    else{
       
        if( !image.empty() ){
            cout << "In image read" << endl;
            detectAndDraw( image,regressor,  scale, tryflip );
            waitKey(0);
        }
        else if( !inputName.empty() ){
            /* assume it is a text file containing the
            list of the image filenames to be processed - one per line */
            cout << "In image set model" << endl;
            FILE* f = fopen( inputName.c_str(), "rt" );
            if( f ){
                char buf[1000+1];
                while( fgets( buf, 1000, f ) ){
                    int len = (int)strlen(buf), c;
                    while( len > 0 && isspace(buf[len-1]) )
                        len--;
                    buf[len] = '\0';
                    cout << "file " << buf << endl;
                    image = imread( buf, 1 );    
                    if( !image.empty() ){
                        detectAndDraw(image, regressor,scale, tryflip );
                        c = waitKey(0);
                        if( c == 27 || c == 'q' || c == 'Q' )
                            break;
                    }
                    else{
                        cerr << "Aw snap, couldn't read image " << buf << endl;
                    }
                }
                fclose(f);
            }
        }
    }

    cvDestroyWindow("result");

    return 0;
}




Mat detectAndDraw(Mat& img,LBFRegressor& regressor,double scale, bool tryflip) {
	Mat myclip;
	int i = 0;
	double t = 0;
	vector<Rect> faces, faces2;
	const static Scalar colors[] = { CV_RGB(0,0,255),
		CV_RGB(0,128,255),
		CV_RGB(0,255,255),
		CV_RGB(0,255,0),
		CV_RGB(255,128,0),
		CV_RGB(255,255,0),
		CV_RGB(255,0,0),
		CV_RGB(255,0,255) };
	Mat gray, smallImg(cvRound(img.rows / scale), cvRound(img.cols / scale), CV_8UC1);

	cvtColor(img, gray, CV_BGR2GRAY);
	resize(gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR);
	equalizeHist(smallImg, smallImg);

	// --Detection
	t = (double)cvGetTickCount();
	int * pResults = NULL;
	pResults = facedetect_frontal_surveillance((unsigned char*)(smallImg.ptr(0)), smallImg.cols, smallImg.rows, smallImg.step, 1.2f, 3, 24);
	for (int i = 0; i < (pResults ? *pResults : 0); i++)
	{
		short * p = ((short*)(pResults + 1)) + 6 * i;
		int x = p[0];
		int y = p[1];
		int w = p[2];
		int h = p[3];
		int neighbors = p[4];
		cout << x <<" "<<y<<" "<<w<<" "<<h<<" "<<p[5]<<endl;
		faces.push_back(Rect(x, y, w, h));
	}
	t = (double)cvGetTickCount() - t;
	printf("detection time = %g ms\n", t / ((double)cvGetTickFrequency()*1000.));
	t = (double)cvGetTickCount();
	Mat totalclip;
	Mat oldtotalclip;
	bool pinjie = false;
	for (vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++) {
		Point center;
		Scalar color = colors[i % 8];
		BoundingBox boundingbox;	
		boundingbox.start_x = r->x*scale;
		boundingbox.start_y = r->y*scale;
		boundingbox.width = (r->width - 1)*scale;
		boundingbox.height = (r->height - 1)*scale;
		boundingbox.centroid_x = boundingbox.start_x + boundingbox.width / 2.0;
		boundingbox.centroid_y = boundingbox.start_y + boundingbox.height / 2.0;

		t = (double)cvGetTickCount();
		Mat_<double> current_shape = regressor.Predict(gray, boundingbox, 1);
		t = (double)cvGetTickCount() - t;
		printf("alignment time = %g ms\n", t / ((double)cvGetTickFrequency()*1000.));
		vector<Point2f> temp;

		for (int i = 0; i < global_params.landmark_num; i++) {
			temp.push_back(Point2f(current_shape(i, 0), current_shape(i, 1)));
		}

		myclip = getwarpAffineImg(img, temp);
		if (totalclip.empty())
		{
			totalclip = myclip;
			pinjie = false;

		}
		else
		{
			oldtotalclip = totalclip;
			hconcat(oldtotalclip, myclip,totalclip);
		}

	}
	if (totalclip.size().width > 0 && totalclip.size().height > 0)
	{
		//cv::imshow("result1", totalclip);
		//_sleep(00);
		
	}
	else
	{
		cout << "No face!" << endl;
	}
	return totalclip;
}


//void detectAndDraw( Mat& img, CascadeClassifier& cascade,
//                    LBFRegressor& regressor,
//                    double scale, bool tryflip ){
//	Mat myclip;
//    int i = 0;
//    double t = 0;
//    vector<Rect> faces,faces2;
//    const static Scalar colors[] =  { CV_RGB(0,0,255),
//        CV_RGB(0,128,255),
//        CV_RGB(0,255,255),
//        CV_RGB(0,255,0),
//        CV_RGB(255,128,0),
//        CV_RGB(255,255,0),
//        CV_RGB(255,0,0),
//        CV_RGB(255,0,255)} ;
//    Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );
//
//    cvtColor( img, gray, CV_BGR2GRAY );
//    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
//    equalizeHist( smallImg, smallImg );
//	
//    // --Detection
//    t = (double)cvGetTickCount();
//	int * pResults = NULL;
//	pResults= facedetect_frontal((unsigned char*)(smallImg.ptr(0)), smallImg.cols, smallImg.rows, smallImg.step,1.2f, 3,  24);
//    //cascade.detectMultiScale( smallImg, faces,
//    //    1.1, 2, 0
//    //    //|CV_HAAR_FIND_BIGGEST_OBJECT
//    //    //|CV_HAAR_DO_ROUGH_SEARCH
//    //    |CV_HAAR_SCALE_IMAGE
//    //    ,
//    //    Size(30, 30) );
//
//    if( tryflip ){
//        flip(smallImg, smallImg, 1);
//		pResults = facedetect_frontal((unsigned char*)(smallImg.ptr(0)), smallImg.cols, smallImg.rows, smallImg.step, 1.2f, 3, 24);
//			for(int i = 0; i < (pResults ? *pResults : 0); i++)
//			{
//		        short * p = ((short*)(pResults+1))+6*i;
//				int x = p[0];
//				int y = p[1];
//				int w = p[2];
//				int h = p[3];
//				int neighbors = p[4];
//				cout << i << endl;
//				faces.push_back(Rect(smallImg.cols-x-w,y,w,h));
//			}
//		//cascade.detectMultiScale( smallImg, faces2,
//        //                         1.1, 2, 0
//        //                         //|CV_HAAR_FIND_BIGGEST_OBJECT
//        //                         //|CV_HAAR_DO_ROUGH_SEARCH
//        //                         |CV_HAAR_SCALE_IMAGE
//        //                         ,
//        //                         Size(30, 30) );
//    //    for( vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); r++ )
//    //    {
//    //        faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
//    //    }
//    }
//    t = (double)cvGetTickCount() - t;
//    printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
//    
//    // --Alignment
//    t =(double)cvGetTickCount();
//    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ ){
//        Point center;
//        Scalar color = colors[i%8];
//        BoundingBox boundingbox;
//        
//        boundingbox.start_x = r->x*scale;
//        boundingbox.start_y = r->y*scale;
//        boundingbox.width   = (r->width-1)*scale;
//        boundingbox.height  = (r->height-1)*scale;
//        boundingbox.centroid_x = boundingbox.start_x + boundingbox.width/2.0;
//        boundingbox.centroid_y = boundingbox.start_y + boundingbox.height/2.0;
//        
//        t =(double)cvGetTickCount();
//        Mat_<double> current_shape = regressor.Predict(gray,boundingbox,1);
//        t = (double)cvGetTickCount() - t;
//        printf( "alignment time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
////        // draw bounding box
//     // rectangle(img, cvPoint(boundingbox.start_x,boundingbox.start_y),
//     //             cvPoint(boundingbox.start_x+boundingbox.width,boundingbox.start_y+boundingbox.height),Scalar(0,255,0), 1, 8, 0);
//        // draw result :: red
//        for(int i = 0;i < global_params.landmark_num;i++){
//             circle(img,Point2d(current_shape(i,0),current_shape(i,1)),3,Scalar(255,255,255),-1,8,0);
//        }
//		vector<Point2f> temp;
//
//		for (int i = 0; i < global_params.landmark_num; i++) {
//			temp.push_back(Point2f(current_shape(i, 0), current_shape(i, 1)));
//		}
//
//		myclip=getwarpAffineImg(img,temp);
//		if(myclip.size().width>0&&myclip.size().height>0)
//		cv::imshow("result1",myclip);
//		_sleep(15);//
//
//    }
//    cv::imshow( "result", img );
//	
//   // char a = waitKey(0);
//   // if(a=='s'){
//    //    save_count++;
//   //     imwrite(to_string(save_count)+".jpg", img);
//  //  }
//}
