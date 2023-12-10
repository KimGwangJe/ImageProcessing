/*
 * @Name : imgprocessing.c
 * @Description : Image Processing in C
 * @Date : 2023. 9. 12
 * @Revision : 1.1
 * 0.1 : inverse
 * 0.2 : brightness, contrast
 * 0.3 : histogram, gonzales method, binalization
 * 0.4 : histogram stretching, histogram equalization
 * 0.5 : convolution(9~17)
 * 0.6 : laplacian high pass filter
 * 0.7 : Median Filter - MinPooling , MedianPooling, MaxPooling using Bubble Sorting, swap
 * 0.8 : Median Filtering(variable filter), Component Labeling(GrassFire Algorithm using Depth-First Search stack)
 * 0.9 : Detect Object Edge
 * 1.0 : VerticalFlip, HorizontalFlip, Translation, Scaling, Rotation
 * 1.1 : Erosion, Dilation, ZhangSuenAlgorithm, FeatureExtractThinImage
 *         침식      팽창       뒤에 두개는 시험 X
 */

// 지금 어려운게 필터를 사용할때 1,1로 계산을 시작하니까 너무 헷갈림

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>
// 헤더파일
#include "convolution.h"

#pragma pack(push, 1) // 패딩을 최소화하여 메모리를 절약
typedef struct
{
    unsigned short bfType;      // BMP 파일 식별자, "BM"으로 설정됩니다. (2 바이트)
    unsigned int bfSize;        // BMP 파일 전체 크기 (바이트). 파일 헤더와 이미지 데이터의 총 크기를 나타냅니다. (4 바이트)
    unsigned short bfReserved1; // 예약된 필드, 일반적으로 0으로 설정됩니다. (2 바이트)
    unsigned short bfReserved2; // 예약된 필드, 일반적으로 0으로 설정됩니다. (2 바이트)
    unsigned int bfOffBits;     // 이미지 데이터의 시작 위치 (오프셋). 파일 헤더와 정보 헤더의 크기를 더한 값으로 설정됩니다. (4 바이트)
} BITMAPFILEHEADER;

// BMP 정보 헤더 구조체 (40 바이트)
typedef struct
{
    unsigned int biSize;     // 정보 헤더의 크기. 일반적으로 40으로 설정됩니다. (4 바이트)
    int biWidth;             // 이미지의 가로 너비 (픽셀 수). (4 바이트) buWidth랑 biHeight로 이미지 크기
    int biHeight;            // 이미지의 세로 높이 (픽셀 수). 음수 값인 경우 이미지가 위에서 아래로 뒤집힌 이미지임을 나타냅니다. (4 바이트)
    unsigned short biPlanes; // 이미지의 색상 평면 수. 항상 1로 설정됩니다. (2 바이트)
    // biBitCount가 24면 트루컬러 8이면 8비트 그레이컬러
    unsigned short biBitCount;   // 픽셀당 비트 수. 1, 4, 8, 16, 24 또는 32 중 하나의 값으로 설정됩니다. (2 바이트)
    unsigned int biCompression;  // 압축 방법을 나타내는 값. 일반적으로 0 (비압축)으로 설정됩니다. (4 바이트)
    unsigned int biSizeImage;    // 이미지 데이터의 크기 (바이트). 압축되지 않은 경우 이미지의 크기와 동일합니다. (4 바이트)
    int biXPelsPerMeter;         // 수평 해상도 (픽셀당 미터 단위). (4 바이트)
    int biYPelsPerMeter;         // 수직 해상도 (픽셀당 미터 단위). (4 바이트)
    unsigned int biClrUsed;      // 사용된 색상 테이블(팔레트)의 색상 수. 0인 경우 모든 가능한 색상을 사용합니다. (4 바이트)
    unsigned int biClrImportant; // 중요한 색상 테이블의 색상 수. 0인 경우 모든 색상이 중요하다는 것을 나타냅니다. (4 바이트)
} BITMAPINFOHEADER;
#pragma pack(pop) // 이전에 저장도니 패딩 설정으로 복원

// RGBQUAD 구조체 정의 (필요한 경우)
typedef struct
{
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} RGBQUAD;

// 이진영상에서
unsigned char blankPixel = 255, imagePixel = 0;

// pixel 좌표 구조체
typedef struct
{
    int row, col;
} pixel;

/*
 * @Function Name : InverseImage
 * @Description : 픽셀 단위로 밝기 값을 반전시킵니다.
 * @Input : *Input - 입력 이미지 데이터 배열을 가리키는 포인터,
 *          nWidth - 이미지의 너비 (픽셀 단위),
 *          nHeight - 이미지의 높이 (픽셀 단위)
 * @Output : *Output - 출력 이미지 데이터 배열을 가리키는 포인터
 */
// 김광제의 설명 - 밝기값을 255는 0으로 0은 255로 바뀌도록 해둔거임
void InverseImage(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    int nImgSize = nWidth * nHeight; // 이미지 크기 계산

    // 픽셀별로 밝기값을 반전시킴
    for (int i = 0; i < nImgSize; i++)
        Output[i] = 255 - Input[i]; // 반전 연산: 255에서 빼기

    return;
}

/*
 * @Function Name : AdjustBrightness
 * @Description : nBrightness 값에 따라 픽셀 단위로 밝기값을 조절합니다.
 * @Input : *Input - 입력 이미지 데이터 배열을 가리키는 포인터,
 *          nWidth - 이미지의 너비 (픽셀 단위),
 *          nHeight - 이미지의 높이 (픽셀 단위),
 *          nBrightness - 조정할 밝기값
 * @Output : *Output - 출력 이미지 데이터 배열을 가리키는 포인터
 */
// 김광제의 설명 - 픽셀의 밝기값에 + 연산을 진행하는데 범위를 넘어가는 값은 클리핑 처리하여 경계값으로 지정한다.
void AdjustBrightness(BYTE *Input, BYTE *Output, int nWidth, int nHeight, int nBrightness)
{
    int nImgSize = nWidth * nHeight; // 가로 세로를 곱하여 전체 픽셀 수를 구함

    for (int i = 0; i < nImgSize; i++)
    {
        // 밝기값 조정
        if (Input[i] + nBrightness > 255)    // 계산 결과가 255보다 크면
            Output[i] = 255;                 // 최댓값 255로 설정
        else if (Input[i] + nBrightness < 0) // 계산 결과가 0보다 작으면
            Output[i] = 0;                   // 최솟값 0으로 설정
        else
            Output[i] = Input[i] + nBrightness; // 밝기값을 nBrightness만큼 조정
    }

    return;
}

/*
 * @Function Name : AdjustContrast
 * @Descriotion : dContrast에 설정된 값을 Pixel 단위로 *를 통한 대비값을 조정(기준 값 1)
 * @Input : *Input, nWidth, nHeight, dContrast
 * @Output : *Output
 */
// 김광제의 설명 - 대비를 조정하는것인데 dContrast가 1보다 크면 곱하기가 되고 1보다 작으면 나누기 효과를 가진다.
// 곱하기를 하면 영상이 전체적으로 밝아지며 대비가 커지기 때문에 영상이 강렬해지는 효과과 있음
// 반대로 나누기를 하면 전체적으로 어두워지고 대비가 작아지기 때문에 영상이 전체적으로 부드러워짐
void AdjustContrast(BYTE *Input, BYTE *Output, int nWidth, int nHeight, double dContrast)
{
    int nImgSize = nWidth * nHeight; // 사이즈 구하는거임

    for (int i = 0; i < nImgSize; i++)  // 전체 픽셀 순회
        if (Input[i] * dContrast > 255) // 255보다 커지는 경우
            Output[i] = 255;            // 경계값을 넣어줌
        else
            Output[i] = (BYTE)(Input[i] * dContrast); // 그렇지 않다면 연산결과를 넣어줌

    return;
}

/*
 * @Function Name : GenerateHistogram
 * @Descriotion : 입력 이미지에 대한 히스토그램을 버퍼에 출력
 * @Input : *Input, nWidth, nHeight, dContrast
 * @Output : *Histogram
 */
// 김광제의 설명 - input[i]는 값이 들어있고 histogram은 256개가 있으며 해당 값의 갯수를 ++로 늘리는거임
// histogram[i] 에는 값의 갯수가 들어감
void GenerateHistogram(BYTE *Input, int *Histogram, int nWidth, int nHeight)
{
    int nImgSize = nWidth * nHeight; // 전체 이미지 사이즈

    for (int i = 0; i < nImgSize; i++) // 전체 이미지를 순회하며
        Histogram[Input[i]]++;         // 해당 밝기값을 가지는 인덱스의 빈도수를 1씩 늘린다.

    return;
}

/*
 * @Function Name : GenerateBinarization
 * @Descriotion : bThreshold 값을 임계값으로 하여 이진화를 진행
 * @Input : *Input, nWidth, nHeight, bThreshold
 * @Output : *Output
 */
// 김광제의 설명 - 여기에서 스레드 홀드는 곤잘레스 알고리즘으로 받아오거나 사용자에게 받아오는 방법이 사용된다.
void GenerateBinarization(BYTE *Input, BYTE *Output, int nWidth, int nHeight, BYTE bThreshold)
{

    int nImgSize = nWidth * nHeight; // 전체 이미지 사이즈

    for (int i = 0; i < nImgSize; i++)
        if (Input[i] < bThreshold) // 임계값 보다 작다면 0으로 처리함
            Output[i] = 0;
        else
            Output[i] = 255; // 임계값 보다는 크니 255

    return;
}

/*
 * @Function Name : GonzalezMethod
 * @Description : Gonzalez-Woods Method를 사용하여 최적의 이진화 임계값을 계산합니다.
 * @Input : *Histogram - 히스토그램 배열 포인터
 * @Output : bThreshold - 계산된 이진화 임계값
 */
