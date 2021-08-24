#include <stdio.h>
#include <string>

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

#include "DynamsoftLabelRecognizer.h"
#include "DynamsoftCore.h"

#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>

using namespace std;
using namespace dynamsoft::dlr;
using namespace cv;

Mat ori, current;
const char* windowName = "Passport MRZ Recognition";
CLabelRecognizer dlr;
const int maxHeight = 1200, maxWidth = 1200;
int thickness = 2;
Scalar lineColor = Scalar(255, 0, 0);
Scalar textColor = Scalar(0, 0, 255);
Mat before, after;

double hScale = 1.0, wScale = 1.0, scale = 1.0;
int skip = 40;

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
		// imshow(windowName, newMat);
		// imwrite(windowName + ".jpg", newMat);

		return newMat;
	}
	else if (hScale <= wScale && wScale > 1)
	{
		Mat newMat;
		resize(img, newMat, Size(int(imgWidth / wScale), int(imgHeight / wScale)));
		// imshow(windowName, newMat);
		// imwrite(windowName + ".jpg", newMat);
		return newMat;
	}
	else 
	{
		// imshow(windowName, img);
		// imwrite(windowName + ".jpg", img);
	}

	return img;
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
	putText(img, text, Point(x, y), FONT_HERSHEY_COMPLEX, scale, textColor, 1, LINE_AA);
}

void OutputResult(CLabelRecognizer& dlr, int errorcode, float time)
{
	if (errorcode != DM_OK)
		printf("\r\nFailed to recognize label : %s\r\n", dlr.GetErrorString(errorcode));
	else
	{
		DLR_ResultArray* pDLRResults = NULL;
		dlr.GetAllResults(&pDLRResults);
		if (pDLRResults != NULL)
		{
			int rCount = pDLRResults->resultsCount;
			printf("\r\nRecognized %d results\r\n", rCount);
			for (int ri = 0; ri < rCount; ++ri)
			{
				printf("\r\nResult %d :\r\n", ri);
				int startX = 50, startY = 50;
				DLR_Result* result = pDLRResults->results[ri];
				int lCount = result->lineResultsCount;
				for (int li = 0; li < lCount; ++li)
				{
					printf("Line result %d: %s\r\n", li, result->lineResults[li]->text);
					DM_Point *points = result->lineResults[li]->location.points;
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
					drawText(ori, result->lineResults[li]->text, minX, minY - scale * 10);

					startY = minY - scale * 600;
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
				else {
					// https://en.wikipedia.org/wiki/Machine-readable_passport
					// Type
					string tmp = "Type: ";
					tmp.insert(tmp.length(), 1, line1[0]);
					printf("%s\r\n", tmp.c_str());
					drawText(ori, tmp.c_str(), startX, startY);
					startY += skip;

					// Issuing country
					tmp = "Issuing country: ";
					tmp += line1.substr(2, 3);		
					printf("%s\r\n", tmp.c_str());
					drawText(ori, tmp.c_str(), startX, startY);
					startY += skip;

					// Surname
					int index = 5;
					tmp = "Surname: ";
					for (; index < 44; index++)
					{
						if (line1[index] != '<')
						{
							tmp.insert(tmp.length(), 1, line1[index]);
						}
						else 
						{
							break;
						}
					}
					printf("%s\r\n", tmp.c_str());
					drawText(ori, tmp.c_str(), startX, startY);
					startY += skip;

					// Given names
					tmp = "Given Names: ";
					index += 2;
					for (; index < 44; index++)
					{
						if (line1[index] != '<')
						{
							tmp.insert(tmp.length(), 1, line1[index]);
						}
						else 
						{
							tmp.insert(tmp.length(), 1, ' ');
						}
					}
					printf("%s\r\n", tmp.c_str());
					drawText(ori, tmp.c_str(), startX, startY);
					startY += skip;

					// Passport number
					tmp = "Passport number: ";
					index = 0;
					for (; index < 9; index++)
					{
						if (line2[index] != '<')
						{
							tmp.insert(tmp.length(), 1, line2[index]);
						}
						else 
						{
							break;
						}
					}
					printf("%s\r\n", tmp.c_str());
					drawText(ori, tmp.c_str(), startX, startY);
					startY += skip;
 
					// Nationality
					tmp = "Nationality: ";
					tmp += line2.substr(10, 3);
					printf("%s\r\n", tmp.c_str());
					drawText(ori, tmp.c_str(), startX, startY);
					startY += skip;

					// Date of birth
					tmp = line2.substr(13, 6);
					tmp.insert(2, "/");
					tmp.insert(5, "/");
					tmp = "Date of birth (YYMMDD): " + tmp;
					printf("%s\r\n", tmp.c_str());
					drawText(ori, tmp.c_str(), startX, startY);
					startY += skip;

					// Sex
					tmp = "Sex: ";
					tmp.insert(tmp.length(), 1, line2[20]);
					printf("%s\r\n", tmp.c_str());
					drawText(ori, tmp.c_str(), startX, startY);
					startY += skip;

					// Expiration date of passport
					tmp = line2.substr(21, 6);
					tmp.insert(2, "/");
					tmp.insert(5, "/");
					tmp = "Expiration date of passport (YYMMDD): " + tmp;
					printf("%s\r\n", tmp.c_str());
					drawText(ori, tmp.c_str(), startX, startY);
					startY += skip;

					// Personal number
					if (line2[28] != '<')
					{
						tmp = "Personal number: ";
						for (index = 28; index < 42; index++)
						{
							if (line2[index] != '<')
							{
								tmp.insert(tmp.length(), 1, line2[index]);
							}
							else 
							{
								break;
							}
						}
						printf("%s\r\n", tmp.c_str());
						drawText(ori, tmp.c_str(), startX, startY);
						startY += skip;
					}
				}
			}
		}
		else
		{
			printf("\r\nNo data detected.\r\n");
		}
		dlr.FreeResults(&pDLRResults);
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
	char pszImageFile[512] = { 0 };

	printf("*************************************************\r\n");
	printf("Welcome to Dynamsoft Label Recognition Demo\r\n");
	printf("*************************************************\r\n");
	printf("Hints: Please input 'Q' or 'q' to quit the application.\r\n");
	CLabelRecognizer dlr;
	dlr.InitLicense(licenseStr.c_str()); // Get 30-day FREE trial license https://www.dynamsoft.com/customer/license/trialLicense?product=dlr
	int ret = dlr.AppendSettingsFromFile(templateFile.c_str());
	std::cout << "AppendSettingsFromFile: " << ret << std::endl;
	while (1)
	{	
		hScale = 1.0, wScale = 1.0, skip = 50, scale = 1.0;	
		bExit = GetImagePath(pszImageFile);
		if (bExit)
			break;

		// Read an image
		ori = imread(pszImageFile);
		current = ori.clone();

		int imgHeight = ori.rows, imgWidth = ori.cols;
	
		hScale = imgHeight * 1.0 / maxHeight;
		wScale = imgWidth * 1.0 / maxWidth;
		scale = hScale > wScale ? hScale : wScale;
		if (scale > 1) scale = 1;
		skip *= scale;
		int errorCode = 0;

		TickMeter tm;
		tm.start();
		errorCode = dlr.RecognizeByFile(pszImageFile, "locr");
		tm.stop();
		float costTime = tm.getTimeSec();
		std::cout << "Cost time: " << costTime << " s" << std::endl;

		before = showImage("Passport", current);
		OutputResult(dlr, errorCode, costTime);
		Mat newMat;
		hconcat(before, after, newMat);
		imshow("Comparison", newMat);
		destroyWindow();
	}

	return 0;
}
