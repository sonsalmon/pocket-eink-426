# Pocket E-Ink 4.26

폰 뒷면 크기(약 66 × 127 × 6 mm)의 4.26인치 전자잉크 리더입니다. 기기가 EPUB을 직접 읽고, 버튼은 앞면 3개(◀ · ● · ▶)와 옆면 전원 1개입니다.

- 화면: Good Display GDEY0426T82 (800×480, 219 ppi, 부분 갱신 0.42초)
- 칩: ESP32-C3 + 16 MB 플래시 (Xteink X4와 같은 핀 배치)
- 펌웨어: [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader)에 이 기판용 보드 설정을 덧붙인 것
- 배터리: 500 mAh (303450), USB-C 충전, microSD

## 폴더

| 경로 | 내용 |
| --- | --- |
| `docs/spec.md` | 요구사항, 사양, 두께·배터리 계산, 결정 기록 |
| `docs/bom.md` | 부품표와 구매처 |
| `hardware/gen/design.py` | 회로(부품·연결·배치)의 원본. 회로도와 기판은 여기서 생성됩니다 |
| `hardware/gen/build_sch.py`, `build_pcb.py` | KiCad 회로도·기판 생성기 |
| `firmware/patches/` | CrossPoint·FreeInk SDK에 붙이는 보드 패치 |
| `.github/workflows/` | 회로 검사(ERC)·자동 배선·기판 검사(DRC)·거버, 펌웨어 빌드 |

## 빌드

회로도·기판은 GitHub Actions의 `hardware` 작업이 만듭니다. 결과물(`hardware-out`)에 KiCad 파일, 회로도 PDF, 거버 zip, JLCPCB용 BOM·CPL이 들어 있습니다.

펌웨어는 로컬에서도 빌드할 수 있습니다.

```bash
firmware/setup.sh
cd firmware/crosspoint-reader && pio run -e pocket426 -t upload
```

## 라이선스

하드웨어(`hardware/`)는 CERN-OHL-W-2.0, 펌웨어 패치(`firmware/`)는 MIT입니다.