// 김광제의 설명 - 곤잘레스 알고리즘으로 최적의 임계값을 정하는 메소드
BYTE GonzalezMethod(int *Histogram)
{
    BYTE bLow = 0, bHigh = 0;       // 영상의 최소값과 최대값
    BYTE bThreshold, bNewThreshold; // 현재 임계값과 새로운 임계값
    BYTE e = 2;                     // 오차값 설정 (입실론)

    int nG1 = 0, nG2 = 0, nCntG1 = 0, nCntG2 = 0; // nG1, nG2는 밝기값 총합  nCntG1, nCntG2는 픽셀 개수
    int nMeanG1, nMeanG2;                         // G1, G2의 밝기값 평균

    // 초기 Threshold 설정: 영상에서 가장 어두운 값부터 시작하여 최소값, 최대값 찾기
    // 최초로 갯수가 0이 아니게 되는 수를 찾아서 그 밝기값을 넣는다.
    // Histogram[i]는 영상에서 i 밝기값을 가지는 픽셀의 갯수가 들어간다.
    for (int i = 0; i < 256; i++)
    {
        if (Histogram[i] != 0)
        {
            bLow = i; // 최소값 설정
            break;    // 최소값만 찾고 바로 종료
        }
    }

    // 가장 밝은 부분을 찾기위한 반복문으로 최초로 포함된 픽셀의 갯수가 0인 아닌곳을 찾음
    for (int i = 255; i >= 0; i--)
    {
        if (Histogram[i] != 0)
        {
            bHigh = i; // 최대값 설정
            break;     // 최댓값만 찾고 바로 종료
        }
    }

    // 1. Threshold 초기값 추정: 최소값과 최대값의 중간값으로 시작
    bThreshold = (bLow + bHigh) / 2;
    printf("---------------------------\n");
    printf("Initial Threshold = %d\n", bThreshold);

    // 2~4번을 e보다 작을 때까지 반복: e = 2로 설정된 오차 범위 내에서 계산
    while (1)
    {
        // 2. Threshold를 기준으로 영상을 분할하여 G1, G2의 합 및 개수 계산
        for (int i = bLow; i <= bThreshold; i++)
        {
            nG1 += (Histogram[i] * i); // G1의 밝기값 총합
            nCntG1 += Histogram[i];    // G1의 픽셀 갯수
        }

        for (int i = bThreshold + 1; i <= bHigh; i++)
        {
            nG2 += (Histogram[i] * i); // G2의 밝기값 총합
            nCntG2 += Histogram[i];    // G2의 픽셀 갯수
        }

        // 오류 처리: 0으로 나누기 오류 방지
        if (nCntG1 == 0)
            nCntG1 = 1;
        if (nCntG2 == 0)
            nCntG2 = 1;

        // 3. G1, G2의 밝기값 평균 계산
        nMeanG1 = nG1 / nCntG1;
        nMeanG2 = nG2 / nCntG2;

        // 4. 새로운 임계값 계산
        bNewThreshold = (nMeanG1 + nMeanG2) / 2;

        // 5. 오차가 e보다 작은지 검사하여 최종 Threshold 결정
        if (abs(bNewThreshold - bThreshold) < e) // 원래 임계값에서 새로운 임계값을 빼서 절대값으로 e보다 작아야됨
        {
            bThreshold = bNewThreshold; // 오차 범위 내에서 Threshold 결정
            break;                      // 미리 정해둔 오차보다 작은 임계값을 찾았기 때문에 반복분 종료
        }
        else
        {
            bThreshold = bNewThreshold;                    // 새로운 Threshold 설정 하고 이 경우에는 새로운 임계값을 기준으로 다시 돌려야됨
            printf("New Threshold = %d\n", bNewThreshold); // 계산된 새로운 Threshold 출력
        }

        // 반복을 위해 변수 초기화
        nG1 = nG2 = nCntG1 = nCntG2 = 0;
    }

    printf("Last Threshold = %d\n", bThreshold); // 최종 결정된 Threshold 출력
    return bThreshold;                           // 최종적으로 결정된 이진화 임계값 반환
}

/*
 * @Function Name : HistogramStretching
 * @Description : 히스토그램 스트래칭을 수행합니다.
 * @Input : *Input - 입력 이미지 데이터 배열을 가리키는 포인터,
 *          *Histogram - 입력 이미지의 히스토그램을 가리키는 포인터,
 *          nWidth - 이미지의 너비 (픽셀 단위),
 *          nHeight - 이미지의 높이 (픽셀 단위)
 * @Output : *Output - 출력 이미지 데이터 배열을 가리키는 포인터
 */
// 김광제의 설명 - 히스토그램을 0~255로 스트레칭하는것이 아닌 하이와 로우를 잡고서 로우보다 작으면 0으로 보내버리고서 로우보다는 클때 연산
void HistogramStretching(BYTE *Input, BYTE *Output, int *Histogram, int nWidth, int nHeight)
{
    int ImgSize = nWidth * nHeight; // 이미지 크기 계산
    BYTE Low, High;                 // 히스토그램의 최소값과 최대값

    // 히스토램에서 갯수가 최초로 0이 아닌 밝기 값을 찾아 최소값으로 설정
    for (int i = 0; i < 256; i++)
    {
        if (Histogram[i] != 0)
        {
            Low = i; // 밝기값 저장
            break;
        }
    }

    // 히스토그램에서 갯수가 최초로 0이 아닌 밝기 값을 찾아 최대값으로 설정
    for (int i = 255; i >= 0; i--)
    {
        if (Histogram[i] != 0)
        {
            High = i; // 밝기값 저장
            break;
        }
    }

    // 히스토그램 스트래칭을 수행하여 출력 이미지 생성
    for (int i = 0; i < ImgSize; i++)
    {
        // Input[i] - Low = 밝기의 최소값이 0이 되도록 설정
        // High-Low = 최대 밝기 값과 최소 밝기 값의 차이
        // X 255 = 밝기 값을 0 ~ 255 범위로 스케일링
        // 입력 이미지의 각 픽셀 값을 히스토그램의 최소값부터 최대값 사이로 스케일링하여 출력
        if (Input[i] <= Low)
        {
            Output[i] = 0; // 최소값 이하는 0으로 설정하여 검은색 부분 강조
        }
        // 로우보다 크고 하이보다 작은 값들사이에서 스트레칭
        else
        {
            // 입력 값들을 히스토그램의 최소값과 최대값 사이로 스케일링하여 출력
            Output[i] = (BYTE)((Input[i] - Low) / (double)(High - Low) * 255.0);
        }
    }

    return;
}

/*
 * @Function Name : HistogramEqualization
 * @Descriotion : 히스토그램 평활화를 수행
 * @Input : *Input, *Histigrnam, nWidth, nHeight
 * @Output : *Output
 */
// 김광제의 설명 - Gmax는 흑백 영상에서 255로 고정
// 정규화합 공식 - (영상의 최대 밝기값(255) / 총 픽셀) X 해당 픽셀의 누적합
// 정규화합으로 그리는 히스토그램은 255에 가까워질수록 누적합이 계속해서 커지므로 이미지의 밝기 대비가 향상되어 전체적으로 밝은 부분이 더 강조될 수 있다.
void HistogramEqualization(BYTE *Input, BYTE *Output, int *Histogram, int nWidth, int nHeight)
{
    int ImgSize = nWidth * nHeight;

    int Nt = ImgSize; // 총 픽셀수로 이미지 크기와 같음
    int Gmax = 255;   // 흑백 이미지에서 최대 밝기 레벨

    double Ratio = Gmax / (double)Nt; // 최대 밝기 레벨을 전체 픽셀 수로 나눈 비율
    BYTE NormSum[256];                // 정규화된 누적 히스토그램을 저장할 배열

    int AHistogram[256] = {
        0,
    }; // 누적 히스토그램을 저장할 배열로 누적값이 들어감

    // 누적 히스토그램 계산 (값 밝기값의 누적값을 계산함)
    for (int i = 0; i < 256; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            // i가 밝기값 j가 i보다 작은 밝기의 픽셀 수.
            // 2중 반복문으로 i보다 작은 픽셀의 누적합을 계산
            // Histogram은 밝기값의 갯수가 들어가있음
            AHistogram[i] += Histogram[j]; // 최대 밝기 레벨 255까지 히스토그램 값들을 저장
        }
    } // AHistorgram[255]는 전체 픽셀 수 Nt와 같음

    // 최종적으로 정규화된 누적 히스토그램 계산
    // 누적 히스토그램의 각 값에 Ratio를 곱해서 0~255 까지 정규화 진행
    for (int i = 0; i < 256; i++)
    {
        NormSum[i] = (BYTE)(Ratio * AHistogram[i]);
    } // AHistorgram[255] X (Gmax / Nt ) = Nt X ( Gmax / Nt ) = Gmax = 255

    // Input의 각 픽셀값에 대응하는 정규화된 히스토그램 값을 Output에 저장
    for (int i = 0; i < ImgSize; i++)
    {
        Output[i] = NormSum[Input[i]];
    }

    return;
}

/*
 * @Function Name : AverageConvolution
 * @Description : 평균 커널을 적용한 컨볼루션 연산을 수행합니다.
 * @Input : *Input - 입력 이미지 데이터 배열 포인터,
 *          nWidth - 이미지의 너비 (픽셀 단위),
 *          nHeight - 이미지의 높이 (픽셀 단위)
 * @Output : *Output - 출력 이미지 데이터 배열 포인터
 */
// 김광제의 설명 - 입력과 필터에서는 0,0부터 계산하지만 아웃풋을 1,1로 정해두었기 떄문에 마진이 생김
// nWidth와 nHeight는 영상의 가로 세로 길이
// 가우시안과 평활화는 평균값을(소수임) 사용하기 때문에 범위를 넘지않음
// 가우시안 잡음 없애는데 효과적임
// 경계면을 뭉개기 떄문에 경계면이 부드러워짐
void AverageConvolution(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    double SumProduct = 0.0; // 컨볼루션 연산 결과를 저장할 변수

    // Convolution Center를 (1,1)로 설정하기 위해 1부터 시작하여 n-1까지 진행합니다.
    // 이 경우 이미지의 가장자리에 마진이 생김
    for (int i = 1; i < nHeight - 1; i++)
    { // 이미지의 y 행을 순회
        for (int j = 1; j < nWidth - 1; j++)
        { // 이미지의 x 열을 순회
            for (int m = -1; m <= 1; m++)
            { // 커널의 열을 순회합니다. (중요) 2차원 배열의 기준점을 (0,0)으로 설정하기 위해 (-1, -1)부터 시작합니다.
                for (int n = -1; n <= 1; n++)
                { // 커널의 행을 순회합니다.
                    // 입력 이미지와 커널을 이용하여 컨볼루션 연산을 수행합니다.
                    // 입력 이미지의 0,0과 AvgKernel의 0,0부터 시작해서 2,2까지 계산하여 전부 더한 값을 넣는다.
                    // AvgKernel에서 m과 n은 -1부터 시작하기때문에 +1을 해줘야된다.
                    SumProduct += Input[(i + m) * nWidth + (j + n)] * AvgKernel[m + 1][n + 1]; // 입력 이미지와 커널을 곱하고 합산합니다.
                }
            }
            // i * nWidth + j는 2차원 배열에서 원하는 인덱스를 잡음
            // 1,1 부터 값이 들어감
            Output[i * nWidth + j] = (BYTE)SumProduct; // 컨볼루션 연산 결과를 출력 이미지에 저장합니다. (중요) 2차원 배열을 1차원 배열로 변환하여 저장합니다.
            SumProduct = 0.0;                          // SumProduct 변수를 초기화합니다.
        }
    }

    return;
}

