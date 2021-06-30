#include <stdio.h>
#include <string>
#include <vector>


#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/imgcodecs.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <conio.h>
#include <io.h>
#else
#include <cstring>
#include <dirent.h>
#include <sys/time.h>
#endif

#include "DynamsoftLabelRecognition.h"
#include "DynamsoftCommon.h"

#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>

using namespace std;
using namespace dynamsoft::dlr;
using namespace cv;

Mat ori, current;
Rect region(0,0,0,0);
Point startPoint(0,0), endPoint(0,0);
const char* windowName = "Passport MRZ Recognition";
bool clicked = false;
CLabelRecognition dlr;
int maxHeight = 1200, maxWidth = 1200;
double hScale = 1.0, wScale = 1.0;
int thickness = 2;
int skip = 50;
Scalar lineColor = Scalar(255, 0, 0);
Scalar textColor = Scalar(0, 0, 255);
Mat before, after;

void destroyWindow()
{
	waitKey(0);
	destroyAllWindows();
}

Mat showImage(string windowName, Mat &img)
{
	int imgHeight = ori.rows, imgWidth = ori.cols;

	if (hScale >= wScale && hScale > 1)
	{
		Mat newMat;
		resize(img, newMat, Size(int(imgWidth / hScale), int(imgHeight / hScale)));
		imshow(windowName, newMat);
		imwrite(windowName + ".jpg", newMat);

		return newMat;
	}
	else if (hScale <= wScale && wScale > 1)
	{
		Mat newMat;
		resize(img, newMat, Size(int(imgWidth / wScale), int(imgHeight / wScale)));
		imshow(windowName, newMat);
		imwrite(windowName + ".jpg", newMat);
		return newMat;
	}
	else 
	{
		imshow(windowName, img);
		imwrite(windowName + ".jpg", img);
	}

	return img;
}

vector<string> split(const string& str, const string& delim) {
	vector<string> res;
	if ("" == str) return res;

	char * strs = new char[str.length() + 1]; 
	strcpy(strs, str.c_str());

	char * d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());

	char *p = strtok(strs, d);
	while (p) {
		string s = p;
		res.push_back(s);
		p = strtok(NULL, d);
	}

	return res;
}

bool GetImagePath(char* pImagePath)
{
	char pszBuffer[512] = { 0 };
	bool bExit = false;
	size_t iLen = 0;
	FILE* fp = NULL;
	while (1)
	{
		printf("\r\n>> Step 1: Input your image file's full path:\r\n");
#if defined(_WIN32) || defined(_WIN64)
		gets_s(pszBuffer, 512);
#else
		fgets(pszBuffer, 512, stdin);
		strtok(pszBuffer, "\n");
#endif
		iLen = strlen(pszBuffer);
		if (iLen > 0)
		{
			if (strlen(pszBuffer) == 1 && (pszBuffer[0] == 'q' || pszBuffer[0] == 'Q'))
			{
				bExit = true;
				break;
			}

			memset(pImagePath, 0, 512);
			if ((pszBuffer[0] == '\"' && pszBuffer[iLen - 1] == '\"') || (pszBuffer[0] == '\'' && pszBuffer[iLen - 1] == '\''))
				memcpy(pImagePath, &pszBuffer[1], iLen - 2);
			else
				memcpy(pImagePath, pszBuffer, iLen);

#if defined(_WIN32) || defined(_WIN64)
			int err = fopen_s(&fp, pImagePath, "rb");
			if (err == 0)
			{
				fclose(fp);
				break;
			}
#else
			fp = fopen(pImagePath, "rb");
			if (fp != NULL)
			{
				break;
			}
#endif
		}
		printf("Please input a valid path.\r\n");
	}
	return bExit;
}

void drawText(Mat& img, const char* text, int x, int y) 
{
	putText(img, text, Point(x, y), FONT_HERSHEY_COMPLEX, 1, textColor, 1, LINE_AA);
}

