# Pocket E-Ink 4.26

폰 뒷면 크기(약 66 × 127 × 6 mm)의 4.26인치 전자잉크 리더입니다. 폰 앱이 책을 페이지 그림으로 그려서 보내고, 기기는 받은 페이지를 저장해 두고 버튼으로 넘깁니다. 이렇게 하면 기기 부품이 최소로 줄어듭니다(Rev B).

- 화면: Good Display GDEY0426T82 (800×480, 219 ppi, 부분 갱신 0.42초). 고장 난 Xteink X4에 든 화면과 같은 패널입니다.
- 기기: ESP32-C3-WROOM-02-N4 모듈, 4층 기판, 직접 납땜
- 전송: 블루투스(명령), Wi-Fi(페이지 묶음)
- 버튼: 앞면 ◀ · ● · ▶, 옆면 전원
- 배터리: 보호회로가 붙은 303450 500 mAh, USB-C 충전

## 폴더

| 경로 | 내용 |
| --- | --- |
| `docs/rev-b.md` | Rev B 명세와 블루투스·Wi-Fi 프로토콜 v1 (앱이 따라야 할 계약) |
| `docs/bom-rev-b.md` | Rev B 부품표, 구매처, 직접 납땜 비용 |
| `docs/ble-reader.md` | 펌웨어 빌드·업로드, `tools/send_book.py` 사용법 |
| `docs/panel-mechanics.md` | 화면 FPC 치수와 커넥터 방향 |
| `docs/bench.md` | 기판 주문 전 시험판(개발보드 + Waveshare HAT) |
| `hardware/gen/design.py` | 회로(부품·연결·배치)의 원본. 회로도와 기판은 여기서 생성됩니다 |
| `firmware/ble-reader/` | Rev B 기기 펌웨어 |
| `tools/send_book.py` | 텍스트 파일을 페이지로 그려서 기기로 보내는 도구 (앱의 기준 구현) |
| `firmware/patches/`, `docs/spec.md`, `docs/bom.md` | Rev A(CrossPoint로 EPUB 직접 읽기, JLC 조립). 태그 `rev-a-crosspoint` |

## 빌드

회로도·기판은 GitHub Actions의 `hardware` 작업이 만듭니다. 결과물(`hardware-out`)에는 KiCad 파일, 회로도 PDF, 거버 zip, BOM·CPL이 들어 있습니다.

```bash
cd firmware/ble-reader && pio run -t upload
```

## 라이선스

하드웨어(`hardware/`)는 CERN-OHL-W-2.0, 펌웨어와 도구는 MIT입니다.