/*
 * @Function Name : GaussianConvolution
 * @Descriotion : Gaussian Kernel을 적용한 Convolution
 * @Input : *Input, nWidth, nHeight
 * @Output : *Output
 */
// 김광제의 설명 - AvgKernel과 과정과 연산이 모두 일치하지만 커널의 픽셀값이 다르기 때문에 결과는 다르다.
// 가우시안과 평활화는 평균값을(소수임) 사용하기 때문에 범위를 넘지않음
// 가우시안 커널은 잡음을 없애기 위해 사용하는 경우도 있고. 고주파 및 저주파 성분을 동시에 잡아 이미지의 부드러움을 조절하는데 사용
void GaussianConvolution(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    double SumProduct = 0.0;

    // Convolution Center를 (1,1)로 잡기 위해 1부터 시작, n-1까지 진행
    // 아웃풋 영상에 1,1부터 값이 들어가게됨
    for (int i = 1; i < nHeight - 1; i++)
    { // y 행
        for (int j = 1; j < nWidth - 1; j++)
        { // x
            for (int m = -1; m <= 1; m++)
            { // kernel의 행
                for (int n = -1; n <= 1; n++)
                { // kernel의 열
                    // 입력 이미지와 커널을 이용하여 컨볼루션 연산을 수행합니다.
                    // 입력 이미지의 0,0과 GaussKernel의 0,0부터 시작해서 2,2까지 계산하여 전부 더한 값을 넣는다.
                    // GaussKernel에서 m과 n은 -1부터 시작하기때문에 +1을 해줘야된다.
                    // AvgKernel과 연산은 같지만 커널의 값이 다르니 결과도 다르다. 결국 전부 곱하고 더한값을 넣음
                    SumProduct += Input[(i + m) * nWidth + (j + n)] * GaussKernel[m + 1][n + 1]; // y * 전체 x + x
                }
            }
            Output[i * nWidth + j] = (BYTE)SumProduct;
            SumProduct = 0.0; // 초기화
        }
    }

    return;
}

/*
 * @Function Name : LaplacianConvolution
 * @Descriotion : Laplacian Kernel을 적용한 Edge 검출 Convolution
 * @Input : *Input, nWidth, nHeight
 * @Output : *Output
 */
// 김광제의 설명 - 이미지의 높은 주파수 성분을 감지하고 강조하는데 사용
// 범위를 벗어날수있다 그렇기때문에 범위를 다시 조정해줘야됨
// 경계값이 인풋영상보다는 작아지지만 경계의 8방향의 픽셀들이 더 작은 값으로 변하기때문에 경계가 돋보임
void LaplacianConvolution(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    double SumProduct = 0.0;

    // Convolution Center를 (1,1)로 잡기 위해 1부터 시작, n-1까지 진행
    for (int i = 1; i < nHeight - 1; i++)
    { // y 행
        for (int j = 1; j < nWidth - 1; j++)
        { // x
            for (int m = -1; m <= 1; m++)
            { // kernel의 행
                for (int n = -1; n <= 1; n++)
                {
                    // 입력 이미지와 커널을 이용하여 컨볼루션 연산을 수행합니다.
                    // 입력 이미지의 0,0과 LaplacianKernel의 0,0부터 시작해서 2,2까지 계산하여 전부 더한 값을 넣는다.
                    // LaplacianKernel에서 m과 n은 -1부터 시작하기때문에 +1을 해줘야된다.                                                                   // kernel의 열
                    SumProduct += Input[(i + m) * nWidth + (j + n)] * LaplacianKernel[m + 1][n + 1]; // y * 전체 x + x
                }
            }

            // 라플라시안 커널에는 현재 -1 8개와 8 1개가 들어가서 총 합이 0이 되어 높은 주파수 성분을 감지하고 강조한다.
            //  0 ~ +- 2040 값이 나오기 때문에, 절대값 / 8을 취하여 0 ~ 255 값으로 조정
            Output[i * nWidth + j] = abs((long)SumProduct) / 8;
            SumProduct = 0.0; // 초기화
        }
    }

    return;
}

/*
 * @Function Name : X_PrewittConvolution
 * @Descriotion : X_Prewitt Kernel을 적용한 Edge 검출 Convolution
 * @Input : *Input, nWidth, nHeight
 * @Output : *Output
 */
// 김광제의 설명 - 프리윗은 경계를 검출하기 위해서 사용한다.
// X는 오른쪽으로 값이 바뀐다. 1차원으로 표시하면 [-1, 0, 1     -1, 0, 1      -1, 0, 1]
// Y방향으로 라인이 생기고 값이 변하는건 아웃풋의 X라인이다.
void X_PrewittConvolution(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    double SumProduct = 0.0;

    // Convolution Center를 (1,1)로 잡기 위해 1부터 시작, n-1까지 진행
    for (int i = 1; i < nHeight - 1; i++)
    { // y 행
        for (int j = 1; j < nWidth - 1; j++)
        { // x
            for (int m = -1; m <= 1; m++)
            { // kernel의 행
                for (int n = -1; n <= 1; n++)
                { // kernel의 열
                    // 입력 이미지와 커널을 이용하여 컨볼루션 연산을 수행합니다.
                    // 입력 이미지의 0,0과 PrewittKernel_X의 0,0부터 시작해서 2,2까지 계산하여 전부 더한 값을 넣는다.
                    // PrewittKernel_X에서 m과 n은 -1부터 시작하기때문에 +1을 해줘야된다.
                    SumProduct += Input[(i + m) * nWidth + (j + n)] * PrewittKernel_X[m + 1][n + 1]; // y * 전체 x + x
                }
            }

            // 0 ~ +- 765 값이 나오기 때문에, 절대값 / 3을 취하여 0 ~ 255 값으로 조정
            Output[i * nWidth + j] = abs((long)SumProduct) / 3;
            SumProduct = 0.0; // 초기화
        }
    }

    return;
}

/*
 * @Function Name : Y_PrewittConvolution
 * @Descriotion : Y_Prewitt Kernel을 적용한 Edge 검출 Convolution
 * @Input : *Input, nWidth, nHeight
 * @Output : *Output
 */
// 김광제의 설명 - Y는 오른쪽으로 값이 바뀐다. 1차원으로 표시하면 [-1, -1, -1    0, 0, 0     1, 1, 1]
// X방향으로 라인이 생기고 값이 변하는건 아웃풋의 Y라인이다.
void Y_PrewittConvolution(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    double SumProduct = 0.0;

    // Convolution Center를 (1,1)로 잡기 위해 1부터 시작, n-1까지 진행
    for (int i = 1; i < nHeight - 1; i++)
    { // y 행
        for (int j = 1; j < nWidth - 1; j++)
        { // x
            for (int m = -1; m <= 1; m++)
            { // kernel의 행
                for (int n = -1; n <= 1; n++)
                {
                    // 입력 이미지와 커널을 이용하여 컨볼루션 연산을 수행합니다.
                    // 입력 이미지의 0,0과 PrewittKernel_Y의 0,0부터 시작해서 2,2까지 계산하여 전부 더한 값을 넣는다.
                    // PrewittKernel_Y에서 m과 n은 -1부터 시작하기때문에 +1을 해줘야된다.                                                                                    // kernel의 열
                    SumProduct += Input[(i + m) * nWidth + (j + n)] * PrewittKernel_Y[m + 1][n + 1]; // y * 전체 x + x
                }
            }

            // 0 ~ +- 765 값이 나오기 때문에, 절대값 / 3을 취하여 0 ~ 255 값으로 조정
            Output[i * nWidth + j] = abs((long)SumProduct) / 3;
            SumProduct = 0.0; // 초기화
        }
    }

    return;
}

/*
 * @Function Name : X_SobelConvolution
 * @Descriotion : X_Sobel Kernel을 적용한 Edge 검출 Convolution
 * @Input : *Input, nWidth, nHeight
 * @Output : *Output
 */
// 김광제의 설명 - Sobel이 Prewitt보다 더 날카로운 경계를 검출한다
// Sobel은 Prewitt과 다르게 [-1,0,1   -2,0,2   -1,0,1] 이렇게 중간에 2를 사용하여 더 날카롭게 나옴
void X_SobelConvolution(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    double SumProduct = 0.0;

    // Convolution Center를 (1,1)로 잡기 위해 1부터 시작, n-1까지 진행
    for (int i = 1; i < nHeight - 1; i++)
    { // y 행
        for (int j = 1; j < nWidth - 1; j++)
        { // x
            for (int m = -1; m <= 1; m++)
            { // kernel의 행
                for (int n = -1; n <= 1; n++)
                { // kernel의 열
                    // 입력 이미지와 커널을 이용하여 컨볼루션 연산을 수행합니다.
                    // 입력 이미지의 0,0과 SobelKernel_X의 0,0부터 시작해서 2,2까지 계산하여 전부 더한 값을 넣는다.
                    // SobelKernel_X에서 m과 n은 -1부터 시작하기때문에 +1을 해줘야된다.
                    SumProduct += Input[(i + m) * nWidth + (j + n)] * SobelKernel_X[m + 1][n + 1]; // y * 전체 x + x
                }
            }

            // 0 ~ +- 1020 값이 나오기 때문에, 절대값 / 4을 취하여 0 ~ 255 값으로 조정
            Output[i * nWidth + j] = abs((long)SumProduct) / 4;
            SumProduct = 0.0; // 초기화
        }
    }

    return;
}

/*
 * @Function Name : Y_SobelConvolution
 * @Descriotion : Y_Sobel Kernel을 적용한 Edge 검출 Convolution
 * @Input : *Input, nWidth, nHeight
 * @Output : *Output
 */
// 김광제의 설명 - 1차원 배열로는 [-1,-2,-1   0,0,0   1,2,1] 요렇게 들어감
void Y_SobelConvolution(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    double SumProduct = 0.0;

    // Convolution Center를 (1,1)로 잡기 위해 1부터 시작, n-1까지 진행
    for (int i = 1; i < nHeight - 1; i++)
    { // y 행
        for (int j = 1; j < nWidth - 1; j++)
        { // x
            for (int m = -1; m <= 1; m++)
            { // kernel의 행
                for (int n = -1; n <= 1; n++)
                { // kernel의 열
                    // 입력 이미지와 커널을 이용하여 컨볼루션 연산을 수행합니다.
                    // 입력 이미지의 0,0과 SobelKernel_Y의 0,0부터 시작해서 2,2까지 계산하여 전부 더한 값을 넣는다.
                    // SobelKernel_Y에서 m과 n은 -1부터 시작하기때문에 +1을 해줘야된다.
                    SumProduct += Input[(i + m) * nWidth + (j + n)] * SobelKernel_Y[m + 1][n + 1]; // y * 전체 x + x
                }
            }

            // 0 ~ +- 1020 값이 나오기 때문에, 절대값 / 4을 취하여 0 ~ 255 값으로 조정
            Output[i * nWidth + j] = abs((long)SumProduct) / 4;
            SumProduct = 0.0; // 초기화
        }
    }

    return;
}

