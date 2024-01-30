#include "InputManager.h"
#include "../DisplayGLFW/display.h"
#include "game.h"
#include "../res/includes/glm/glm.hpp"
#include "stb_image.h"
#include "vector"
#include <cmath>
#include <queue>
#include <filesystem>
#include <fstream>
#include <iostream>
#define M_PI 3.14159265358979323846
using namespace std;

unsigned char* convertToFloydSteinberg(unsigned char* imageData, int width, int height) {
	unsigned char* newData = (unsigned char*)malloc(width * height * 4);

	// Copy the original image data to newData
	memcpy(newData, imageData, width * height * 4);

	for (int y = 0; y < height - 1; y++) {
		for (int x = 1; x < width - 1; x++) {
			int pixelIndex = (y * width + x) * 4;

			// Old pixel values
			double oldR = newData[pixelIndex];
			double oldG = newData[pixelIndex + 1];
			double oldB = newData[pixelIndex + 2];
			// New pixel values after trunc
			int newR = (int)oldR;
			int newG = (int)oldG;
			int newB = (int)oldB;

			// Update the current pixel
			newData[pixelIndex] = newR;
			newData[pixelIndex + 1] = newG;
			newData[pixelIndex + 2] = newB;

			// Calculate the error
			int errorR = oldR - newR;
			int errorG = oldG - newG;
			int errorB = oldB - newB;

			// Diffusion to neighboring pixels
			newData[((y + 0) * width + (x + 1)) * 4] += static_cast<unsigned char>(errorR * 5 / 16);
			newData[((y + 1) * width + (x - 1)) * 4] += static_cast<unsigned char>(errorR * 3 / 16);
			newData[((y + 1) * width + (x + 0)) * 4] += static_cast<unsigned char>(errorR * 7 / 16);
			newData[((y + 1) * width + (x + 1)) * 4] += static_cast<unsigned char>(errorR * 1 / 16);

			newData[((y + 0) * width + (x + 1)) * 4 + 1] += static_cast<unsigned char>(errorG * 5 / 16);
			newData[((y + 1) * width + (x - 1)) * 4 + 1] += static_cast<unsigned char>(errorG * 3 / 16);
			newData[((y + 1) * width + (x + 0)) * 4 + 1] += static_cast<unsigned char>(errorG * 7 / 16);
			newData[((y + 1) * width + (x + 1)) * 4 + 1] += static_cast<unsigned char>(errorG * 1 / 16);

			newData[((y + 0) * width + (x + 1)) * 4 + 2] += static_cast<unsigned char>(errorB * 5 / 16);
			newData[((y + 1) * width + (x - 1)) * 4 + 2] += static_cast<unsigned char>(errorB * 3 / 16);
			newData[((y + 1) * width + (x + 0)) * 4 + 2] += static_cast<unsigned char>(errorB * 7 / 16);
			newData[((y + 1) * width + (x + 1)) * 4 + 2] += static_cast<unsigned char>(errorB * 1 / 16);
		}
	}

	return newData;
}


