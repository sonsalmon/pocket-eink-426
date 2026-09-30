# GDEY0426T82 패널 기계·FPC 실장 조사

PCB 뒤쪽에 Hirose `FH12-24S-0.5SH`를 배치하기 위한 치수와 방향을 정리했다. 기준 도면은 Good Display의 **GDEY0426T82 Rev.03 (2023-05-16)** 이다.

## 출처

| ID | 원자료 |
|---|---|
| S1 | Good Display 최신 GDEY 자료 페이지: <https://www.good-display.com/companyfile/2000.html> |
| S2 | Good Display 최신 GDEY PDF: <https://v1.cecdn.yun300.cn/100001_1909185147/GDEY0426T82.pdf> |
| S3 | Good Display 구형 GDEQ Rev.01 PDF: <https://www.laskakit.cz/user/related_files/gdeq0426t82.pdf> |
| S4 | Waveshare raw panel, SKU 26175: <https://www.waveshare.com/4.26inch-e-paper.htm> |
| S5 | Waveshare 4.26inch e-Paper HAT Wiki: <https://www.waveshare.com/wiki/4.26inch_e-Paper_HAT_Manual> |
| S6 | Waveshare HAT 회로도: <https://files.waveshare.com/wiki/4.26inch-e-Paper-HAT/4.26inch_e-Paper_HAT.pdf> |
| S7 | Hirose FH12-24S-0.5SH 제품 페이지: <https://www.hirose.com/product/p/CL0586-0521-0-55> |
| S8 | Hirose FH12 시리즈 카탈로그: <https://www.hirose.com/en/product/document?clcode=CL0586-0521-0-55&documentid=D31648_en&documenttype=Catalog&lang=en&productname=FH12-24S-0.5SH%2855%29&series=FH12> |
| S9 | IS7V4N PCB: <https://github.com/IS7V4N/ESP32_E-Reader/blob/main/Hardware/PCB/ESP32_C3_EReader.kicad_pcb> |
| S10 | IS7V4N E-Ink 심볼: <https://github.com/IS7V4N/ESP32_E-Reader/blob/main/Hardware/PCB/Eink.kicad_sym> |

## 결론 요약

패널을 앞에서 보고, 도면처럼 `480 × 800` 세로 방향으로 두며 FPC가 아래를 향한다고 정의한다.

| 항목 | 값 | 출처 |
|---|---:|---|
| 유리/TFT 외형 | `105.33 ±0.1 × 62.37 ±0.1 × 0.926 ±0.1 mm` | S2 p.5 |
| 활성영역 | `92.8 × 55.68 mm` | S2 pp.4-5 |
| FPC 출구 | `62.37 mm` 짧은 변의 아래쪽 | S2 p.5 |
| 유리 밖 돌출 길이 | `24 ±0.3 mm` | S2 p.5 |
| 커넥터 삽입부 폭 | `12.5 ±0.1 mm` | S2 p.5 |
| 삽입부 중심 | 유리 왼쪽에서 `31.10 mm` (**추정**: `24.85 + 12.5/2`) | S2 p.5 |
| 유리 중심 대비 삽입부 중심 | 왼쪽으로 `0.085 mm` (**추정**: `31.10 - 62.37/2`) | S2 p.5 |
| 접점 면 | 앞면, 즉 표시 면과 같은 면 | S2 p.5 앞면도 |
| 핀 1 | 앞에서 보고 FPC가 아래일 때 왼쪽 | S2 p.5 |
| FPC 끝 총 두께 | `0.30 ±0.03 mm` (`FPC+PI`) | S2 p.5 |

Waveshare는 `129.33 × 62.37 × 0.926 mm`를 “screen outline”으로 적는다. `129.33 mm`는 유리 `105.33 mm`에 FPC 돌출 `24 mm`를 더한 전체 외곽이다. PCB 유리 포켓에는 `105.33 mm`를 써야 한다. [S2 p.5, S4, S5]

## 1. 유리와 활성영역

| 방향 | 도면값 | 반대쪽 여백 | 출처 |
|---|---:|---:|---|
| 짧은 축 `62.37 mm` | 활성영역 `55.68 mm`, 오른쪽 여백 `3.35 mm` | 왼쪽 `3.34 mm` (**추정**: `62.37-55.68-3.35`) | S2 p.5 |
| 긴 축 `105.33 mm` | 활성영역 `92.8 mm`, FPC 반대쪽 여백 `2.74 mm` | FPC 쪽 `9.79 mm` (**추정**: `105.33-92.8-2.74`) | S2 p.5 |

활성영역은 짧은 축에서는 사실상 중앙이고, 긴 축에서는 FPC 반대쪽으로 치우쳐 있다. [S2 p.5]

## 2. FPC 꼬리 형상

