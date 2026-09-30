# 부품표와 구매처

가격은 2026-09-30에 JLCPCB 부품 검색 API로 조회한 1개당 가격(20개 구간)입니다. 재고와 가격은 주문할 때 다시 확인합니다.

## 1. 직접 사는 부품 (기판 조립에 포함되지 않음)

| 부품 | 모델 | 구매처 | 가격(참고) |
| --- | --- | --- | --- |
| 전자잉크 화면 | Good Display GDEY0426T82 (4.26", 800×480, SSD1677, 24핀 FPC) | [buyepaper.com](https://www.buyepaper.com/products/gdey0426t82), [buy-lcd.com](https://buy-lcd.com/products/gdey0426t82) (Good Display 공식 판매) | $15~17 + 배송 |
| 같은 화면(대체) | Waveshare 4.26inch e-Paper raw (SKU 26175) | [waveshare.com](https://www.waveshare.com/product/4.26inch-e-paper.htm) | $22.99 |
| 배터리 | 303450 LiPo 3.7 V 500 mAh (3.0 × 34 × 50 mm), KC 인증품 권장 | 국내 쇼핑몰(옥션 등, 약 15,000원) 또는 알리익스프레스 "303450 500mAh" | 약 5,000~15,000원 |
| microSD | 아무 microSDHC 8~32 GB | 국내 어디서나 | 약 5,000원 |
| 케이스 | PETG 3D 프린팅 (도면 미작성) | JLC3DP 또는 직접 출력 | 미정 |

## 2. 기판 조립 부품 (JLCPCB/LCSC, 기판 1장 기준)

| 참조 | 부품 | 값 | 풋프린트 | LCSC | 라이브러리 | 수량 | 단가 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| U1 | ESP32-C3 | ESP32-C3 | QFN-32-1EP_5x5mm_P0.5mm_EP3.7x3.7mm | [C2838500](https://jlcpcb.com/partdetail/C2838500) | 확장 | 1 | $1.6237 |
| U2 | W25Q128JVPIQ | W25Q128JVPIQ | WSON-8-1EP_6x5mm_P1.27mm_EP3.4x4.3mm | [C190862](https://jlcpcb.com/partdetail/C190862) | 확장 | 1 | $2.9434 |
| Y1 | X3S040000FA1H | 40MHz | Crystal_SMD_3225-4Pin_3.2x2.5mm | [C254336](https://jlcpcb.com/partdetail/C254336) | 확장 | 1 | $0.1541 |
| R1 R2 | 0402WGF0000TCE | 0R | R_0402_1005Metric | [C17168](https://jlcpcb.com/partdetail/C17168) | 기본 | 2 | $0.0025 |
| C1 C2 | 0402CG120J500NT | 12pF | C_0402_1005Metric | [C1547](https://jlcpcb.com/partdetail/C1547) | 기본 | 2 | $0.0036 |
| AE1 | RFANT3216120A5T | RFANT3216120A5T | FILTER-SMD_1206-2P-L3.2-W1.6-L | [C127629](https://jlcpcb.com/partdetail/C127629) | 확장 | 1 | $0.1171 |
| C5 C14 C19 C20 | CL10A106KP8NNNC | 10uF | C_0603_1608Metric | [C19702](https://jlcpcb.com/partdetail/C19702) | 기본 | 4 | $0.0319 |
| C6 C7 C8 C10 C11 C13 C15 C30 C31 | CL05B104KO5NNNC | 100nF | C_0402_1005Metric | [C1525](https://jlcpcb.com/partdetail/C1525) | 기본 | 9 | $0.0045 |
| C9 C12 | CL05A105KP5NNNC | 1uF | C_0402_1005Metric | [C14445](https://jlcpcb.com/partdetail/C14445) | 확장 | 2 | $0.0071 |
| R3 R4 R5 R12 R13 R14 R15 R16 R19 R22 | 0402WGF1002TCE | 10k | R_0402_1005Metric | [C25744](https://jlcpcb.com/partdetail/C25744) | 기본 | 10 | $0.0034 |
| U3 | DW01A | DW01A | SOT-23-6 | [C351410](https://jlcpcb.com/partdetail/C351410) | 확장 | 1 | $0.0418 |
| Q1 | FS8205A | FS8205A | SOT-23-6 | [C2830320](https://jlcpcb.com/partdetail/C2830320) | 확장 | 1 | $0.0496 |
| R6 | 0402WGF4700TCE | 470R | R_0402_1005Metric | [C25117](https://jlcpcb.com/partdetail/C25117) | 기본 | 1 | $0.0027 |
| R7 | 0402WGF2701TCE | 2.7k | R_0402_1005Metric | [C25885](https://jlcpcb.com/partdetail/C25885) | 확장 | 1 | $0.0023 |
| R8 R9 R18 | 0402WGF1004TCE | 1M | R_0402_1005Metric | [C26083](https://jlcpcb.com/partdetail/C26083) | 기본 | 3 | $0.0026 |
| U4 | MCP73831T-2ACI/OT | MCP73831-2 | SOT-23-5 | [C424093](https://jlcpcb.com/partdetail/C424093) | 확장 | 1 | $0.6323 |
| R10 | 0402WGF3901TCE | 3.9k | R_0402_1005Metric | [C51721](https://jlcpcb.com/partdetail/C51721) | 확장 | 1 | $0.0016 |
| C16 C17 | CL10A475KO8NNNC | 4.7uF | C_0603_1608Metric | [C19666](https://jlcpcb.com/partdetail/C19666) | 기본 | 2 | $0.0295 |
| D1 | KT-0603R | RED | LED_0603_1608Metric | [C2286](https://jlcpcb.com/partdetail/C2286) | 기본 | 1 | $0.0076 |
| R11 | 0402WGF1001TCE | 1k | R_0402_1005Metric | [C11702](https://jlcpcb.com/partdetail/C11702) | 기본 | 1 | $0.0018 |
| U5 | RT9080-33GJ5 | RT9080-33GJ5 | SOT-23-5 | [C841192](https://jlcpcb.com/partdetail/C841192) | 확장 | 1 | $0.1200 |
| C18 C27 C28 C29 | CL10A105KB8NNNC | 1uF | C_0603_1608Metric | [C15849](https://jlcpcb.com/partdetail/C15849) | 기본 | 4 | $0.0166 |
| J1 | DM3AT-SF-PEJM5 | microSD | microSD_HC_Hirose_DM3AT-SF-PEJM5 | [C114218](https://jlcpcb.com/partdetail/C114218) | 확장 | 1 | $1.1605 |
| Q2 | AO3401A | AO3401A | SOT-23 | [C15127](https://jlcpcb.com/partdetail/C15127) | 기본 | 1 | $0.0942 |
| J2 | FH12-24S-0.5SH(55) | FH12-24S-0.5SH | Hirose_FH12-24S-0.5SH_1x24-1MP_P0.50mm_Horizontal | [C202112](https://jlcpcb.com/partdetail/C202112) | 확장 | 1 | $0.4616 |
| L1 | FNR4012S470MT | 47uH | IND-SMD_L4.0-W4.0_FNR4012S | [C167794](https://jlcpcb.com/partdetail/C167794) | 확장 | 1 | $0.0608 |
| Q3 | SI1308EDL-T1-GE3 | Si1308EDL | SOT-323_SC-70 | [C469327](https://jlcpcb.com/partdetail/C469327) | 확장 | 1 | $0.2960 |
| R17 | 0603WAF220KT5E | 2.2R | R_0603_1608Metric | [C22939](https://jlcpcb.com/partdetail/C22939) | 기본 | 1 | $0.0033 |
| D2 D3 D4 | MBR0530 | MBR0530 | D_SOD-123 | [C77336](https://jlcpcb.com/partdetail/C77336) | 확장 | 3 | $0.0355 |
| C21 C22 C23 C24 C25 C26 | CL21A475KBQNNNE | 4.7uF | C_0805_2012Metric | [C98192](https://jlcpcb.com/partdetail/C98192) | 확장 | 6 | $0.0953 |
| SW1 SW2 SW3 | SKRPACE010 | PREV | KEY-SMD_4P-L4.2-W3.2-P2.20-LS4.6 | [C139797](https://jlcpcb.com/partdetail/C139797) | 확장 | 3 | $0.0633 |
| R20 | 0402WGF1502TCE | 15k | R_0402_1005Metric | [C25756](https://jlcpcb.com/partdetail/C25756) | 기본 | 1 | $0.0016 |
| R21 | 0402WGF6801TCE | 6.8k | R_0402_1005Metric | [C25917](https://jlcpcb.com/partdetail/C25917) | 확장 | 1 | $0.0015 |
| SW4 | EVQP7C01P | POWER | SW_SPST_EVQP7C | [C388883](https://jlcpcb.com/partdetail/C388883) | 확장 | 1 | $0.1853 |
| J3 | TYPE-C-31-M-12 | USB-C | USB_C_Receptacle_HRO_TYPE-C-31-M-12 | [C165948](https://jlcpcb.com/partdetail/C165948) | 확장 | 1 | $0.1857 |
| F1 | 0603WAF0000T5E | 0R | R_0603_1608Metric | [C21189](https://jlcpcb.com/partdetail/C21189) | 기본 | 1 | $0.0023 |
| R23 R24 | 0402WGF5101TCE | 5.1k | R_0402_1005Metric | [C25905](https://jlcpcb.com/partdetail/C25905) | 기본 | 2 | $0.0024 |
| U6 | USBLC6-2SC6 | USBLC6-2SC6 | SOT-23-6 | [C7519](https://jlcpcb.com/partdetail/C7519) | 확장 | 1 | $0.1766 |
| R25 R26 | 0402WGF1003TCE | 100k | R_0402_1005Metric | [C25741](https://jlcpcb.com/partdetail/C25741) | 기본 | 2 | $0.0024 |

### 독립 회로 검토 뒤 바뀐 부품 (2026-09-30)

| 참조 | 변경 | LCSC | 이유 |
| --- | --- | --- | --- |
| Q1 FS8205A | 핀 배치 수정(1=S1, 2·5=D, 3=S2, 4=G2, 6=G1) | [C2830320](https://jlcpcb.com/partdetail/C2830320) | 기존 배치에서는 보호 회로가 동작하지 않았음 |
| Y1 | 40 MHz ±10 ppm, 12 pF로 교체 | [C5380316](https://jlcpcb.com/partdetail/C5380316) | Espressif 권장 ±10 ppm |
| C3·C4 | 1.5 pF 실장 | [C1552](https://jlcpcb.com/partdetail/C1552) | 칩 쪽 RF 정합(C-L-C) |
| L2 (구 R2) | 2.7 nH | [C77108](https://jlcpcb.com/partdetail/C77108) | 칩 쪽 RF 정합 |
| L3 (추가) | 6.8 nH, 안테나 직렬 | [C77110](https://jlcpcb.com/partdetail/C77110) | 칩 안테나가 2.45 GHz에 맞도록 |
| C20 | 10 µF → 1 µF | [C15849](https://jlcpcb.com/partdetail/C15849) | SD 전원을 켤 때 3.3 V가 순간적으로 떨어지는 것 완화 |
| C32 (추가) | 4.7 µF, 승압 인덕터 입력 | [C19666](https://jlcpcb.com/partdetail/C19666) | 승압 회로 입력 안정 |

RF 정합 값은 시작점입니다. 첫 기판에서 Wi-Fi 거리를 보고 조정합니다. 위 표 2절의 가격 목록은 변경 전 목록이며, 합계 변동은 1장당 $0.2 이내입니다.

배터리 선은 TP1(+)·TP2(-)에 직접 납땜합니다.

## 3. 예상 비용 (5장 주문, 추정)

| 항목 | 금액 |
| --- | --- |
| 부품 (1장 $9.57 × 5) | 약 $48 |
| 확장 부품 장착비 (22종 × $3) | 약 $66 |
| 4층 0.8 mm 기판 5장 | 약 $10~20 |
| 조립 기본비 + 스텐실 | 약 $15 |
| 배송 (한국) | 약 $15~25 |
| 합계 | 약 $154~174 (화면·배터리 별도) |

확장 부품 장착비가 가장 큰 항목입니다. 수동 부품 몇 개(1 µF 0402, 2.7 kΩ, 3.9 kΩ, 6.8 kΩ, 4.7 µF 0805)를 기본 라이브러리 값으로 바꾸면 종류당 $3씩 줄어듭니다.
