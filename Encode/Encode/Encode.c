#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#define PI 3.1415926535
#define WIDTH 352
#define HEIGHT 288
int frame_size;

#define INTER_BLOCK_SIZE 8 // inter 블록의 사이즈
#define SEARCH_RANGE 15  // inter search range 가로, 세로 찾는 횟수 inter search range -> 15 + (8 - 1) = 22
#define SEARCH_INDEX 7 // reference_arr index 범위 때 사용

int DPCM_MOD;
int QP_DC;
int QP_AC;

void INTRA(int, int, unsigned char*);
void DPCM(int, int, int, int*);
void REVERSE_INTRA(int, int, int*);
void REVERSE_DPCM(int, int, int, int*);

void INTER(int, int, int, unsigned char*, unsigned char*);
void REVERSE_INTER(int, int, int, int*, unsigned char*, int*);


void DCT(int, int, int, int, int*);
void IDCT(int, int, double, double, int*);
//void IDCT(int, int, double*);


// intra, intra dpcm
int intra_period = 10;
int intra_enable = 0;
unsigned char intra_out[288 * 352]; // 파일 생성 때 사용
int intra_out_int[288 * 352]; // 부수가 들어 있는 intra_out
int mod_arr[44 * 36];  // 각 블록의 intra model 를 저장 
unsigned char intra_dpcm_out[288 * 352]; // dpcm해서 파일 출력 때 사용
int intra_dpcm_int[288 * 352]; // 부수를 저장하는 dpcm 배열

// reverse intra, reverse intra dpcm
unsigned char re_intra_out[288 * 352]; // 역intra 함수 사용해서 출력한 배열(파일 생성 때 사용)
int re_intra_int[288 * 352];
int re_intra_dpcm_int[288 * 352];
unsigned char re_intra_dpcm_out[288 * 352];

// dct, idct
int AC_Flag[44 * 36];
double dct_out_double[288 * 352];
int dct_out_int[288 * 352];
int idct_out_int[288 * 352];
unsigned char dct_out[288 * 352];
unsigned char idct_out[288 * 352];

// inter 전 프레임 데이터
unsigned char ref_arr_y[288 * 352]; //메모리
unsigned char ref_arr_u[144 * 176];
unsigned char ref_arr_v[144 * 176];
int MV[(HEIGHT / INTER_BLOCK_SIZE) * (WIDTH / INTER_BLOCK_SIZE) * 2];
unsigned char inter_out[288 * 352];
int inter_out_int[288 * 352];
unsigned char re_cur_arr[288 * 352];
int* dpcm_input;
unsigned char* out_put;

// 현재 프레임을 저장하는 포인터
unsigned char* out_y;
unsigned char* out_u;
unsigned char* out_v;

int arr_y_int[288 * 352];
int arr_u_int[144 * 176];
int arr_v_int[144 * 176];

// entropy code 
#define N 8
unsigned char zigzag[288 * 352];
int zigzag_int[288 * 352];
void zig_zag_scan(int, int, int*, int*);