/*
 * @Function Name : HPF_LaplacianConvolution
 * @Descriotion : Laplacian Kernel을 적용한 High Pass Filter Convolution
 * @Input : *Input, nWidth, nHeight
 * @Output : *Output
 */
// 김광제의 설명 - 고역통과 필터로 고주파 성분은 패스하고 저주파 성분은 더욱 감쇄한다.
// 샤프닝 효과를 가지며 흐릿한 영상에 샤프닝 처리할때 많이 사용한다.
// 다른것과 다르게 일정한 비율로 나누지않고 클리핑처리를 한다. 0과 255가 엄청나게 많아지니 대비가 커지겠지? 날카롭겠지?? 응???
void HPF_LaplacianConvolution(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    double SumProduct = 0.0;

    // Convolution Center를 (1,1)로 잡기 위해 1부터 시작, n-1까지 진행
    for (int i = 1; i < nHeight - 1; i++)
    { // y 행
        for (int j = 1; j < nWidth - 1; j++)
        { // x
            for (int m = -1; m <= 1; m++)
            { // kernel의 행
                for (int n = -1; n <= 1; n++)
                { // kernel의 열
                    // 입력 이미지와 커널을 이용하여 컨볼루션 연산을 수행합니다.
                    // 입력 이미지의 0,0과 LaplacianKernel_HPF의 0,0부터 시작해서 2,2까지 계산하여 전부 더한 값을 넣는다.
                    // LaplacianKernel_HPF에서 m과 n은 -1부터 시작하기때문에 +1을 해줘야된다.
                    SumProduct += Input[(i + m) * nWidth + (j + n)] * LaplacianKernel_HPF[m + 1][n + 1]; // y * 전체 x + x
                }
            }

            // 255보다 크면 255로 조정, 0보다 작으면 0으로 조정
            // 다른곳에서는 절대값을 취하고 일정한 비율로 나누었지만 고역통과 필터에서는 그냥 클리핑 처리한다.
            // 이러면 결과적으로 영상의 대비가 높아져 샤프닝 효과를 얻는다.
            if (SumProduct > 255.0)
                Output[i * nWidth + j] = 255;
            else if (SumProduct < 0.0)
                Output[i * nWidth + j] = 0;
            else // 0~255 사이에 있는 값이라면 그냥 넣는다.
                Output[i * nWidth + j] = (BYTE)SumProduct;

            SumProduct = 0.0; // 초기화
        }
    }

    return;
}

/*
 * @Function Name : swap
 * @Descriotion : 두 개의 입력값을 swap
 * @Input : *left, *right
 * @Output : *left, *right
 */
// 김광제의 설명 - 좌우 변경
void swap(BYTE *left, BYTE *right)
{
    BYTE temp = *left;
    *left = *right;
    *right = temp;

    return;
}

/*
 * @Function Name : MinPooling
 * @Descriotion : 입력 버퍼 중에서 가장 작은 값을 선택
 * @Input : *bArr, nSize
 * @Output : bArr[0]
 */
// 김광제의 설명 - 정렬 이후 가장 작은 값을 선택함
// 솔트 노이즈 제거
BYTE MinPooling(BYTE *bArr, int nSize)
{
    // Sorting
    for (int i = 0; i < nSize - 1; i++)
    {
        for (int j = i + 1; j < nSize; j++)
        {
            if (bArr[i] > bArr[j])
                swap(&bArr[i], &bArr[j]);
        }
    }

    // 정렬 됐으니 0번 인덱스가 가장 작은 값임
    return bArr[0];
}

/*
 * @Function Name : MedianPooling
 * @Descriotion : 입력 버퍼 중에서 중간 값을 선택
 * @Input : *bArr, nSize
 * @Output : *bArr[4]
 */
// 김광제의 설명 - 정렬 이후 중간값을 선택함
// 경계면이 뭉개지지않음 (라인은 살아있고 라인 안쪽이 뭉개진다.)
BYTE MedianPooling(BYTE *bArr, int nSize)
{

    const int nMedian = nSize;
    // Sorting
    for (int i = 0; i < nSize - 1; i++)
    {
        for (int j = i + 1; j < nSize; j++)
        {
            if (bArr[i] > bArr[j])
                swap(&bArr[i], &bArr[j]);
        }
    }
    // 정렬 이후 전체 값에서 중간값을 구함
    // 9 / 2 == 4
    return bArr[nMedian / 2];
}

/*
 * @Function Name : MaxPooling
 * @Descriotion : 입력 버퍼 중에서 가장 큰 값을 선택
 * @Input : *bArr, nSize
 * @Output : bArr[8]
 */
// 김광제의 설명 - 정렬 이후 가장 큰 값을 선택함
// 페퍼노이즈 제거
BYTE MaxPooling(BYTE *bArr, int nSize)
{
    // Sorting
    for (int i = 0; i < nSize - 1; i++)
    {
        for (int j = i + 1; j < nSize; j++)
        {
            if (bArr[i] > bArr[j])
                swap(&bArr[i], &bArr[j]);
        }
    }
    // 여기에서는 가장 큰 값을 선택함
    return bArr[8];
}

/*
 * @Function Name : MedianFilter
 * @Description : 3x3 필터를 이용하여 입력 이미지에 중앙값 필터링을 적용합니다.
 *                각 픽셀에 대해 3x3 영역의 픽셀 값을 가져와 정렬하여 중앙값을 찾아 출력 이미지에 적용합니다.
 * @Input : *Input - 입력 이미지 데이터 배열 포인터,
 *          nWidth - 이미지의 너비 (픽셀 단위),
 *          nHeight - 이미지의 높이 (픽셀 단위)
 * @Output : *Output - 출력 이미지 데이터 배열 포인터
 */
// 김광제의 설명 - 입력영상의 특정 픽셀과 맞닿는 8개의 픽셀을 가져와서 MeddianPooling으로 중간값을 가져와서 출력 화소로 대응
// 중간값이 255일 경우에는 필터 윈도우를 5x5 or 7x7로 늘리자
// 필터 윈도우 늘렸을때 단점 - 필터 크기를 키우면 확인할게 많아져서 성능이 떨어지게 됨 또한 정상값도 중간값으로 변경되어서 영상이 변질
void MedianFilter(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    BYTE temp[9]; // 3x3 필터링을 위한 임시 배열
    int i, j;     // 반복문을 위한 변수

    // 이미지 내부 픽셀을 중심으로 순회합니다.
    for (i = 1; i < nHeight - 1; i++)
    {
        for (j = 1; j < nWidth - 1; j++)
        {
            // 3x3 영역의 픽셀 값을 가져와 temp 배열에 저장합니다.
            temp[0] = Input[(i - 1) * nWidth + j - 1];
            temp[1] = Input[(i - 1) * nWidth + j];
            temp[2] = Input[(i - 1) * nWidth + j + 1];
            temp[3] = Input[i * nWidth + j - 1];
            temp[4] = Input[i * nWidth + j];
            temp[5] = Input[i * nWidth + j + 1];
            temp[6] = Input[(i + 1) * nWidth + j - 1];
            temp[7] = Input[(i + 1) * nWidth + j];
            temp[8] = Input[(i + 1) * nWidth + j + 1];

            // temp 배열에 저장된 값을 정렬한 후 중앙값을 출력 이미지에 저장합니다.
            // MedianPooling은 중간값을 반환해주는 메서드임
            Output[i * nWidth + j] = MedianPooling(temp, 9);
        }
    }

    return;
}

/*
 * @Function Name : MedianFiltering
 * @Description : Filter 크기를 입력받아 Median Filter를 수행합니다.
 * @Input : *Input - 입력 이미지 데이터 배열 포인터,
 *          nWidth - 이미지의 너비 (픽셀 단위),
 *          nHeight - 이미지의 높이 (픽셀 단위),
 *          nSize - 필터 크기
 * @Output : *Output - 출력 이미지 데이터 배열 포인터
 */
// 김광제의 설명 - 사용자로부터 필터의 크기를 입력받아 해당 크기의 중앙값 필터를 이미지에 적용
// 마스크(윈도우)의 크기를 조절하여 다양한 크기의 잡음에 대응할 수 있다.
// 마스크의 크기가 클수록 더 넓은 영역의 픽셀을 고려하기 때문에 더 강한 잡음 제거 효과를 얻을 수 있지만,
// 동시에 이미지의 세부 사항이 더 흐려질 수 있다.
void MedianFiltering(BYTE *Input, BYTE *Output, int nWidth, int nHeight, int nSize)
{
    int nLength = nSize;                                 // 마스크의 한 변의 길이
    int nMargin = nLength / 2;                           // 마스크의 가장자리 크기
    int nWSize = nLength * nLength;                      // 마스크 크기
    BYTE *pTemp = (BYTE *)malloc(sizeof(BYTE) * nWSize); // 필터링을 위한 임시 배열 동적 할당
    int i, j, m, n;                                      // 반복문을 위한 변수들

    // 마스크 영역 내부 픽셀을 중심으로 순회합니다.
    for (i = nMargin; i < nHeight - nMargin; i++)
    {
        for (j = nMargin; j < nWidth - nMargin; j++)
        { // 이미지의 가로, 세로축을 반복하지만 nMargin픽셀은 제외하고 처리한다.
            // 마스크 크기에 따른 픽셀 값을 임시 배열에 저장합니다.
            for (m = -nMargin; m <= nMargin; m++)
            { // 마진 값을 이용하여 커널의 중간에 있는 픽셀을 0,0으로 맞출수있다.
                for (n = -nMargin; n <= nMargin; n++)
                {
                    // 현재 마스크 내의 각 픽셀('m', 'n')에 대해, 원본 이미지(Input)에서 해당 위치의 픽셀 값을 pTemp 배열에 복사한다.
                    pTemp[(m + nMargin) * nLength + (n + nMargin)] = Input[(i + m) * nWidth + j + n];
                }
            }

            // 임시 배열에 저장된 값 중앙값을 출력 이미지에 저장합니다.
            // pTemp 배열에 저장된 픽셀 값들의 중앙값을 계산
            Output[i * nWidth + j] = MedianPooling(pTemp, nWSize);
        }
    }

    free(pTemp); // 동적 할당된 메모리 해제
}

/*
 * @Function Name : push
 * @Descriotion : pStack_x, pStack_y에 vx, vy를 push
 * @Input : *pStack_x, *pStack_yn, arr_size, vx, vy, *top
 * @Output : 0, *pStack_x, *pStack_yn,*top
 * **short *pStack_x, short* pStack_y**: 스택을 나타내는 두 개의 배열 포인터
pStack_x는 x좌표를, pStack_y는 y좌표를 저장한다.
int arr_size: 스택의 최대 크기를 의미한다.
short vx, short vy: 스택에 추가할 x, y 좌표값을 의미한다.
int* top: 현재 스택의 최상단 위치를 나타내는 포인터이다.
 */