| 항목 | GDEY Rev.03 | GDEQ Rev.01 | 출처 |
|---|---:|---:|---|
| 유리 밖 돌출 | `24 ±0.3 mm` | `24 ±0.3 mm` | S2 p.5, S3 p.5 |
| 커넥터 삽입부 폭 | `12.5 ±0.1 mm` | `12.5 ±0.1 mm` | S2 p.5, S3 p.5 |
| 삽입부 왼쪽 위치 | 유리 왼쪽에서 `24.85 ±0.3 mm` | 동일 | S2 p.5, S3 p.5 |
| 유리 근처 넓은 부분 | 왼쪽 `17.84 ±0.3 mm`, 폭 `23.5 ±0.2 mm` | 왼쪽 `15.45 ±0.3 mm`, 폭 `31.3 ±0.2 mm` | S2 p.5, S3 p.5 |
| 접점 길이 | `3.55 ±0.3 mm` | `3.55 ±0.3 mm` | S2 p.5, S3 p.5 |
| PI 보강부 길이 | `4 ±0.3 mm` | `6 ±0.3 mm` | S2 p.5, S3 p.5 |

주의:

- GDEY와 GDEQ의 **넓은 FPC 부분 형상은 같지 않다**. PCB 외곽 간섭 검토에는 실제 구매 로트 모델을 확인해야 한다. [S2 p.5, S3 p.5]
- 두 도면 모두 커넥터 삽입부 폭·위치·피치는 같다. FH12 배치는 같은 중심을 쓸 수 있다. [S2 p.5, S3 p.5]
- 도면은 유리 접합부 근처를 `Bending Area`라고만 표시한다. **FPC-유리 ACF 접합 길이는 별도 수치가 없다.** 따라서 접합 길이를 `3.15 mm` 등으로 해석하지 않는다. [S2 p.5]

## 3. 접점과 FH12 방향

| 항목 | 값/판정 | 출처 |
|---|---|---|
| 핀 수·피치 | 24핀, `0.5 mm` | S2 p.5, S4, S7 |
| 첫 핀-끝 핀 중심 거리 | `11.5 ±0.05 mm` | S2 p.5 |
| 접점 폭 | `0.35 ±0.03 mm` | S2 p.5 |
| FPC 끝 총 두께 | `0.30 ±0.03 mm` (`FPC+PI`) | S2 p.5 |
| FH12 허용 FPC 두께 | `0.30 ±0.05 mm` | S7, S8 |
| FH12 접촉 방식 | `FH12-24S-0.5SH`는 bottom-contact | S7, S8 |
| 앞면 기준 핀 1 | 꼬리를 아래로 두면 왼쪽 | S2 p.5 |
| 뒤쪽 PCB 기준 핀 1 | 오른쪽으로 보임 (**추정**: 패널을 뒤집어 보는 순간 좌우가 반전됨) | S2 p.5, S9 |

도면의 `0.30 ±0.03 mm`는 **보강판만의 두께가 아니라 FPC+PI 합계**다. FH12의 `0.30 ±0.05 mm` 요구와 맞는다. [S2 p.5, S8]

## 4. 핀 배정

Good Display의 최신 GDEY Rev.03 본문 표와 기계 도면은 핀 6·7을 `NC`로 적는다. 하지만 같은 PDF의 기준회로는 핀 6·7을 `TSCL`, `TSDA`라고 적는다. 따라서 요청에 제시된 표는 **그대로 확정할 수 없다**. IS7V4N 설계도 핀 6·7을 NC로 처리한다. [S2 pp.5-6, p.11, S10]

| Pin | 확정 이름 | 비고 | 출처 |
|---:|---|---|---|
| 1 | NC |  | S2 pp.5-6 |
| 2 | GDR |  | S2 pp.5-6 |
| 3 | RESE |  | S2 pp.5-6 |
| 4 | NC |  | S2 pp.5-6 |
| 5 | VSH2 |  | S2 pp.5-6 |
| 6 | NC | 기준회로에서만 `TSCL` | S2 pp.5-6, p.11 |
| 7 | NC | 기준회로에서만 `TSDA` | S2 pp.5-6, p.11 |
| 8 | BS1 | 기준회로 표기는 `BS` | S2 pp.5-6, p.11 |
| 9 | BUSY |  | S2 pp.5-6 |
| 10 | RES# |  | S2 pp.5-6 |
| 11 | D/C# |  | S2 pp.5-6 |
| 12 | CS# |  | S2 pp.5-6 |
| 13 | SCL | 기준회로 표기는 `SCLK` | S2 pp.5-6, p.11 |
| 14 | SDA | 기준회로 표기는 `SDI` | S2 pp.5-6, p.11 |
| 15 | VDDIO |  | S2 pp.5-6 |
| 16 | VCI |  | S2 pp.5-6 |
| 17 | VSS |  | S2 pp.5-6 |
| 18 | VDD |  | S2 pp.5-6 |
| 19 | VPP |  | S2 pp.5-6 |
| 20 | VSH1 |  | S2 pp.5-6 |
| 21 | VGH |  | S2 pp.5-6 |
| 22 | VSL |  | S2 pp.5-6 |
| 23 | VGL |  | S2 pp.5-6 |
| 24 | VCOM |  | S2 pp.5-6 |

