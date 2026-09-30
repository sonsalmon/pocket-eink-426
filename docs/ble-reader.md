# Rev B 리더 빌드와 책 전송

## 펌웨어 빌드

Python 3.12와 PlatformIO 6.1.18을 사용한다.

```bash
python3.12 -m pip install platformio==6.1.18
cd firmware/ble-reader
pio run -e ble-reader
```

생성 파일은 `firmware/ble-reader/.pio/build/ble-reader/firmware.bin`에 있다.

## 기기에 올리기

ESP32-C3의 USB-C를 연결한 뒤 다음 명령을 실행한다.

```bash
cd firmware/ble-reader
pio run -e ble-reader -t upload
```

업로드 포트를 자동으로 찾지 못하면 `--upload-port /dev/tty...`를 덧붙인다.

## 책 미리 보기

도구 의존성을 설치하고 UTF-8 텍스트를 800×480 미리 보기와 raw deflate
페이지로 만든다.

```bash
python3.12 -m pip install -r tools/requirements.txt
python3.12 tools/send_book.py book.txt --book-id sample --dry-run /tmp/sample-pages
```

Noto Sans KR 또는 나눔 글꼴을 자동으로 찾지 못하면
`--font /경로/NotoSansKR-Regular.otf`를 지정한다.

## 블루투스로 보내기

기기의 전원 키를 눌러 깨운 뒤 실행한다.

```bash
python3.12 tools/send_book.py book.txt --book-id sample --title "샘플 책"
```

일부 페이지만 보낼 때는 `--window 20:170`처럼 포함 범위를 지정한다.
기기는 전체 책이 공간에 들어가면 전부 보존하고, 공간이 부족할 때만 현재
쪽 기준 뒤 20쪽·앞 150쪽의 최소 창을 유지한다.

## Wi-Fi로 빠르게 보내기

```bash
python3.12 tools/send_book.py book.txt --book-id sample --wifi
```

도구가 임시 Wi-Fi 이름과 비밀번호를 출력한다. 컴퓨터를 그 Wi-Fi에 연결하고
Enter를 누르면 HTTP로 페이지를 보낸다. 임시 Wi-Fi는 요청이 120초 동안 없으면
자동으로 꺼진다. 명령과 목록은 전송 중에도 블루투스 프로토콜 v1을 사용한다.