void entropy_encode(int, int*, int*);
int entropy_total_size = 0;
int entropy_index = 0;
//----------------------------------------------------------------------------------------------------------
// argv[1]: frame_size						argv[2]: QP_DC						argv[3]: QP_AC
// argv[4]: pixel DPCM mode(mod_arr)		argv[5]: intra period				argv[6]: intra enable	
// argv[7]: original file location			argv[8]: restoration file location
//----------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{

	frame_size = atoi(argv[1]);
	QP_DC = atoi(argv[2]);
	QP_AC = atoi(argv[3]);
	DPCM_MOD = atoi(argv[4]);
	intra_period = atoi(argv[5]);
	intra_enable = atoi(argv[6]); // 0 / 1: Disable / Enable

	//frame_size = 300;
	//QP_DC = 1;
	//QP_AC = 1;
	//DPCM_MOD = 0;
	//intra_period = 0;
	//intra_enable = 0; // 0 / 1: Disable / Enable

	FILE* fp;
	FILE* fp1;
	FILE* fp2;
	FILE* fp3;

	FILE* fp_intra;
	FILE* fp_re_file;
	FILE* fp_intra_dpcm_y;
	FILE* fp_re_intra_dpcm_y;
	FILE* fp_re_intra_y;

	FILE* fp_inter;
	FILE* fp_re_inter;

	FILE* fp_dct;
	FILE* fp_idct;

	FILE* fp_compress;

	fopen_s(&fp, argv[7], "rb");   //영상 읽어오기
	fopen_s(&fp_re_intra_y, argv[8], "wb");  //re_intra_y
	fopen_s(&fp1, "C:/Users/a/Documents/Connect_Git/ICSP_Codec/CIF(352x288)/football_y.yuv", "wb");  //y
	fopen_s(&fp2, "C:/Users/a/Documents/Connect_Git/ICSP_Codec/CIF(352x288)/football_u.yuv", "wb");  //u
	fopen_s(&fp3, "C:/Users/a/Documents/Connect_Git/ICSP_Codec/CIF(352x288)/football_v.yuv", "wb");  //v

	fopen_s(&fp_intra, "C:/Users/a/Documents/Connect_Git/ICSP_Codec/CIF(352x288)/football_intra.yuv", "wb");  //intra
	fopen_s(&fp_re_file, "C:/Users/a/Documents/Connect_Git/ICSP_Codec/CIF(352x288)/football_intra_y.yuv", "wb");  //intra_y
	fopen_s(&fp_intra_dpcm_y, "C:/Users/a/Documents/Connect_Git/ICSP_Codec/CIF(352x288)/football_intra_dpcm_y.yuv", "wb");  //intra_dpcm_y
	fopen_s(&fp_re_intra_dpcm_y, "C:/Users/a/Documents/Connect_Git/ICSP_Codec/CIF(352x288)/football_re_intra_dpcm_y.yuv", "wb");  //re_intra_dpcm_y = intra_out


	fopen_s(&fp_inter, "C:/Users/a/Documents/Connect_Git/ICSP_Codec/CIF(352x288)/football_inter.yuv", "wb");
	fopen_s(&fp_re_inter, "C:/Users/a/Documents/Connect_Git/ICSP_Codec/CIF(352x288)/football_re_inter.yuv", "wb");

	fopen_s(&fp_dct, "C:/Users/a/Documents/Connect_Git/ICSP_Codec/CIF(352x288)/football_dct.yuv", "wb");  //y
	fopen_s(&fp_idct, "C:/Users/a/Documents/Connect_Git/ICSP_Codec/CIF(352x288)/football_idct.yuv", "wb");  //y


	FILE* fp_entropy;
	fopen_s(&fp_entropy, "../../result/entropy.txt", "wb");

	fopen_s(&fp_compress, "../../result/compress.txt", "ab+");


	unsigned char* pic = (unsigned char*)malloc(WIDTH * HEIGHT * 3 / 2); //시작위치

	int i, j;
	int frame_con;


	for (frame_con = 1; frame_con <= frame_size; frame_con++) {
		//한 프레임씩 읽어오기

		fread(pic, 1, WIDTH * HEIGHT * 3 / 2, fp);

		// Y, U, V 성분을 저장한 2중 포인터

		unsigned char* arr_y = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH * HEIGHT);			//Y
		unsigned char* arr_u = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH * HEIGHT / 4);		//U
		unsigned char* arr_v = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH * HEIGHT / 4);		//V

		for (i = 0; i < HEIGHT; i++)
		{
			for (j = 0; j < WIDTH; j++)
			{
				arr_y[i * WIDTH + j] = pic[i * WIDTH + j];  // Y 성분을 읽어와서 2중 포인터에 저장
			}
		}
		for (i = 0; i < HEIGHT; i++)
		{
			for (j = 0; j < WIDTH; j++)
			{
				arr_y_int[i * WIDTH + j] = arr_y[i * WIDTH + j];
			}
		}

		for (i = 0; i < HEIGHT / 2; i++)
		{
			for (j = 0; j < WIDTH / 2; j++)
			{
				arr_u[i * WIDTH / 2 + j] = pic[HEIGHT * WIDTH + i * WIDTH / 2 + j];                 //U

				arr_v[i * WIDTH / 2 + j] = pic[HEIGHT * WIDTH * 5 / 4 + i * WIDTH / 2 + j];         //V
			}
		}
		for (i = 0; i < HEIGHT / 2; i++)
		{
			for (j = 0; j < WIDTH / 2; j++)
			{
				arr_u_int[i * WIDTH / 2 + j] = arr_u[i * WIDTH / 2 + j];
				arr_v_int[i * WIDTH / 2 + j] = arr_v[i * WIDTH / 2 + j];         //V
			}
		}

		// 새로운 파일에 쓰기
		// Y, U, V성분으로 나눔
		fwrite(&arr_y[0], 1, WIDTH * HEIGHT, fp1); // Y
		fwrite(pic + WIDTH * HEIGHT, 1, WIDTH * HEIGHT / 4, fp2); // U
		fwrite(pic + WIDTH * HEIGHT * 5 / 4, 1, WIDTH * HEIGHT / 4, fp3); // V

		/*int* connection_code = (int*)malloc(sizeof(int) * 105000);*/
		int connection_code[110000];

		int* entropy_result = (int*)malloc(sizeof(int) * 20000);

		int index;

		if ((frame_con % (intra_period + 1) == 1) || (intra_period == 0)) {

			//---------------------------------------------
			//					intra y성분
			//---------------------------------------------
			// intra_y와 dpcm

			if (intra_enable == 1) {

				INTRA(352, 288, arr_y);

				dpcm_input = intra_out_int;
				out_put = intra_out_int;

				fwrite(&out_put[0], 1, WIDTH * HEIGHT, fp_intra);	// intra_y
			}
			else {
				dpcm_input = arr_y_int;
			}

			/*fwrite(&out_put[0], 1, WIDTH * HEIGHT, fp_re_file);*/

			DPCM(DPCM_MOD, 352, 288, dpcm_input);
			fwrite(&intra_dpcm_out[0], 1, WIDTH * HEIGHT, fp_intra_dpcm_y);

			/*for (int a = 0; a < 288; a++) {
				for (int b = 0; b < 352; b++) {
					printf("%d,", intra_dpcm_int[a * 352 + b]);
				}
				printf("\n");
			}*/

			//	DCT
			DCT(WIDTH, HEIGHT, QP_DC, QP_AC, intra_dpcm_int);
			fwrite(&dct_out[0], 1, WIDTH * HEIGHT, fp_dct);

			/*for (int a = 0; a < 288; a++) {
				for (int b = 0; b < 352; b++) {
					printf("%d,", dct_out_int[a * 352 + b]);
				}
				printf("\n");
			}*/


			//--------------------
			//		코드 연결
			//--------------------

			index = 0; // connection_code 배열 길이
			for (int a = 0; a < 36; a++) {
				for (int b = 0; b < 44; b++) {
					if (intra_enable == 1) {
						connection_code[index++] = mod_arr[a * 44 + b];
					}
					connection_code[index++] = dct_out_int[a * 8 * WIDTH + b * 8];
					connection_code[index++] = AC_Flag[a * 44 + b];
					if (AC_Flag[a * 44 + b] == 1) {
						continue;
					}
					else {
						for (int c = 0; c < 8; c++) {
							for (int d = 0; d < 8; d++) {
								if (!((c == 0) && (d == 0))) {
									connection_code[index++] = dct_out_int[(a * 8 + c) * WIDTH + (b * 8 + d)];
								}
							}
						}
					}
				}
			}


			entropy_encode(index, connection_code, entropy_result);

			/*for (int a = 0; a < entropy_index + 1; a++) {
				printf("%x,", entropy_result[a]);
			}*/

			/*fwrite(&entropy_result[0], 4, entropy_index + 1, fp_entropy);*/
			fwrite(&entropy_result[0], 4, entropy_index + 1, fp_entropy);
			entropy_total_size += entropy_index + 1;


			IDCT(WIDTH, HEIGHT, QP_DC, QP_AC, dct_out_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT, fp_idct);


			REVERSE_DPCM(DPCM_MOD, 352, 288, idct_out_int);
			fwrite(&re_intra_dpcm_out[0], 1, WIDTH * HEIGHT, fp_re_intra_dpcm_y);

			if (intra_enable == 1) {

				// 역intra
				REVERSE_INTRA(352, 288, re_intra_dpcm_int);

				out_y = re_intra_out;
			}
			else {
				out_y = re_intra_dpcm_out;

			}

			for (i = 0; i < HEIGHT; i++)
			{
				for (j = 0; j < WIDTH; j++)
				{
					ref_arr_y[i * WIDTH + j] = out_y[i * WIDTH + j];
				}
			}
			/*fwrite(&out_y[0], 1, WIDTH * HEIGHT, fp_re_intra_y);*/



			//---------------------------------------------
			//					intra u성분
			//---------------------------------------------

			// intra_u


			DPCM(DPCM_MOD, 176, 144, arr_u_int);
			fwrite(&intra_dpcm_out[0], 1, WIDTH * HEIGHT / 4, fp_intra_dpcm_y);

			//	DCT
			DCT(176, 144, QP_DC, QP_AC, intra_dpcm_int);
			fwrite(&dct_out[0], 1, WIDTH * HEIGHT / 4, fp_dct);



			//--------------------
			//		코드 연결
			//--------------------
			index = 0; // connection_code 배열 길이
			for (int a = 0; a < 18; a++) {
				for (int b = 0; b < 22; b++) {
					connection_code[index++] = dct_out_int[a * 8 * WIDTH / 2 + b * 8];
					connection_code[index++] = AC_Flag[a * 22 + b];
					if (AC_Flag[a * 22 + b] == 1) {
						continue;
					}
					else {
						for (int c = 0; c < 8; c++) {
							for (int d = 0; d < 8; d++) {
								if (!((c == 0) && (d == 0))) {
									connection_code[index++] = dct_out_int[(a * 8 + c) * WIDTH / 2 + (b * 8 + d)];
								}
							}
						}
					}
				}
			}

			entropy_encode(index, connection_code, entropy_result);
			fwrite(&entropy_result[0], 4, entropy_index + 1, fp_entropy);
			entropy_total_size += entropy_index + 1;

			IDCT(176, 144, QP_DC, QP_AC, dct_out_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT / 4, fp_idct);
			/*for (int a = 0; a < 144; a++) {
				for (int b = 0; b < 176; b++) {
					printf("%d,", idct_out[a * 176 + b]);
				}
				printf("\n");
			}*/


			REVERSE_DPCM(DPCM_MOD, 176, 144, idct_out_int);
			fwrite(&re_intra_dpcm_out[0], 1, WIDTH * HEIGHT / 4, fp_re_intra_dpcm_y);

			/*fwrite(&re_intra_dpcm_out[0], 1, WIDTH * HEIGHT / 4, fp_re_intra_y);*/
			for (i = 0; i < HEIGHT / 2; i++)
			{
				for (j = 0; j < WIDTH / 2; j++)
				{
					ref_arr_u[i * WIDTH / 2 + j] = re_intra_dpcm_out[i * WIDTH / 2 + j];
				}
			}

			//---------------------------------------------
			//					intra v성분
			//---------------------------------------------

			// intra_v

			DPCM(DPCM_MOD, 176, 144, arr_v_int);
			fwrite(&intra_dpcm_out[0], 1, WIDTH * HEIGHT / 4, fp_intra_dpcm_y);

			//	DCT
			DCT(176, 144, QP_DC, QP_AC, intra_dpcm_int);
			fwrite(&dct_out[0], 1, WIDTH * HEIGHT / 4, fp_dct);



			//--------------------
			//		코드 연결
			//--------------------
			index = 0; // connection_code 배열 길이
			for (int a = 0; a < 18; a++) {
				for (int b = 0; b < 22; b++) {
					connection_code[index++] = dct_out_int[a * 8 * WIDTH / 2 + b * 8];
					connection_code[index++] = AC_Flag[a * 22 + b];
					if (AC_Flag[a * 22 + b] == 1) {
						continue;
					}
					else {
						for (int c = 0; c < 8; c++) {
							for (int d = 0; d < 8; d++) {
								if (!((c == 0) && (d == 0))) {
									connection_code[index++] = dct_out_int[(a * 8 + c) * WIDTH / 2 + (b * 8 + d)];
								}
							}
						}
					}
				}
			}

			entropy_encode(index, connection_code, entropy_result);
			fwrite(&entropy_result[0], 4, entropy_index + 1, fp_entropy);
			/*fwrite(&entropy_result[0], 4, index, fp_entropy);*/
			entropy_total_size += entropy_index + 1;

			IDCT(176, 144, QP_DC, QP_AC, dct_out_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT / 4, fp_idct);


			REVERSE_DPCM(DPCM_MOD, 176, 144, idct_out_int);
			fwrite(&re_intra_dpcm_out[0], 1, WIDTH * HEIGHT / 4, fp_re_intra_dpcm_y);

			/*fwrite(&re_intra_dpcm_out[0], 1, WIDTH * HEIGHT / 4, fp_re_intra_y);*/

			for (i = 0; i < HEIGHT / 2; i++)
			{
				for (j = 0; j < WIDTH / 2; j++)
				{
					ref_arr_v[i * WIDTH / 2 + j] = re_intra_dpcm_out[i * WIDTH / 2 + j];
				}
			}
		}

		else {
			//---------------------------------------------
			//					inter y성분
			//---------------------------------------------

			INTER(352, 288, INTER_BLOCK_SIZE, arr_y, ref_arr_y);
			out_put = inter_out;
			fwrite(&out_put[0], 1, WIDTH * HEIGHT, fp_re_file);

			DCT(WIDTH, HEIGHT, QP_DC, QP_AC, inter_out_int);
			fwrite(&dct_out[0], 1, WIDTH * HEIGHT, fp_dct);

			

			//--------------------
			//		코드 연결
			//--------------------
			index = 0; // connection_code 배열 길이
			for (int a = 0; a < 36; a++) {
				for (int b = 0; b < 44; b++) {
					connection_code[index++] = MV[(a * 44 + b) * 2]; // MY_x
					connection_code[index++] = MV[(a * 44 + b) * 2 + 1]; //MV_y
					connection_code[index++] = dct_out_int[a * 8 * WIDTH + b * 8];
					connection_code[index++] = AC_Flag[a * 44 + b];
					if (AC_Flag[a * 44 + b] == 1) {
						continue;
					}
					else {
						for (int c = 0; c < 8; c++) {
							for (int d = 0; d < 8; d++) {
								if (!((c == 0) && (d == 0))) {
									connection_code[index++] = dct_out_int[(a * 8 + c) * WIDTH + (b * 8 + d)];
								}
							}
						}
					}
				}
			}

			

			/*for (int a = 0; a < index; a++) {
				printf("%d,", connection_code[a]);
			}*/

			entropy_encode(index, connection_code, entropy_result);
			fwrite(&entropy_result[0], 4, entropy_index + 1, fp_entropy);
			/*fwrite(&entropy_result[0], 4, index, fp_entropy);*/
			entropy_total_size += entropy_index + 1;

			

			IDCT(WIDTH, HEIGHT, QP_DC, QP_AC, dct_out_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT, fp_idct);


			// 역inter
			REVERSE_INTER(352, 288, INTER_BLOCK_SIZE, MV, ref_arr_y, idct_out_int);
			/*fwrite(&re_cur_arr[0], 1, WIDTH * HEIGHT, fp_re_intra_y);*/

			for (i = 0; i < HEIGHT; i++)
			{
				for (j = 0; j < WIDTH; j++)
				{
					ref_arr_y[i * WIDTH + j] = re_cur_arr[i * WIDTH + j];
				}
			}

			//---------------------------------------------
			//					inter u성분
			//---------------------------------------------

			INTER(WIDTH / 2, HEIGHT / 2, INTER_BLOCK_SIZE, arr_u, ref_arr_u);


			DCT(WIDTH / 2, HEIGHT / 2, QP_DC, QP_AC, inter_out_int);
			fwrite(&dct_out[0], 1, WIDTH * HEIGHT / 4, fp_dct);

			
			//--------------------
			//		코드 연결
			//--------------------
			index = 0; // connection_code 배열 길이
			for (int a = 0; a < 18; a++) {
				for (int b = 0; b < 22; b++) {
					connection_code[index++] = dct_out_int[a * 8 * WIDTH / 2 + b * 8];
					connection_code[index++] = AC_Flag[a * 22 + b];
					if (AC_Flag[a * 22 + b] == 1) {
						continue;
					}
					else {
						for (int c = 0; c < 8; c++) {
							for (int d = 0; d < 8; d++) {
								if (!((c == 0) && (d == 0))) {
									connection_code[index++] = dct_out_int[(a * 8 + c) * WIDTH / 2 + (b * 8 + d)];
								}
							}
						}
					}
				}
			}

			entropy_encode(index, connection_code, entropy_result);
			fwrite(&entropy_result[0], 4, entropy_index + 1, fp_entropy);
			/*fwrite(&entropy_result[0], 4, index, fp_entropy);*/
			entropy_total_size += entropy_index + 1;
			


			IDCT(WIDTH / 2, HEIGHT / 2, QP_DC, QP_AC, dct_out_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT / 4, fp_idct);


			// 역inter
			REVERSE_INTER(WIDTH / 2, HEIGHT / 2, INTER_BLOCK_SIZE, MV, ref_arr_u, idct_out_int);
			/*fwrite(&re_cur_arr[0], 1, WIDTH * HEIGHT / 4, fp_re_intra_y);*/

			for (i = 0; i < HEIGHT / 2; i++)
			{
				for (j = 0; j < WIDTH / 2; j++)
				{
					ref_arr_u[i * WIDTH / 2 + j] = re_cur_arr[i * WIDTH / 2 + j];
				}
			}

			//---------------------------------------------
			//					inter v성분
			//---------------------------------------------
			INTER(WIDTH / 2, HEIGHT / 2, INTER_BLOCK_SIZE, arr_v, ref_arr_v);

			DCT(WIDTH / 2, HEIGHT / 2, QP_DC, QP_AC, inter_out_int);
			fwrite(&dct_out[0], 1, WIDTH * HEIGHT / 4, fp_dct);

			//--------------------
			//		코드 연결
			//--------------------
			index = 0; // connection_code 배열 길이
			for (int a = 0; a < 18; a++) {
				for (int b = 0; b < 22; b++) {
					connection_code[index++] = dct_out_int[a * 8 * WIDTH / 2 + b * 8];
					connection_code[index++] = AC_Flag[a * 22 + b];
					if (AC_Flag[a * 22 + b] == 1) {
						continue;
					}
					else {
						for (int c = 0; c < 8; c++) {
							for (int d = 0; d < 8; d++) {
								if (!((c == 0) && (d == 0))) {
									connection_code[index++] = dct_out_int[(a * 8 + c) * WIDTH / 2 + (b * 8 + d)];
								}
							}
						}
					}
				}
			}
			entropy_encode(index, connection_code, entropy_result);
			fwrite(&entropy_result[0], 4, entropy_index + 1, fp_entropy);
			/*fwrite(&entropy_result[0], 4, index, fp_entropy);*/
			entropy_total_size += entropy_index + 1;

			IDCT(WIDTH / 2, HEIGHT / 2, QP_DC, QP_AC, dct_out_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT / 4, fp_idct);


			// 역inter
			REVERSE_INTER(WIDTH / 2, HEIGHT / 2, INTER_BLOCK_SIZE, MV, ref_arr_v, idct_out_int);
			/*fwrite(&re_cur_arr[0], 1, WIDTH * HEIGHT / 4, fp_re_intra_y);*/

			for (i = 0; i < HEIGHT / 2; i++)
			{
				for (j = 0; j < WIDTH / 2; j++)
				{
					ref_arr_v[i * WIDTH / 2 + j] = re_cur_arr[i * WIDTH / 2 + j];
				}
			}

		}


		free(arr_y);
		free(arr_u);
		free(arr_v);

		free(entropy_result);

	}

	int compressibility = 0; // 압축률

	char compress_result[150] = "\0";
	char str[4] = "\0"; // 소수점 앞
	char str1[2] = "\0"; // 소수점 뒤 첫번째 수
	char str2[2] = "\0"; //소수점 뒤 두번째 수
	char str_t[6] = "\0";

	double x; // 원본 크기 / 4

	x= (WIDTH * HEIGHT * frame_size * 3 / 2) / 4;
	compressibility = (entropy_total_size / x) * 10000; // 소수점 뒤 두자리까지
	_itoa_s(compressibility / 100, str, 4, 10); // 소수점 앞 
	strcpy(str_t, str);
	strcat(str_t,".");
	_itoa_s((compressibility % 100) / 10, str1, 2, 10); // 소수점 뒤 첫번째 수
	strcat(str_t, str1);
	_itoa_s(compressibility % 10, str2, 2, 10); //소수점 뒤 두번째 수
	strcat(str_t, str2);
	

	strcpy(compress_result, "\n");
	strcat(compress_result, argv[9]);
	strcat(compress_result, ", DC_QP: ");
	strcat(compress_result, argv[2]);
	strcat(compress_result, ", AC_QP: ");
	strcat(compress_result, argv[3]);
	strcat(compress_result, ", Pixel DPCM mode: ");
	strcat(compress_result, argv[4]);
	strcat(compress_result, ", Intra period: ");
	strcat(compress_result, argv[5]);
	strcat(compress_result, ", intra prediction enable: ");
	strcat(compress_result, argv[6]);
	strcat(compress_result, ", compressibility: ");
	strcat(compress_result, str_t);
	strcat(compress_result, "%");


	fwrite(compress_result, 1, 150, fp_compress);


	free(pic);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	fclose(fp_intra);
	fclose(fp_re_file);
	fclose(fp_intra_dpcm_y);
	fclose(fp_re_intra_dpcm_y);
	/*fclose(fp_re_intra_y);*/

	fclose(fp_inter);
	fclose(fp_re_inter);

	fclose(fp_dct);
	fclose(fp_idct);

	fclose(fp_entropy);


	return 0;

}