// 김광제의 설명 - 스택 자료구조에 새로운 요소를 추가하는 함수 push이다.
int push(short *pStack_x, short *pStack_y, int arr_size, short vx, short vy, int *top)
{
    if (*top >= arr_size)
        return (-1); // 스택이 가득 찼을 때 오류 반환

    // 스택이 꽉 차있지 않았다면 top을 하나 증가시키고, pStack_x와 pStack_y에 새로운 좌표 값을 저장한다. (vx, vy)
    (*top)++;
    pStack_x[*top] = vx; // x값 push
    pStack_y[*top] = vy; // y값 push

    return 0;
}

/*
 * @Function Name : pop
 * @Descriotion : *vx, *vy를 pop
 * @Input : *pStack_x, *pStack_yn, *vx, *vy, *top
 * @Output : 0, *vx, *vy
 * short pStack_x, short pStack_y: 스택을 나타내는 두 개의 배열 포인터
pStack_x는 x좌표를, pStack_y는 y좌표를 저장한다.
short vx, short vy: 제거된 요소의 x, y 좌표값을 저장할 포인터.
int top: 현재 스택의 최상단 위치를 나타내는 포인터이다.
 */
// 김광제의 설명 - 스택 자료구조에서 최상단의 요소를 제거한다.
int pop(short *pStack_x, short *pStack_y, short *vx, short *vy, int *top)
{
    if (*top == 0)
        return (-1); // 스택이 비었을 때 오류 반환

    // 스택이 비어있지 않다면, pStack_x와 pStack_y에서 최상단 요소의 값을 vx와 vy에 복사하고, top을 하나 감소시킨다.
    *vx = pStack_x[*top]; // x값 pop
    *vy = pStack_y[*top]; // y값 pop
    (*top)--;

    return 0;
}

/*
 * @Function Name : ComponentLabeling
 * @Descriotion : 이미지의 Component를 추출하고 Labeling을 진행
 * @Input : *CutImage, nWidth, nHeight
 * @Output : *CutImage
 * BYTE* CutImage: 이미지 데이터를 가리키는 포인터이다. 이 배열에 있는 픽셀 값을 사용하여 레이블링을 수행한다.
 * int nWidth, int nHeight: 각각 입력 이미지의 너비와 높이를 나타낸다.
 * int nLabel : 레이블링 모드를 지정하는 매개변수이다. 이 값에 따라 함수는 다른 종류의 레이블링을 수행한다.
 */
// 김광제의 설명 - 이미지에서 컴포넌트 레이블링을 수행하는 함수이다. 이 방법은 이미지에서 서로 연결된 픽셀 집합(블롭)을 찾아 각각에 고유한 레이블을 할당하는 알고리즘이다.
// 주로 흑백 이미지에서 사용되며, 각 블롭은 픽셀 값이 255(흰색)으로 구성된다. 컴포넌트 레이블링은 이미지의 개별 연결 요소를 식별하고 레이블을 지정하는 과정이다.
void ComponentLabeling(BYTE *CutImage, int nHeight, int nWidth, int nLabel)
{
    // nLabel 값별 실행 동작
    // 1. Max Size Labeling
    // 2. Size Filter Labeling
    // 3. Gray Gap Labeling
    int i, j, m, n, top, area, Out_Area, index;
    int BlobArea[1000] = {
        0,
    };
    long k;
    short curColor = 0, r, c;
    Out_Area = 1;

    // 스택으로 사용할 메모리를 할당하고, 레이블링된 픽셀을 저장하기 위한 메모리를 할당
    short *pStack_x = (short *)malloc(nHeight * nWidth * sizeof(short));
    short *pStack_y = (short *)malloc(nHeight * nWidth * sizeof(short));
    short *pColoring = (short *)malloc(nHeight * nWidth * sizeof(short));

    int arr_size = nHeight * nWidth;

    // 레이블링된 픽셀을 저장하기 위해 메모리 할당
    for (k = 0; k < nHeight * nWidth; k++)
        pColoring[k] = 0; // 메모리 초기화

    for (i = 0; i < nHeight; i++)
    {
        index = i * nWidth;
        for (j = 0; j < nWidth; j++)
        {
            // 이미 방문했거나 픽셀값이 255가 아니라면 처리 안함
            if (pColoring[index + j] != 0 || CutImage[index + j] != 255)
                continue;

            r = i;
            c = j;
            top = 0;
            area = 1;
            curColor++;

            while (1)
            {
            // 이미지의 픽셀을 순회하면서, 아직 레이블이 지정되지 않은 픽셀(값이 255인 픽셀)을 찾는다. 그리고 이 픽셀을 시작점으로 하여 연결된 모든 픽셀에 같은 레이블을 할당한다. 이 과정에서 스택을 사용하여 연결 요소를 추적한다.
            // Grassfire 알고리즘은 연결된 픽셀을 효율적으로 찾는데 사용된다. 이 알고리즘은 이미지 처리, 객체 감지나 세분화 작업에 널리 사용된다.
            GRASSFIRE:
                for (m = r - 1; m <= r + 1; m++)
                {
                    index = m * nWidth;
                    for (n = c - 1; n <= c + 1; n++)
                    {
                        // 관심 픽셀이 영상 경계를 벗어나면 처리 안함
                        if (m < 0 || m >= nHeight || n < 0 || n >= nWidth)
                            continue;

                        if ((int)CutImage[index + n] == 255 && pColoring[index + n] == 0)
                        {
                            pColoring[index + n] = curColor; // 현재 레이블로 마크
                            if (push(pStack_x, pStack_y, arr_size, (short)m, (short)n, &top) == -1)
                                continue;
                            r = m;
                            c = n;
                            area++;
                            goto GRASSFIRE;
                        }
                    }
                }
                if (pop(pStack_x, pStack_y, &r, &c, &top) == -1)
                    break;
            }
            if (curColor < 1000)
                BlobArea[curColor] = area;
        }
    }

    // 가장 면적이 넓은 영역을 찾아내기 위함
    // 레이블링이 끝난 후, 가장 큰 영역("Out_Area")을 찾는다.
    float grayGap = 255.0f / (float)curColor;
    for (i = 1; i <= curColor; i++)
    {
        if (BlobArea[i] >= BlobArea[Out_Area])
            Out_Area = i;
    }

    // CutImage 배열 255로 초기화
    for (k = 0; k < nWidth * nHeight; k++)
        CutImage[k] = 255;

    if (nLabel == 1)
    {
        for (k = 0; k < nWidth * nHeight; k++)
        {
            if (pColoring[k] == Out_Area)
                CutImage[k] = 0; // 가장 큰 것만 저장 (size filtering)
        }
    }
    else if (nLabel == 2)
    {
        for (k = 0; k < nWidth * nHeight; k++)
        {
            if (BlobArea[pColoring[k]] > 500)
                CutImage[k] = 0; // 특정 면적 이상되는 영역만 출력 (500 이상)
        }
    }
    else if (nLabel == 3)
    {
        for (k = 0; k < nWidth * nHeight; k++)
        {
            CutImage[k] = (unsigned char)(pColoring[k] * grayGap); // 밝기값으로 레이블링
        }
    }
    else
    {
        printf("Labeling Mode Error\n");
    }

    // 동적으로 할당된 메모리를 해제하여 메모리 누수를 방지한다.
    free(pStack_x);
    free(pStack_y);
    free(pColoring);

    return;
}

/*
 * @Function Name : DetectObjectEdge
 * @Descriotion : Component Labeling 결과를 입력받아서 Edge를 추출
 * @Input : *Input, nWidth, nHeight
 * @Output : *Output
 */
// 김광제의 설명 - 주어진 이미지에서 객체의 경계를 감지하는 함수이다. 이 함수는 픽셀 단위로 이미지를 처리하여 전경(객체) 픽셀의 가장자리를 찾아내는 역할
// 경계는 객체의 내부이다. 이때 경계를 4방향으로 보냐 8방향으로 보냐를 알아야됨
void DetectObjectEdge(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    int nImgSize = nWidth * nHeight; // 전체 이미지 사이즈

    // 아웃풋을 전부 255로 초기화
    for (int i = 0; i < nImgSize; i++)
        Output[i] = 255;

    for (int i = 0; i < nHeight; i++)
    {
        for (int j = 0; j < nWidth; j++)
        {                                   // 전체 순회
            if (Input[i * nWidth + j] == 0) // 0은 전경(forground) (객체)
            {
                // 위/아래/좌/우 픽셀이 전경이 아니라면(하나라도 0이 아니라면) 경계로 판단
                // 여기에서는 4방향으로 확인하는 코드임
                if (!(Input[(i - 1) * nWidth + j] == 0 && Input[(i + 1) * nWidth + j] == 0 &&
                      Input[i * nWidth + j - 1] == 0 && Input[i * nWidth + j + 1] == 0))
                {
                    // 경계 좌표만 아웃풋에서 밝기값 0으로 표시한다.
                    Output[i * nWidth + j] = 0;
                }
            }
        }
    }
}

/*
 * @Function Name : VerticalFlip
 * @Descriotion : 원본 영상에 대해 Vertical Flip을 수행
 * @Input : *Input, nWidth, nHeight
 * @Output : *Input
 */
// 김광제의 설명 - 단순하게 col을 2로 나누어서 맨 위와 맨 아래 부터 시작해서 값을 전부 바꿈
void VerticalFlip(BYTE *Input, int nWidth, int nHeight)
{
    // 제일 위쪽 픽셀 값과 제일 아래쪽 픽셀 값부터 교환
    for (int i = 0; i < nHeight / 2; i++)
    { // y좌표 : 교환이기 때문에 1/2만 돌면 됨
        for (int j = 0; j < nWidth; j++)
        { // x좌표
            swap(&Input[i * nWidth + j], &Input[(nHeight - 1 - i) * nWidth + j]);
        }
    }
}

/*
 * @Function Name : HorizontalFlip
 * @Descriotion : 원본 영상에 대해 Horizontal Flip을 수행
 * @Input : *Input, nWidth, nHeight
 * @Output : *Input
 */
// 김광제의 설명 - row를 2로 나누어서 좌 우 값을 바꿈
void HorizontalFlip(BYTE *Input, int nWidth, int nHeight)
{
    // 제일 왼쪽 픽셀 값과 제일 오른쪽 픽셀 값부터 교환
    for (int i = 0; i < nWidth / 2; i++)
    { // x좌표 : 교환이기 때문에 1/2만 돌면 됨
        for (int j = 0; j < nHeight; j++)
        { // y좌표
            swap(&Input[j * nWidth + i], &Input[j * nWidth + (nWidth - 1 - i)]);
        }
    }
}