unsigned char* convertToHalftone(unsigned char* imageData, int width, int height) {
	vector<vector<unsigned char>> vecmat;

	for (int h = 0; h < height; ++h) {
		vector<unsigned char> vecRowup;
		vector<unsigned char> vecRowdown;

		for (int w = 0; w < width; ++w) {
			int pixelIndex = (h * width + w) * 4;
			int avg = (imageData[pixelIndex] + imageData[pixelIndex + 1] + imageData[pixelIndex + 2]) / 3;

			unsigned char vecPix1, vecPix2, vecPix3, vecPix4;

			if (avg > 205) {
				vecPix1 = vecPix2 = vecPix3 = vecPix4 = 255;
			}
			else if (avg > 153) {
				vecPix1 = 0;
				vecPix2 = vecPix3 = vecPix4 = 255;
			}
			else if (avg > 101) {
				vecPix1 = vecPix2 = 0;
				vecPix3 = vecPix4 = 255;
			}
			else if (avg > 50) {
				vecPix1 = 0;
				vecPix2 = vecPix3 = 0;
				vecPix4 = 255;
			}
			else {
				vecPix1 = vecPix2 = vecPix3 = vecPix4 = 0;
			}

			vecRowup.push_back(vecPix1);
			vecRowup.push_back(vecPix2);
			vecRowdown.push_back(vecPix3);
			vecRowdown.push_back(vecPix4);
		}

		vecmat.push_back(vecRowup);
		vecmat.push_back(vecRowdown);
	}

	unsigned char* newData = (unsigned char*)malloc(width * height * 4 * 4);

	for (int h = 0; h < height * 2; h++) {
		for (int w = 0; w < width * 2; w++) {
			int pixelIndex = (h * width * 2 + w) * 4;
			newData[pixelIndex] = vecmat[h][w];
			newData[pixelIndex + 1] = vecmat[h][w];
			newData[pixelIndex + 2] = vecmat[h][w];
			newData[pixelIndex + 3] = 255;
		}
	}

	return newData;
}
/*
// Apply Gaussian blur to a pixel
unsigned char applyGaussianFilter(const unsigned char* input, int width, int height, int x, int y) {
	double sigma = 1.0;
	double sum = 0.0;
	double filteredPixel = 0.0;

	int guas[3][3] = { {1, 2, 1}, {2,4,2}, {1, 2, 1} };


	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			int newX = x + i;
			int newY = y + j;

			if (newX >= 0 && newX < width && newY >= 0 && newY < height) {
				double weight = exp(-(i * i + j * j) / (2.0 * sigma * sigma));
				filteredPixel += input[(newY * width + newX) * 4] * weight;
				sum += weight;
			}
		}
	}

	return static_cast<unsigned char>(filteredPixel / sum);
}

// Apply Gaussian blur to the entire image
void applyGaussianBlur(unsigned char* input, int width, int height) {
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			int pixelIndex = (y * width + x) * 4;
			input[pixelIndex] = applyGaussianFilter(input, width, height, x, y);
			input[pixelIndex + 1] = input[pixelIndex];
			input[pixelIndex + 2] = input[pixelIndex];
			input[pixelIndex + 3] = 255;
		}
	}
}
*/
vector<vector<int>>* gaussianKernel(int* div) {
	vector<vector<int>>* kernel = new vector<vector<int>>();
	vector<int> vec1 = { 1, 2, 1 };
	kernel->push_back(vec1);
	vector<int> vec2 = { 2, 4, 2 };
	kernel->push_back(vec2);
	vector<int> vec3 = { 1, 2, 1 };
	kernel->push_back(vec3);
	*div = 16;

	return kernel;
}

void applyFilter1(unsigned char* mat, int width, int height, vector<vector<int>>* kernel, int div) {
	vector<vector<unsigned char>>* newMat = new vector<vector<unsigned char>>();

	//upper end of the photo
	vector<unsigned char> upper;
	for (int i = 0; i < width; i++) {
		upper.push_back(i % 100 + 100);
	}
	newMat->push_back(upper);

	for (int j = 1; j < height - 1; j++) {
		vector<unsigned char> innerMat;
		innerMat.push_back(j % 100 + 100); // left most end of the photo
		for (int i = 1; i < width - 1; i++) {
			int temp = 0;
			temp += (*kernel)[0][0] * mat[((j-1) * width + i-1) * 4];
			temp += (*kernel)[0][1] * mat[((j - 1) * width + i) * 4];
			temp += (*kernel)[0][2] * mat[((j - 1) * width + i + 1) * 4];
			temp += (*kernel)[1][0] * mat[(j * width + i - 1) * 4];
			temp += (*kernel)[1][1] * mat[(j * width + i) * 4];
			temp += (*kernel)[1][2] * mat[(j * width + i + 1) * 4];
			temp += (*kernel)[2][0] * mat[((j + 1) * width + i - 1) * 4];
			temp += (*kernel)[2][1] * mat[((j + 1) * width + i) * 4];
			temp += (*kernel)[2][2] * mat[((j + 1) * width + i + 1) * 4];
			temp /= div;
			innerMat.push_back(abs(temp));
		}
		innerMat.push_back(j % 100 + 100); // right most end of the photo
		newMat->push_back(innerMat);
	}

	// lower end of the photo
	vector<unsigned char> lower;
	for (int i = 0; i < width; i++) {
		lower.push_back(i % 100 + 100);
	}
	newMat->push_back(lower);
	for (int j = 1; j < height - 1; j++) {
		for (int i = 1; i < width - 1; i++) {
			int pixelIndex = (j * width + i) * 4;
			mat[pixelIndex] = newMat->at(j).at(i);
			mat[pixelIndex+1] = newMat->at(j).at(i);
			mat[pixelIndex+2] = newMat->at(j).at(i);
		}
	}
}


