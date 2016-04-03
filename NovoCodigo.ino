#include <Wire.h>
#include <HMC5883L.h>

#define velocidadeMotorDireita 6 
#define ativadorMotorDireita 7
#define velocidadeMotorEsquerda 5
#define ativadorMotorEsquerda 12

HMC5883L compass;
//Variaveis da bussola
  float bussolaX = 0; 
  float bussolaY = 0; 
  float bussolaZ = 0;

 //controle de lado
 bool ladoDireito = false;
 bool ladoEsquerdo = false;
 bool ladoFrente = true;

 //Direção para cada lado.
 int direcaoDireita;
 int direcaoEsquerda;
 int direcaoFrente;
 
 //contador lado
 int contadorDireita = 0;
 int contadorEsquerda = 0;
 int contadorFrente = 0;
 
 //registro de volta ao meio
 int registroEsquerda;
 int registroDireita;

#define infraReceptor  17//infra-vermelho receptor.
#define infraEmissorEsquerda 13//infra-vermelho tranmissor da esquerda
#define infraEmissorDireita 8//infra-vermelho tranmissor da direita

int contadorPulsos;//contador de pulsos recebidos

  
//variaveis de controle de movimento.
#define frente 0
#define tras 1

int sensorLinhas[5];

void motor(int motorEsquerda,int velocidadeEsquerda, int motorDireita, int velocidadeDireita){
      digitalWrite(ativadorMotorEsquerda, motorEsquerda);
      analogWrite(velocidadeMotorEsquerda, velocidadeEsquerda);
      digitalWrite(ativadorMotorDireita, motorDireita);
      analogWrite(velocidadeMotorDireita, velocidadeDireita);
}

void valoresSensorLinhas(){// busca os valores de que cada porta dos sensores de linha retorna.
  int i;
  for(i=0;i<5;i++){
    sensorLinhas[i] = analogRead(i);
  }
}

void bussola(){
  MagnetometerRaw raw = compass.ReadRawAxis();// Retrive the raw values from the compass
  MagnetometerScaled scaled = compass.ReadScaledAxis();// Retrived the scaled values from the compass (scaled to the configured scale).
  float xHeading = atan2(scaled.YAxis, scaled.XAxis);
  float yHeading = atan2(scaled.ZAxis, scaled.XAxis);
  float zHeading = atan2(scaled.ZAxis, scaled.YAxis);
  if(xHeading < 0)    xHeading += 2*PI;
  if(xHeading > 2*PI)    xHeading -= 2*PI;
  if(yHeading < 0)    yHeading += 2*PI;
  if(yHeading > 2*PI)    yHeading -= 2*PI;
  if(zHeading < 0)    zHeading += 2*PI;
  if(zHeading > 2*PI)    zHeading -= 2*PI;
  bussolaX = xHeading * 180/M_PI; 
  bussolaY = yHeading * 180/M_PI; 
  bussolaZ = zHeading * 180/M_PI; 
  bussolaX = 0.0011*bussolaX*bussolaX + 0.5746*bussolaX + 17.011;
  Serial.print(bussolaX);
  Serial.print("    ");
  Serial.print(bussolaY);
  Serial.print("    ");
  Serial.print(bussolaZ);
  Serial.println(";");
  delay(10);
}

void E_Sensor40KHZ(void)//  left infrared LED send 40kHZ pulse
{
  int i;
  for(i=0;i<24;i++)
  {
    digitalWrite(infraEmissorEsquerda,LOW);
    delayMicroseconds(8);
    digitalWrite(infraEmissorEsquerda,HIGH);
    delayMicroseconds(8);
  }
}

void D_Sensor40KHZ(void)//right infrared LED send 40kHZ pulse
{
  int i;
  for(i=0;i<24;i++)
  {
    digitalWrite(infraEmissorDireita,LOW);//
    delayMicroseconds(8);
    digitalWrite(infraEmissorDireita,HIGH);
    delayMicroseconds(8);
  }
}

void pcint0_init(void)//interrupt initialize
{
  PCICR = 0X01;//enable pins in set 0 
  PCMSK0 = 0X01;//enable pin of number 0
}
ISR(PCINT0_vect)//Pin PB0 interrupt function
{
  contadorPulsos++;//count the pulse
}

void pulsosSensorProx(){
  char i;
  contadorPulsos = 0;
  for(i=0;i<20;i++)//send 20 pulse
  {
    E_Sensor40KHZ();
    D_Sensor40KHZ();
    delayMicroseconds(600);    
  }
}

void setup(){
  int i = 0;
  for(i=0;i<5;i++){
    pinMode(i,INPUT);
  }
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(12, OUTPUT);
  
  Wire.begin();// Start the I2C interface.
  compass = HMC5883L(); // Construct a new HMC5883 compass.
  compass.SetScale(1.3);
  compass.SetMeasurementMode(Measurement_Continuous);// Set the measurement mode to Continuous
  
  Serial.begin(9600);
  
  //Iniciar parador
  motor(frente,0,frente,0);
  delay(2000);
}

void loop(){
 
  if(contadorFrente < 10){
    bussola();
    pulsosSensorProx();
    motor(frente,50,frente,50);
    if(bussolaX>direcaoFrente){
      motor(frente,50,frente,70);
    }else if(bussolaX<direcaoFrente){
      motor(frente,70,frente,50);
    }
    if(sensorLinhas[2] < 500 && contadorPulsos < 20){
          contadorFrente++;
          delay(200);
    }
    
    if(sensorLinhas[2] < 500 && contadorPulsos > 20){ 
         if(ladoFrente){
            bussola();
            pulsosSensorProx();
            motor(tras,50,tras,50);
            delay(250);
            while(bussolaX > direcaoEsquerda){
              bussola();
              motor(tras,50,frente,50);
            } 
            motor(frente,50,frente,50);
            delay(500);
            ladoEsquerdo = true;
            ladoFrente = false;
            registroEsquerda = contadorFrente+2;
            contadorEsquerda++;
            while(bussolaX < direcaoFrente){
              bussola();
              motor(frente,50,tras,50);
            }
         }else if(ladoEsquerdo == true && registroEsquerda == contadorFrente){
             bussola();
             pulsosSensorProx();
             while(bussolaX > direcaoDireita){
               bussola();
               motor(frente,50,tras,50);
             }
             motor(frente,50,frente,50);
             delay(contadorEsquerda*500);
             registroEsquerda = 0;
             contadorEsquerda = 0;
         }
    }
   
    }
}

