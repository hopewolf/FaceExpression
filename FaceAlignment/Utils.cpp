#include "string.h"

#include "LBF.h"
#include "LBFRegressor.h"

using namespace std;
using namespace cv;




Mat_<double> GetMeanShape(const vector<Mat_<double> >& shapes,
                          const vector<BoundingBox>& bounding_box){
	cout << shapes[0].rows << endl;
    Mat_<double> result = Mat::zeros(shapes[0].rows,2,CV_64FC1);
    for(int i = 0;i < shapes.size();i++){
       
        result = result + ProjectShape(shapes[i],bounding_box[i]);
		cout << i << endl;
    }
    result = 1.0 / shapes.size() * result;
    return result;
}

void GetShapeResidual(const vector<Mat_<double> >& ground_truth_shapes,
                      const vector<Mat_<double> >& current_shapes,
                      const vector<BoundingBox>& bounding_boxs,
                      const Mat_<double>& mean_shape,
                      vector<Mat_<double> >& shape_residuals){
    
    Mat_<double> rotation;
    double scale;
    shape_residuals.resize(bounding_boxs.size());
    for (int i = 0;i < bounding_boxs.size(); i++){
        shape_residuals[i] = ProjectShape(ground_truth_shapes[i], bounding_boxs[i])
        - ProjectShape(current_shapes[i], bounding_boxs[i]);
        SimilarityTransform(mean_shape, ProjectShape(current_shapes[i],bounding_boxs[i]),rotation,scale);
        transpose(rotation,rotation);
        shape_residuals[i] = scale * shape_residuals[i] * rotation;
    }
}


Mat_<double> ProjectShape(const Mat_<double>& shape, const BoundingBox& bounding_box){
    Mat_<double> temp(shape.rows,2);
    for(int j = 0;j < shape.rows;j++){
        temp(j,0) = (shape(j,0)-bounding_box.centroid_x) / (bounding_box.width / 2.0);
        temp(j,1) = (shape(j,1)-bounding_box.centroid_y) / (bounding_box.height / 2.0);  
    } 
    return temp;  
}

Mat_<double> ReProjectShape(const Mat_<double>& shape, const BoundingBox& bounding_box){
    Mat_<double> temp(shape.rows,2);
    for(int j = 0;j < shape.rows;j++){
        temp(j,0) = (shape(j,0) * bounding_box.width / 2.0 + bounding_box.centroid_x);
        temp(j,1) = (shape(j,1) * bounding_box.height / 2.0 + bounding_box.centroid_y);
    } 
    return temp; 
}


void SimilarityTransform(const Mat_<double>& shape1, const Mat_<double>& shape2, 
                         Mat_<double>& rotation,double& scale){
    rotation = Mat::zeros(2,2,CV_64FC1);
    scale = 0;
    
    // center the data
    double center_x_1 = 0;
    double center_y_1 = 0;
    double center_x_2 = 0;
    double center_y_2 = 0;
    for(int i = 0;i < shape1.rows;i++){
        center_x_1 += shape1(i,0);
        center_y_1 += shape1(i,1);
        center_x_2 += shape2(i,0);
        center_y_2 += shape2(i,1); 
    }
    center_x_1 /= shape1.rows;
    center_y_1 /= shape1.rows;
    center_x_2 /= shape2.rows;
    center_y_2 /= shape2.rows;
    
    Mat_<double> temp1 = shape1.clone();
    Mat_<double> temp2 = shape2.clone();
    for(int i = 0;i < shape1.rows;i++){
        temp1(i,0) -= center_x_1;
        temp1(i,1) -= center_y_1;
        temp2(i,0) -= center_x_2;
        temp2(i,1) -= center_y_2;
    }

     
    Mat_<double> covariance1, covariance2;
    Mat_<double> mean1,mean2;
    // calculate covariance matrix
    calcCovarMatrix(temp1,covariance1,mean1,CV_COVAR_COLS);
    calcCovarMatrix(temp2,covariance2,mean2,CV_COVAR_COLS);

    double s1 = sqrt(norm(covariance1));
    double s2 = sqrt(norm(covariance2));
    scale = s1 / s2; 
    temp1 = 1.0 / s1 * temp1;
    temp2 = 1.0 / s2 * temp2;

    double num = 0;
    double den = 0;
    for(int i = 0;i < shape1.rows;i++){
        num = num + temp1(i,1) * temp2(i,0) - temp1(i,0) * temp2(i,1);
        den = den + temp1(i,0) * temp2(i,0) + temp1(i,1) * temp2(i,1);      
    }
    
    double norm = sqrt(num*num + den*den);    
    double sin_theta = num / norm;
    double cos_theta = den / norm;
    rotation(0,0) = cos_theta;
    rotation(0,1) = -sin_theta;
    rotation(1,0) = sin_theta;
    rotation(1,1) = cos_theta;
}

