## RC_yacht — ESP32 Wi‑Fi サーボコントローラ付き RC ヨット計画

3D プリンターで船体とセールを作り、ESP32 でラジコン風のヨットを動かすプロジェクト。リポジトリには以下が入ってる。

- `CAD_Data/` 3D データ一式（船体やパーツ）
- `Program_Data/` PlatformIO（ESP32/Arduino）プロジェクト。Wi‑Fi アクセスポイント＋ Web サーバでサーボを操作する実装

この README は、とくに `Program_Data/src/main.cpp` の実装をベースに、配線・ビルド・使い方をまとめた。

## できること

- ESP32 がスタンドアロンの Wi‑Fi アクセスポイント（AP）になる（デフォルト SSID: `WIFI` ／パスワードなし）
- ブラウザで `http://192.168.4.1` にアクセスすると、モダン UI のスライダーでサーボ角度を 0–180° で操作
- クイックボタン（左端/中央/右端）で即移動
- REST 風エンドポイントからも操作可能（後述）
- 起動時にサーボ自己テスト（0°→90°→180°）

## ハード構成と配線

必須部品（例）

- ESP32 DevKit（PlatformIO の `esp32dev` ボード想定）
- マイクロサーボ（SG90 等）
- 5V サーボ用電源（推奨：USB 5V or 2S バッテリ＋レギュレータ）
- 共通 GND 配線

配線（デフォルト設定）

- サーボ信号線 → ESP32 GPIO 18（`servoPin = 18`）
- サーボ電源（赤） → 外部 5V（ESP32 の 5V ピンでも動くことはあるが非推奨）
- サーボ GND（茶/黒） → 外部電源の GND
- ESP32 GND → 外部電源の GND（サーボ電源と必ず共通化）

注意点

- サーボは瞬間的に大電流を食う。ESP32 の 3.3V からは絶対に給電しない。外部 5V を使う。
- ジッターやリセットが出るときは、外部電源を強化し、GND 共通を確認。

## ソフト/ビルド環境（PlatformIO）

`Program_Data/platformio.ini` 抜粋：

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = madhephaestus/ESP32Servo@^3.0.5
monitor_speed = 115200
# 注意: 現状ファイルには "jupload_speed" と書かれている
# 正しくは ↓ のキー名（必要なら修正して使う）
# upload_speed = 115200
board_build.flash_mode = dio
board_build.f_flash = 40000000L
```

依存ライブラリ

- ESP32Servo（PWM/Servo 制御）

書き込み手順（VS Code + PlatformIO）

1. VS Code で `Program_Data/` を PlatformIO プロジェクトとして開く
2. ボードを PC に接続し、COM ポートを確認
3. 「PlatformIO: Upload」で書き込み
4. 「PlatformIO: Monitor」でシリアルを 115200bps で開く

## 使い方（Wi‑Fi と Web UI）

初期設定（`src/main.cpp` より）

- AP SSID: `WIFI`
- パスワード: なし（オープン）
- AP IP: `192.168.4.1`
- Web サーバ: ポート 80

手順

1. 書き込み後、ESP32 が再起動。シリアルに起動ログが出る
2. スマホ/PC の Wi‑Fi で SSID `WIFI` に接続（パスワード不要）
3. ブラウザで `http://192.168.4.1` を開く
4. 画面のスライダーやボタンでサーボを操作（初期角度は 90°）

UI からの操作に加えて、以下の HTTP エンドポイントでも制御できる：

- `GET /servo?angle=<0..180>` … 指定角へ移動（プレーンテキストで現在角度を返す）
- `GET /left` … 現在値から −30°
- `GET /right` … 現在値から +30°
- `GET /center` … 90° に移動

## コードの主な設定ポイント

`Program_Data/src/main.cpp` より：

- サーボピン: `const int servoPin = 18;`
- 初期角度: `int servoPosition = 90;`
- 角度範囲: 0–180°（`constrain` で保護）
- 起動時自己テスト: 0→90→180→ 初期角
- AP 設定: `WiFi.softAPConfig(local_IP, gateway, subnet); WiFi.softAP(ap_ssid);`

パスワードを付けたいなら（例）：

```cpp
const char* ap_ssid = "MyYacht";
const char* ap_password = "pass1234"; // 8文字以上
...
WiFi.softAPConfig(local_IP, gateway, subnet);
WiFi.softAP(ap_ssid, ap_password); // 第2引数を渡す
```

SG90 等のパルス幅を明示すると安定する場合がある：

```cpp
// 例: 500–2400µs（サーボにより適正値が違う）
myServo.attach(servoPin, 500, 2400);
```

## 注意: SSID 表示の不一致について

HTML の表示テキストとシリアルの案内には `ESP32_ServoController` という文言が出る箇所がある一方、実際の AP SSID はコード上 `WIFI` になっている。迷ったら Wi‑Fi リストで見える SSID（`WIFI`）を信じれば OK。

統一したければ、以下を同じ文字列に合わせる：

- `const char* ap_ssid = "...";`（実際の SSID）
- HTML 内の案内テキスト（`htmlPage` 文字列）
- シリアルログの文言

## よくあるトラブルと対処

- アップロードが失敗する / 遅い

  - `platformio.ini` の `upload_speed` を設定（現在ファイルには `jupload_speed` と書かれてるので修正推奨）
  - 正しい COM ポートを選択。必要なら BOOT ボタンを押しながら接続

- SSID が見つからない

  - 電源を入れ直す。チャンネル干渉があり得る。別の場所で試す
  - シリアルモニタでブートログを確認（115200bps）

- `http://192.168.4.1` に開けない

  - 端末が本当に `WIFI` に接続できているか確認（モバイルデータを切ると安定）
  - 端末のプロキシや VPN を切る

- サーボがガクガク / リセットする
  - 外部 5V 電源を使い、ESP32 と GND を共通化
  - `attach` のパルス幅を調整

## 将来やりたいこと（メモ）

- 2 系統サーボ（舵とセール）対応、UI のチャンネル追加
- ステッピングモーターとジャイロセンサーを使用しバランスを取れるようにする
- より簡単にアセンブリできるようにする

## フォルダ構成（抜粋）

- `CAD_Data/` … ヨットの 3D データ
- `Program_Data/` … ESP32/Arduino（PlatformIO）プロジェクト
- `Program_Data/src/main.cpp` … WebUI ＋サーボ制御本体
- `Program_Data/platformio.ini` … ビルド/ボード設定

— さあ、海風に乗せよう。コードはもう走る。あとは帆を張るだけだ。