void INTRA(int w, int h, unsigned char* input) // y,u,v성분(2차원)을 받아서 intra함수로 intra_out(1차원) 출력
{
	unsigned char temp_up[8] = { 0, };	//	temp_up[8]
	unsigned char temp_left[8] = { 0, };	//	temp_left[8]

	//	only intra signal 블록별로 0, 1, 2 모드
	int intra0[8][8];
	int intra1[8][8];
	int intra2[8][8];

	int a, b, x;
	int i, j;
	int index = 0;

	for (i = 0; i < h; i += 8) {
		for (j = 0; j < w; j += 8) {
			// 첫번째 블록 위와 좌의 값은 128      if i , j=0
			if ((i == 0) && (j == 0)) {
				for (a = 0; a < 8; a++) {
					temp_left[a] = 128;
					temp_up[a] = 128;
				}
			}

			if ((i == 0) && (j != 0)) {
				for (a = 0; a < 8; a++) {
					temp_up[a] = 128;
					temp_left[a] = input[(i + a) * w + (j - 1)];
				}
			}
			else if ((i != 0) && (j == 0)) {
				for (a = 0; a < 8; a++) {
					temp_left[a] = 128;
					temp_up[a] = input[(i - 1) * w + (j + a)];
				}
			}

			else if ((i != 0) && (j != 0)) {
				for (a = 0; a < 8; a++) { //블록단위
					temp_up[a] = input[(i - 1) * w + (j + a)];
					temp_left[a] = input[(i + a) * w + (j - 1)];
				}
			}

			// SAE 계산
			int sae0 = 0, sae1 = 0, sae2 = 0;

			//		intra 0모드
			for (a = 0; a < 8; a++) { //블록단위
				for (b = 0; b < 8; b++) {
					intra0[a][b] = temp_up[b] - input[(i + a) * w + (j + b)];
					/*intra0[a][b] = abs(temp_up[b] - input_2d[i + a][j + b]);*/
					sae0 += abs(temp_up[b] - input[(i + a) * w + (j + b)]);
				}
			}

			//		intra 1모드
			for (a = 0; a < 8; a++) { //블록단위
				for (b = 0; b < 8; b++) {
					intra1[a][b] = temp_left[a] - input[(i + a) * w + (j + b)];
					/*intra1[a][b] = abs(temp_left[a] - input_2d[i + a][j + b]);*/
					sae1 += abs(temp_left[a] - input[(i + a) * w + (j + b)]);
				}
			}

			//		intra 2모드
			int sum = 0, ave = 0;

			for (x = 0; x < 8; x++) {
				sum += temp_up[x] + temp_left[x];
			}
			ave = sum / 16;

			for (a = 0; a < 8; a++) { //블록단위
				for (b = 0; b < 8; b++) {
					intra2[a][b] = ave - input[(i + a) * w + (j + b)];
					/*intra2[a][b] = abs(ave - input_2d[i + a][j + b]);*/
					sae2 += abs(ave - input[(i + a) * w + (j + b)]);
				}
			}

			// SAE 값비교
			int min = sae0;
			int mod = 0;
			if (sae1 <= min) {
				min = sae1;
				mod = 1;
				if (sae2 <= min) {
					min = sae2;
					mod = 2;
				}
			}
			else if (min <= sae1) {
				if (sae2 <= min) {
					min = sae2;
					mod = 2;
				}
			}

			// 출력

			if (mod == 0) {
				for (a = 0; a < 8; a++) {
					for (b = 0; b < 8; b++) {
						intra_out[(i + a) * w + (j + b)] = intra0[a][b];
						intra_out_int[(i + a) * w + (j + b)] = intra0[a][b];
					}
				}
			}

			else if (mod == 1) {
				for (a = 0; a < 8; a++) {
					for (b = 0; b < 8; b++) {
						intra_out[(i + a) * w + (j + b)] = intra1[a][b];
						intra_out_int[(i + a) * w + (j + b)] = intra1[a][b];
					}
				}
			}
			else if (mod == 2) {
				for (a = 0; a < 8; a++) {
					for (b = 0; b < 8; b++) {
						intra_out[(i + a) * w + (j + b)] = intra2[a][b];
						intra_out_int[(i + a) * w + (j + b)] = intra2[a][b];
					}
				}
			}
			mod_arr[index] = mod;
			index++;
		}
	}
}