double calculate_covariance(const vector<double>& v_1, 
                            const vector<double>& v_2){
    Mat_<double> v1(v_1);
    Mat_<double> v2(v_2);
    double mean_1 = mean(v1)[0];
    double mean_2 = mean(v2)[0];
    v1 = v1 - mean_1;
    v2 = v2 - mean_2;
    return mean(v1.mul(v2))[0]; 
}
Mat_<double> LoadGroundTruthShape(string& filename){
    Mat_<double> shape(global_params.landmark_num,2);
    ifstream fin;
    string temp;
    
    fin.open(filename);
    getline(fin, temp);
    getline(fin, temp);
    getline(fin, temp);
    for (int i=0;i<global_params.landmark_num;i++){
        fin >> shape(i,0) >> shape(i,1);
    }
    fin.close();
    return shape;
    
}
BoundingBox CalculateBoundingBox(Mat_<double>& shape){
    BoundingBox bbx;
    double left_x = 10000;
    double right_x = 0;
    double top_y = 10000;
    double bottom_y = 0;
    for (int i=0; i < shape.rows;i++){
        if (shape(i,0) < left_x)
            left_x = shape(i,0);
        if (shape(i,0) > right_x)
            right_x = shape(i,0);
        if (shape(i,1) < top_y)
            top_y = shape(i,1);
        if (shape(i,1) > bottom_y)
            bottom_y = shape(i,1);
    }
    bbx.start_x = left_x;
    bbx.start_y = top_y;
    bbx.height  = bottom_y - top_y;
    bbx.width   = right_x - left_x;
    bbx.centroid_x = bbx.start_x + bbx.width/2.0;
    bbx.centroid_y = bbx.start_y + bbx.height/2.0;
    return bbx;
}
void adjustImage(Mat_<uchar>& img,
                 Mat_<double>& ground_truth_shape,
                 BoundingBox& bounding_box){
    double left_x  = max(1.0, bounding_box.centroid_x - bounding_box.width*2/3);
    double top_y   = max(1.0, bounding_box.centroid_y - bounding_box.height*2/3);
    double right_x = min(img.cols-1.0,bounding_box.centroid_x+bounding_box.width);
    double bottom_y= min(img.rows-1.0,bounding_box.centroid_y+bounding_box.height);
    img = img.rowRange((int)top_y,(int)bottom_y).colRange((int)left_x,(int)right_x).clone();
    
    bounding_box.start_x = bounding_box.start_x-left_x;
    bounding_box.start_y = bounding_box.start_y-top_y;
    bounding_box.centroid_x = bounding_box.start_x + bounding_box.width/2.0;
    bounding_box.centroid_y = bounding_box.start_y + bounding_box.height/2.0;
    
    for(int i=0;i<ground_truth_shape.rows;i++){
        ground_truth_shape(i,0) = ground_truth_shape(i,0)-left_x;
        ground_truth_shape(i,1) = ground_truth_shape(i,1)-top_y;
    }
}
void LoadData(string filepath,
              vector<Mat_<uchar> >& images,
              vector<Mat_<double> >& ground_truth_shapes,
              vector<BoundingBox> & bounding_boxs
              ){
    ifstream fin;
    fin.open(filepath);
    string name;
    while(getline(fin,name)){
        name.erase(0, name.find_first_not_of(" \t"));
        name.erase(name.find_last_not_of(" \t") + 1);
        cout << "file:" << name <<endl;
        
        // Read Image
        Mat_<uchar> image = imread(name,0);
        images.push_back(image);
        
        // Read ground truth shapes
        name.replace(name.find_last_of("."), 4,".pts");
        Mat_<double> ground_truth_shape = LoadGroundTruthShape(name);
        ground_truth_shapes.push_back(ground_truth_shape);
            
        // Read Bounding box
        BoundingBox bbx = CalculateBoundingBox(ground_truth_shape);
        bounding_boxs.push_back(bbx);
    }
    fin.close();
}