/*
 * @Function Name : Translation
 * @Descriotion : 원본 영상과 이동할 x, y 좌표를 입력받아서 영상을 이동
 * @Input : *Input, nWidth, nHeight, Tx, Ty
 * @Output : *Output
 */
// 김광제의 설명 - 이동은 클리핑 처리를 해주어야되며
// BMP는 영상이 거꾸로 들어가있다
// Tx와 Ty는 오프셋거리이다. (이동 거리)
// 영상이 상하반전 되어있기 때문에 0,0의 위치는 맨 왼쪽 아래이다.
void Translation(BYTE *Input, BYTE *Output, int nWidth, int nHeight, int Tx, int Ty)
{
    // 영상이 상하반전되어있기 때문에 y축의 이동거리에만 -1을 곱해줌
    // 결과적으로 offset이 (양수,양수)로 되어있다면 오른쪽 아래로 이동한것으로 보여짐
    Ty *= -1; // 영상이 거꾸로 되어 있기 때문에, y 좌표 값을 -로 처리
    for (int i = 0; i < nHeight; i++)
    { // 2차원 배열의 영상을 처리하는 기본적인 구분(암기)
        for (int j = 0; j < nWidth; j++)
        { // 2차원 배열의 영상을 처리하는 기본적인 구분(암기)
            // 영상이 프레임 범위를 벗어나는지 확인
            // 기하학적 변환에서는 범위를 벗어나는 값은 버림
            // 이동하는 좌표가 0보다 크고 기존 높이/폭 보다 작아야 처리
            if ((i + Ty < nHeight && i + Ty >= 0) && (j + Tx < nWidth && j + Tx >= 0))
                // 순방향 사상(Forward Mapping)으로 진행
                Output[(i + Ty) * nWidth + (j + Tx)] = Input[i * nWidth + j];
        }
    }
}

/*
 * @Function Name : Scaling
 * @Descriotion : 원본 영상과 확장할 x, y 좌표를 입력받아서 영상을 확장
 * @Input : *Input, nWidth, nHeight, Sx, Sy
 * @Output : *Output
 */
// 김광제의 설명 - 확장을 위한건데 0,0을 기준으로 시작해서 인풋에서 비율을 고려하여 값을 넣음
// Sx, Sy는 확대 비율 Sx, Sy가 1보다 작다면 축소 1보다 크면 확대
// 순방향 사상은 인풋에서 아웃풋으로 이동 (이때 홀이 생기는데 Scaling은 홀이 일정한 간격으로 발생한다.)
// 확대시에는 홀이 발생하고 축소시에는 오버랩이 발생한다.
// 역방향 사상은 아웃풋에서 인풋픽셀을 정하여 매핑한다.
// 역방향 사상은 출력 이미지의 각 픽셀에 대해 원본 이미지에서 해당 값을 가져온다. 이 방법은 빈 공간을 방지하지만, 원본 이미지의 일부 픽셀이 결과 이미지에 반영되지 않을 수 있다.
void Scaling(BYTE *Input, BYTE *Output, int nWidth, int nHeight, double Sx, double Sy)
{
    // Scaling
    int tmpX, tmpY; // 선택한 X,Y좌표를 담기 위한 변수
    for (int i = 0; i < nHeight; i++)
    { // 2차원 배열의 영상을 처리하는 기본적인 구분(암기)
        for (int j = 0; j < nWidth; j++)
        { // 2차원 배열의 영상을 처리하는 기본적인 구분(암기)
            /*1. 순방향 사상으로 진행하는 코드입니다. 주석처리를 바꿔가면서 테스트 해보세요*/
            // 순방향으로 입력영상의 픽셀을 가져갈 목적영상의 픽셀을 구함
            // 순방향으로 확대시에 홀이 발생하게됨 이걸 해결하기위해 보간법을 사용해야됨
            tmpX = (int)(j * Sx); // Sx가 double 이라서 int로 형변환
            tmpY = (int)(i * Sy); // Sy가 double 이라서 int로 형변환

            // 영상이 프레임 범위를 벗어나는지 확인
            // 이동하는 좌표가 기존 높이/폭 보다 작아야 처리(음수는 처리하지 않음)
            if (tmpY < nHeight && tmpX < nWidth)
                // 순방향 사상(Reverse Mapping)으로 진행
                // input 영상의 모든 픽셀을 스캔하면서 tmpX와 tmpY가 반영된 Output 영상의 좌표에 픽셀 값을 저장
                Output[tmpY * nWidth + tmpX] = Input[i * nWidth + j];

            /*2. 역방향 사상으로 진행하는 코드입니다. 주석처리를 바꿔가면서 테스트 해보세요
                // 역방향으로 목적 영상에 대응시킬 입력영상의 픽셀을 구함
                // 출력 이미지의 각 픽셀에 대해 원본 이미지에서 해당 값을 가져온다. 이 방법은 빈 공간을 방지하지만, 원본 이미지의 일부 픽셀이 결과 이미지에 반영되지 않을 수 있다.
                tmpX = (int)(j / Sx);					// 역방향 이기 때문에, *를 /로 수정
                tmpY = (int)(i / Sy);					// 역방향 이기 때문에, *를 /로 수정
                // 영상이 프레임 범위를 벗어나는지 확인
                // 이동하는 좌표가 기존 높이/폭 보다 작아야 처리(음수는 처리하지 않음)
                if (tmpY < nHeight && tmpX < nWidth)
                    // 역방향 사상(Reverse Mapping)으로 진행
                    // input 영상에서 tmpX와 tmpY가 반영된 좌표의 픽셀 값을 Output 영상의 좌표에 픽셀 값을 저장
                    // output 영상의 모든 픽셀을 스캔하면서 input 영상에 해당하는 좌표의 픽셀 값을 가져오는 것과 같은 의미임
                    Output[i * nWidth + j] = Input[tmpY * nWidth + tmpX];
            */
        }
    }
}

/*
 * @Function Name : Rotation
 * @Descriotion : 원본 영상과 회전할 각도를 입력받아서 영상을 회전
 * @Input : *Input, nWidth, nHeight, Tx, Ty
 * @Output : *Output
 */
// 김광제의 설명 - 0,0을 기준으로 잡고 영상을 돌린다. 다만 영상이 거꾸로 되어있기 떄문에 0,0의 위치는 왼쪽 아래임 이러한 이유로 반시계방향으로 돈다.
// 이때도 순방향사상으로 코드를 돌릴시에 홀이 발생한다. 홀이 생기는 모양?은 Translation과 다르다.
//  Angle은 회전 각도
void Rotation(BYTE *Input, BYTE *Output, int nWidth, int nHeight, int Angle)
{
    int tmpX, tmpY;
    double Radian = Angle * 3.141592 / 180.0; // 각도를 라디안 단위로 변환한다. 이미지처리에서 회전을 수행할 때는 라디안 단위를 사용하는 것이 일반적이다.
    for (int i = 0; i < nHeight; i++)
    { // 2차원 배열의 영상을 처리하는 기본적인 구분(암기)
        for (int j = 0; j < nWidth; j++)
        { // 2차원 배열의 영상을 처리하는 기본적인 구분(암기)

            // 순방향 사상
            // 회전된 이미지에서의 각 픽셀의 좌표를 계산하는 부분입니다.
            // 각 픽셀의 새 위치는 회전 변환 행렬을 사용하여 계산된다. 이는 표준 2D 회전 변환 공식을 따른다.
            tmpX = (int)(cos(Radian) * j - sin(Radian) * i); // j의 코사인과 i의 사인 값의 조합으로 계산
            tmpY = (int)(sin(Radian) * j + cos(Radian) * i); // j의 사인과 i의 코사인 값의 조합으로 계산

            // 계산된 위치가 이미지 프레임 내에 있는지 확인한다.
            // 이동하는 좌표가 0보다 크고 기존 높이/폭 보다 작아야 처리
            if ((tmpY < nHeight && tmpY >= 0) && (tmpX < nWidth && tmpX >= 0))
                // 순방향 사상(Forward Mapping)으로 진행
                // 순방향 사상은 원본 이미지의 각 픽셀을 타겟 이미지의 새 위치로 직접 매핑한다.
                // 원본 이미지(Input)의 픽셀 값을 새 위치에 있는 출력 이미지(Output)의 해당 픽셀에 할당한다.
                Output[tmpY * nWidth + tmpX] = Input[i * nWidth + j];

            /*2. 역방향 사상
            // 역방향 사상은 회전 변환의 역행렬을 사용한다.
            tmpX = (int)(cos(Radian) * j + sin(Radian) * i);			// 역방향을 위해서 역행렬을 적용
            tmpY = (int)(-sin(Radian) * j + cos(Radian) * i);			// 역방향을 위해서 역행렬을 적용
            // 영상이 프레임 범위를 벗어나는지 확인
            // 이동하는 좌표가 0보다 크고 기존 높이/폭 보다 작아야 처리
            if ((tmpY < nHeight && tmpY >= 0) && (tmpX < nWidth && tmpX >= 0))
                // 역방향 사상(Reverse Mapping)으로 진행
                // 출력 이미지의 각 픽셀에 대해 원본 이미지에서 해당 픽셀 값을 가져온다.
                // 이 방법은 결과 이미지의 모든 픽셀을 채우는 데 더 효과적일 수 있으며, 순방향 사상에서 발생할 수 있는 "홀"이나 빈 공간을 방지한다.
                Output[i * nWidth + j] = Input[tmpY * nWidth + tmpX];
            */
        }
    }
}

/*
 * @Function Name : Erosion
 * @Descriotion : 입력 영상을 침식합니다. 전경화소 주변에 배경화소가 있는 경우를 검사하여 처리합니다.
 * @Input : *Input - 입력 이미지 데이터 배열 포인터,
 *          nWidth - 이미지의 너비 (픽셀 단위),
 *          nHeight - 이미지의 높이 (픽셀 단위)
 * @Output : *Output - 출력 이미지 데이터 배열 포인터
 */
