#pragma once
#include <windows.h>
#include<winnt.h>
#include "cv.h"
template <class T>
T**	New2DPointer(int n1D, int n2D)
{
	T**		pp;
	typedef		T*	T_P;
	int		i, j;
	if (n1D <= 0 || n2D <= 0)
		return FALSE;

	pp = new T_P[n1D];
	if (!pp)
		return NULL;
	for (i = 0; i<n1D; i++)
	{
		pp[i] = new T[n2D];
		if (!pp[i])
		{
			for (j = 0; j<i; j++)
			{
				delete	pp[j];
			}
			delete	pp;
			return NULL;
		}
	}
	return pp;
}//New2DPointer

template <class T>
void	Delete2DPointer(T **pp, int n1D)
{
	int		i;
	if (pp == NULL)
		return;
	for (i = 0; i<n1D; i++)
	{
		if (pp[i])
			delete[] pp[i];// the original is "delete pp[i];", add [] after delete to avoid the memory leak
	}
	delete[]	pp; // the original is "delete pp; ", add [] after delete to avoid the memory leak
	pp = NULL;
}//Delete2DPointer

#define	BOUND(x, lowerbound, upperbound)  { (x) = (x) > (lowerbound) ? (x) : (lowerbound); \
                                            (x) = (x) < (upperbound) ? (x) : (upperbound); };
typedef   unsigned char BYTE;
#define PI 3.1415926535897932
#define POW(nBit)   (1 << (nBit))
#define FREE(ptr) 	{if (NULL!=(ptr)) {delete[] ptr;  ptr=NULL;}}
#define MAXNUMIMAGE 100
void Test2(char file[MAXNUMIMAGE][MAX_PATH], int len, char* lpath);
int ProcssEmotion(char * lpPath);

class LBFRegressor;
void ProcessDirImage(char * lpPath, void(*handle)(char imagefile[MAXNUMIMAGE][MAX_PATH], int len, char *lpath, LBFRegressor &regressor));
void ProcessDirImage(char * lpPath, void(*handle)(char imagefile[MAXNUMIMAGE][MAX_PATH], int len, char *lpath));
void ListImage(char * lpPath, void(*handle)(char imagefile[MAXNUMIMAGE][MAX_PATH], int len, char *lpath));
void ListImage(char * lpPath, void(*handle)(char imagefile[MAXNUMIMAGE][MAX_PATH], int len, char *lpath, LBFRegressor &regressor), LBFRegressor &regressor);

// simple example how to computer LBP-TOP and VLBP.
void Test();
//void Output(FILE * fp, char* lpath, float ** result, int Dim, int emotion);

//void Output2(FILE * fp, char *lpath, float * result, int Dim, int emotion);

// For computing the LBP-TOP.
void LBP_TOP(BYTE ***fg, int Length, int height, int width,
	int FxRadius, int FyRadius, int TInterval,
	int XYNeighborPoints, int XTNeighborPoints, int YTNeighborPoints,
	int TimeLength, int BoderLength, int Bincount,
	bool bBilinearInterpolation, float **Histogram);

// For computing basic VLBP and two rotation invariant VLBP.
void RIVLBP(BYTE ***fg, int Length, int height, int width,
	int TInterval, int FRadius, int NeighborPoints,
	int BoderLength, int TimeLength, int RotateIndex,
	bool bBilinearInterpolation, float *Histogram);

int RotLBP(int LBPCode, int NeighborPoints);
void ComputeRotationInvariance(int RotateIndex, int NeighborPoints,
	int tempLBPpre, int tempLBPcur, int tempLBPpos,
	int tempLBPpreC, int tempLBPposC, int binCount,
	int& BasicLBP);

void GetLbp(cv::Mat face[3], const int &fnum);
void PredictFromCam();

