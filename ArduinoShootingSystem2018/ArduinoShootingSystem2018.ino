// delay 안쓰고 시작시간, 실행시간 차로 delay 하기
#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial Serial7Segment(7, 8); //RX pin, TX pin

//
//#define APOSTROPHE  5                            // ' 위치
//#define COLON       4                            // : 위치
//#define DECIMAL4    3                            // DECIMAL4 = 왼쪽에서 네번째 점
//#define DECIMAL3    2                            // DECIMAL3 = 왼쪽에서 세번째 점
//#define DECIMAL2    1                            // DECIMAL2 = 왼쪽에서 두번째 점
#define DECIMAL1    0                            // DECIMAL1 = 왼쪽에서 첫번째 점

// 진동감지센서
#define vib0  2   // 10점짜리(가장 큰)
#define vib1  3   // 30점짜리(중간)
#define vib2  4   // 50점짜리(가장작은)

// 서보모터
#define motor_control0  9
#define motor_control1  10
#define motor_control2  11
Servo servo[3];   // 0 = 10점짜리(가장 큰), 1 = 30점짜리(중간), 2 = 50점짜리(가장작은)

long millisTimer;
boolean colonOn = false;                          // colonOn = false(꺼짐), true(켜짐)

// 서보모터 타이머
int servo_before_timer[3], servo_current_timer[3], servo_lay[3];    // servo_lay[i] == 1(누움), 0(안누움)

char tempString[100]; //Used for sprintf

// 변수
int timer[100];
int total = 0;
long per[3];                // 0 = 큰 진동감지, 1 = 중간 진동감지, 2 = 작은 진동감지

void servo_move(int n);

void setup() {    
  int i=0;   
  
  // 7Segment        
  Serial7Segment.begin(9600);                     // Serial7Segment 초기화
  Serial7Segment.write('v');                      // 화면 초기화
  Serial7Segment.print("0822");                   // Send the hour and minutes to the display
  Serial7Segment.write(0x77);                     // Decimal, colon, apostrophe control command
  Serial7Segment.write(2 << DECIMAL1);            // Turns on colon, apostrophoe, and far-right decimal

  millisTimer = millis();                         // 아두이노 코드 실행 시작점으로부터의 경과시간(0으로 초기화됨.)

 // 진동감지센서
  pinMode(vib0, INPUT);
  pinMode(vib1, INPUT);
  pinMode(vib2, INPUT);
  
  // 서보모터
  servo[0].attach(motor_control0);  // 서보모터에 펄스 공급해줄 아두이노 보드 포트번호 라이브러리에 전달
  servo[1].attach(motor_control1);  // 서보모터에 펄스 공급해줄 아두이노 보드 포트번호 라이브러리에 전달
  servo[2].attach(motor_control2);  // 서보모터에 펄스 공급해줄 아두이노 보드 포트번호 라이브러리에 전달
  for(i=0;i<3;i++)
    servo[i].write(0);  // 0 = 서있음.(처음에 세워놓음)
  
  Serial.begin(9600);
  Serial.println("Game Start\n");
}