void gaussian(unsigned char* img, int width, int height) {
	int gaus[3][3] = { {1, 2, 1}, {2,4,2}, {1, 2, 1} };
	vector<vector<unsigned char>> vecOut;
	for (int y = 0; y < height; ++y) {
		vector<unsigned char> vecIn;
		for (int x = 0; x < width; ++x) {
			vecIn.push_back(0);
		}
		vecOut.push_back(vecIn);
	}
	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {	
			int pixelIndex = (y * width + x) * 4;
			int sum = 0;
			for (int i = -1; i <= 1; ++i) {
				for (int j = -1; j <= 1; ++j) {
					unsigned char pixelValue = img[((y + i) * width + (x + j)) * 4];
					sum = sum + (gaus[i][j]) * pixelValue;
				}
			}
			vecOut.at(y).at(x) = sum / 16;
		}
	}
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			int pixelIndex = (y * width + x) * 4;
			img[pixelIndex] = vecOut.at(y).at(x);
			img[pixelIndex + 1] = vecOut.at(y).at(x);
			img[pixelIndex + 2] = vecOut.at(y).at(x);
		}
	}

}

// Compute gradients using the Sobel operator
void computeGradients(const unsigned char* input, int width, int height, std::vector<char>& gradientX, std::vector<char>& gradientY) {
	// Sobel kernels
	int sobelKernelX[3][3] = { {0, 0, 0}, {0,-1,1}, {0, 0, 0} };
	int sobelKernelY[3][3] = { {0, 1, 0}, {0, -1, 0}, {0, 0,0} };

	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {
			int gx = 0, gy = 0;

			// Apply Sobel kernels
			for (int i = -1; i <= 1; ++i) {
				for (int j = -1; j <= 1; ++j) {
					char pixelValue = input[((y + i) * width + (x + j)) * 4];
					gx += sobelKernelX[i + 1][j + 1] * pixelValue;
					gy += sobelKernelY[i + 1][j + 1] * pixelValue;
				}
			}
			gradientX[y * width + x] = gx;//before just gx
			gradientY[y * width + x] = gy;//before just gy
		}
	}
}

