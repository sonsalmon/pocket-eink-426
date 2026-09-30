# Rev B 부품표와 비용 (직접 납땜, 4층)

기준 커밋은 `2095293`(rev-b)입니다. 기판 배선을 확정하는 중이라, 부품 구성이 바뀌면 이 표도 함께 고칩니다. 가격은 2026-09-30에 JLCPCB/LCSC 부품 검색 API로 조회한 1개당 가격이고, 재고와 가격은 주문할 때 다시 확인합니다. Rev A(CrossPoint, JLC 조립)의 부품표는 `docs/bom.md`에 있습니다.

## 1. 직접 사는 부품

| 부품 | 모델 | 구매처 | 가격(참고) |
| --- | --- | --- | --- |
| 전자잉크 화면 | Good Display GDEY0426T82 (4.26", 800×480, SSD1677, 24핀 FPC) | [buyepaper.com](https://www.buyepaper.com/products/gdey0426t82), [buy-lcd.com](https://buy-lcd.com/products/gdey0426t82) | $15~17 + 배송 |
| 화면(대체) | 고장 난 Xteink X4에서 떼어 낸 화면 (같은 패널) | 중고 거래 | 미확인 |
| 배터리 | 303450 LiPo 3.7 V 500 mAh, 보호회로(PCM) 붙은 것 | 국내 쇼핑몰 / 알리익스프레스 | 약 5,000~15,000원 |

Rev B 기판에는 배터리 보호회로가 없습니다. 반드시 보호회로가 붙은 셀을 사야 합니다.

## 2. 기판 부품 (기판 1장 기준, 54개 / 26종)

| 참조 | 부품 | 값 | 풋프린트 | LCSC | 수량 | 단가 |
| --- | --- | --- | --- | --- | --- | --- |
| U1 | ESP32-C3-WROOM-02-N4 | ESP32-C3-WROOM-02-N4 | ESP32-C3-WROOM-02-N4 | [C2934560](https://www.lcsc.com/product-detail/C2934560.html) | 1 | $3.2880 |
| U4 | MCP73831T-2ACI/OT | MCP73831-2 | SOT-23-5 | [C424093](https://www.lcsc.com/product-detail/C424093.html) | 1 | $0.7818 |
| C21 C22 C23 C24 C25 C26 | CL21A475KBQNNNE | 4.7uF | C_0805_2012Metric | [C98192](https://www.lcsc.com/product-detail/C98192.html) | 6 | $0.0953 |
| J2 | FH12-24S-0.5SH(55) | FH12-24S-0.5SH | Hirose_FH12-24S-0.5SH_1x24-1MP_P0.50mm_Horizontal | [C202112](https://www.lcsc.com/product-detail/C202112.html) | 1 | $0.5120 |
| Q3 | SI1308EDL-T1-GE3 | Si1308EDL | SOT-323_SC-70 | [C469327](https://www.lcsc.com/product-detail/C469327.html) | 1 | $0.2960 |
| SW1 SW2 SW3 | SKRPACE010 | PREV | KEY-SMD_4P-L4.2-W3.2-P2.20-LS4.6 | [C139797](https://www.lcsc.com/product-detail/C139797.html) | 3 | $0.0633 |
| J3 | TYPE-C-31-M-12 | USB-C | USB_C_Receptacle_HRO_TYPE-C-31-M-12 | [C165948](https://www.lcsc.com/product-detail/C165948.html) | 1 | $0.1857 |
| SW4 | EVQP7C01P | POWER | SW_SPST_EVQP7C | [C388883](https://www.lcsc.com/product-detail/C388883.html) | 1 | $0.1853 |
| U5 | RT9080-33GJ5 | RT9080-33GJ5 | SOT-23-5 | [C841192](https://www.lcsc.com/product-detail/C841192.html) | 1 | $0.1200 |
| D2 D3 D4 | MBR0530 | MBR0530 | D_SOD-123 | [C77336](https://www.lcsc.com/product-detail/C77336.html) | 3 | $0.0355 |
| C5 C14 C19 | CL10A106KP8NNNC | 10uF | C_0603_1608Metric | [C19702](https://www.lcsc.com/product-detail/C19702.html) | 3 | $0.0319 |
| C16 C17 C32 | CL10A475KO8NNNC | 4.7uF | C_0603_1608Metric | [C19666](https://www.lcsc.com/product-detail/C19666.html) | 3 | $0.0295 |
| C12 C18 C27 C28 C29 | CL10A105KB8NNNC | 1uF | C_0603_1608Metric | [C15849](https://www.lcsc.com/product-detail/C15849.html) | 5 | $0.0166 |
| L1 | FNR4012S470MT | 47uH | IND-SMD_L4.0-W4.0_FNR4012S | [C167794](https://www.lcsc.com/product-detail/C167794.html) | 1 | $0.0608 |
| C6 C15 C30 C31 | CC0603KRX7R9BB104 | 100nF | C_0603_1608Metric | [C14663](https://www.lcsc.com/product-detail/C14663.html) | 4 | $0.0123 |
| R3 R4 R5 R19 R22 | 0603WAF1002T5E | 10k | R_0603_1608Metric | [C25804](https://www.lcsc.com/product-detail/C25804.html) | 5 | $0.0018 |
| D1 | KT-0603R | RED | LED_0603_1608Metric | [C2286](https://www.lcsc.com/product-detail/C2286.html) | 1 | $0.0076 |
| R25 R26 | 0603WAF1003T5E | 100k | R_0603_1608Metric | [C25803](https://www.lcsc.com/product-detail/C25803.html) | 2 | $0.0031 |
| R8 R9 R18 | 0603WAF1004T5E | 1M | R_0603_1608Metric | [C22935](https://www.lcsc.com/product-detail/C22935.html) | 3 | $0.0019 |
| R17 | 0603WAF220KT5E | 2.2R | R_0603_1608Metric | [C22939](https://www.lcsc.com/product-detail/C22939.html) | 1 | $0.0033 |
| R23 R24 | 0603WAF5101T5E | 5.1k | R_0603_1608Metric | [C23186](https://www.lcsc.com/product-detail/C23186.html) | 2 | $0.0015 |
| R20 | 0603WAF1502T5E | 15k | R_0603_1608Metric | [C22809](https://www.lcsc.com/product-detail/C22809.html) | 1 | $0.0029 |
| R11 | 0603WAF1001T5E | 1k | R_0603_1608Metric | [C21190](https://www.lcsc.com/product-detail/C21190.html) | 1 | $0.0026 |
| R21 | 0603WAF6801T5E | 6.8k | R_0603_1608Metric | [C23212](https://www.lcsc.com/product-detail/C23212.html) | 1 | $0.0023 |
| F1 | 0603WAF0000T5E | 0R | R_0603_1608Metric | [C21189](https://www.lcsc.com/product-detail/C21189.html) | 1 | $0.0023 |
| R10 | 0603WAF3601T5E | 3.9k | R_0603_1608Metric | [C22980](https://www.lcsc.com/product-detail/C22980.html) | 1 | $0.0019 |

부품값 합계는 1장당 $6.66입니다. 5장 분량을 LCSC 최소 구매 수량에 맞춰 사면 $43.05입니다. ESP32 모듈 5개($16.4)가 이 금액의 대부분을 차지합니다.

## 3. 예상 비용 (5장, 화면·배터리 제외, 추정)

| 항목 | 금액 |
| --- | --- |
| 부품 (LCSC, 최소 수량 포함) | 약 $43 |
| 4층 기판 5장 (JLCPCB, 0.8 mm) | 약 $10~20 |
| 스텐실 (선택) | 약 $7 |
| 배송 (LCSC + JLCPCB, 한국) | 약 $25~40 |
| 합계 | 약 $78~110 (약 11~15만 원) |

Rev A(JLC 조립)는 약 $155~175였습니다. Rev B는 부품 수가 83개에서 54개로 줄었고, 확장 부품 장착비(약 $66)도 없어집니다.

## 4. 납땜 난이도

- ESP32-C3-WROOM-02 모듈은 가장자리 패드 18개와 바닥 GND 패드로 되어 있습니다. 가장자리는 인두로 붙입니다. 바닥 GND는 모듈 아래에 뚫은 비아로 기판 뒤에서 땜납을 흘려 넣어 붙입니다. 핫에어가 있으면 그것으로 붙여도 됩니다.
- 가장 어려운 부품은 화면 커넥터 FH12(0.5 mm 간격 24핀)와 USB-C(0.5 mm)입니다. 플럭스와 솔더윅을 쓰고, 확대경으로 브릿지가 없는지 확인합니다.
- 수동 부품은 0603(1.6 × 0.8 mm)이고, 승압용 커패시터만 0805입니다.
- 공구가 없으면 따로 약 7~10만 원이 듭니다: 온도 조절 인두, 플럭스, 솔더윅, 핀셋, 확대경. 핫에어가 있으면 좋습니다.