PCB에서는 핀 6·7을 연결하지 않는 보수적 해석이 제조사 표와 IS7V4N 실장례에 맞는다. [S2 pp.5-6, S9, S10]

## 5. Good Display 권장 기준회로

아래 값은 패널 사양서의 회로다. Waveshare HAT 회로값과 섞지 않는다. [S2 p.11]

| 기능 | 부품·값 | 출처 |
|---|---|---|
| 승압 인덕터 | L1 `47 µH`, `500 mA`; 부품 조건 표는 NR3015 참조, `Io=500 mA max` | S2 p.11 |
| VSH2 안정화 | C2 `4.7 µF / 25 V` | S2 p.11 |
| VGH/PREVGH 안정화 | C5 `4.7 µF / 25 V` | S2 p.11 |
| VSH1 안정화 | C9 `4.7 µF / 25 V` | S2 p.11 |
| VSL 안정화 | C10 `4.7 µF / 25 V` | S2 p.11 |
| VGL 안정화 | C11 `4.7 µF / 25 V` | S2 p.11 |
| VCOM 안정화 | C12 `1 µF / 25 V` | S2 p.11 |
| VCI·VDD 바이패스 | C6, C7 각각 `1 µF / 25 V` | S2 p.11 |
| 입력·차지펌프 | C3, C4 각각 `4.7 µF / 25 V` | S2 p.11 |
| 전류 감지 | R2 `0.22 Ω`, `1%`, `≥0.05 W` | S2 p.11 |
| GDR 풀다운 | R1 `1 MΩ`, `1%`, `≥0.05 W` | S2 p.11 |
| MOSFET | Q1 `Si1308EDL` | S2 p.11 |
| 쇼트키 다이오드 | D1-D3 `MBR0530`, `VR≥30 V`, `Io≥500 mA`, `Vf≤430 mV` | S2 p.11 |
| 수동소자 조건 | C1-C12: 0603/0805, X5R/X7R, 정격 `≥25 V` | S2 p.11 |

대조용 Waveshare HAT 회로는 L1 `10 µH`, R1 감지저항 `3 Ω`, VGH/VGL/VSH/VSL/VCOM 계열에 주로 `1 µF / 50 V`를 사용한다. 이는 별도 드라이버 보드 설계이며 Good Display p.11의 패널 기준회로와 값이 다르다. [S6]

## 6. IS7V4N 설계의 미러링

IS7V4N PCB의 J2는 실제로 다음 풋프린트를 쓴다. [S9]

```text
Connector_FFC-FPC:
Mirrored_Hirose_FH12-24S-0.5SH_1x24-1MP_P0.50mm_Horizontal
```

| 항목 | IS7V4N 값 | 출처 |
|---|---|---|
| PCB 면 | `F.Cu` | S9 |
| 위치·회전 | `(180.5, 115.5576)`, 회전 `0°` | S9 |
| 패드 1 | 로컬 X `+5.75 mm` | S9 |
| 패드 24 | 로컬 X `-5.75 mm` | S9 |
| 심볼의 핀 6·7 | NC | S10 |

**판정: 미러링한다.** 풋프린트 이름 자체가 `Mirrored_...`이고 패드 1→24가 `+X`에서 `-X`로 진행한다. 이유는 **추정**이지만 기계 도면과 정확히 맞는다. 패널 앞면에서는 핀 1이 왼쪽이고 접점도 앞면에 있다. FPC를 180° 접어 패널 뒤 PCB로 보낸 뒤 PCB 뒤쪽 시점에서 보면 좌우가 반전되어 핀 1이 오른쪽에 온다. 미러 풋프린트는 전기 핀 번호를 바꾸는 것이 아니라 이 물리적 좌우 반전을 보정한다. [S2 p.5, S9]

## PCB 배치 기준

1. 패널 앞면 기준 좌표를 잡고, FPC 삽입부 중심을 유리 왼쪽에서 `31.10 mm`에 둔다. [S2 p.5]
2. PCB를 패널 뒤에서 보는 레이아웃에는 핀 1이 오른쪽인 미러 풋프린트를 쓴다. [S2 p.5, S9]
3. 커넥터는 24핀, 0.5 mm 피치, bottom-contact, 0.30 mm FPC용 `FH12-24S-0.5SH`를 쓴다. [S7, S8]
4. FPC 넓은 부분의 간섭 여유는 실제 로트가 GDEY인지 GDEQ인지 확인한 뒤 결정한다. [S2 p.5, S3 p.5]