// 김광제의 설명 - 이미지 처리에서 사용하는 "침식" 개념을 구현하는 함수이다. 침식은 주로 이진 이미지 처리에서 사용되며, 전경 객체의 경계를 축소하거나 객체 내의 작은 구멍을 제거하는 데 사용된다.
// 입력영상에서 값을 바꾸는것이 아닌 결과영상에 값을 옮기기 때문에 이미 바꾼 픽셀은 다른 픽셀에 영향을 주지 않음
void Erosion(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    for (int i = 1; i < nHeight - 1; i++)
    {
        for (int j = 1; j < nWidth - 1; j++)
        {                                     // 이중 for문을 사용하여 이미지의 모든 픽셀을 순회한다. 경계픽셀은 제외하고 처리된다.
            if (Input[i * nWidth + j] == 255) // 전경화소를 기준으로 검사합니다.
            {
                // 4주변 화소가 모두 전경화소가 아닌 경우에 처리합니다.
                // 즉, 전경화소 주변에 배경화소가 하나라도 존재한다면 침식을 수행합니다.
                if (!(Input[(i - 1) * nWidth + j] == 255 &&
                      Input[(i + 1) * nWidth + j] == 255 &&
                      Input[i * nWidth + j - 1] == 255 &&
                      Input[i * nWidth + j + 1] == 255))
                {
                    // 검은색(배경) 값 0으로 처리합니다.
                    Output[i * nWidth + j] = 0;
                }
                else
                {
                    // 모두 전경화소이면, 흰색(전경) 값 255로 처리합니다.
                    Output[i * nWidth + j] = 255;
                }
            }
            else
            {
                // 배경화소는 검은색(배경) 값 0으로 처리합니다.
                Output[i * nWidth + j] = 0;
            }
        }
    }
}

/*
 * @Function Name : Dilation
 * @Descriotion : 입력 영상을 팽창
 * @Input : *Input, nWidth, nHeight
 * @Output : *Output
 */
// 김광제의 설명 - 배경에 대해서 4방향에 객체가 닿아있다면 그 픽셀까지 전경으로 변경
//  이미지 처리에서 사용되는 "팽창" 개념의 구현이다. 팽창은 주로 이진 이미지 처리에서 사용되며, 전경 객체의 경계를 확장하거나 객체들을 연결하는데 사용
void Dilation(BYTE *Input, BYTE *Output, int nWidth, int nHeight)
{
    for (int i = 1; i < nHeight - 1; i++)
    {
        for (int j = 1; j < nWidth - 1; j++)
        {                                   // 이중 for 반복문을 사용하여 이미지의 모든 픽셀을 순회한다. 경계 픽셀은 제외하고 처리한다.
            if (Input[i * nWidth + j] == 0) // 배경화소를 기준으로 검사
            {
                // 4주변화소가 모두 배경화소가 아닌 경우(하나라도 전경화소가 있다면~)
                if (!(Input[(i - 1) * nWidth + j] == 0 &&
                      Input[(i + 1) * nWidth + j] == 0 &&
                      Input[i * nWidth + j - 1] == 0 &&
                      Input[i * nWidth + j + 1] == 0))

                    // 흰색(전경) 값 255로 처리
                    Output[i * nWidth + j] = 255;
                else
                    // 모두 배경화소이면, 검은색(배경) 값 0으로 처리
                    Output[i * nWidth + j] = 0;
            }
            else
                // 전경화소는 흰색(전경) 값 255로 처리
                Output[i * nWidth + j] = 255;
        }
    }
}

/*
 * @Function Name : getBlackNeighbours
 * @Descriotion : 입력 영상을 팽창
 * @Input : *Input, nWidth, nHeight
 * @Output : *Output
 * int row, int col: 검사할 픽셀의 행과 열 위치
 * imageMatrix: 이미지 데이터를 나타내는 2차원 배열이다. (전역)
 * imagePixel: 이 함수에서 찾고자 하는 특정 색상의 픽셀 값으로, 여기서는 검은색 픽셀을 의미한다. (전역)
 */
// 김광제의 설명 - 이미지 행렬 내에서 특정 픽셀 주변의 특정 색상 픽셀의 개수를 계산하는 함수이다.
// 이 함수는 주어진 행(row)과 열(col)에 위치한 픽셀을 중심으로 8방향 이웃 픽셀들을 검사하여, 주어진 색상의 픽셀 수를 계산
int getBlackNeighbours(int row, int col)
{
    int i, j, sum = 0;

    for (i = -1; i <= 1; i++)
    {
        for (j = -1; j <= 1; j++)
        {                         // 중심 픽셀 주변의 모든 픽셀을 순회한다. 여기서 i와 j는 중심 픽셀로부터의 상대적 위치를 나타낸다
            if (i != 0 || j != 0) // 주변 픽셀이 imagePixel(검은색)과 일치하는지 확인
                // 일치하는 경우 sum을 1 증가시켜 해당 색상의 픽셀 수를 계산
                sum += (imageMatrix[row + i][col + j] == imagePixel);
        }
    }

    return sum;
}

/*
 * @Function Name : getBWTransitions
 * @Descriotion : 픽셀의 중심을 기준으로 3X3 영역에서 흰색과 검은색의 전환 개수를 계산
 * @Input : row, col
 * @Output : 흰색, 검은색 전환 개수
 * int row, int col: 검사할 픽셀의 행과 열 위치
 * imageMatrix: 이미지 데이터를 나타내는 2차원 배열이다. (전역)
 * blackPixel: 여기서는 흰색 픽셀을 의미한다.
 * imagePixel: 검은색 픽셀을 의미한다.
 */
// 김광제의 설명 - 주어진 픽셀의 8-방향 이웃에 대해 흰색에서 검은색으로 바뀌는 전환을 감지하여 그 수를 계산
int getBWTransitions(int row, int col)
{
    // 중심 픽셀을 기준으로 상하좌우 및 대각선 방향에 있는 이웃 픽셀들을 순회하며 흰색에서 검은색으로의 전환을 감지
    // 각 조건문은 두 개의 인접 픽셀을 검사한다. 첫 번째 픽셀이 흰색(blankPixel)이고, 두 번째 픽셀이 검은색(imagePixel)인 경우에 한해 전환을 감지
    // 모든 방향에 대해 이러한 검사를 수행하고, 각각의 검사 결과(전환 발생 시 1, 아닐 시 0)를 합산하여 반환
    return ((imageMatrix[row - 1][col] == blankPixel && imageMatrix[row - 1][col + 1] == imagePixel) + (imageMatrix[row - 1][col + 1] == blankPixel && imageMatrix[row][col + 1] == imagePixel) + (imageMatrix[row][col + 1] == blankPixel && imageMatrix[row + 1][col + 1] == imagePixel) + (imageMatrix[row + 1][col + 1] == blankPixel && imageMatrix[row + 1][col] == imagePixel) + (imageMatrix[row + 1][col] == blankPixel && imageMatrix[row + 1][col - 1] == imagePixel) + (imageMatrix[row + 1][col - 1] == blankPixel && imageMatrix[row][col - 1] == imagePixel) + (imageMatrix[row][col - 1] == blankPixel && imageMatrix[row - 1][col - 1] == imagePixel) + (imageMatrix[row - 1][col - 1] == blankPixel && imageMatrix[row - 1][col] == imagePixel));
    // 주어진 픽셀 주변에서 발생하는 흑백 전환의 총 횟수를 반환
}

/*
 * @Function Name : main
 * @Descriotion : Image Processing main 함수로 switch 문에 따라 함수를 호출하여 기능을 수행
 */
