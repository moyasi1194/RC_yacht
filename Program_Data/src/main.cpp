#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// Wi-Fi AP設定
const char* ap_ssid = "WIFI";      // アクセスポイント名
const char* ap_password = "";                       // パスワードなし（オープンAP）
IPAddress local_IP(192, 168, 4, 1);                // ESP32のIPアドレス
IPAddress gateway(192, 168, 4, 1);                 // ゲートウェイ
IPAddress subnet(255, 255, 255, 0);                // サブネットマスク

// Webサーバーのポート
WebServer server(80);

// サーボ設定
Servo myServo;
const int servoPin = 18;  // サーボモーターのPWMピン
int servoPosition = 90;   // サーボの初期位置（中央）

// HTMLページ
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>🚀 ESP32 モダン サーボコントローラー</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Hiragino Kaku Gothic ProN', 'Hiragino Sans', 'BIZ UDPGothic', 'Meiryo', sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        
        .container {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 40px;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
            max-width: 450px;
            width: 100%;
            text-align: center;
        }
        
        h1 {
            color: #333;
            margin-bottom: 8px;
            font-size: 24px;
            font-weight: 700;
        }
        
        h2 {
            color: #666;
            margin-bottom: 25px;
            font-size: 16px;
            font-weight: 400;
        }
        
        .info {
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
            padding: 15px;
            border-radius: 15px;
            margin-bottom: 30px;
            font-size: 14px;
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.3);
        }
        
        .angle-display {
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            color: white;
            padding: 20px;
            border-radius: 15px;
            margin-bottom: 30px;
            font-size: 28px;
            font-weight: bold;
            box-shadow: 0 10px 25px rgba(79, 172, 254, 0.3);
            transition: all 0.3s ease;
        }
        
        .slider-container {
            margin: 30px 0;
            position: relative;
        }
        
        .slider-label {
            color: #333;
            font-weight: 600;
            margin-bottom: 20px;
            font-size: 16px;
        }
        
        .custom-slider {
            -webkit-appearance: none;
            width: 100%;
            height: 8px;
            border-radius: 5px;
            background: linear-gradient(to right, #ff7eb3, #ff758c, #ff7eb3);
            outline: none;
            transition: all 0.3s ease;
            cursor: pointer;
        }
        
        .custom-slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 25px;
            height: 25px;
            border-radius: 50%;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            cursor: pointer;
            box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);
            transition: all 0.3s ease;
        }
        
        .custom-slider::-webkit-slider-thumb:hover {
            transform: scale(1.2);
            box-shadow: 0 6px 20px rgba(102, 126, 234, 0.6);
        }
        
        .custom-slider::-moz-range-thumb {
            width: 25px;
            height: 25px;
            border-radius: 50%;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            cursor: pointer;
            border: none;
            box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);
            transition: all 0.3s ease;
        }
        
        .slider-marks {
            display: flex;
            justify-content: space-between;
            margin-top: 10px;
            font-size: 12px;
            color: #666;
        }
        
        .quick-buttons {
            display: flex;
            gap: 10px;
            margin-top: 25px;
            justify-content: center;
        }
        
        .btn {
            padding: 12px 20px;
            border: none;
            border-radius: 12px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            color: white;
            min-width: 80px;
        }
        
        .btn-left {
            background: linear-gradient(135deg, #667eea, #764ba2);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.3);
        }
        
        .btn-center {
            background: linear-gradient(135deg, #4facfe, #00f2fe);
            box-shadow: 0 5px 15px rgba(79, 172, 254, 0.3);
        }
        
        .btn-right {
            background: linear-gradient(135deg, #43e97b, #38f9d7);
            box-shadow: 0 5px 15px rgba(67, 233, 123, 0.3);
        }
        
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(0, 0, 0, 0.2);
        }
        
        .btn:active {
            transform: translateY(0);
        }
        
        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.05); }
            100% { transform: scale(1); }
        }
        
        .angle-display.updating {
            animation: pulse 0.3s ease;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 ESP32 モダンコントローラー</h1>
        <h2>🤖 サーボモーター制御</h2>
        
        <div class="info">
            📡 Wi-Fi: ESP32_ServoController (オープン)<br>
            🌐 IP: 192.168.4.1
        </div>
        
        <div class="angle-display" id="angleDisplay">
            <div>現在の角度</div>
            <div style="font-size: 36px; margin-top: 5px;"><span id="position">90</span>°</div>
        </div>
        
        <div class="slider-container">
            <div class="slider-label">🎚️ サーボ位置調整</div>
            <input type="range" min="0" max="180" value="90" class="custom-slider" id="servoSlider" oninput="updateServo(this.value)">
            <div class="slider-marks">
                <span>0°</span>
                <span>45°</span>
                <span>90°</span>
                <span>135°</span>
                <span>180°</span>
            </div>
        </div>
        
        <div class="quick-buttons">
            <button class="btn btn-left" onclick="setAngle(0)">⬅️ 左端 (0°)</button>
            <button class="btn btn-center" onclick="setAngle(90)">🎯 中央 (90°)</button>
            <button class="btn btn-right" onclick="setAngle(180)">➡️ 右端 (180°)</button>
        </div>
    </div>

    <script>
        let isUpdating = false;
        
        function updateServo(angle) {
            if (isUpdating) return;
            isUpdating = true;
            
            const angleDisplay = document.getElementById('angleDisplay');
            const positionElement = document.getElementById('position');
            
            angleDisplay.classList.add('updating');
            
            fetch('/servo?angle=' + angle)
                .then(response => response.text())
                .then(data => {
                    positionElement.innerHTML = data;
                    setTimeout(() => {
                        angleDisplay.classList.remove('updating');
                        isUpdating = false;
                    }, 300);
                })
                .catch(error => {
                    console.error('エラー:', error);
                    isUpdating = false;
                    angleDisplay.classList.remove('updating');
                });
        }
        
        function setAngle(angle) {
            document.getElementById('servoSlider').value = angle;
            updateServo(angle);
        }
        
        // 初期化時にスライダーと表示を同期
        window.onload = function() {
            const slider = document.getElementById('servoSlider');
            document.getElementById('position').innerHTML = slider.value;
        }
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// スライダー用の新しいハンドラー
void handleServo() {
  if (server.hasArg("angle")) {
    int angle = server.arg("angle").toInt();
    
    // 角度を0-180度の範囲に制限
    angle = constrain(angle, 0, 180);
    
    servoPosition = angle;
    myServo.write(servoPosition);
    server.send(200, "text/plain", String(servoPosition));
    Serial.println("🎚️ スライダーでサーボ移動: " + String(servoPosition) + "度");
  } else {
    server.send(400, "text/plain", "角度パラメータが見つかりません");
  }
}

void handleLeft() {
  servoPosition = max(0, servoPosition - 30);  // 30度ずつ左に回転
  myServo.write(servoPosition);
  server.send(200, "text/plain", String(servoPosition));
  Serial.println("サーボを左に移動: " + String(servoPosition) + "度");
}

void handleRight() {
  servoPosition = min(180, servoPosition + 30);  // 30度ずつ右に回転
  myServo.write(servoPosition);
  server.send(200, "text/plain", String(servoPosition));
  Serial.println("サーボを右に移動: " + String(servoPosition) + "度");
}

void handleCenter() {
  servoPosition = 90;  // センターポジション
  myServo.write(servoPosition);
  server.send(200, "text/plain", String(servoPosition));
  Serial.println("サーボをセンターに移動: " + String(servoPosition) + "度");
}

void setup() {
  // 初期化遅延で安定化
  delay(1000);
  
  Serial.begin(115200);
  delay(100);
  Serial.println("ESP32 サーボコントローラー起動中...");
  
  // サーボモーターの設定
  myServo.attach(servoPin);
  delay(500);
  Serial.println("サーボモーター初期化完了");
  
  // サーボテスト: 左右に動かして動作確認
  Serial.println("サーボテスト開始...");
  myServo.write(0);    // 左端
  delay(1000);
  myServo.write(90);   // 中央
  delay(1000);
  myServo.write(180);  // 右端
  delay(1000);
  myServo.write(servoPosition);  // 初期位置に戻す
  delay(500);
  Serial.println("サーボテスト完了");
  
  // Wi-Fi APモード設定
  Serial.println("Wi-Fi APモードを設定中...");
  
  // APのIP設定を構成
  WiFi.softAPConfig(local_IP, gateway, subnet);
  
  // アクセスポイントを開始（パスワードなし）
  WiFi.softAP(ap_ssid);
  
  Serial.println("");
  Serial.println("Wi-Fi APモード開始成功！");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.println("Password: なし（オープンAP）");
  Serial.print("IPアドレス: ");
  Serial.println(WiFi.softAPIP());
  
  // Webサーバーのルート設定
  server.on("/", handleRoot);
  server.on("/servo", handleServo);        // 新しいスライダー用エンドポイント
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/center", handleCenter);
  
  // サーバー開始
  server.begin();
  Serial.println("Webサーバー開始！");
  Serial.println("================================");
  Serial.println("📱 接続方法:");
  Serial.println("1. スマホ/PCのWi-Fi設定を開く");
  Serial.println("2. 'ESP32_ServoController' を選択");
  Serial.println("3. パスワードなし（そのまま接続）");
  Serial.println("4. ブラウザで http://192.168.4.1 にアクセス");
  Serial.println("================================");
}

void loop() {
  server.handleClient();
}