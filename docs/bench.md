# 시험판 (기판 주문 전 확인)

기판을 주문하기 전에 파는 부품으로 화면·케이블·버튼·넘김 속도를 먼저 확인합니다. CrossPoint 전체는 16 MB 플래시가 필요해서 파는 ESP32-C3 개발보드(대부분 4 MB)에는 들어가지 않습니다. 그래서 시험판에서는 확인용 펌웨어(`firmware/bench`)를 돌리고, CrossPoint는 첫 기판에서 확인합니다.

## 살 것 (약 4~5만 원, 추정)

| 부품 | 모델 | 구매처 | 가격 |
| --- | --- | --- | --- |
| 화면 + 연결보드 | Waveshare 4.26inch e-Paper HAT (SKU 26376) | [waveshare.com](https://www.waveshare.com/4.26inch-e-paper-hat.htm), 국내 수입 쇼핑몰 | $31.99 |
| 개발보드 | ESP32-C3 SuperMini (USB-C) | 알리익스프레스, 국내 쇼핑몰 | 약 5,000~8,000원 |
| 버튼 사다리 | 택트 스위치 3개, 저항 10 kΩ·15 kΩ·6.8 kΩ 각 1개 | 국내 쇼핑몰 | 약 2,000원 |
| 기타 | 브레드보드, 점퍼선(암-수) | 국내 쇼핑몰 | 약 5,000원 |

화면 HAT에 붙어 있는 패널은 GDEY0426T82와 같은 800×480 SSD1677 패널입니다. HAT에서 패널 케이블을 조심히 빼면 케이블 길이·접점 면·1번 핀 위치를 직접 잴 수 있습니다.

## 배선 (진짜 기판과 같은 GPIO)

| HAT 핀 | 개발보드 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| DIN | GPIO10 |
| CLK | GPIO8 |
| CS | GPIO21 |
| DC | GPIO4 |
| RST | GPIO5 |
| BUSY | GPIO6 |
| PWR (있으면) | 3V3 |

버튼 사다리는 GPIO1 하나에 연결합니다.

1. GPIO1 → 10 kΩ → 3V3 (풀업)
2. ◀ 스위치: GPIO1 → 스위치 → 15 kΩ → GND
3. ● 스위치: GPIO1 → 스위치 → GND
4. ▶ 스위치: GPIO1 → 스위치 → 6.8 kΩ → GND

## 실행과 확인

1. GitHub Actions의 `firmware` 결과물 `bench-firmware`를 받거나, `cd firmware/bench && pio run -e bench -t upload`로 올립니다.
2. 화면에 "Pocket426 bench - page 1"이 뜨면 패널과 배선은 정상입니다.
3. ◀ / ▶: 페이지가 바뀌고 화면 아래에 부분 갱신 시간(ms)이 나옵니다. 목표는 약 420 ms입니다.
4. ● 짧게: 전체 새로고침 시간이 나옵니다.
5. ● 길게(0.65초): 10초 동안 버튼 전압이 시리얼 로그에 찍힙니다. ◀ 약 1980 mV, ▶ 약 1340 mV, ● 약 0 mV, 누르지 않으면 약 3300 mV가 정상입니다.
6. 패널 케이블을 재서 `hardware/gen/design.py`의 `FPC_OPENING_Y`와 J2 방향을 맞춥니다.