void REVERSE_INTRA(int w, int h, int* re_intra_dpcm) { // re_intra_dpcm_int = intra_out_int

	int temp_up[8] = { 0, };	//	temp_up[8]
	int temp_left[8] = { 0, };	//	temp_left[8]

	//	only intra signal 블록별로 0, 1, 2 모드
	int re_intra0[8][8]; // = 원본값
	int re_intra1[8][8]; // = 원본값
	int re_intra2[8][8]; // = 원본값

	int index = 0; // mod_arr 배열 index

	int a, b, x;
	int i, j;

	for (i = 0; i < h; i += 8) {
		for (j = 0; j < w; j += 8) {
			// 첫번째 블록 위와 좌의 값은 128      if i , j=0
			if ((i == 0) && (j == 0)) {
				for (a = 0; a < 8; a++) {
					temp_left[a] = 128;
					temp_up[a] = 128;
				}
			}

			if ((i == 0) && (j != 0)) {
				for (a = 0; a < 8; a++) {
					temp_up[a] = 128;
					temp_left[a] = re_intra_int[(i + a) * w + (j - 1)];
				}
			}
			else if ((i != 0) && (j == 0)) {
				for (a = 0; a < 8; a++) {
					temp_left[a] = 128;
					temp_up[a] = re_intra_int[(i - 1) * w + (j + a)];
				}
			}

			else if ((i != 0) && (j != 0)) {
				for (a = 0; a < 8; a++) { //블록단위
					temp_up[a] = re_intra_int[(i - 1) * w + (j + a)];
					temp_left[a] = re_intra_int[(i + a) * w + (j - 1)];
				}
			}

			// temp_up[x], temp_left[x] 평균 계산
			int sum = 0, ave = 0;

			for (x = 0; x < 8; x++) {
				sum += temp_up[x] + temp_left[x];
			}
			ave = sum / 16;

			// 출력
			if (mod_arr[index] == 0) {
				for (a = 0; a < 8; a++) {
					for (b = 0; b < 8; b++) {
						re_intra_out[(i + a) * w + (j + b)] = temp_up[b] - re_intra_dpcm[(i + a) * w + (j + b)];
						re_intra_int[(i + a) * w + (j + b)] = temp_up[b] - re_intra_dpcm[(i + a) * w + (j + b)];
					}
				}
			}

			else if (mod_arr[index] == 1) {
				for (a = 0; a < 8; a++) {
					for (b = 0; b < 8; b++) {
						re_intra_out[(i + a) * w + (j + b)] = temp_left[a] - re_intra_dpcm[(i + a) * w + (j + b)];
						re_intra_int[(i + a) * w + (j + b)] = temp_left[a] - re_intra_dpcm[(i + a) * w + (j + b)];
					}
				}
			}
			else if (mod_arr[index] == 2) {
				for (a = 0; a < 8; a++) {
					for (b = 0; b < 8; b++) {
						re_intra_out[(i + a) * w + (j + b)] = ave - re_intra_dpcm[(i + a) * w + (j + b)];
						re_intra_int[(i + a) * w + (j + b)] = ave - re_intra_dpcm[(i + a) * w + (j + b)];
					}
				}
			}
			index++;
		}
	}
}


