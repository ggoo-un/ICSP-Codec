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

// [�߰�
#define N 8
unsigned char zigzag[288 * 352];
void zig_zag_scan(int, int, int*, int*);
// �߰�]


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

	err = fopen_s(&fp, "C:/Users/GUN/Desktop/�ڵ�/���Ի��ڵ�/CIF(352x288)/football_cif(352X288)_90f.yuv", "rb");   //���� �о����

	fopen_s(&fp_zigzag, "C:/Users/GUN/Desktop/�ڵ�/���Ի��ڵ�/CIF(352x288)/football_zigzag.yuv", "wb");

	unsigned char* pic = (unsigned char*)malloc(WIDTH * HIGHT * 3 / 2); //������ġ

	int i, j;
	int frame_con;

	int a, b, a_max, b_max, a_val, b_val, c, d;
	int MV_x = 0, MV_y = 0;
	int sad_min;

	for (frame_con = 1; frame_con <= frame_size; frame_con++) {
		//�� �����Ӿ� �о����

		fread(pic, 1, WIDTH * HIGHT * 3 / 2, fp);  //?

		int* entropy_result = (int*)malloc(sizeof(int) * 20000);

		unsigned char** p1 = (unsigned char**)malloc(sizeof(unsigned char*) * HIGHT);			//Y������ ������ 2�� ������
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
				p1[i][j] = pic[i * WIDTH + j];  //Y������ �о�ͼ� 2�� �����Ϳ� ����
				arr1[i * WIDTH + j] = p1[i][j];	//2�������͸� ����ؼ� arr1�� ���
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

		//���ο� ���Ͽ� ����

		fwrite(&zigzag[0], 1, WIDTH * HIGHT, fp_zigzag);

	}


	free(pic);
	fclose(fp);


	fclose(fp_zigzag);



	return 0;
}

// [�߰�
void zig_zag_scan(int w, int h, int* zigzag_input, int* entropy_result) {

	//----------
	// �������� ��ĵ�ؼ� ��� ������ ���ٷ� ���ش��� entropy code ����,
	// entropy code�� ������ �ϳ��ϳ��� �����ؼ� 4byte(int��)�� �Ǹ� �ϳ� ����, 
	// �������� 4byte�� �ȵǸ� �׳� �������ش�. (������ ����Ʈ���� �� �� bits�� �� byte�� �տ� ����)
	//----------

	int i, j, a, b, ind = 0;
	int n, x, y;
	int* temp = (int*)malloc(w * h * sizeof(int));

	int value, size = 0, code_word; // value ->���밪,   num->���а�,   size -> 2������ ����
	int ne_code; // �μ��� �ڵ� -> ������ �ݴ�
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

		// 0�������� �ʾ��� !!!!!
		do {

			if (value == 0) {
				size = 0;
			}
			else {
				value = value / 2;
				size++;
			}
		} while (value != 0); // 2���� ����

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
			if (total_length > 32) { // ���� ��Ʈ���� 32��Ʈ�� �Ѵ� ���
				back_f = 0;
				data_length = data_length - (total_length - 32); // ������ ����ؾ� �� bit ��
				entropy_cd = (entropy_cd << data_length) ^ (temp_cur >> (total_length - 32));
				entropy_result[entropy_index++] = entropy_cd;
				for (int a = 0; a < total_length - 32; a++) {
					back_f += pow(2.0, a);
				}
				entropy_cd = back_f & temp_cur; // 32bit�� �����ϰ� ������ entorpy_cd�� �ڿ� �ִ� bit ��
				total_length = total_length - 32;
				entropy_result[entropy_index] = entropy_cd; // ���� �κ��� ���� �迭(entropy_result) ���� ����
			}
			else { // 32��Ʈ �̳��� ���
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

// �߰�]