bool IsShapeInRect(Mat_<double>& shape, Rect& rect,double scale){
    double sum1 = 0;
    double sum2 = 0;
    double max_x=0,min_x=10000,max_y=0,min_y=10000;
    for (int i= 0;i < shape.rows;i++){
        if (shape(i,0)>max_x) max_x = shape(i,0);
        if (shape(i,0)<min_x) min_x = shape(i,0);
        if (shape(i,1)>max_y) max_y = shape(i,1);
        if (shape(i,1)<min_y) min_y = shape(i,1);
        
        sum1 += shape(i,0);
        sum2 += shape(i,1);
    }
    if ((max_x-min_x)>rect.width*1.5){
        return false;
    }
    if ((max_y-min_y)>rect.height*1.5){
        return false;
    }
    if (abs(sum1/shape.rows - (rect.x+rect.width/2.0)*scale) > rect.width*scale/2.0){
        return false;
    }
    if (abs(sum2/shape.rows - (rect.y+rect.height/2.0)*scale) > rect.height*scale/2.0){
        return false;
    }
    return true;
}

void LoadOpencvBbxData(string filepath,
                       vector<Mat_<uchar> >& images,
                       vector<Mat_<double> >& ground_truth_shapes,
                       vector<BoundingBox> & bounding_boxs
              ){
    ifstream fin;
	cout << "ok" << endl;
    fin.open(filepath);

    CascadeClassifier cascade;
    double scale = 1.3;
    extern string cascadeName;
    vector<Rect> faces;
    Mat gray;
    // --Detection
	cout << cascadeName<< endl;
    cascade.load(cascadeName);
	
	
    string name;
    while(getline(fin,name)){
        name.erase(0, name.find_first_not_of(" \t"));
        name.erase(name.find_last_not_of(" \t") + 1);
        cout << "file:" << name <<endl;
        // Read Image
        Mat_<uchar> image = imread(name,0);
        
        
        // Read ground truth shapes
        name.replace(name.find_last_of("."), 4,".pts");
        Mat_<double> ground_truth_shape = LoadGroundTruthShape(name);
        
        // Read OPencv Detection Bbx
        Mat smallImg( cvRound (image.rows/scale), cvRound(image.cols/scale), CV_8UC1 );
        resize( image, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
        equalizeHist( smallImg, smallImg );
        
        // --Detection
        cascade.detectMultiScale( smallImg, faces,
                                 1.1, 2, 0
                                 //|CV_HAAR_FIND_BIGGEST_OBJECT
                                 //|CV_HAAR_DO_ROUGH_SEARCH
                                 |CV_HAAR_SCALE_IMAGE
                                 ,
                                 Size(30, 30) );

        for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++){
            Rect rect = *r;

            if (IsShapeInRect(ground_truth_shape,rect,scale)){
                Point center;
                BoundingBox boundingbox;
                
                boundingbox.start_x = r->x*scale;
                boundingbox.start_y = r->y*scale;
                boundingbox.width   = (r->width-1)*scale;
                boundingbox.height  = (r->height-1)*scale;
                boundingbox.centroid_x = boundingbox.start_x + boundingbox.width/2.0;
                boundingbox.centroid_y = boundingbox.start_y + boundingbox.height/2.0;
                
                
                adjustImage(image,ground_truth_shape,boundingbox);
                images.push_back(image);
                ground_truth_shapes.push_back(ground_truth_shape);
                bounding_boxs.push_back(boundingbox);
			
//                // add train data
             /*   bounding_boxs.push_back(boundingbox);
                images.push_back(image);
                ground_truth_shapes.push_back(ground_truth_shape);
                
                rectangle(image, cvPoint(boundingbox.start_x,boundingbox.start_y),
                          cvPoint(boundingbox.start_x+boundingbox.width,boundingbox.start_y+boundingbox.height),Scalar(0,255,0), 1, 8, 0);
                for (int i = 0;i<ground_truth_shape.rows;i++){
                    circle(image,Point2d(ground_truth_shape(i,0),ground_truth_shape(i,1)),1,Scalar(255,0,0),-1,8,0);

                }
                imshow("bbx",image);
                cvWaitKey(0);*/
                break;
            }
        }
    }
	cout << images.size() << endl;
	cout<<ground_truth_shapes.size() << endl;
	cout << bounding_boxs.size() << endl;

    fin.close();
}
double CalculateError(const Mat_<double>& ground_truth_shape, const Mat_<double>& predicted_shape){
    Mat_<double> temp;
    temp = ground_truth_shape.rowRange(36, 41)-ground_truth_shape.rowRange(42, 47);
    double x =mean(temp.col(0))[0];
    double y = mean(temp.col(1))[1];
    double interocular_distance = sqrt(x*x+y*y);
    double sum = 0;
    for (int i=0;i<ground_truth_shape.rows;i++){
        sum += norm(ground_truth_shape.row(i)-predicted_shape.row(i));
    }
    return sum/(ground_truth_shape.rows*interocular_distance);
}