void DPCM(int dpcm_mod, int w, int h, int* intra_out_int) {
	int i = 0, j = 0;
	int left, upper, upper_right, median, mean;
	double mean_double;
	int sum;
	if (dpcm_mod == 0) {
		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				if ((i == 0) && (j == 0)) {
					median = 128;
				}
				else if (j == 351) {
					median = intra_out_int[i * w + j - 1];
				}
				else if ((i == 0) && (j != 0)) {
					median = intra_out_int[j - 1];
				}
				else if ((i != 0) && (j == 0)) {
					median = intra_out_int[(i - 1) * w + j];
				}
				else if ((i != 0) && (j != 0)) {
					left = intra_out_int[i * w + (j - 1)];
					upper = intra_out_int[(i - 1) * w + j];
					upper_right = intra_out_int[(i - 1) * w + (j + 1)];
					if ((left - upper) * (upper - upper_right) >= 0) {
						median = upper;
					}
					else if ((upper - left) * (left - upper_right) >= 0) {
						median = left;
					}
					else if ((upper - upper_right) * (upper_right - left) >= 0) {
						median = upper_right;
					}
				}
				intra_dpcm_out[i * w + j] = intra_out_int[i * w + j] - median;
				intra_dpcm_int[i * w + j] = intra_out_int[i * w + j] - median;
			}
		}
	}
	else if (dpcm_mod == 1) {
		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				if ((i == 0) && (j == 0)) {
					mean = 128;
				}
				else if ((i == 0) && (j != 0)) {
					mean = intra_out_int[j - 1];
				}
				else if ((i != 0) && (j == 351)) {
					//sum = intra_out_int[i * w + j - 1] * 2 + intra_out_int[(i - 1) * w + j];
					//mean_double = sum / 3.0;
					//mean = (int)(mean_double < 0 ? mean_double - 0.5 : mean_double + 0.5); // 반 올림
					mean = (intra_out_int[i * w + j - 1] * 2 + intra_out_int[(i - 1) * w + j]) / 3;
				}
				else if ((i != 0) && (j == 0)) {
					/*sum = intra_out_int[(i - 1) * w] * 2 + intra_out_int[(i - 1) * w + j + 1];
					mean_double = sum / 3.0;
					mean = (int)(mean_double < 0 ? mean_double - 0.5 : mean_double + 0.5);*/
					mean = (intra_out_int[(i - 1) * w] * 2 + intra_out_int[(i - 1) * w + j + 1]) / 3;
				}
				else if ((i != 0) && (j != 0)) {
					left = intra_out_int[i * w + (j - 1)];
					upper = intra_out_int[(i - 1) * w + j];
					upper_right = intra_out_int[(i - 1) * w + (j + 1)];
					/*sum = left + upper + upper_right;
					mean_double = sum / 3.0;
					mean = (int)(mean_double < 0 ? mean_double - 0.5 : mean_double + 0.5);*/
					mean = (left + upper + upper_right) / 3;
				}
				intra_dpcm_out[i * w + j] = intra_out_int[i * w + j] - mean;
				intra_dpcm_int[i * w + j] = intra_out_int[i * w + j] - mean;
			}
		}
	}
	else if (dpcm_mod == 2) {
		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				if (j == 0) {
					intra_dpcm_out[i * w + j] = intra_out_int[i * w + j] - 128;
					intra_dpcm_int[i * w + j] = intra_out_int[i * w + j] - 128;
				}
				else {
					intra_dpcm_out[i * w + j] = intra_out_int[i * w + j] - intra_out_int[i * w + j - 1];
					intra_dpcm_int[i * w + j] = intra_out_int[i * w + j] - intra_out_int[i * w + j - 1];
				}

			}
		}
	}
	else if (dpcm_mod == 6) {
		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				intra_dpcm_out[i * w + j] = intra_out_int[i * w + j];
				intra_dpcm_int[i * w + j] = intra_out_int[i * w + j];
			}
		}
	}
}

