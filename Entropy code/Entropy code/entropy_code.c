#include <stdio.h>
#include <math.h>

int main()
{

    int value, size = 0, code_word; // value ->절대값,   num->차분값,   size -> 2진수의 길이
    int ne_code; // 부수의 코드 -> 정수의 반대
    int count[10];
    char* range = (char*)malloc(sizeof(char));
    int total_length = 0;
    int data_length = 0;
    int entropy_cd;
    int temp_ref = 0, temp_cur = 0;
    int entropy_result[10];

    int back_f = 0;

    int index = 0;

    int num[64] = { -1 ,- 2, - 1, - 2, - 3, - 4, - 2, - 2, 
        - 4,	4	,0,	4, - 3, - 2, - 3, - 3, 
        - 2, - 2,	4,	0, - 1,	1, - 2,	0,
        4, - 2, - 1 ,- 3,	1	,0	,0,	2,
        0 ,- 2	,1 ,- 1, - 1	,1	,1 ,- 1,
        1,	0	,0, - 1	,1 ,- 1,	0	,0 ,
        - 1, -1,	1	,1 ,- 1	,0 ,- 1,	0,
        0,	0,	0,	1,	1,	0,	0,	1,};

    /*int num[13] = {0,0,0,1,2,3,0,0,0,0,0,0,0};*/

    for (int con = 0; con < 64; con++) {
        value = abs(num[con]);
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
        }
        total_length += data_length;

        if (num[con] > 0) {
            temp_cur = code_word ^ num[con];
        }
        else {
            ne_code = (0xffffffff << size) ^ (~abs(num[con]));
            temp_cur = code_word ^ ne_code;
        }
        
        if (con != 0) {
            if (total_length > 32) {
                back_f = 0;
                data_length = data_length - (total_length - 32); // 마지막 출력해야 할 bit 수
                entropy_cd = (entropy_cd << data_length) ^ (temp_cur >> total_length - 32);
                entropy_result[index++] = entropy_cd;
                for (int a = 0; a < total_length - 32; a++) {
                    back_f += pow(2.0, a);
                }
                entropy_cd = back_f & temp_cur; // 32bit를 저장하고 마지막 entorpy_cd의 뒤에 있는 bit 값
                total_length = total_length - 32;
                entropy_result[index] = entropy_cd;
            }
            else {
                entropy_cd = (entropy_cd << data_length) ^ temp_cur;
                entropy_result[index] = entropy_cd;
            }

        }
        else {
            entropy_cd = temp_cur;
            entropy_result[index] = entropy_cd;
        }
        
    }
    for (int i = 0; i < index + 1; i++) {
        printf("%x,", entropy_result[i]);
     }

    return 0;
}