void OutputResult(CLabelRecognition& dlr, int errorcode, float time)
{
	if (errorcode != DLR_OK)
		printf("\r\nFailed to recognize label : %s\r\n", dlr.GetErrorString(errorcode));
	else
	{
		DLRResultArray* pDLRResults = NULL;
		dlr.GetAllDLRResults(&pDLRResults);
		if (pDLRResults != NULL)
		{
			int rCount = pDLRResults->resultsCount;
			printf("\r\nRecognized %d results\r\n", rCount);
			for (int ri = 0; ri < rCount; ++ri)
			{
				printf("\r\nResult %d :\r\n", ri);
				int startX = 50, startY = 50;
				DLRResult* result = pDLRResults->results[ri];
				int lCount = result->lineResultsCount;
				for (int li = 0; li < lCount; ++li)
				{
					printf("Line result %d: %s\r\n", li, result->lineResults[li]->text);
					DLRPoint *points = result->lineResults[li]->location.points;
					printf("x1: %d, y1: %d, x2: %d, y2: %d, x3: %d, y3: %d, x4: %d, y4: %d\r\n", points[0].x, 
					points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, points[3].x, points[3].y);
					int x1 = points[0].x, y1 = points[0].y;
					int minX = x1, minY = y1;
					int x2 = points[1].x, y2 = points[1].y;
					minX = minX < x2 ? minX : x2;
					minY = minY < y2 ? minY : y2;
					int x3 = points[2].x, y3 = points[2].y;
					minX = minX < x3 ? minX : x3;
					minY = minY < y3 ? minY : y3;
					int x4 = points[3].x, y4 = points[3].y;
					minX = minX < x4 ? minX : x4;
					minY = minY < y4 ? minY : y4;
					
					line( ori, Point(x1, y1), Point(x2, y2), lineColor, thickness);
					line( ori, Point(x2, y2), Point(x3, y3), lineColor, thickness);
					line( ori, Point(x3, y3), Point(x4, y4), lineColor, thickness);
					line( ori, Point(x4, y4), Point(x1, y1), lineColor, thickness);
					drawText(ori, result->lineResults[li]->text, minX, minY - 10);

					startY = minY - 600;
				}

				printf("\nPassport Info \r\n");
				if (lCount < 2)
				{
					continue;
				}
				string line1 = result->lineResults[0]->text;
				string line2 = result->lineResults[1]->text;
				if (line1.length() != 44 || line2.length() != 44)
				{
					continue;
				}
				if (line1[0] != 'P')
					continue;
				string tmpString = line1.substr(5);			
				int pos = tmpString.find("<<");
				if (pos > 0)
				{
					string surname = tmpString.substr(0, pos);
					vector<string> surnames = split(surname, "<");
					surname = "";
					for (int i = 0; i < surnames.size(); ++i)
					{
						if (i != surnames.size() - 1)
							surname = surname + surnames[i] + " ";
						else
							surname = surname + surnames[i];
					}
					printf("\tSurname : %s\r\n", surname.c_str());
					drawText(ori, ("Surname: " + surname).c_str(), startX, startY);
					startY += skip;

					string tmp2 = tmpString.substr(pos+2);
					pos = tmp2.find_last_not_of('<');
					if (pos > 0)
					{
						string givenName = tmp2.substr(0, pos+1);
						vector<string> givenNames = split(givenName, "<");
						givenName = "";
						for (int i = 0; i < givenNames.size(); ++i)
						{
							if (i != givenNames.size() - 1)
								givenName = givenName + givenNames[i] + " ";
							else
								givenName = givenName + givenNames[i];
						}
						printf("\tGiven Names : ");

						printf("%s\r\n", givenName.c_str());
						drawText(ori, ("Given name: " + givenName).c_str(), startX, startY);
						startY += skip;
					}
				}
				string na = line1.substr(2, 3);
				if (na.back() == '<')
				{
					na = na.substr(0, 2);
				}
				printf("\tNationality : %s\r\n", na.c_str());
				drawText(ori, ("Nationality: " + na).c_str(), startX, startY);
				startY += skip;

				tmpString = line2.substr(0, 9);
				pos = tmpString.find_first_of('<');
				if (pos > 0)
				{
					tmpString = tmpString.substr(0, pos);
				}
				printf("\tPassport Number : %s\r\n", tmpString.c_str());
				drawText(ori, ("Passport Number: " + tmpString).c_str(), startX, startY);
				startY += skip;

				tmpString = line2.substr(10, 3);
				if (tmpString.back() == '<')
				{
					tmpString = tmpString.substr(0, 2);
				}
				printf("\tIssuing Country or Organization : %s\r\n", tmpString.c_str());
				drawText(ori, ("Issuing Country or Organization:" + tmpString).c_str(), startX, startY);
				startY += skip;

				tmpString = line2.substr(13,6);
				tmpString.insert(2,"-");
				tmpString.insert(5,"-");
				printf("\tDate of Birth(YY-MM-DD) : %s\r\n", tmpString.c_str());
				drawText(ori, ("Date of Birth(YY-MM-DD): " + tmpString).c_str(), startX, startY);
				startY += skip;
				/*if(line2[20] == 'F' )
					tmpString = "Female";
				if (line2[20] == 'M')
					tmpString = "Male";*/
				printf("\tSex/Gender : %c\r\n", line2[20]);
				string gender = "Sex/Gender :";
				gender.insert(gender.length(), 1, line2[20]);
				drawText(ori, gender.c_str(), startX, startY);
				startY += skip;

				tmpString = line2.substr(21, 6);
				tmpString.insert(2, "-");
				tmpString.insert(5, "-");
				printf("\tPassport Expiration Date(YY-MM-DD) : %s\r\n", tmpString.c_str());
				drawText(ori, ("Passport Expiration Date(YY-MM-DD):" + tmpString).c_str(), startX, startY);
				startY += skip;
			}
		}
		else
		{
			printf("\r\nNo data detected.\r\n");
		}
		dlr.FreeDLRResults(&pDLRResults);
	}

	printf("\r\nTotal time spent: %.3f s\r\n", time);
	after = showImage(windowName, ori);
}

