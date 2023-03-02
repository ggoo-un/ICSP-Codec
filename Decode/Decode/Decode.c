#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#define PI 3.1415926535
#define WIDTH 352
#define HEIGHT 288
#define Frame_size 90

#define INTER_BLOCK_SIZE 8 // inter 블록의 사이즈
#define SEARCH_RANGE 15  // inter search range 가로, 세로 찾는 횟수 inter search range -> 15 + (8 - 1) = 22
#define SEARCH_INDEX 7 // reference_arr index 범위 때 사용

#define DPCM_MOD_Y 6
#define DPCM_MOD_UV 6
#define QP_DC 2.0
#define QP_AC 16.0


void REVERSE_INTRA(int, int, int*);
void REVERSE_DPCM(int, int, int, int*);

void REVERSE_INTER(int, int, int, int*, unsigned char*, int*);

void IDCT(int, int, double, double, int*);
//void IDCT(int, int, double*);

// intra, intra dpcm
int mod_arr[44 * 36];  // 각 블록의 intra model 를 저장 

// reverse intra, reverse intra dpcm
unsigned char re_intra_out[288 * 352]; // 역intra 함수 사용해서 출력한 배열(파일 생성 때 사용)
int re_intra_int[288 * 352];
int re_intra_dpcm_int[288 * 352];
unsigned char re_intra_dpcm_out[288 * 352];

// inter
unsigned  char inter_out[288 * 352];

// dct, idct
int AC_Flag[44 * 36];
int dct_out_int[288 * 352];
int idct_out_int[288 * 352];
unsigned char idct_out[288 * 352];


// 현재 프레임을 저장하는 포인터
unsigned char* out_y;
unsigned char* out_u;
unsigned char* out_v;

// 전 프레임 데이터
unsigned char ref_arr_y[288 * 352]; //메모리
unsigned char ref_arr_u[144 * 176];
unsigned char ref_arr_v[144 * 176];
int MV[(HEIGHT / INTER_BLOCK_SIZE) * (WIDTH / INTER_BLOCK_SIZE) * 2];
unsigned char re_cur_arr[288 * 352];

int dct_y_int[288 * 352];
int dct_u_int[144 * 176];
int dct_v_int[144 * 176];

#define N 8
unsigned char zigzag[288 * 352];
int zigzag_int[288 * 352];

int entropy_index = 0;




