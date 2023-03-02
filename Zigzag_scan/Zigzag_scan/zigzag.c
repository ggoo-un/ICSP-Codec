#include <stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include <math.h>
#define PI 3.1415926535
#define WIDTH 352
#define HIGHT 288


int entropy_index = 0;
unsigned char arr1[288 * 352]; //y
unsigned char arr2[144 * 176]; //u
unsigned char arr3[144 * 176]; //v

int zigzag_int[288 * 352];

// [추가
#define N 8
unsigned char zigzag[288 * 352];
void zig_zag_scan(int, int, int*, int*);
// 추가]


int main()
{
	errno_t err;
	errno_t err1;
	errno_t err2;
	errno_t err3;


	FILE* fp;
	FILE* fp1;
	FILE* fp2;
	FILE* fp3;

	FILE* fp_zigzag;

	int frame_size = 1;

	err = fopen_s(&fp, "C:/Users/GUN/Desktop/코덱/신입생코덱/CIF(352x288)/football_cif(352X288)_90f.yuv", "rb");   //영상 읽어오기

	fopen_s(&fp_zigzag, "C:/Users/GUN/Desktop/코덱/신입생코덱/CIF(352x288)/football_zigzag.yuv", "wb");

	unsigned char* pic = (unsigned char*)malloc(WIDTH * HIGHT * 3 / 2); //시작위치

	int i, j;
	int frame_con;

	int a, b, a_max, b_max, a_val, b_val, c, d;
	int MV_x = 0, MV_y = 0;
	int sad_min;

	for (frame_con = 1; frame_con <= frame_size; frame_con++) {
		//한 프레임씩 읽어오기

		fread(pic, 1, WIDTH * HIGHT * 3 / 2, fp);  //?

		int* entropy_result = (int*)malloc(sizeof(int) * 20000);

		unsigned char** p1 = (unsigned char**)malloc(sizeof(unsigned char*) * HIGHT);			//Y성분을 저장한 2중 포인터
		unsigned char** p2 = (unsigned char**)malloc(sizeof(unsigned char*) * HIGHT / 2);		//U
		unsigned char** p3 = (unsigned char**)malloc(sizeof(unsigned char*) * HIGHT / 2);		//V

		for (i = 0; i < HIGHT; i++) {
			p1[i] = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH);
		}
		for (i = 0; i < HIGHT / 2; i++) {
			p2[i] = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH / 2);
			p3[i] = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH / 2);
		}


		int z = 0;
		for (i = 0; i < HIGHT; i++)
		{
			for (j = 0; j < WIDTH; j++)
			{
				p1[i][j] = pic[i * WIDTH + j];  //Y성분을 읽어와서 2중 포인터에 저장
				arr1[i * WIDTH + j] = p1[i][j];	//2중포인터를 사용해서 arr1로 출력
			}
		}


		for (i = 0; i < HIGHT / 2; i++)
		{
			for (j = 0; j < WIDTH / 2; j++)
			{
				p2[i][j] = pic[HIGHT * WIDTH + i * WIDTH / 2 + j];                 //U
				arr2[i * WIDTH / 2 + j] = p2[i][j];

				p3[i][j] = pic[HIGHT * WIDTH * 5 / 4 + i * WIDTH / 2 + j];         //V
				arr3[i * WIDTH / 2 + j] = p3[i][j];
			}
		}
	/*	for (i = 0; i < HIGHT; i++) {
			for (j = 0; j < WIDTH; j++) {
				printf("%d,",arr1[i*WIDTH+j]);
			}
			printf("\n");
		}*/

		

		zig_zag_scan(352, 288, arr1, entropy_result);

		

		for (i = 0; i < HIGHT; i++) {
			free(p1[i]);
		}
		free(p1);

		for (i = 0; i < HIGHT / 2; i++) {
			free(p2[i]);
			free(p3[i]);
		}
		free(p2);
		free(p3);

		//새로운 파일에 쓰기

		fwrite(&zigzag[0], 1, WIDTH * HIGHT, fp_zigzag);

	}


	free(pic);
	fclose(fp);


	fclose(fp_zigzag);



	return 0;
}

// [추가
void zig_zag_scan(int w, int h, int* zigzag_input, int* entropy_result) {

	//----------
	// 지그제그 스캔해서 모든 정보를 한줄로 해준다음 entropy code 진행,
	// entropy code는 정보를 하나하나씩 저장해서 4byte(int형)가 되면 하나 저장, 
	// 마지막에 4byte가 안되면 그냥 저장해준다. (마지막 바이트에서 안 쓴 bits는 그 byte의 앞에 있음)
	//----------

	int i, j, a, b, ind = 0;
	int n, x, y;
	int* temp = (int*)malloc(w * h * sizeof(int));

	int value, size = 0, code_word; // value ->절대값,   num->차분값,   size -> 2진수의 길이
	int ne_code; // 부수의 코드 -> 정수의 반대
	int total_length = 0;
	int data_length = 0;
	int entropy_cd;
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

		// 0정의하지 않았음 !!!!!
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
		}
		total_length += data_length;

		if (temp[con] > 0) {
			temp_cur = code_word ^ temp[con];
		}
		else {
			ne_code = (0xffffffff << size) ^ (~abs(temp[con]));
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
	//for (a = 0; a < entropy_index + 1; a++) {
	//	printf("%x,", entropy_result[a]); 
	//}

	//printf("\n--------------------------%d-----------------------------,\n", entropy_index);
	//printf("%d,\n", entropy_index);

	free(temp);


	/*for (a = 0; a < HEIGHT; a++) {
		for (b = 0; b < WIDTH; b++) {
			printf("%d,", temp[a * WIDTH + b]);
		}
		printf("\n");
	}*/
}

// 추가]