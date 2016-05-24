//-----------------------------------------------
// Sample code provided by KRIDA Electronics
// https://www.facebook.com/krida.electronics/
//-----------------------------------------------

unsigned char AC_LOAD = 7;    // Output to Opto Triac pin
unsigned char dimming = 3;  // Dimming level (0-100)
unsigned char i;


void setup() {
  // put your setup code here, to run once:
  pinMode(AC_LOAD, OUTPUT);// Set AC Load pin as output
  attachInterrupt(1, zero_crosss_int, RISING);

  // Serial.begin(9600);

}

void zero_crosss_int()  // function to be fired at the zero crossing to dim the light
{
  // Firing angle calculation : 1 full 50Hz wave =1/50=20ms 
  // Every zerocrossing : (50Hz)-> 10ms (1/2 Cycle) For 60Hz (1/2 Cycle) => 8.33ms 
  // 10ms=10000us
  
  int dimtime = (100*dimming);    // For 60Hz =>65    
  delayMicroseconds(dimtime);    // Off cycle
  digitalWrite(AC_LOAD, HIGH);   // triac firing
  delayMicroseconds(10);         // triac On propogation delay (for 60Hz use 8.33)
  digitalWrite(AC_LOAD, LOW);    // triac Off
}



void loop() {
    
       
           
          // Serial.println(pulseIn(8, HIGH));
          
          for (i=5;i<85;i++)
          {
            dimming=i;
            delay(20);
          }
        
          for (i=85;i>5;i--)
          {
            dimming=i;
            delay(20);
          }
                     

}