void main()
{
    // ver 0.2 변수 추가
    // 밝기 값 조정시에 사용함
    int nBrigntness = 0; // 밝기 값

    // 대비 값 조정시에 사용함
    double dContrast = 0; // 대비 값

    // ver 0.3 변수 추가
    // Histogram 생성한 것을 이곳에 넣음
    int nHisto[256] = {
        0,
    };

    // 곤잘레스 알고리즘으로 얻는 Histogram의 임계값을 저장
    BYTE bThreshold;

    // 사용자가 지정한 Histogram 임계값 저장
    int nThreshold = 0; // threshold를 입력
    // 사용자에게 입력받는 필터 한 변의 크기 홀수여야됨
    int nFilter = 0; // filter의 한변의 크기
    // 사용자가 원하는 레이블링 모드 저장
    int nLabel = 0; // Labeling 모드

    // 원하는 이미지 처리 기능 저장
    int nMode = 0; // 기능 선택

    // 사용자가 입력하는 이미지 경로 저장
    CHAR PATH[256] = {
        0,
    }; // 파일 경로

    // 오프셋 거리 (이동 거리)
    int Tx, Ty;
    // 확대 축소 비율
    double Sx, Sy;
    // 회전각도
    int Angle;

    // 사용자 입력
    printf("=================================\n\n");
    printf("Image Processing Program\n\n");
    printf("1.  Inverse Image\n");
    printf("2.  Adjust Brightness\n");
    printf("3.  Adjust Contrast\n");
    printf("4.  Generate Histogram\n");
    printf("5.  Generate Binarization - Gonzalez Method\n");
    printf("6.  Generate Binarization\n");
    printf("7.  Histogram Stretching\n");
    printf("8.  Histogram Equalization\n");
    printf("9.  Average Convolution\n");
    printf("10. Gaussian Convolution\n");
    printf("11. Laplacian Convolution\n");
    printf("12. Prewitt X Convolution\n");
    printf("13. Prewitt Y Convolution\n");
    printf("14. Prewitt Convolution\n");
    printf("15. Sobel X Convolution\n");
    printf("16. Sobel Y Convolution\n");
    printf("17. Sobel Convolution\n");
    printf("18. Laplacian High Pass Filter Convolution\n");
    printf("19. Meadian Filter, Min Pooling, Min Pooling, Max Pooling\n");
    printf("20. Meadian Filter using Variable Filter\n");
    printf("21. Component Labeling\n");
    printf("22. DetectObjectEdge\n");
    printf("23. VerticalFlip\n");
    printf("24. HorizontalFlip\n");
    printf("25. Translation\n");
    printf("26. Scaling\n");
    printf("27. Rotation\n");
    printf("28. Erosion\n");
    printf("29. Dilation\n");
    printf("=================================\n\n");

    printf("원하는 기능의 번호를 입력하세요 : ");
    scanf_s("%d", &nMode);

    printf("원본 이미지 파일의 경로를 입력하세요 : ");
    scanf_s("%s", PATH, sizeof(PATH));

    // 이미지 파일 오픈
    nErr = fopen_s(&fp, PATH, "rb");

    if (NULL == fp)
    {
        printf("Error : file open error = %d\n", nErr);
        return;
    }

    // BMT Header 구조체 선언
    BITMAPFILEHEADER hf;    // BMP 파일헤더 14Bytes
    BITMAPINFOHEADER hInfo; // BMP 인포헤더 40Bytes
    RGBQUAD hRGB[256] = {
        0,
    }; // 파레트 (256 * 4Bytes)

    // BITMAPFILEHEADER
    fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);

    // BITMAPINFOHEADER
    fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);

    // RGBQUAD
    fread(hRGB, sizeof(RGBQUAD), 256, fp);

    // 변수 선언
    FILE *fp = NULL;  // 파일 포인터
    errno_t nErr = 0; // Error
    int nImgSize = 0; // 이미지 크기

    // 이미지 크기 계산(가로 X 세로)
    nImgSize = hInfo.biWidth * hInfo.biHeight;

    // 원본 이미지와 출력 이미지를 저장할 버퍼 할당
    BYTE *Input = (BYTE *)malloc(nImgSize);
    BYTE *Output = (BYTE *)malloc(nImgSize);

    // Ver 0.5
    BYTE *Temp = (BYTE *)malloc(nImgSize); // prewitt convolution과 sobel convolution을 위해 임시 버퍼 생성

    if (NULL == Input || NULL == Output || NULL == Temp)
    {
        printf("Error : memory allocation error\n");
        return;
    }

    // 버퍼 초기화
    memset(Input, 0, nImgSize);
    memset(Output, 0, nImgSize);
    memset(Temp, 0, nImgSize); // prewitt convolution과 sobel convolution을 위해 임시 버퍼 초기화

    //
    fread(Input, sizeof(BYTE), nImgSize, fp);
    fclose(fp);

    // nMode에 따라 기능을 계속 추가하면서 진행할 예정임
    switch (nMode)
    {

    case 1:
        // Inverse
        InverseImage(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../inverse.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 2:
        // brightness
        printf("밝기 조절 값(정수)을 입력하세요 : ");
        scanf_s("%d", &nBrigntness);
        // scanf로 수치를 받아서 이만큼 더하거나 뺄거임
        AdjustBrightness(Input, Output, hInfo.biWidth, hInfo.biHeight, nBrigntness);

        nErr = fopen_s(&fp, "../brigntness.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 3:
        // contrast
        printf("대비 조절 값(0보다 큰 실수 값)을 입력하세요 : ");
        scanf_s("%lf", &dContrast);

        if (dContrast < 0)
        {
            printf("Error : input value error = %d\n", dContrast);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        AdjustContrast(Input, Output, hInfo.biWidth, hInfo.biHeight, dContrast);

        nErr = fopen_s(&fp, "../contrast.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 4:
        // Histogram 생성
        GenerateHistogram(Input, nHisto, hInfo.biWidth, hInfo.biHeight);

        // 히스토그램 값을 화면에 출력
        for (int i = 0; i < 256; i++)
            printf("%d, %d\n", i, nHisto[i]);

        free(Input);
        free(Output);
        free(Temp);
        return;

    case 5: // 이부분은 곤잘레스를 사용해서 최적의 임계값을 찾아서 히스토그램 생성
        // Histogram 생성
        GenerateHistogram(Input, nHisto, hInfo.biWidth, hInfo.biHeight);

        // Gonzales Method로 threshold를 결정
        bThreshold = GonzalezMethod(nHisto);

        // 이진화 진행
        GenerateBinarization(Input, Output, hInfo.biWidth, hInfo.biHeight, bThreshold);

        nErr = fopen_s(&fp, "../gonzalez_binarization.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 6: // 이부분은 곤잘레스를 사용하지않고 사용자가 입력하는 임계값을 사용한다.
        printf("이진화 임계값(Threshold)를 입력하세요 : ");
        scanf_s("%d", &nThreshold);

        GenerateBinarization(Input, Output, hInfo.biWidth, hInfo.biHeight, (BYTE)nThreshold);

        nErr = fopen_s(&fp, "../binarization.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 7:
        // Histogram 생성
        GenerateHistogram(Input, nHisto, hInfo.biWidth, hInfo.biHeight);

        // 히스토그램 스트래칭 진행
        HistogramStretching(Input, Output, nHisto, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../stretching.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 8:
        // Histogram 생성
        GenerateHistogram(Input, nHisto, hInfo.biWidth, hInfo.biHeight);

        // 히스토그램 평활화 진행
        HistogramEqualization(Input, Output, nHisto, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../equalization.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 9:
        // Average Convolution
        AverageConvolution(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../average.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 10:
        // Gaussian Convolution
        GaussianConvolution(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../guassian.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 11:
        // Laplacian Convolution
        LaplacianConvolution(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../laplacian_edge.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 12:
        // Prewitt X Convolution
        X_PrewittConvolution(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../prewitt_x_edge.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 13:
        // Prewitt Y Convolution
        Y_PrewittConvolution(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../prewitt_y_edge.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 14:
        // Prewitt Convolution

        // Prewitt X 결과를 Temp에 저장
        // 1. Prewitt X Convolution 적용 :
        // X_PrewittConvolution 함수를 사용하여 입력 이미지에 프레윗 X 방향 컨볼루션 필터를 적용하고, 결과를 임시 배열 Temp에 저장한다.
        X_PrewittConvolution(Input, Temp, hInfo.biWidth, hInfo.biHeight);

        // Prewitt Y 결과를 Output에 저장
        // 2. Prewitt Y Convolution 적용 :
        // Y_PrewittConvolution 함수를 사용하여 입력 이미지에 프레윗 Y 방향 컨볼루션 필터를 적용하고, 결과를 Output 배열에 저장한다.
        Y_PrewittConvolution(Input, Output, hInfo.biWidth, hInfo.biHeight);

        // Prewitt X 결과와 Y 결과를 비교하여 더 큰 값을 Output에 저장
        // 3. X, Y 결과 비교 및 저장:
        // 프레윗 X와 Y 컨볼루션 결과를 비교하여 각 픽셀 위치에서 더 큰 값을 Output 배열에 저장한다. 이는 각 방향의 가장자리 강도를 결합한다.
        for (int i = 0; i < nImgSize; i++)
        {
            if (Temp[i] > Output[i])
                Output[i] = Temp[i];
        }

        nErr = fopen_s(&fp, "../prewitt_edge.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 15:
        // Sebel X Convolution
        X_SobelConvolution(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../sobel_x_edge.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 16:
        // Sobel Y Convolution
        Y_SobelConvolution(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../sobel_y_edge.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 17:
        // Sobel Convolution

        // Sobel X 결과를 Temp에 저장
        X_SobelConvolution(Input, Temp, hInfo.biWidth, hInfo.biHeight);

        // Sobel Y 결과를 Output에 저장
        Y_SobelConvolution(Input, Output, hInfo.biWidth, hInfo.biHeight);

        // 원본 이미지에 Sobel X와 Sobel Y Convolution 필터를 적용한 후, 두 결과 중 더 큰 값을 sobel_edge.bmp 파일로 저장하는 과정을 수행한다.
        for (int i = 0; i < nImgSize; i++)
        {
            if (Temp[i] > Output[i])
                Output[i] = Temp[i];
        }

        nErr = fopen_s(&fp, "../sobel_edge.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 18:
        // Laplacian High-pass Filter Convolution
        HPF_LaplacianConvolution(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../laplacian_HPF.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            return;
        }

        break;

    case 19:
        // MedianFilter Filter Convolution
        MedianFilter(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../median.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            return;
        }

        break;

    case 20:
        // MedianFilter Filter Convolution
        printf("Filter의 한변의 크기를 입력하세요 : ");
        scanf_s("%d", &nFilter);

        MedianFiltering(Input, Output, hInfo.biWidth, hInfo.biHeight, nFilter);

        nErr = fopen_s(&fp, "../median_filter.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            return;
        }

        break;

    case 21:
        printf("\n========== Labeling Mode =========\n");
        printf("1. Max Size Labeling \n");
        printf("2. Size Filter Labeling ( > 500 )\n");
        printf("3. Gray Gap Labeling\n");
        printf("Labeling Mode를 입력하세요 : ");
        scanf_s("%d", &nLabel);

        // 1. 최대 크기 레이블링 : 가장 큰 영역만 레이블링한다.
        // 2. 크기 필터 레이블링(500이상) : 특정 크기(여기서는 500) 이상의 영역만 레이블링한다.
        // 3. 회색 간격 레이블링 : 레이블에 따라 다른 회색조를 할당한다.

        ComponentLabeling(Input, hInfo.biWidth, hInfo.biHeight, nLabel);
        nErr = fopen_s(&fp, "../labeling.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            return;
        }

        for (int i = 0; i < nImgSize; i++)
        {
            Output[i] = Input[i];
        }

        break;

    case 22:
        DetectObjectEdge(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../enge.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            return;
        }

        break;

    case 23:
        VerticalFlip(Input, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../vflip.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            return;
        }

        for (int i = 0; i < nImgSize; i++)
        {
            Output[i] = Input[i];
        }

        break;

    case 24:
        HorizontalFlip(Input, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../hflip.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            return;
        }

        for (int i = 0; i < nImgSize; i++)
        {
            Output[i] = Input[i];
        }

        break;

    case 25:
        printf("이동 X 축 오프셋 값을 입력하세요 : ");
        scanf_s("%d", &Tx);
        printf("이동 Y 축 오프셋 값을 입력하세요 : ");
        scanf_s("%d", &Ty);
        Translation(Input, Output, hInfo.biWidth, hInfo.biHeight, Tx, Ty);

        nErr = fopen_s(&fp, "../translation.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            return;
        }

        break;

    case 26:
        printf("확대/축소 X 축 비율 값을 입력하세요 : ");
        scanf_s("%lf", &Sx);
        printf("확대/축소 Y 축 비율 값을 입력하세요 : ");
        scanf_s("%lf", &Sy);

        // 사용자로부터 X축과 Y축의 확대/축소 비율을 입력받은 후, Scaling 함수를 사용하여 이미지를 해당 비율로 확대 또는 축소시키고,
        // 결과를 scaling.bmp 파일로 저장하는 과정을 수행한다.
        // 입력된 비율 값이 음수일 경우 오류 메시지를 출력

        if (Sx < 0 || Sy < 0)
        {
            printf("Error : input value error = %lf, %lf\n", Sx, Sy);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        Scaling(Input, Output, hInfo.biWidth, hInfo.biHeight, Sx, Sy);

        nErr = fopen_s(&fp, "../scaling.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            return;
        }

        break;

    case 27:
        printf("회전할 각도를 입력하세요 : ");
        scanf_s("%d", &Angle);

        Rotation(Input, Output, hInfo.biWidth, hInfo.biHeight, Angle);

        nErr = fopen_s(&fp, "../rotation.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            return;
        }

        break;

    case 28:
        // Gaussian Convolution
        Erosion(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../erosion.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    case 29:
        // Gaussian Convolution
        Dilation(Input, Output, hInfo.biWidth, hInfo.biHeight);

        nErr = fopen_s(&fp, "../dilation.bmp", "wb");
        if (NULL == fp)
        {
            printf("Error : file open error = %d\n", nErr);
            free(Input);
            free(Output);
            free(Temp);
            return;
        }

        break;

    default:
        printf("입력 값이 잘못되었습니다.\n");
        free(Input);
        free(Output);
        free(Temp);
        return;
    }

    fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
    fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
    fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
    fwrite(Output, sizeof(BYTE), nImgSize, fp);
    fclose(fp);

    free(Input);
    free(Output);
    free(Temp);

    return;
}