void REVERSE_DPCM(int dpcm_mod, int w, int h, int* intra_dpcm) {
	int i = 0, j = 0;
	int left, upper, upper_right, median, mean;
	double mean_double;
	int sum;

	if (dpcm_mod == 0) {
		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				if ((i == 0) && (j == 0)) {
					median = 128;
				}
				else if (j == 351) {
					median = re_intra_dpcm_int[i * w + j - 1];
				}
				else if ((i == 0) && (j != 0)) {
					median = re_intra_dpcm_int[j - 1];
				}
				else if ((i != 0) && (j == 0)) {
					median = re_intra_dpcm_int[(i - 1) * w];
				}
				else if ((i != 0) && (j != 0)) {
					left = re_intra_dpcm_int[i * w + (j - 1)];
					upper = re_intra_dpcm_int[(i - 1) * w + j];
					upper_right = re_intra_dpcm_int[(i - 1) * w + (j + 1)];
					if ((left - upper) * (upper - upper_right) >= 0) {
						median = upper;
					}
					else if ((upper - left) * (left - upper_right) >= 0) {
						median = left;
					}
					else if ((upper - upper_right) * (upper_right - left) >= 0) {
						median = upper_right;
					}
				}
				re_intra_dpcm_int[i * w + j] = median + intra_dpcm[i * w + j];
				re_intra_dpcm_out[i * w + j] = median + intra_dpcm[i * w + j];

			}

		}
	}
	else if (dpcm_mod == 1) {
		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				if ((i == 0) && (j == 0)) {
					mean = 128;
				}
				else if ((i != 0) && (j == 351)) {
					/*sum = re_intra_dpcm_int[i * w + j - 1] * 2 + re_intra_dpcm_int[(i - 1) * w + j];
					mean_double = sum / 3.0;
					mean = (int)(mean_double < 0 ? mean_double - 0.5 : mean_double + 0.5);*/
					mean = (re_intra_dpcm_int[i * w + j - 1] * 2 + re_intra_dpcm_int[(i - 1) * w + j]) / 3;
				}
				else if ((i == 0) && (j != 0)) {
					mean = re_intra_dpcm_int[j - 1];
				}
				else if ((i != 0) && (j == 0)) {
					/*sum = re_intra_dpcm_int[(i - 1) * w] * 2 + re_intra_dpcm_int[(i - 1) * w + j + 1];
					mean_double = sum / 3.0;
					mean = (int)(mean_double < 0 ? mean_double - 0.5 : mean_double + 0.5);*/
					mean = (re_intra_dpcm_int[(i - 1) * w] * 2 + re_intra_dpcm_int[(i - 1) * w + j + 1]) / 3;
				}
				else if ((i != 0) && (j != 0)) {
					left = re_intra_dpcm_int[i * w + (j - 1)];
					upper = re_intra_dpcm_int[(i - 1) * w + j];
					upper_right = re_intra_dpcm_int[(i - 1) * w + (j + 1)];
					/*sum = left + upper + upper_right;
					mean_double = sum / 3.0;
					mean = (int)(mean_double < 0 ? mean_double - 0.5 : mean_double + 0.5);*/
					mean = (left + upper + upper_right) / 3;
				}
				re_intra_dpcm_int[i * w + j] = intra_dpcm[i * w + j] + mean;
				re_intra_dpcm_out[i * w + j] = intra_dpcm[i * w + j] + mean;
			}
		}
	}
	else if (dpcm_mod == 2) {
		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				if (j == 0) {
					re_intra_dpcm_int[i * w + j] = 128 + intra_dpcm[i * w + j];
					re_intra_dpcm_out[i * w + j] = 128 + intra_dpcm[i * w + j];
				}
				else {
					re_intra_dpcm_int[i * w + j] = re_intra_dpcm_int[i * w + j - 1] + intra_dpcm[i * w + j];
					re_intra_dpcm_out[i * w + j] = re_intra_dpcm_int[i * w + j - 1] + intra_dpcm[i * w + j];
				}

			}
		}
	}
	else if (dpcm_mod == 6) {
		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				re_intra_dpcm_int[i * w + j] = intra_dpcm[i * w + j];
				re_intra_dpcm_out[i * w + j] = intra_dpcm[i * w + j];
			}
		}
	}
}


void INTER(int w, int h, int block_size, unsigned char* cur_arr, unsigned char* ref_arr) {
	int i, j;
	int a, b, a_max, b_max, a_val, b_val, c, d;
	int MV_x = 0, MV_y = 0;
	int sad_min;

	for (i = 0; i < h; i += block_size) { // (i, j) -> current frame의 각 메크로 블록(8x8)의 시작위치
		for (j = 0; j < w; j += block_size) {
			a_val = -SEARCH_INDEX;
			a_max = SEARCH_INDEX;
			b_val = -SEARCH_INDEX;
			b_max = SEARCH_INDEX;
			if (i == 0) {
				a_val = 0;
				a_max = SEARCH_INDEX;
			}
			if (j == 0) {
				b_val = 0;
				b_max = SEARCH_INDEX;
			}
			if (i == h - block_size) {
				a_val = -SEARCH_INDEX;
				a_max = 0;
			}
			if (j == w - block_size) {
				b_val = -SEARCH_INDEX;
				b_max = 0;
			}
			int sad[SEARCH_RANGE * SEARCH_RANGE] = { 0, };
			int ccc = 0;
			for (a = a_val; a <= a_max; a++) {
				for (b = b_val; b <= b_max; b++) {

					for (c = 0; c < block_size; c++) {
						for (d = 0; d < block_size; d++) {
							sad[(a + SEARCH_INDEX) * SEARCH_RANGE + (b + SEARCH_INDEX)] += abs(cur_arr[(i + c) * w + (j + d)] - ref_arr[(i + a + c) * w + (j + b + d)]);
						}
					}
					if ((a == a_val) && (b == b_val)) {
						sad_min = sad[(a_val + SEARCH_INDEX) * SEARCH_RANGE + (b_val + SEARCH_INDEX)];
						MV_x = -b;
						MV_y = -a;
					}

					else if (sad[(a + SEARCH_INDEX) * SEARCH_RANGE + (b + SEARCH_INDEX)] < sad_min) {
						sad_min = sad[(a + SEARCH_INDEX) * SEARCH_RANGE + (b + SEARCH_INDEX)];
						MV_x = -b;
						MV_y = -a;
					}

					/*ccc++;
					printf("MV_x = %d, MV_y = %d, sad_min = %d\n", MV_x, MV_y, sad_min);*/
				}
			}

			/*printf("ccc =%d ,\ni = %d, j = %d, sad_min = %d, MV_x = %d, MV_y = %d\n\n", ccc,i, j, sad_min, MV_x, MV_y);*/

			for (c = 0; c < INTER_BLOCK_SIZE; c++) {
				for (d = 0; d < INTER_BLOCK_SIZE; d++) {
					// ref_arr[(i - MV_y + c) * WIDTH + (j - MV_x + d)] -> 보상영상
					inter_out[(i + c) * w + (j + d)] = abs(ref_arr[(i - MV_y + c) * w + (j - MV_x + d)] - cur_arr[(i + c) * w + (j + d)]);
					inter_out_int[(i + c) * w + (j + d)] = cur_arr[(i + c) * w + (j + d)] - ref_arr[(i - MV_y + c) * w + (j - MV_x + d)];
					/*inter_out[(i + c) * WIDTH + (j + d)] = ref_arr[(i - MV_y + c) * WIDTH + (j - MV_x + d)];*/
				}
			}
			MV[((i / INTER_BLOCK_SIZE) * (w / INTER_BLOCK_SIZE) + j / INTER_BLOCK_SIZE) * 2] = MV_x;
			MV[((i / INTER_BLOCK_SIZE) * (w / INTER_BLOCK_SIZE) + j / INTER_BLOCK_SIZE) * 2 + 1] = MV_y;
		}
	}
}

