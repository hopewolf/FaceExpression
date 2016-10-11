//
//  LBF.cpp
//  myopencv
//
//  Created by lequan on 1/24/15.
//  Copyright (c) 2015 lequan. All rights reserved.
//

#include "LBF.h"
#include "LBFRegressor.h"
#include"lbp.h"

using namespace std;
using namespace cv;

// parameters
Params global_params;


string modelPath ="./../model/";
string dataPath = "./../DataSets/";
string cascadeName = "lbpcascade_frontalface.xml";

void InitializeGlobalParam();
void PrintHelp();


int main( int argc, const char** argv ){

	FpOfVlbp = fopen("./resultvlbp.txt", "w");
	FpOfLbpTop = fopen("./resulttop.txt", "w");

    if (argc > 1 && strcmp(argv[1],"TrainModel")==0){
        InitializeGlobalParam();
    }
    else {
        ReadGlobalParamFromFile(modelPath+"LBF.model");
    }
    
    // main process
    if (argc==1){
        PrintHelp();
    }
 else if(strcmp(argv[1],"TrainModel")==0){
        vector<string> trainDataName;
     // you need to modify this section according to your training dataset
       // trainDataName.push_back("afw");
        //trainDataName.push_back("helen");
        trainDataName.push_back("lfpw");
        TrainModel(trainDataName);
    }
    else if (strcmp(argv[1], "TestModel")==0){
		vector<string> testDataName;
        testDataName.push_back("ibug");
        double MRSE = TestModel(testDataName);
        
    }
    else if (strcmp(argv[1], "Demo")==0){
		PredictFromCam();
		return 0;
        if (argc == 2){
            return FaceDetectionAndAlignment("");
        }
        else if(argc ==3){
            return FaceDetectionAndAlignment(argv[2]);
        }
    }
	else if (strcmp(argv[1], "HandleImages") == 0)
	{
		LBFRegressor myregressor;
		myregressor.Load(modelPath + "LBF.model");
		//you need to modify this path to your face store
		ListImage("G:\\extended-cohn-kanade-images\\cohn-kanade-images",cutFace,myregressor);
	}
    else {
        PrintHelp();
    }
    return 0;
}

// set the parameters when training models.
void InitializeGlobalParam(){
    global_params.bagging_overlap = 0.4;
    global_params.max_numtrees = 10;
    global_params.max_depth = 5;
    global_params.landmark_num = 68;
    global_params.initial_num = 5;
    
    global_params.max_numstage = 7;
    double m_max_radio_radius[10] = {0.4,0.3,0.2,0.15, 0.12, 0.10, 0.08, 0.06, 0.06,0.05};
    double m_max_numfeats[10] = {500, 500, 500, 300, 300, 200, 200,200,100,100};
    for (int i=0;i<10;i++){
        global_params.max_radio_radius[i] = m_max_radio_radius[i];
    }
    for (int i=0;i<10;i++){
        global_params.max_numfeats[i] = m_max_numfeats[i];
    }
    global_params.max_numthreshs = 500;
}

void ReadGlobalParamFromFile(string path){
    cout << "Loading GlobalParam..." << endl;
    ifstream fin;
    fin.open(path);
    fin >> global_params.bagging_overlap;
    fin >> global_params.max_numtrees;
    fin >> global_params.max_depth;
    fin >> global_params.max_numthreshs;
    fin >> global_params.landmark_num;
    fin >> global_params.initial_num;
    fin >> global_params.max_numstage;
    
    for (int i = 0; i< global_params.max_numstage; i++){
        fin >> global_params.max_radio_radius[i];
    }
    
    for (int i = 0; i < global_params.max_numstage; i++){
        fin >> global_params.max_numfeats[i];
    }
    cout << "Loading GlobalParam end"<<endl;
    fin.close();
}
void PrintHelp(){
    cout << "Useage:"<<endl;
    cout << "1. train your own model:    LBF.out  TrainModel "<<endl;
    cout << "2. test model on dataset:   LBF.out  TestModel"<<endl;
    cout << "3. test model via a camera: LBF.out  Demo "<<endl;
    cout << "4. test model on a pic:     LBF.out  Demo xx.jpg"<<endl;
    cout << "5. test model on pic set:   LBF.out  Demo Img_Path.txt"<<endl;
    cout << endl;
}
