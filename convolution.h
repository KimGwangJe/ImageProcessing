/*
 * @DName : convolution.h
 * @Description : Image Processing in C
 * @Date : 2023. 10. 03
 * @Revision : 1.0
 *	1.0 : convolution kernel
 * @Author : Howoong Lee, Division of Computer Enginnering, Hoseo Univ.
 */

// Convolution Kernel

// Average
// 가우시안 잡음 없애는데 효과적임
// 저역통과 필터에도 사용
double AvgKernel[3][3] = {0.11111, 0.11111, 0.11111,
						  0.11111, 0.11111, 0.11111,
						  0.11111, 0.11111, 0.11111};

// Gaussian
// 가우시안 커널은 잡음을 없애기 위해 사용하는 경우도 있고. 고주파 및 저주파 성분을 동시에 잡아 이미지의 부드러움을 조절하는데 사용
double GaussKernel[3][3] = {0.0625, 0.125, 0.0625,
							0.125, 0.25, 0.125,
							0.0625, 0.125, 0.0625};

// Prewitt
// 경계선 검출
double PrewittKernel_X[3][3] = {
	-1.0,
	0.0,
	1.0,
	-1.0,
	0.0,
	1.0,
	-1.0,
	0.0,
	1.0,
};

// 경계선 검출
double PrewittKernel_Y[3][3] = {
	-1.0,
	-1.0,
	-1.0,
	0.0,
	0.0,
	0.0,
	1.0,
	1.0,
	1.0,
};

// Sobel이 PrewittKernel보다 조금 더 날카로운 경계를 검출한다.
// 경계선 검출
double SobelKernel_X[3][3] = {
	-1.0,
	0.0,
	1.0,
	-2.0,
	0.0,
	2.0,
	-1.0,
	0.0,
	1.0,
};

// 경계선 검출
double SobelKernel_Y[3][3] = {
	-1.0,
	-2.0,
	-1.0,
	0.0,
	0.0,
	0.0,
	1.0,
	2.0,
	1.0,
};

// Laplacian
// 이것도 마찬가지로 고역통과 필터에도 사용된다. 샤프닝효과
double LaplacianKernel[3][3] = {
	-1.0,
	-1.0,
	-1.0,
	-1.0,
	8.0,
	-1.0,
	-1.0,
	-1.0,
	-1.0,
};

// 고역통과 필터에 사용하는데 9로 두는 이유는 주변의 픽셀들과의 차이를 크게 강조하는 역할
double LaplacianKernel_HPF[3][3] = {
	-1.0,
	-1.0,
	-1.0,
	-1.0,
	9.0,
	-1.0,
	-1.0,
	-1.0,
	-1.0,
};