int main()
{

	FILE* fp;

	FILE* fp_re_intra_dpcm_de;
	FILE* fp_re_intra_de;

	FILE* fp_re_inter_de;

	FILE* fp_idct_de;


	fopen_s(&fp, "E:/코덱/신입생코덱/CIF(352x288)/result/football_dct_copy.txt", "rb");   //영상 읽어오기

	fopen_s(&fp_re_intra_dpcm_de, "E:/코덱/신입생코덱/CIF(352x288)/result/football_re_intra_dpcm_de.yuv", "wb");  //re_intra_dpcm_y = intra_out
	fopen_s(&fp_re_intra_de, "E:/코덱/신입생코덱/CIF(352x288)/result/football_re_intra_de.yuv", "wb");  //re_intra_y

	fopen_s(&fp_re_inter_de, "E:/코덱/신입생코덱/CIF(352x288)/result/football_re_inter_de.yuv", "wb");

	fopen_s(&fp_idct_de, "E:/코덱/신입생코덱/CIF(352x288)/result/football_idct_de.yuv", "wb");  //y


	int i, j;
	int frame_con;


	//한 프레임씩 읽어오기
	int* entropy_result = (int*)malloc(sizeof(int) * 160000 * Frame_size);
	/*int entropy_result[160000 * Frame_size];*/

	fread(entropy_result, 4, 160000 * Frame_size, fp);

	// Y, U, V 성분을 저장한 2중 포인터

	unsigned char* re_output_y = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH * HEIGHT);
	unsigned char* re_output_u = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH * HEIGHT / 4);
	unsigned char* re_output_v = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH * HEIGHT / 4);
	int index = 0;
	for (frame_con = 1; frame_con <= Frame_size; frame_con++) {
		// Y신호
		for (int a = 0; a < 36; a++) {
			for (int b = 0; b < 44; b++) {
				mod_arr[a * 44 + b] = entropy_result[index++];
				dct_y_int[a * 8 * WIDTH + b * 8] = entropy_result[index++];
				AC_Flag[a * 44 + b] = entropy_result[index++];
				if (AC_Flag[a * 44 + b] == 1) {
					for (int c = 0; c < 8; c++) {
						for (int d = 0; d < 8; d++) {
							if (!((c == 0) && (d == 0))) {
								dct_y_int[(a * 8 + c) * WIDTH + (b * 8 + d)] = 0;
							}
						}
					}
				}
				else {
					for (int c = 0; c < 8; c++) {
						for (int d = 0; d < 8; d++) {
							if (!((c == 0) && (d == 0))) {
								dct_y_int[(a * 8 + c) * WIDTH + (b * 8 + d)] = entropy_result[index++];
							}
						}
					}
				}
			}
		}


		// U신호

		for (int a = 0; a < 18; a++) {
			for (int b = 0; b < 22; b++) {
				dct_u_int[a * 8 * WIDTH / 2 + b * 8] = entropy_result[index++];
				AC_Flag[a * 22 + b] = entropy_result[index++];
				if (AC_Flag[a * 22 + b] == 1) {
					for (int c = 0; c < 8; c++) {
						for (int d = 0; d < 8; d++) {
							if (!((c == 0) && (d == 0))) {
								dct_u_int[(a * 8 + c) * WIDTH + (b * 8 + d)] = 0;
							}
						}
					}
				}
				else {
					for (int c = 0; c < 8; c++) {
						for (int d = 0; d < 8; d++) {
							if (!((c == 0) && (d == 0))) {
								dct_u_int[(a * 8 + c) * WIDTH / 2 + (b * 8 + d)] = entropy_result[index++];
							}
						}
					}
				}
			}
		}

		// V신호

		for (int a = 0; a < 18; a++) {
			for (int b = 0; b < 22; b++) {
				dct_v_int[a * 8 * WIDTH / 2 + b * 8] = entropy_result[index++];
				AC_Flag[a * 22 + b] = entropy_result[index++];
				if (AC_Flag[a * 22 + b] == 1) {
					for (int c = 0; c < 8; c++) {
						for (int d = 0; d < 8; d++) {
							if (!((c == 0) && (d == 0))) {
								dct_v_int[(a * 8 + c) * WIDTH + (b * 8 + d)] = 0;
							}
						}
					}
				}
				else {
					for (int c = 0; c < 8; c++) {
						for (int d = 0; d < 8; d++) {
							if (!((c == 0) && (d == 0))) {
								dct_v_int[(a * 8 + c) * WIDTH / 2 + (b * 8 + d)] = entropy_result[index++];
							}
						}
					}
				}
			}
		}

		/*for (int a = 0; a < 36; a++) {
			for (int b = 0; b < 44; b++) {
				printf("%d,", mod_arr[a * 44 + b]);
			}
			printf("\n");
		}
		printf("\n---------------------------------\n");

		for (int a = 0; a < 176; a++) {
			for (int b = 0; b < 288; b++) {
				printf("%d,", dct_v_int[a * 288 + b]);
			}
			printf("\n");
		}*/



		// 새로운 파일에 쓰기
		// Y, U, V성분으로 나눔


		if (frame_con % 10 == 1) {

			//---------------------------------------------
			//					intra y성분
			//---------------------------------------------

			IDCT(WIDTH, HEIGHT, QP_DC, QP_AC, dct_y_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT, fp_idct_de);


			REVERSE_DPCM(DPCM_MOD_Y, 352, 288, idct_out_int);


			// 역intra
			REVERSE_INTRA(352, 288, re_intra_dpcm_int);
			fwrite(&re_intra_out[0], 1, WIDTH * HEIGHT, fp_re_intra_de);

			out_y = re_intra_out;

			//---------------------------------------------
			//					intra u성분
			//---------------------------------------------


			IDCT(176, 144, QP_DC, QP_AC, dct_u_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT / 4, fp_idct_de);
			/*for (int a = 0; a < 144; a++) {
				for (int b = 0; b < 176; b++) {
					printf("%d,", idct_out[a * 176 + b]);
				}
				printf("\n");
			}*/


			REVERSE_DPCM(DPCM_MOD_UV, 176, 144, idct_out_int);

			fwrite(&re_intra_dpcm_out[0], 1, WIDTH * HEIGHT / 4, fp_re_intra_de);
			out_u = re_intra_dpcm_out;

			//---------------------------------------------
			//					intra v성분
			//---------------------------------------------

			IDCT(176, 144, QP_DC, QP_AC, dct_v_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT / 4, fp_idct_de);


			REVERSE_DPCM(DPCM_MOD_UV, 176, 144, idct_out_int);

			fwrite(&re_intra_dpcm_out[0], 1, WIDTH * HEIGHT / 4, fp_re_intra_de);

			out_v = re_intra_dpcm_out;
		}

		else {
			//---------------------------------------------
			//					inter y성분
			//---------------------------------------------


			IDCT(WIDTH, HEIGHT, QP_DC, QP_AC, dct_y_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT, fp_idct_de);


			// 역inter
			REVERSE_INTER(352, 288, INTER_BLOCK_SIZE, MV, ref_arr_y, idct_out_int);
			fwrite(&re_cur_arr[0], 1, WIDTH * HEIGHT, fp_re_intra_de);

			out_y = re_cur_arr;

			//---------------------------------------------
			//					inter u성분
			//---------------------------------------------

			IDCT(WIDTH / 2, HEIGHT / 2, QP_DC, QP_AC, dct_u_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT / 4, fp_idct_de);


			// 역inter
			REVERSE_INTER(WIDTH / 2, HEIGHT / 2, INTER_BLOCK_SIZE, MV, ref_arr_u, idct_out_int);
			fwrite(&re_cur_arr[0], 1, WIDTH * HEIGHT / 4, fp_re_intra_de);

			out_u = re_cur_arr;

			//---------------------------------------------
			//					inter v성분
			//---------------------------------------------

			IDCT(WIDTH / 2, HEIGHT / 2, QP_DC, QP_AC, dct_v_int);
			fwrite(&idct_out[0], 1, WIDTH * HEIGHT / 4, fp_idct_de);


			// 역inter
			REVERSE_INTER(WIDTH / 2, HEIGHT / 2, INTER_BLOCK_SIZE, MV, ref_arr_v, idct_out_int);
			fwrite(&re_cur_arr[0], 1, WIDTH * HEIGHT / 4, fp_re_intra_de);

			out_v = re_cur_arr;

		}

		for (i = 0; i < HEIGHT / 2; i++)
		{
			for (j = 0; j < WIDTH / 2; j++)
			{
				re_output_u[i * WIDTH / 2 + j] = out_u[i * WIDTH / 2 + j];
				re_output_v[i * WIDTH / 2 + j] = out_v[i * WIDTH / 2 + j];
			}
		}



		for (i = 0; i < HEIGHT; i++)
		{
			for (j = 0; j < WIDTH; j++)
			{
				re_output_y[i * WIDTH + j] = out_y[i * WIDTH + j];
			}
		}


		free(re_output_y);
		free(re_output_u);
		free(re_output_v);






		fclose(fp);
		fclose(fp_re_intra_dpcm_de);
		fclose(fp_re_intra_de);

		fclose(fp_re_inter_de);

		fclose(fp_idct_de);
	}

	free(entropy_result);
	return 0;
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
					temp_left[a] = re_intra_out[(i + a) * w + (j - 1)];
				}
			}
			else if ((i != 0) && (j == 0)) {
				for (a = 0; a < 8; a++) {
					temp_left[a] = 128;
					temp_up[a] = re_intra_out[(i - 1) * w + (j + a)];
				}
			}

			else if ((i != 0) && (j != 0)) {
				for (a = 0; a < 8; a++) { //블록단위
					temp_up[a] = re_intra_out[(i - 1) * w + (j + a)];
					temp_left[a] = re_intra_out[(i + a) * w + (j - 1)];
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
						re_intra_int[(i + a) * w + (j + b)] = temp_up[b] - re_intra_dpcm[(i + a) * w + (j + b)];
					}
				}
			}
			else if (mod_arr[index] == 2) {
				for (a = 0; a < 8; a++) {
					for (b = 0; b < 8; b++) {
						re_intra_out[(i + a) * w + (j + b)] = ave - re_intra_dpcm[(i + a) * w + (j + b)];
						re_intra_int[(i + a) * w + (j + b)] = temp_up[b] - re_intra_dpcm[(i + a) * w + (j + b)];
					}
				}
			}
			index++;
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
							temp = temp + DCT_AT[a][k] * dct_out_int[(k + i) * w + b + j] * qp_dc;
						}
						else {
							temp = temp + DCT_AT[a][k] * dct_out_int[(k + i) * w + b + j] * qp_ac;
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