int main(int argc, const char* argv[])
{
	if (argc < 2) {
		printf("Usage: mrz license.txt template.json\n");
        return 0;
	}

	// License file
	std::ifstream licenseFile(argv[1]);
	std::stringstream strStream;
    strStream << licenseFile.rdbuf(); 
    std::string licenseStr = strStream.str(); 

	// Template file
	string templateFile = "wholeImgMRZTemplate.json";
	if (argc == 3) {
		templateFile = argv[2];
	}

	bool bExit = false;
	char szErrorMsg[512];
	char pszImageFile[512] = { 0 };
	bool autoRegion = false;
	tagDLRPoint region[4] = { { 0,0 },{ 100,0 },{ 100,100 },{ 0,100 } };
#if defined(_WIN32) || defined(_WIN64)
	unsigned _int64 ullTimeBegin = 0;
	unsigned _int64 ullTimeEnd = 0;
#else
	struct timeval ullTimeBegin, ullTimeEnd;
#endif


	printf("*************************************************\r\n");
	printf("Welcome to Dynamsoft Label Recognition Demo\r\n");
	printf("*************************************************\r\n");
	printf("Hints: Please input 'Q' or 'q' to quit the application.\r\n");

	CLabelRecognition dlr;
	dlr.InitLicense(licenseStr.c_str()); // Get 30-day FREE trial license https://www.dynamsoft.com/customer/license/trialLicense?product=dlr
	int ret = dlr.AppendSettingsFromFile(templateFile.c_str());
	while (1)
	{	
		bExit = GetImagePath(pszImageFile);
		if (bExit)
			break;

		// Read an image
		ori = imread(pszImageFile);
		current = ori.clone();
		namedWindow(windowName);

		int imgHeight = ori.rows, imgWidth = ori.cols;
	
		if (imgHeight > maxHeight) 
		{
			hScale = imgHeight * 1.0 / maxHeight;
			// thickness = 6;
		}
			

		if (imgWidth > maxWidth)
		{
			wScale = imgWidth * 1.0 / maxWidth;
			// thickness = 6;
		}

		float costTime = 0.0;
		int errorCode = 0;

#if defined(_WIN32) || defined(_WIN64)
		ullTimeBegin = GetTickCount();
		errorCode = dlr.RecognizeByFile(pszImageFile, "locr");
		ullTimeEnd = GetTickCount();
		costTime = ((float)(ullTimeEnd - ullTimeBegin)) / 1000;
#else
		gettimeofday(&ullTimeBegin, NULL);
		errorCode = dlr.RecognizeByFile(pszImageFile, "locr");
		gettimeofday(&ullTimeEnd, NULL);
		costTime = (float)((ullTimeEnd.tv_sec * 1000 * 1000 + ullTimeEnd.tv_usec) - (ullTimeBegin.tv_sec * 1000 * 1000 + ullTimeBegin.tv_usec)) / (1000 * 1000);
#endif
		before = showImage("Passport", current);
		OutputResult(dlr, errorCode, costTime);
		Mat newMat;
		hconcat(before, after, newMat);
		imshow("Comparison", newMat);
		destroyWindow();
	}

	return 0;
}