void REVERSE_INTER(int w, int h, int block_size, int* MV, unsigned char* ref_arr, int* inter_out_int) {
	int i, j;
	int a, b, a_max, b_max, a_val, b_val, c, d;
	int MV_x = 0, MV_y = 0;

	for (i = 0; i < h; i += block_size) { // (i, j) -> current frame의 각 메크로 블록(8x8)의 시작위치
		for (j = 0; j < w; j += block_size) {

			MV_x = MV[((i / block_size) * (w / block_size) + j / block_size) * 2];
			MV_y = MV[((i / block_size) * (w / block_size) + j / block_size) * 2 + 1];

			for (c = 0; c < block_size; c++) {
				for (d = 0; d < block_size; d++) {
					// ref_arr[(i - MV_y + c) * WIDTH + (j - MV_x + d)] -> 보상영상
					re_cur_arr[(i + c) * w + (j + d)] = inter_out_int[(i + c) * w + (j + d)] + ref_arr[(i - MV_y + c) * w + (j - MV_x + d)];
					/*inter_out[(i + c) * WIDTH + (j + d)] = ref_arr[(i - MV_y + c) * WIDTH + (j - MV_x + d)];*/
				}
			}

		}
	}
}

void DCT(int w, int h, int qp_dc, int qp_ac, int* arr1) {

	int i, j, a, b, k;
	double cm;

	double DCT_A[8][8];
	double DCT_AT[8][8];
	double DCT_Temp[8][8];
	double temp;

	//	DCT_A[i][j]
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (i == 0)
			{
				cm = sqrt((double)1 / 8); // cu * 2 / 1
			}
			else
			{
				cm = sqrt((double)2 / 8);
			}
			DCT_A[i][j] = cm * cos(((double)2 * j + 1) * PI * i / 16);
		}
	}
	//	DCT_AT[i][j]
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			DCT_AT[i][j] = DCT_A[j][i];
		}
	}

	for (i = 0; i < h; i += 8)
	{
		for (j = 0; j < w; j += 8)
		{
			for (a = 0; a < 8; a++)
			{
				for (b = 0; b < 8; b++)
				{
					temp = 0;
					for (k = 0; k < 8; k++) {
						temp += DCT_A[a][k] * arr1[(k + i) * w + b + j];
					}
					DCT_Temp[a][b] = temp;
				}
			}

			for (a = 0; a < 8; a++)
			{
				for (b = 0; b < 8; b++)
				{
					temp = 0;
					for (k = 0; k < 8; k++) {
						temp += DCT_Temp[a][k] * DCT_AT[k][b];
					}

					if ((a == 0) && (b == 0)) {
						temp = temp / qp_dc;
						dct_out_double[(a + i) * w + b + j] = temp;
					}
					else {
						temp = temp / qp_ac;
						dct_out_double[(a + i) * w + b + j] = temp;
					}


					dct_out_int[(a + i) * w + b + j] = (int)(temp < 0 ? temp - 0.5 : temp + 0.5);
					/*dct_out_int[(a + i) * w + b + j] = temp;*/
					dct_out[(a + i) * w + b + j] = temp;
				}
			}
			int count = 0;
			AC_Flag[(i / 8) * (w / 8) + (j / 8)] = 0;
			for (a = 0; a < 8; a++)
			{
				for (b = 0; b < 8; b++)
				{
					if (!((a == 0) && (b == 0)) && (dct_out_int[(a + i) * w + b + j] == 0)) {
						count++;
					}
					if (count == 63) {
						AC_Flag[(i / 8) * (w / 8) + (j / 8)] = 1;
					}
				}
			}

		}
	}

}

void IDCT(int w, int h, double qp_dc, double qp_ac, int* dct_out_int) {


	int i, j, a, b, k;
	double cm;

	double DCT_A[8][8];
	double DCT_AT[8][8];
	double DCT_Temp[8][8];
	double temp;

	//	DCT_A[i][j]
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (i == 0)
			{
				cm = sqrt((double)1 / 8); // cu * 2 / 1
			}
			else
			{
				cm = sqrt((double)2 / 8);
			}
			DCT_A[i][j] = cm * cos((2 * j + 1) * PI * i / 16);
		}
	}
	//	DCT_AT[i][j]
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			DCT_AT[i][j] = DCT_A[j][i];
		}
	}

	for (i = 0; i < h; i += 8)
	{
		for (j = 0; j < w; j += 8)
		{
			for (a = 0; a < 8; a++)	// u
			{
				for (b = 0; b < 8; b++) // v
				{
					temp = 0;
					for (k = 0; k < 8; k++) {
						if ((k == 0) && (b == 0)) {
							temp = temp + DCT_AT[a][k] * dct_out_double[(k + i) * w + b + j] * qp_dc;
						}
						else {
							temp = temp + DCT_AT[a][k] * dct_out_double[(k + i) * w + b + j] * qp_ac;
						}

						/*temp = temp + DCT_AT[a][k] * dct_out_int[(k + i) * w + b + j];*/
					}
					DCT_Temp[a][b] = temp;
				}
			}

			for (a = 0; a < 8; a++)
			{
				for (b = 0; b < 8; b++)
				{
					temp = 0;
					for (k = 0; k < 8; k++) {
						temp += DCT_Temp[a][k] * DCT_A[k][b];
					}

					idct_out_int[(a + i) * w + b + j] = (int)(temp < 0 ? temp - 0.5 : temp + 0.5);
					/*idct_out_int[(a + i) * w + b + j] = temp;*/
					idct_out[(a + i) * w + b + j] = temp;

				}
			}

		}
	}

	/*for (a = 0; a < HEIGHT; a++) {
		for (b = 0; b < WIDTH; b++) {
			printf("%d,", idct_out_int[a * WIDTH + b]);
		}
		printf("\n");
	}*/

}

