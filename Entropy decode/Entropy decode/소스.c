#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>



int main()
{
	int aaa[4] = { 0x41e88a00,0x8a008088,0xaa820082,0x80000000, };
	int arr_size = 4;
	int data_size = 20;
	int out[100];
	int index = 0;
	char str[33] = { 0, };
	char buffer[33] = { 0, };
	char str_total[32 * 6 * 3] = { 0, }; //str_total[32*4*3]
	int data_length = 0;
	int total_length = 0; // 복원완료한 길이
	int category;
	int temp = 0;
	int aa;
	int buffer_len = 0;
	int ref_buffer_len = 0;
	int ref_buffer_len2 = 0;
	int ref_buffer_code = 0;
	int buffer_code = 0;
	int over_flow = 32; // 하나씩 읽어와서 남은 저장공간
	int ref_over_flow = 0;
	for (int con = 0; con < arr_size; con++) {

		total_length = 0;
		buffer_len = 0;
		buffer_code = 0;
		over_flow = 32;
		temp = 0;
		do {
			if (ref_over_flow < 0) {
				if (ref_buffer_len <= category) {

					for (int a = ref_buffer_len - 1; a >= 0; a--) {
						temp += pow(2.0, a);
					}
					int x = ((aaa[con] >> (32 - ref_buffer_len))) & temp;
					aa = (ref_buffer_code ^ (((aaa[con] >> (32 - ref_buffer_len))) & temp));

					if (aa >> ref_buffer_len == 1) {

						out[index - 1] = aa;
					}
					else if (aa >> ref_buffer_len == 0) {
						temp = 0;
						for (int a = category - 1; a >= 0; a--) {
							temp += pow(2.0, a);
						}
						out[index - 1] = -((~aa) & temp);

					}
					aa = aaa[con];
					total_length = ref_buffer_len;
					over_flow = 32 - ref_buffer_len;
				}
				else if (ref_over_flow == -1) {
					temp = 0;
					int temp2 = 0;
					for (int a = ref_buffer_len2 - 1; a >= 0; a--) {
						temp += pow(2.0, a);
					}
					for (int a = 31; a >= 32 - ref_buffer_len2; a--) {
						temp2 += pow(2.0, a);
					}
					int x = ((aaa[con + 1] >> ref_buffer_len2) ^ temp2);
					aa = ((aaa[con] & temp) << (32 - ref_buffer_len2)) ^ ((aaa[con + 1] >> ref_buffer_len2) ^ temp2); // ref_buffer_len: 전에 있던 거의 완성 안된 코드 길이
					total_length = 0;
					temp = 0;

					if (((aa >> (31 - total_length)) & 1) == 0) {
						if (((aa >> (31 - total_length - 1)) & 1) == 0) { // 00
							data_length = 2;
							category = 0;
							out[index++] = 0;
						}
						else if (((aa >> (31 - total_length - 1)) & 1) == 1) {
							if (((aa >> (31 - total_length - 2)) & 1) == 0) { // 010
								category = 1;
								for (int a = category - 1; a >= 0; a--) {
									temp += pow(2.0, a);
								}

								if (((aa >> (31 - total_length - 3)) & 1) == 1) {
									out[index++] = (aa >> (32 - total_length - 3 - category)) & temp;
								}
								else {
									out[index++] = -(~((aa >> (32 - total_length - 3 - category)) & temp) & temp);
								}
								data_length = 4;
							}


							else if (((aa >> (31 - total_length - 2)) & 1) == 1) { // 011
								category = 2;
								for (int a = category - 1; a >= 0; a--) {
									temp += pow(2.0, a);
								}

								if (((aa >> (31 - total_length - 3)) & 1) == 1) {
									out[index++] = (aa >> (32 - total_length - 3 - category)) & temp;
								}
								else {
									out[index++] = -(~((aa >> (32 - total_length - 3 - category)) & temp) & temp);
								}
								data_length = 5;
							}
						}

					}
					else if (((aa >> (31 - total_length)) & 1) == 1) {
						if (((aa >> (31 - total_length - 1)) & 1) == 0) {
							if (((aa >> (31 - total_length - 2)) & 1) == 0) { // 100
								category = 3;
								for (int a = category - 1; a >= 0; a--) {
									temp += pow(2.0, a);
								}

								if (((aa >> (31 - total_length - 3)) & 1) == 1) {
									out[index++] = (aa >> (32 - total_length - 3 - category)) & temp;
								}
								else {
									out[index++] = -(~((aa >> (32 - total_length - 3 - category)) & temp) & temp);
								}
								data_length = 6;
							}
							else if (((aa >> (31 - total_length - 2)) & 1) == 1) { // 101
								category = 4;
								for (int a = category - 1; a >= 0; a--) {
									temp += pow(2.0, a);
								}

								if (((aa >> (31 - total_length - 3)) & 1) == 1) {
									out[index++] = (aa >> (32 - total_length - 3 - category)) & temp;
								}
								else {
									out[index++] = -(~((aa >> (32 - total_length - 3 - category)) & temp) & temp);
								}
								data_length = 7;

							}
						}
						else if (((aa >> (31 - total_length - 1)) & 1) == 1) {
							if (((aa >> (31 - total_length - 2)) & 1) == 0) { // 110
								category = 5;
								for (int a = category - 1; a >= 0; a--) {
									temp += pow(2.0, a);
								}

								if (((aa >> (31 - total_length - 3)) & 1) == 1) {
									out[index++] = (aa >> (32 - total_length - 3 - category)) & temp;
								}
								else {
									out[index++] = -(~((aa >> (32 - total_length - 3 - category)) & temp) & temp);
								}
								data_length = 8;

							}
							else if (((aa >> (31 - total_length - 2)) & 1) == 1) {
								if (((aa >> (31 - total_length - 3)) & 1) == 0) { // 1110
									category = 6;
									data_length = 10;

									for (int a = category - 1; a >= 0; a--) {
										temp += pow(2.0, a);
									}

									if (((aa >> (31 - total_length - 4)) & 1) == 1) {
										out[index++] = (aa >> (32 - total_length - 4 - category)) & temp;
									}
									else {

										out[index++] = -(~((aa >> (32 - total_length - 4 - category)) & temp) & temp);
									}


								}
								else if (((aa >> (31 - total_length - 3)) & 1) == 1) {
									if (((aa >> (31 - total_length - 4)) & 1) == 0) { // 11110
										category = 7;
										for (int a = category - 1; a >= 0; a--) {
											temp += pow(2.0, a);
										}

										if (((aa >> (31 - total_length - 5)) & 1) == 1) {
											out[index++] = (aa >> (32 - total_length - 5 - category)) & temp;
										}
										else {
											out[index++] = -(~((aa >> (32 - total_length - 5 - category)) & temp) & temp);
										}
										data_length = 12;

									}
									else if (((aa >> (31 - total_length - 4)) & 1) == 1) {
										if (((aa >> (31 - total_length - 5)) & 1) == 0) { // 111110
											category = 8;
											for (int a = category - 1; a >= 0; a--) {
												temp += pow(2.0, a);
											}

											if (((aa >> (31 - total_length - 6)) & 1) == 1) {
												out[index++] = (aa >> (32 - total_length - 6 - category)) & temp;
											}
											else {
												out[index++] = -(~((aa >> (32 - total_length - 6 - category)) & temp) & temp);
											}
											data_length = 14;

										}
										else if (((aa >> (31 - total_length - 5)) & 1) == 1) {
											if (((aa >> (31 - total_length - 6)) & 1) == 0) { // 1111110
												category = 9;
												for (int a = category - 1; a >= 0; a--) {
													temp += pow(2.0, a);
												}

												if (((aa >> (31 - total_length - 7)) & 1) == 1) {
													out[index++] = (aa >> (32 - total_length - 7 - category)) & temp;
												}
												else {
													out[index++] = -(~((aa >> (32 - total_length - 7 - category)) & temp) & temp);
												}
												data_length = 16;

											}
										}
									}
								}
							}
						}
					}

					total_length = data_length - over_flow;
					over_flow = 32 - total_length;
					if (con + 1 < arr_size) {
						aa = aaa[++con];
					}

				}


				ref_buffer_len = 0;
				buffer_code = 0;
				ref_over_flow = 0;
			}
			else {
				aa = aaa[con];

			}
			temp = 0;

			if (((aa >> (31 - total_length)) & 1) == 0) {
				if (((aa >> (31 - total_length - 1)) & 1) == 0) { // 00
					data_length = 2;
					category = 0;
					out[index++] = 0;
				}
				else if (((aa >> (31 - total_length - 1)) & 1) == 1) {
					if (((aa >> (31 - total_length - 2)) & 1) == 0) { // 010
						category = 1;
						for (int a = category - 1; a >= 0; a--) {
							temp += pow(2.0, a);
						}

						if (((aa >> (31 - total_length - 3)) & 1) == 1) {
							out[index++] = (aa >> (32 - total_length - 3 - category)) & temp;
						}
						else {
							out[index++] = -(~((aa >> (32 - total_length - 3 - category)) & temp) & temp);
						}
						data_length = 4;
					}


					else if (((aa >> (31 - total_length - 2)) & 1) == 1) { // 011
						category = 2;
						for (int a = category - 1; a >= 0; a--) {
							temp += pow(2.0, a);
						}

						if (((aa >> (31 - total_length - 3)) & 1) == 1) {
							out[index++] = (aa >> (32 - total_length - 3 - category)) & temp;
						}
						else {
							out[index++] = -(~((aa >> (32 - total_length - 3 - category)) & temp) & temp);
						}
						data_length = 5;
					}
				}
			}
			else if (((aa >> (31 - total_length)) & 1) == 1) {
				if (((aa >> (31 - total_length - 1)) & 1) == 0) {
					if (((aa >> (31 - total_length - 2)) & 1) == 0) { // 100
						category = 3;
						for (int a = category - 1; a >= 0; a--) {
							temp += pow(2.0, a);
						}

						if (((aa >> (31 - total_length - 3)) & 1) == 1) {
							out[index++] = (aa >> (32 - total_length - 3 - category)) & temp;
						}
						else {
							out[index++] = -(~((aa >> (32 - total_length - 3 - category)) & temp) & temp);
						}
						data_length = 6;
					}
					else if (((aa >> (31 - total_length - 2)) & 1) == 1) { // 101
						category = 4;
						for (int a = category - 1; a >= 0; a--) {
							temp += pow(2.0, a);
						}

						if (((aa >> (31 - total_length - 3)) & 1) == 1) {
							out[index++] = (aa >> (32 - total_length - 3 - category)) & temp;
						}
						else {
							out[index++] = -(~((aa >> (32 - total_length - 3 - category)) & temp) & temp);
						}
						data_length = 7;

					}
				}
				else if (((aa >> (31 - total_length - 1)) & 1) == 1) {
					if (((aa >> (31 - total_length - 2)) & 1) == 0) { // 110
						category = 5;
						for (int a = category - 1; a >= 0; a--) {
							temp += pow(2.0, a);
						}

						if (((aa >> (31 - total_length - 3)) & 1) == 1) {
							out[index++] = (aa >> (32 - total_length - 3 - category)) & temp;
						}
						else {
							out[index++] = -(~((aa >> (32 - total_length - 3 - category)) & temp) & temp);
						}
						data_length = 8;

					}
					else if (((aa >> (31 - total_length - 2)) & 1) == 1) {
						if (((aa >> (31 - total_length - 3)) & 1) == 0) { // 1110
							category = 6;
							for (int a = category - 1; a >= 0; a--) {
								temp += pow(2.0, a);
							}

							if (((aa >> (31 - total_length - 4)) & 1) == 1) {
								out[index++] = (aa >> (32 - total_length - 4 - category)) & temp;
							}
							else {
								out[index++] = -(~((aa >> (32 - total_length - 4 - category)) & temp) & temp);
							}
							data_length = 10;

						}
						else if (((aa >> (31 - total_length - 3)) & 1) == 1) {
							if (((aa >> (31 - total_length - 4)) & 1) == 0) { // 11110
								category = 7;
								for (int a = category - 1; a >= 0; a--) {
									temp += pow(2.0, a);
								}

								if (((aa >> (31 - total_length - 5)) & 1) == 1) {

									out[index++] = (aa >> (32 - total_length - 5 - category)) & temp;
								}
								else {
									out[index++] = -(~((aa >> (32 - total_length - 5 - category)) & temp) & temp);
								}
								data_length = 12;

							}
							else if (((aa >> (31 - total_length - 4)) & 1) == 1) {
								if (((aa >> (31 - total_length - 5)) & 1) == 0) { // 111110
									category = 8;
									for (int a = category - 1; a >= 0; a--) {
										temp += pow(2.0, a);
									}

									if (((aa >> (31 - total_length - 6)) & 1) == 1) {
										out[index++] = (aa >> (32 - total_length - 6 - category)) & temp;
									}
									else {
										out[index++] = -(~((aa >> (32 - total_length - 6 - category)) & temp) & temp);
									}
									data_length = 14;

								}
								else if (((aa >> (31 - total_length - 5)) & 1) == 1) {
									if (((aa >> (31 - total_length - 6)) & 1) == 0) { // 1111110
										category = 9;
										for (int a = category - 1; a >= 0; a--) {
											temp += pow(2.0, a);
										}

										if (((aa >> (31 - total_length - 7)) & 1) == 1) {
											out[index++] = (aa >> (32 - total_length - 7 - category)) & temp;
										}
										else {
											out[index++] = -(~((aa >> (32 - total_length - 7 - category)) & temp) & temp);
										}
										data_length = 16;

									}
								}
							}
						}
					}
				}
			}
			if (index == data_size) {
				break;
			}
			over_flow -= data_length;

			ref_over_flow = over_flow;
			temp = 0;
			if (over_flow == 1) {
				ref_buffer_code = aaa[con] & 1;
				ref_buffer_len2 = 1;
				ref_over_flow = -1;
				category = -1;
			}
			else if (over_flow == 2) {
				for (int a = 1; a >= 0; a--) {
					temp += pow(2.0, a);
				}
				buffer_code = aaa[con] & temp;
				if (buffer_code != 0) {
					ref_buffer_code = buffer_code;
					ref_buffer_len2 = 2;
					ref_over_flow = -1;
					category = -1;
				}
			}
			else if (over_flow == 3) {
				for (int a = 2; a >= 0; a--) {
					temp += pow(2.0, a);
				}
				buffer_code = aaa[con] & temp;
				if (buffer_code == temp) {
					ref_buffer_code = buffer_code;
					ref_buffer_len2 = 3;
					ref_over_flow = -1;
					category = -1;
				}
			}
			else if (over_flow == 4) {
				for (int a = 3; a >= 0; a--) {
					temp += pow(2.0, a);
				}
				buffer_code = aaa[con] & temp;
				if (buffer_code == temp) {
					ref_buffer_code = buffer_code;
					ref_buffer_len2 = 4;
					ref_over_flow = -1;
					category = -1;
				}
			}
			else if (over_flow == 5) {
				for (int a = 4; a >= 0; a--) {
					temp += pow(2.0, a);
				}
				buffer_code = aaa[con] & temp;
				if (buffer_code == temp) {
					ref_buffer_code = buffer_code;
					ref_buffer_len2 = 5;
					ref_over_flow = -1;
					category = -1;
				}
			}
			else if (over_flow == 6) {
				for (int a = 5; a >= 0; a--) {
					temp += pow(2.0, a);
				}
				buffer_code = aaa[con] & temp;
				if (buffer_code == temp) {
					ref_buffer_code = buffer_code;
					ref_buffer_len2 = 6;
					ref_over_flow = -1;
					category = -1;
				}
			}
			else if (over_flow < 0) {
				buffer_len = abs(over_flow);
				ref_buffer_len = buffer_len;
				int ttt;
				temp = 0;
				if (buffer_len <= category) {
					ttt = 32 - total_length - data_length + category;
					for (int a = ttt - 1; a >= 0; a--) {
						temp += pow(2.0, a);
					}
					ref_buffer_code = (aaa[con] & temp << buffer_len);
				}
			}
			total_length += data_length;

		} while (over_flow > 0);


	}

	for (int a = 0; a < 14; a++) {
		printf("%d,", out[a]);
	}



	return 0;
}