void LoadCofwTrainData(vector<Mat_<uchar> >& images,
                       vector<Mat_<double> >& ground_truth_shapes,
                       vector<BoundingBox>& bounding_boxs){
    int img_num = 1345;
    cout<<"Read images..."<<endl;
    for(int i = 0;i < img_num;i++){
        string image_name = "/Users/lequan/workspace/xcode/myopencv/COFW_Dataset/trainingImages/";
        image_name = image_name + to_string(i+1) + ".jpg";
        Mat_<uchar> temp = imread(image_name,0);
        images.push_back(temp);
    }
    
    ifstream fin;
    fin.open("/Users/lequan/workspace/xcode/myopencv/COFW_Dataset/boundingbox.txt");
    for(int i = 0;i < img_num;i++){
        BoundingBox temp;
        fin>>temp.start_x>>temp.start_y>>temp.width>>temp.height;
        temp.centroid_x = temp.start_x + temp.width/2.0;
        temp.centroid_y = temp.start_y + temp.height/2.0; 
        bounding_boxs.push_back(temp);
    }
    fin.close(); 

    fin.open("/Users/lequan/workspace/xcode/myopencv/COFW_Dataset/keypoints.txt");
    for(int i = 0;i < img_num;i++){
        Mat_<double> temp(global_params.landmark_num,2);
        for(int j = 0;j < global_params.landmark_num;j++){
            fin>>temp(j,0); 
        }
        for(int j = 0;j < global_params.landmark_num;j++){
            fin>>temp(j,1); 
        }
        ground_truth_shapes.push_back(temp);
    }
    fin.close();
}
void LoadCofwTestData(vector<Mat_<uchar> >& images,
                      vector<Mat_<double> >& ground_truth_shapes,
                      vector<BoundingBox>& bounding_boxs){
    int img_num = 507;
    cout<<"Read images..."<<endl;
    for(int i = 0;i < img_num;i++){
        string image_name = "/Users/lequan/workspace/xcode/myopencv/COFW_Dataset/testImages/";
        image_name = image_name + to_string(i+1) + ".jpg";
        Mat_<uchar> temp = imread(image_name,0);
        images.push_back(temp);
    }
    
    ifstream fin;
    fin.open("/Users/lequan/workspace/xcode/myopencv/COFW_Dataset/boundingbox_test.txt");
    for(int i = 0;i < img_num;i++){
        BoundingBox temp;
        fin>>temp.start_x>>temp.start_y>>temp.width>>temp.height;
        temp.centroid_x = temp.start_x + temp.width/2.0;
        temp.centroid_y = temp.start_y + temp.height/2.0;
        bounding_boxs.push_back(temp);
    }
    fin.close();
    
    fin.open("/Users/lequan/workspace/xcode/myopencv/COFW_Dataset/keypoints_test.txt");
    for(int i = 0;i < img_num;i++){
        Mat_<double> temp(global_params.landmark_num,2);
        for(int j = 0;j < global_params.landmark_num;j++){
            fin>>temp(j,0);
        }
        for(int j = 0;j < global_params.landmark_num;j++){
            fin>>temp(j,1);
        }
        ground_truth_shapes.push_back(temp);
    }
    fin.close();
}

void LoadDataAdjust(string filepath,
              vector<Mat_<uchar> >& images,
              vector<Mat_<double> >& ground_truth_shapes,
              vector<BoundingBox> & bounding_boxs
              ){
    ifstream fin;
    fin.open(filepath);
    string name;
    while(getline(fin,name)){
        name.erase(0, name.find_first_not_of(" \t"));
        name.erase(name.find_last_not_of(" \t") + 1);
        cout << "file:" << name <<endl;
        
        // Read Image
        Mat_<uchar> image = imread(name,0);
        
        // Read ground truth shapes
        name.replace(name.find_last_of("."), 4,".pts");
        Mat_<double> ground_truth_shape = LoadGroundTruthShape(name);
       
        // Read Bounding box
        BoundingBox bbx = CalculateBoundingBox(ground_truth_shape);
        adjustImage(image,ground_truth_shape,bbx);
        images.push_back(image);
        ground_truth_shapes.push_back(ground_truth_shape);    
        bounding_boxs.push_back(bbx);
    }
    fin.close();
}



