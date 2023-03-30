# ICSP Codec

[![report](https://img.shields.io/badge/Report-brightgreen.svg)](./Codec_report.pdf)

> *H.264/MPEG 동영상 압축 코덱의 인코더, 디코더 구현*



## 프로젝트 목적
- C프로그래밍을 활용하여 코덱의 전체과정 구현
- 각 단계의 출력 영상을 확인하면서 동영상 압축과정을 이해하고 영상신호처리의 역량 향상

## 구현 내용
- 8 x 8 macroblock partitioning
- 예측 부호화
	* Intra prediction: Vertical, Horizontal, DC mode
	* Inter prediction: Spiral scan, Motion compensation
	* Pixel DPCM, DC DPCM 
- 변환 부호화: DCT, Inverse DCT, Quantization
- Reordering: Zigzag scan
- 엔트로피 부호화: Huffman coding
- 3개의 실험영상에 대해 총 144개의 case를 확인
- 실험 과정의 효율을 높이기 위해 실험용 코드의 조건을 변경 가능한 Bat file 구현

## Input Data
- YUV 4:2:0

<hr />

- This project shows the process of image codec.
- The report can be found [here](./Codec_report.pdf).
- To confirm image codec processing, run:
```
./Encode/Release/ICSP.bat
``` 


## Contact
Should you have any question, please contact gooni0906@gmail.com.