// Perform non-maximum suppression
void nonMaximumSuppression(const std::vector<char>& gradientX, const std::vector<char>& gradientY, int width, int height, unsigned char* data) {
	// compute the gradient magnitude at each pixel
	std::vector<unsigned char> gradientMagnitude(width * height, 0.0); //initialize by zeros
	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {
			char gx = gradientX[y * width + x]; //the change in x
			char gy = gradientY[y * width + x]; //the change in y

			gradientMagnitude[y * width + x] = min(255, int(abs(gx)+abs(gy))); 
		}
	}
	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {
			int pixelIndex = (y * width + x) * 4;
			//consider the gradient direction to decide which neighboring pixels to compare
			unsigned char gx = gradientX[y * width + x];
			unsigned char gy = gradientY[y * width + x];
			unsigned char gradientMagnitudeXY = gradientMagnitude[y * width + x]; //the magnitude in the current pixel

			data[pixelIndex] = gradientMagnitudeXY;
			data[pixelIndex + 1] = gradientMagnitudeXY;
			data[pixelIndex + 2] = gradientMagnitudeXY;
		}
	}
	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {
			int pixelIndex = (y * width + x) * 4;
			// Perform non-maximum suppression
			unsigned char gradientMagnitudeXY = data[pixelIndex];
				// Calculate the angle in radians
				unsigned char angle =gradientMagnitudeXY; //angle in radians
				// Convert radians to degrees
				angle = angle * (unsigned char)(180.0 / 3.14); //angle in degrees
				if (angle < 0) {
					angle = angle + (unsigned char)180;
				}
				unsigned char pixel1 = 255;
				unsigned char pixel2 = 255;
				if ((0 <= angle && angle < 22.5) || (157.5 <= angle && angle <= 180) ){
						pixel1 = data[(y * width + x+1) * 4];
						pixel2 = data[(y * width + x-1) * 4];
					}
				else if (22.5 <= angle && angle < 67.5) {
					pixel2 = data[((y - 1) * width + x + 1) * 4];
					pixel1 = data[((y + 1) * width + x - 1) * 4];
				}
				else if (67.5 <= angle && angle <112.5) {
					pixel1 = data[((y+1)  * width + x ) * 4];
					pixel2 = data[((y-1)  * width + x) * 4];
				}
				else if (112.5 <= angle && angle < 157.5) {
					pixel1 = data[((y - 1) * width + x - 1) * 4];
					pixel2 = data[((y + 1) * width + x + 1) * 4];
				}
				bool q = (data[pixelIndex] >= pixel1 && data[pixelIndex] >= pixel2);
				data[pixelIndex] = q ? gradientMagnitudeXY : 0;
				data[pixelIndex + 1] = q ? gradientMagnitudeXY : 0;
				data[pixelIndex + 2] = q ? gradientMagnitudeXY : 0;
			
		}
		
	}
	
}

void threshold(unsigned char* img, int width, int height, unsigned char highThreshold) {
	for (int i = 0; i < width * height * 4; ++i) {
		if (img[i] >= highThreshold) {
			img[i] = 255; // Strong edge
		}
		else {
			img[i] = 0;   // Non-edge
		}
	}
}
/*
void edgeTrackingByHysteresis(unsigned char* img, int width, int height) {
	for (int i = 1; i < height - 1; ++i) {
		for (int j = 1; j < width - 1; ++j) {
			int pixelIndex = (i * width + j) * 4;
			if (img[pixelIndex] == 255) {
				bool neighbors = false;
				for (int m = -1; m <= 1; ++m) {
					for (int n = -1; n <= 1; ++n) {
						if (m != 0 && n != 0) {
							int neighborRow = i + m;
							int neighborCol = j + n;
							if (img[(neighborRow * width + neighborCol) * 4] == 255) {
								img[pixelIndex] = 255; 
								img[pixelIndex + 1] = 255; 
								img[pixelIndex + 2] = 255; 
								neighbors = true;
							}	
						}
					}
				}
				if (!neighbors) {
					img[pixelIndex] = 0;
					img[pixelIndex+1] = 0;
					img[pixelIndex+2] = 0;
				}

			}
		}
	}
}
*/


void convertToCanny(unsigned char* imageData, int width, int height) {//this u use?
	int div;
	vector<vector<int>>*  gaus = gaussianKernel(&div);
	applyFilter1(imageData, width, height, gaus, div);
	vector< char> gradientX(width * height, 0);
	vector< char> gradientY(width * height, 0);
	computeGradients(imageData, width, height, gradientX, gradientY);
	nonMaximumSuppression(gradientX, gradientY, width, height, imageData);
	unsigned char highThreshold = 25; 
	threshold(imageData, width, height, highThreshold);
	//edgeTrackingByHysteresis(imageData, width, height);
}