//根据眼睛坐标对图像进行仿射变换
//src - 原图像
//landmarks - 原图像中68个关键点
Mat getwarpAffineImg(Mat &src, vector<Point2f> &landmarks)
{
	const int left = 36;
	const int right = 45;
	const int widthResult = 110;
	const int heightResult = 150;
	cv::Mat oral; src.copyTo(oral);
	for (int j = 0; j < landmarks.size(); j++)
	{
		circle(oral, landmarks[j], 2, Scalar(255, 0, 0));
	}
	//计算两眼中心点，按照此中心点进行旋转， 第36个为左眼坐标，45为右眼坐标

	Point2f eyesCenter = Point2f((landmarks[right].x + landmarks[left].x) * 0.5f, (landmarks[right].y + landmarks[left].y) * 0.5f);

	// 计算两个眼睛间的角度
	double dy = (landmarks[right].y - landmarks[left].y);
	double dx = (landmarks[right].x - landmarks[left].x);
	double angle = atan2(dy, dx) * 180.0 / CV_PI; // Convert from radians to degrees.

	

												  //由eyesCenter, andle, scale按照公式计算仿射变换矩阵，此时1.0表示不进行缩放
	cv::Mat rot_mat = getRotationMatrix2D(eyesCenter, angle, 1.0);

	cv::Mat rot;
	// 进行仿射变换，变换后大小为src的大小
	warpAffine(src, rot, rot_mat, src.size());
	//warpAffine(src, rot, rot_mat,Size(110,150));
	vector<Point2f> marks;

	//按照仿射变换矩阵，计算变换后各关键点在新图中所对应的位置坐标。
	for (int n = 0; n<landmarks.size(); n++)
	{
		Point2f p = Point2f(0, 0);
		p.x = rot_mat.ptr<double>(0)[0] * landmarks[n].x + rot_mat.ptr<double>(0)[1] * landmarks[n].y + rot_mat.ptr<double>(0)[2];
		p.y = rot_mat.ptr<double>(1)[0] * landmarks[n].x + rot_mat.ptr<double>(1)[1] * landmarks[n].y + rot_mat.ptr<double>(1)[2];
		marks.push_back(p);
	}
	//标出关键点

	double eyewidth = marks[45].x - marks[36].x;
	//求出人眼距离


	int maxheight=0;
	int maxwidth = 0;
	Point2f Facelocation=Point2f(marks[0].x,marks[0].y);
	for (int m = 0; m< landmarks.size(); m++)
	{
		if (marks[m].x <Facelocation.x)
			Facelocation.x = marks[m].x;
		if (marks[m].y <Facelocation.y)
			Facelocation.y = marks[m].y;
		
		for (int n = m; n < landmarks.size(); n++)
		{
			maxheight = maxheight > abs(marks[m].y - marks[n].y) ? maxheight : abs(marks[m].y - marks[n].y);
			maxwidth = maxwidth > abs(marks[m].x - marks[n].x) ? maxwidth : abs(marks[m].x - marks[n].x);
		}
	}
	//cout << "eyewidth:" << eyewidth << endl;
	//cout << maxheight << " " << maxwidth << endl;

	for (int j = 0; j < landmarks.size(); j++)
	{
		

		//	circle(rot, marks[j], 2, Scalar(0, 255, 0));	
	}

//	return rot;

	Mat resultRot0(150, 160, CV_8UC3, cv::Scalar(0, 0, 255));
	Mat resultRot1(150, 160, CV_8UC3, cv::Scalar(0, 0, 255));
	if (((Facelocation.x + maxwidth) < rot.size().width) && (((Facelocation.y + maxheight ))< rot.size().height))
	{
		//cout <<"width:"<< maxwidth << " height" << maxheight - 1 << endl;
		//cout << rot.size() << endl;
		//cout << Facelocation.x << " + " << maxwidth << " = " << Facelocation.x + maxwidth << endl;
		//cout<< Facelocation.y << " + "<< maxheight <<" = " << Facelocation.y + maxheight << endl;
		resultRot0 = rot(Rect(Facelocation.x, Facelocation.y, maxwidth , maxheight ));
	}


	resize(resultRot0, resultRot1, Size(150,160));
	return resultRot1;

}