void loop() {
  int i=0;
  

  // 서보모터 시간 잼
  for(i=0;i<3;i++){
    servo_current_timer[i] = millis();
  }
  
  millisTimer = millis()/1000;           // 매번 루프 돌때마다 초 받아옴(시간은 알아서 흘러감)
  if(timer[millisTimer] == 0){           // 각 초마다 timer[] 자리 있고, 그 자리가 0이면(그시간이 흐르지 않음)      
    Serial.print("Time = ");
    Serial.println(millisTimer);
    Serial.println("=================================================");
    timer[millisTimer] = 1;       // 시간 흘렀다고 표시해주고, 빠져나감
  }
                  //  sprintf(tempString, "00%02d", (int)millisTimer); // 분,초 숫자 -> 문자열로 변환
  sprintf(tempString, "%04d", (int)total); // 분,초 숫자 -> 문자열로 변환
  Serial7Segment.print(tempString);                  // 이 문자열을 segment에 써줌.
   
  if(millisTimer > 100){  // 10초가 흐르면
      Serial.print("Total Score = ");
      Serial.println(total);
      Serial.println("game end");
      delay(1000);
      exit(1);
  }

  // 모터가 누워있는지 확인

  if(servo_lay[0] == 1 && servo_lay[1] == 1 && servo_lay[2] == 1){       // 모터가 누워있으면
    for(i=0;i<3;i++){
      servo[i].write(0);      // 그 모터를 세워준다
      servo_lay[i] = 0;          // 모터가 안누웠다고 표시해준다.
      delay(500);
    }
    delay(500);
   }

  per[0] = pulseIn(vib0, HIGH);                // 큰 진동감지
  // 진동감지
  if(per[0] > 0 && per[0] < 3000){     // 0번째 진동이 일정수준 이상이 되면(0번째는 감지가 너무잘됨. 따로설정.)
    if(servo_lay[0] == 0){          // servo_lay == 0 이면(서있는게 맞으면)
      servo_before_timer[0] = millis();
      servo_move(0);                // 함수 부르면 servo[0] 누움
      servo_lay[0] = 1;             // servo_lay[0] = 1 -> 누움.
    }
  }
  else{                             // 진동 감지된게 아니면 매 루프마다
    Serial.print("per[0](큰거) = ");    // 그 진동수 출력해준다.
    Serial.println(per[0]);
  }

  per[1] = pulseIn(vib1, HIGH);                // 중간 진동감지 
  if(per[1] > 0){
    if(servo_lay[1] == 0){          // servo_lay == 0 이면(서있는게 맞으면)
      servo_before_timer[1] = millis();
      servo_move(1);                // 함수 부르면 servo[0] 누움
      servo_lay[1] = 1;             // servo_lay[0] = 1 -> 누움.
    }
  }
  else{                             // 진동 감지된게 아니면 매 루프마다
    Serial.print("per[1](중간거) = ");
    Serial.println(per[1]);
  }

  per[2] = pulseIn(vib2, HIGH);                // 작은 진동감지
  if(per[2] > 0 && per[2] < 3000){
    if(servo_lay[2] == 0){          // servo_lay == 0 이면(서있는게 맞으면)
      servo_before_timer[2] = millis();
      servo_move(2);                // 함수 부르면 servo[0] 누움
      servo_lay[2] = 1;             // servo_lay[0] = 1 -> 누움.
    }
  }
  else{                             // 진동 감지된게 아니면 매 루프마다
    Serial.print("per[2](작은거) = ");    // 그 진동수 출력해준다.
    Serial.println(per[2]);
  }
  /*
  for(i=0; i<3; i++){
    if(i!=0 && per[i] > 40){           // 진동이 일정수준 이상이 되면(1, 2번이)
      if(servo_lay[i] == 0){           // servo_lay[i] == 0 이면(서있는게 맞으면)
        servo_before_timer[i] = millis(); 
        servo_move(i);              // 함수 부르면 i번째 servo 누움
        servo_lay[i] = 1;             // servo_lay[i] = 1 -> 누웠다고 표시
      }
    }
    else if(per[0] > 50000){         // 0번째 진동이 일정수준 이상이 되면(0번째는 감지가 너무잘됨. 따로설정.)
      if(servo_lay[0] == 0){          // servo_lay == 0 이면(서있는게 맞으면)
        servo_before_timer[0] = millis();
        servo_move(0);              // 함수 부르면 servo[0] 누움
        servo_lay[0] = 1;             // servo_lay[0] = 1 -> 누움.
      }
    }
    else{                             // 진동 감지된게 아니면 매 루프마다
      if(i==0)                        // 그 진동수 출력해준다.
        Serial.print("per[0](큰거) = ");
      else if(i==1)
        Serial.print("per[1](중간거) = ");
      else
        Serial.print("per[2](작은거) = ");
      Serial.println(per[i]);
    } 
  }
  */
  //delay(1000);
}
 
// 서보모터
void servo_move(int n){
 // int i;
 // servo.write(0); // 0 = 서있음
  
  switch(n){
    case 0:
      total += 10;
      Serial.print("per[0](큰거 맞음) = ");
      Serial.println(per[0]);
      Serial.println("total+10");
      break;
    case 1:
      total += 30;
      Serial.print("per[1](중간거 맞음) = ");
      Serial.println(per[1]);
      Serial.println("total+30");
      break;
    case 2:
      total += 50;
      Serial.print("per[2](작은거 맞음) = ");
      Serial.println(per[2]);
      Serial.println("total+50");
      break;
  }
  
  servo[n].write(60);  // 90 = 내려감

//  servo[n].write(0);
  
  Serial.print("total = ");
  Serial.println(total);
  
  delay(1000);
}