void zig_zag_scan(int w, int h, int* zigzag_input, int* entropy_result) {

	//----------
	// 지그제그 스캔해서 모든 정보를 한줄로 해준다음 entropy code 진행,
	// entropy code는 정보를 하나하나씩 저장해서 4byte(int형)가 되면 하나 저장, 
	// 마지막에 4byte가 안되면 그냥 저장해준다. (마지막 바이트에서 안 쓴 bits는 그 byte의 앞에 있음)
	//----------

	int i, j, a, b, ind = 0;
	int n, x, y;
	int* temp = (int*)malloc(w * h * sizeof(int));

	int value, size = 0, code_word = 0; // value ->절대값,   num->차분값,   size -> 2진수의 길이
	int ne_code; // 부수의 코드 -> 정수의 반대
	int total_length = 0;
	int data_length = 0;
	int entropy_cd = 0;
	int temp_ref = 0, temp_cur = 0;

	int back_f = 0;
	entropy_index = 0;

	for (i = 0; i < h; i += 8) {
		for (j = 0; j < w; j += 8) {
			n = 1;
			x = 0;
			y = 0;

			for (int idx = 0, d = 1; idx <= 2 * (N - 1); idx++, d *= (-1))
			{
				if (idx <= N - 1)
				{
					if (d == 1)
					{
						y = idx;
						x = idx - y;
						/*printf("x = %d, y = %d \n", x, y);*/
					}
					else
					{
						x = idx;
						y = idx - x;
						/*printf("x = %d, y = %d \n", x, y);*/
					}
				}
				else
				{
					if (d == 1)
					{
						x = idx - (N - 1);
						y = idx - x;
					}
					else
					{
						y = idx - (N - 1);
						x = idx - y;
					}
				}

				for (; x >= 0 && y >= 0 && x < N && y < N; x = x + d, y = y - d)
				{
					if (ind < w * h) {
						temp[ind++] = zigzag_input[(i + y) * w + (j + x)];
					}
				}
			}

			for (a = 0; a < N; a++) {
				for (b = 0; b < N; b++) {
					zigzag[(i + a) * w + (j + b)] = temp[a * N + b];
					zigzag_int[(i + a) * w + (j + b)] = temp[a * N + b];
				}
			}

		}

	}



	// ------------------------------------------
	// entropy_code
	// ------------------------------------------

	for (int con = 0; con < w * h; con++) {
		value = abs(temp[con]);
		size = 0;

		do {

			if (value == 0) {
				size = 0;
			}
			else {
				value = value / 2;
				size++;
			}
		} while (value != 0); // 2진수 생성

		switch (size) {
		case 0:
			code_word = 0 << 1;
			data_length = 2;
			break; // 00
		case 1:
			code_word = 2 << size;
			data_length = 4;
			break; // 010
		case 2:
			code_word = 3 << size;
			data_length = 5;
			break; // 011
		case 3:
			code_word = 4 << size;
			data_length = 6;
			break; // 100
		case 4:
			code_word = 5 << size;
			data_length = 7;
			break; // 101
		case 5:
			code_word = 6 << size;
			data_length = 8;
			break; // 110
		case 6:
			code_word = 14 << size;
			data_length = 10;
			break; // 1110
		case 7:
			code_word = 30 << size;
			data_length = 12;
			break; // 11110
		case 8: code_word = 62 << size;
			data_length = 14;
			break; // 111110
		case 9:
			code_word = 126 << size;
			data_length = 16;
			break; //1111110
		case 10:
			code_word = 254 << size;
			data_length = 18;
			break; //11111110
		case 11:
			code_word = 510 << size;
			data_length = 20;
			break; //111111110
		}

		total_length += data_length;

		if (temp[con] > 0) { // temp -> zigzag후 배열
			temp_cur = code_word ^ temp[con];
		}
		else {
			ne_code = (0xffffffff << size) ^ (~abs(temp[con])); // 정수 반대
			temp_cur = code_word ^ ne_code;
		}

		if (con != 0) {
			if (total_length > 32) { // 쌓인 비트수가 32비트를 넘는 경우
				back_f = 0;
				data_length = data_length - (total_length - 32); // 마지막 출력해야 할 bit 수
				entropy_cd = (entropy_cd << data_length) ^ (temp_cur >> (total_length - 32));
				entropy_result[entropy_index++] = entropy_cd;
				for (int a = 0; a < total_length - 32; a++) {
					back_f += pow(2.0, a);
				}
				entropy_cd = back_f & temp_cur; // 32bit를 저장하고 마지막 entorpy_cd의 뒤에 있는 bit 값
				total_length = total_length - 32;
				entropy_result[entropy_index] = entropy_cd; // 남는 부분을 다음 배열(entropy_result) 값에 저장
			}
			else { // 32비트 이내의 경우
				entropy_cd = (entropy_cd << data_length) ^ temp_cur;
				entropy_result[entropy_index] = entropy_cd;

			}

		}
		else {
			entropy_cd = temp_cur;
			entropy_result[entropy_index] = entropy_cd;
		}


	}
	if (total_length < 32) {
		entropy_result[entropy_index] = entropy_cd << (32 - total_length);
	}
	/*for (a = 0; a < entropy_index + 1; a++) {
		printf("%x,", entropy_result[a]); 
	}*/

	//printf("\n--------------------------%d-----------------------------,\n", entropy_index);
	/*printf("%d,\n", entropy_index);*/

	free(temp);


	/*for (a = 0; a < HEIGHT; a++) {
		for (b = 0; b < WIDTH; b++) {
			printf("%d,", temp[a * WIDTH + b]);
		}
		printf("\n");
	}*/
}

void entropy_encode(int index, int* entropy_input, int* entropy_result) {

	//----------
	// 지그제그 스캔해서 모든 정보를 한줄로 해준다음 entropy code 진행,
	// entropy code는 정보를 하나하나씩 저장해서 4byte(int형)가 되면 하나 저장, 
	// 마지막에 4byte가 안되면 그냥 저장해준다. (마지막 바이트에서 안 쓴 bits는 그 byte의 앞에 있음)
	//----------

	int i, j, a, b, ind = 0;
	int n, x, y;
	int* temp = entropy_input;

	int value, size = 0, code_word = 0; // value ->절대값,   num->차분값,   size -> 2진수의 길이
	int ne_code; // 부수의 코드 -> 정수의 반대
	int total_length = 0;
	int data_length = 0;
	int entropy_cd = 0;
	int temp_ref = 0, temp_cur = 0;

	int back_f = 0;
	entropy_index = 0;


	// ------------------------------------------
	// entropy_code
	// ------------------------------------------

	for (int con = 0; con < index; con++) {
		value = abs(temp[con]);
		size = 0;

		do {

			if (value == 0) {
				size = 0;
			}
			else {
				value = value / 2;
				size++;
			}
		} while (value != 0); // 2진수 생성

		switch (size) {
		case 0:
			code_word = 0 << 1;
			data_length = 2;
			break; // 00
		case 1:
			code_word = 2 << size;
			data_length = 4;
			break; // 010
		case 2:
			code_word = 3 << size;
			data_length = 5;
			break; // 011
		case 3:
			code_word = 4 << size;
			data_length = 6;
			break; // 100
		case 4:
			code_word = 5 << size;
			data_length = 7;
			break; // 101
		case 5:
			code_word = 6 << size;
			data_length = 8;
			break; // 110
		case 6:
			code_word = 14 << size;
			data_length = 10;
			break; // 1110
		case 7:
			code_word = 30 << size;
			data_length = 12;
			break; // 11110
		case 8: code_word = 62 << size;
			data_length = 14;
			break; // 111110
		case 9:
			code_word = 126 << size;
			data_length = 16;
			break; //1111110
		case 10:
			code_word = 254 << size;
			data_length = 18;
			break; //11111110
		case 11:
			code_word = 510 << size;
			data_length = 20;
			break; //111111110
		}

		total_length += data_length;

		if (temp[con] > 0) { // temp -> zigzag후 배열
			temp_cur = code_word ^ temp[con];
		}
		else {
			ne_code = (0xffffffff << size) ^ (~abs(temp[con])); // 정수 반대
			temp_cur = code_word ^ ne_code;
		}

		if (con != 0) {
			if (total_length > 32) { // 쌓인 비트수가 32비트를 넘는 경우
				back_f = 0;
				data_length = data_length - (total_length - 32); // 마지막 출력해야 할 bit 수
				entropy_cd = (entropy_cd << data_length) ^ (temp_cur >> (total_length - 32));
				entropy_result[entropy_index++] = entropy_cd;
				for (int a = 0; a < total_length - 32; a++) {
					back_f += pow(2.0, a);
				}
				entropy_cd = back_f & temp_cur; // 32bit를 저장하고 마지막 entorpy_cd의 뒤에 있는 bit 값
				total_length = total_length - 32;
				entropy_result[entropy_index] = entropy_cd; // 남는 부분을 다음 배열(entropy_result) 값에 저장
			}
			else { // 32비트 이내의 경우
				entropy_cd = (entropy_cd << data_length) ^ temp_cur;
				entropy_result[entropy_index] = entropy_cd;

			}

		}
		else {
			entropy_cd = temp_cur;
			entropy_result[entropy_index] = entropy_cd;
		}


	}
	if (total_length < 32) {
		entropy_result[entropy_index] = entropy_cd << (32 - total_length);
	}
	//for (a = 0; a < entropy_index + 1; a++) {
	//	printf("%x,", entropy_result[a]); 
	//}

	//printf("\n--------------------------%d-----------------------------,\n", entropy_index);
	/*printf("%d,\n", entropy_index);*/




	/*for (a = 0; a < HEIGHT; a++) {
		for (b = 0; b < WIDTH; b++) {
			printf("%d,", temp[a * WIDTH + b]);
		}
		printf("\n");
	}*/
}