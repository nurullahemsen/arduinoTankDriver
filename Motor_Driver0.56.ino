/**
 * Bu kod Uzaktan Kumandalı Tank Projesi için tankın sürülmesine 
 * yöneliktir.
 * 
 * Kullanılan devre modülleri ve aksamlar:
 * 1 Adet Arduino UNO
 * 1 Adet L298N Motor Driver modül
 * 1 Adet HC-05 Bluetooth Modül (Slave)
 * 1 Adet oyuncak tank şasisi
 * Piller, Jumper kablolar
 * 
 * Bu kısımda tank şasisini sürecek arduino programlanmıştır.
 * Bu arduino uzaktan kumandadan komutları serial a bağlanmış
 * bluetooth modülü üzerinden alır. Sistem X ve Y değerlerini doğru 
 * bir şekilde alıp almadığını bir dizi hata kontrolü ile anlar.
 * Eğer değerler sıralı ve düzgünse tankın motorları bu değerlere 
 * göre sürülür.
 * 
 * Tank hızı joystick ile ayarlanabilmektedir.
 * X 180 ile 255 değeri arasındaysa yani joystick sağa yatırlımışsa
 * tank sağa, 75 ile 0 arasındaysa yani joystick sola yatırılmışsa
 * tank sola dönecektir. döndürücü etki nekadar fazlaysa yani sağ 
 * için 255 e nekadar yakınsa veya sol için 0 a ne kadar yakınsa 
 * dönüş hızı da okadar çok olacaktır. 75 ile 180 arasındaki değerlerin
 * donüş sağlamamasının nedeni bu değerlerde motora çok az güç gideceği 
 * için motorun dönememesi ve cihazın Y ekseninde gidiyor olabileceğidir.
 * Tank sağa dönerken sağ palet geri sol palet ileri hareket eder, sağa 
 * dönerken bunun tam tersi şekilde çalışır.
 * 
 * Y ekseni için 130 dan büyükse büyüklüğüne oranlı hızla ileri küçükse 
 * ise küçüklüğüne oranlı hızla geri gidecektir. Yine çok düşük değerler 
 * motora az güç göndereceği için görmezden gelinmektedir. 05.2019
 * 
 * 05160000657 Nurullah EMSEN
 * 05160000784 Elifnaz ÖKLÜ
 * 05160000283 Oğuzhan KATI
 */




#include <SoftwareSerial.h>
//CONSTANTS
#define RIGHT_MOTOR_IN1 7
#define RIGHT_MOTOR_IN2 8
#define RIGHT_MOTOR_EN 9

#define LEFT_MOTOR_IN1 4
#define LEFT_MOTOR_IN2 5
#define LEFT_MOTOR_EN 6


int ledStatus = 0;
SoftwareSerial mySerial(10, 11); //RX,TX

int Vy_R = 0; //RIGHT VELOCITY - SAĞ PALET HIZI
int Vy_L = 0; //LEFT VELOCITY - SOL PALET HIZI
int RIGHT = 1; //FORWARD - SAĞ PALET İLERİ
int LEFT = 1; //FORWARD - SOL PALET İLERİ

/**
 * Setup fonksiyonunda motoru sürdürecek pinlerin konfigürasyonu yapılır.
 * bluetooth iletişim göstergesi olan bultin led yakılır
 */

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(13, OUTPUT);
  
  pinMode(RIGHT_MOTOR_IN1, OUTPUT);
  pinMode(RIGHT_MOTOR_IN2, OUTPUT);
  pinMode(RIGHT_MOTOR_EN, OUTPUT);

  pinMode(LEFT_MOTOR_IN1, OUTPUT);
  pinMode(LEFT_MOTOR_IN2, OUTPUT);
  pinMode(LEFT_MOTOR_EN, OUTPUT);

  delay(4000);
  digitalWrite(13, HIGH);

}

/**
 * loop fonksiyonunda öncelikle bluetooth üzerinden X ve Y değerleri
 * alınır. Bu değerler sıralı ve doğru mu gelmiş değerlendirilir, hatalı
 * değerler ayıklanır. Sonra tank şasisine bağlı iki motor da girilen
 * girilen değerlere göre ayrı ayrı sürülür.
 */

void loop() {
  if (mySerial.available()) {
    if (mySerial.read() == '$') {
      delay(4);
      digitalWrite(13, ledStatus);
      ledStatus = !ledStatus;
      int x = mySerial.read();

      if (x == 131 || x == 36 || x == -1) return; // hatalı değerler
      delay(4);
      int y = mySerial.read();

      if (y == 127 || y == 36 || y == -1) return; // hatalı değerler
      if (y == 0 && x == 0) return;


      if(x > 180){ //TURN RIGHT
        RIGHT = 1; // sağ ileri
        LEFT = 0; // sol geri
        Vy_R = map(x,181,255,120,254); // hiz değeri map'lenir
        Vy_L = Vy_R; // aynı hız değeri sol motorun değişkenine de aktarılır
      }else if(x < 75){ //TURN LEFT
        RIGHT = 0; // sağ geri
        LEFT = 1; // sol ileri
        Vy_R = map(x,75,0,120,254);
        Vy_L = Vy_R;
      }else{
        
        // değerler nötr aralıktaysa yani motoru sürdüremeyecek kadar küçükse 
        //değer ne ile ne geriymiş gibi orta değer alınır.
        y = (y > 120 && y < 140) ? 131 : y; 
        if (y > 130) { //BOTH FORWARD
          Vy_R = map(y, 131, 255, 0, 254);
          RIGHT = 1;
          LEFT = 1;
        } else { //BOTH BACKWARD
          Vy_R = map(y, 0, 130, 254, 0);
          RIGHT = 0;
          LEFT = 0;
        }
        Vy_L = Vy_R;
      
        // VERY LOW PWM 
        // hesaplamalar sonucunda PWM dalgasi %15 ten aşağıysa motor çalıştırılmaz. 
        Vy_R = (Vy_R < 40) ? 0 : Vy_R;
        Vy_L = (Vy_L < 40) ? 0 : Vy_L;
      }
      if (RIGHT) { // sağ motor ileri ve geri gidiş konfigürasyonu
        digitalWrite(RIGHT_MOTOR_IN1, HIGH);
        digitalWrite(RIGHT_MOTOR_IN2, LOW);
      } else {
        digitalWrite(RIGHT_MOTOR_IN1, LOW);
        digitalWrite(RIGHT_MOTOR_IN2, HIGH);
      }

      if (LEFT) { // sol motor ileri ve geri gidiş konfigürasyonu
        digitalWrite(LEFT_MOTOR_IN1, HIGH);
        digitalWrite(LEFT_MOTOR_IN2, LOW);
      } else {
        digitalWrite(LEFT_MOTOR_IN1, LOW);
        digitalWrite(LEFT_MOTOR_IN2, HIGH);
      }

      // hız değerleri PMW olarak sağ ve sol motor enabler'lara yüklenir.
      // bu şekilde motorlar önceden girilmiş yön konfigurasyonlarına göre
      // hareket etmeye başlar
      analogWrite(RIGHT_MOTOR_EN, Vy_R);
      analogWrite(LEFT_MOTOR_EN, Vy_L);
    }
  }
}