void saveImageToTxt(const unsigned char* imageData, int width, int height, const char* filePath, int bits) {
	const char* folderPath = "assignment";
	

	// Open or create the 'img4.txt' file within the 'assignment' folder
	std::ofstream outFile(filePath);

	if (outFile.is_open()) {
		// Iterate through each pixel and write its value to the file
		for (int h = 0; h < height; ++h) {
			for (int w = 0; w < width; ++w) {
				int pixelIndex = (h * width + w)*4; // Assuming 4 channels (e.g., RGBA)
				int pix = 0;
				int cur = imageData[pixelIndex];
				if (bits == 0) { pix = cur % 2;  } //0-1 values
				else { pix = cur % 16; } //0-15 values
				outFile << pix << ' ';
			}
			outFile << '\n';
		}
		std::cout << "Image data saved to 'assignment/img4.txt'" << std::endl;

		// Close the file
		outFile.close();
	}
	else {
		std::cerr << "Failed to open the file." << std::endl;
	}
}





int main(int argc, char* argv[])
{
	const int DISPLAY_WIDTH = 512;
	const int DISPLAY_HEIGHT = 512;
	const float CAMERA_ANGLE = 0.0f;
	const float NEAR = 1.0f;
	const float FAR = 100.0f;

	Game* scn = new Game(CAMERA_ANGLE, (float)DISPLAY_WIDTH / DISPLAY_HEIGHT, NEAR, FAR);

	Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");

	Init(display);

	scn->Init();
	int width, height, numComponents;
	unsigned char* data1 = stbi_load("../res/textures/lena256.jpg", &width, &height, &numComponents, 4);
	unsigned char* data2 = stbi_load("../res/textures/lena256.jpg", &width, &height, &numComponents, 4);
	unsigned char* data3 = stbi_load("../res/textures/lena256.jpg", &width, &height, &numComponents, 4);
	unsigned char* data4 = stbi_load("../res/textures/lena256.jpg", &width, &height, &numComponents, 4);
	display.SetScene(scn);


	//left down: halftone
	unsigned char* data5 = convertToHalftone(data1, width, height);
	scn->AddTexture(width * 2, height * 2, data5);
	scn->SetShapeTex(0, 0);
	scn->customDraw(1, 0, scn->BACK, true, false, 0);
	// write in txt file:
	const char* filePath5 = "assignment/img5.txt";
	saveImageToTxt(data5, width, height, filePath5,0);

	//right down: Floyd-Steinberg Algorithm
	unsigned char* data6 = convertToFloydSteinberg(data2, width, height);
	scn->AddTexture(width, height, data6);
	scn->SetShapeTex(0, 1);
	scn->customDraw(1, 0, scn->BACK, false, false, 1);
	// write in txt file:
	const char* filePath6 = "assignment/img6.txt";
	saveImageToTxt(data6, width, height, filePath6,1);

	// Left up: change nothing
	scn->AddTexture(width, height, data3);
	scn->SetShapeTex(0, 2);
	scn->customDraw(1, 0, scn->BACK, false, false, 2);

	// Right up: canny
	convertToCanny(data4, width, height);
	scn->AddTexture(width, height, data4);
	scn->SetShapeTex(0, 3); 
	scn->customDraw(1, 0, scn->BACK, false, false, 3); 
	// write in txt file:
	const char* filePath4 = "assignment/img4.txt";
	saveImageToTxt(data4, width, height, filePath4,0);

	scn->Motion();
	display.SwapBuffers();

	while (!display.CloseWindow())
	{
		/*scn->Draw(1, 0, scn->BACK, true, false, 512);
		scn->Motion();
		display.SwapBuffers(); */
		display.PollEvents();

	}
	delete scn;
	return 0;
}