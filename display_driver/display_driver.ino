#include <Wire.h>

#define _BV(bit)   (1 << (bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) 
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit)) 

#define CATHODE_0_ON (sbi(PORTD,7))
#define CATHODE_0_OFF (cbi(PORTD,7))

#define CATHODE_1_ON (sbi(PORTD,6))
#define CATHODE_1_OFF (cbi(PORTD,6))

#define COLON_ON (sbi(PORTD,0))
#define COLON_OFF (cbi(PORTD,0))

boolean animation = false;
boolean colon = false;
boolean colon_animation = false;

volatile unsigned char active_cathode=0,temp_digit=0;
volatile unsigned char time[4]={0,0,0,0},can_change=0;
volatile unsigned char temp_time[4]={0,0,0,0},anim=0;

/*
0th bit (LSB): anode ID, 0 or 1
1st-2nd      : PORT ID
                PORTB: 0
                PORTC: 1
                PORTD: 2
3rd-5th      : pin number 0-7
6th-7th      : no meaning (0)
*/

volatile unsigned char segments[28] = {
   0b00011000, //first digit's A segment, ID: 0.
   0b00010001, //first digit's B segment, ID: 1.
   0b00101001, //2
   0b00100000, //3
   0b00100001, //4
   0b00000000, //5
   0b00011001, //6
   
   0b00010011, //second digit's A segment, ID: 7. and so on..
   0b00000011, //8
   0b00001011, //9
   0b00001010, //10
   0b00101000, //11
   0b00010010, //12
   0b00000010, //13
   
   0b00011010, //14
   0b00000000, //15
   0b00101100, //16
   0b00101101, //17
   0b00111001, //18
   0b00011011, //19
   0b00000001, //20
   
   0b00001101, //21
   0b00110001, //22
   0b00100101, //23
   0b00100100, //24
   0b00111000, //25
   0b00001100, //26
   0b00110000  //27
};

String chars=" 0123456789abcdefhilnopruy`'-_:";

volatile unsigned char number_look[30] = {
   0b0000000,   //empty
   0b0111111,   //0
   0b0000110,   //1
   0b1011011,   //2
   0b1001111,   //3
   0b1100110,   //4
   0b1101101,   //5
   0b1111101,   //6
   0b0000111,   //7
   0b1111111,   //8
   0b1101111,   //9
   0b1110111,   //a
   0b1111100,   //b
   0b0111001,   //c
   0b1011110,   //d
   0b1111001,   //e
   0b1110001,   //f
   0b1110110,   //h 
   0b0010000,   //i 
   0b0111000,   //l 
   0b1010100,   //n 
   0b1011100,   //o 
   0b1110011,   //p 
   0b1010000,   //r 
   0b0011100,   //u 
   0b1101110,   //y 
   0b0000010,   //`
   0b1100011,   //°, use ' character to display °
   0b1000000,   //- 
   0b0001000    //_ 
};

volatile unsigned char anim1[10] = {
   0b0000001,
   0b0100011,
   0b1100011,
   0b1110111,
   0b1111111
};

ISR (TIMER2_OVF_vect){
   unsigned char i=0,temp_segment=0,temp_number_look;

   PORTB&=0b00000010;//every anode and cathode off, mind the colon and leds
   PORTC&=0b00000000;
   PORTD&=0b00001101;

//   if(colon)
//      COLON_ON;
//   else
//      COLON_OFF;

   if(active_cathode==0)
   {
      active_cathode=1;
      CATHODE_1_ON;         //switching to the other cathode
   }
   else
   {
      active_cathode=0;
      CATHODE_0_ON;
   }

   while(temp_digit<4) //which digit we are dealing with (0-3)
   {
      while(i<7) //which segment of the digit (o-6 for a-g segments)
      {
//         if(!(time[temp_digit] >= 0) || !(time[temp_digit] <=17))
//         {
//            temp_number_look=number_look[16];            //if we got wrong number somehow
//         }
//         else
//         {
            temp_number_look=number_look[temp_time[temp_digit]]; //get the outlook of the number
//         }

         if(anim && anim<50) //fun part, when minute is changing, it shows an animation, by manipulating the original look of the digit
         {
            temp_number_look |= anim1[anim/10];
         }
         else if(anim && anim<100)
         {
            temp_number_look = ~anim1[(anim-50)/10];
         }
         else if(anim && anim<130)
         {
            temp_number_look = number_look[16];
         }

         if(temp_number_look & (1<<i))   //we masking out the segment from the number_look, then we got that: has it to be turned on or not (oh, what a sentence )
         {
            temp_segment=segments[(temp_digit*7)+(i)]; //selecting the segment from the segments array
            if((temp_segment & 1) == active_cathode)     //we masking out the last bit, which shows the segment's cathode
            {
               temp_segment>>=1;                    //shifting out that bit
               switch(temp_segment%4)               //mask the next two bit is for the PORT selection
               {
                  case 0:
                     sbi(PORTB,temp_segment>>2);   //the rest 3 valid bit is the pin number
                     break;
                  case 1:
                     sbi(PORTC,temp_segment>>2);
                     break;
                  case 2:
                     sbi(PORTD,temp_segment>>2);
                     break;
               }
            }
         }
         i++; //next segment please
      }
      i=0;    //let's go to the first segment of the next digit
      temp_digit++;
   }

   temp_digit=0;  //go back to the first digit

   if(anim) //increase the animation counter, if it's already on
   {
      anim++;
   }
   if(temp_time[3]!=time[3] && !anim && animation) //start the animation if it is currently off AND the minute has changed AND it is not because we adjusting the time right now
   {
      anim=1;
   }

   if(anim==130) anim=0; //and of the animatio; above you can see what is the meaning of this counter

   if(!anim)
   {
      for(unsigned char i=0;i<4;i++) temp_time[i]=time[i]; //if we are not in animation, we store the time for the next comparison
   }
}

volatile uint8_t tot_overflow;

ISR(TIMER1_OVF_vect){
  if(colon){
    if(colon_animation){
      tot_overflow++;
      if(tot_overflow >= 122){
        PORTD ^= 1 << PIND0;
        tot_overflow = 0;
      }
    }else{
      COLON_ON;
    }
  }else{
    COLON_OFF;
  }
  
}

void setup() {
  TIMSK2 |= 1<<TOIE2;
  
  TCCR1B |= 1 << CS11;
  TCNT1 = 0;
  TIMSK1 |= 1 << TOIE1;
  tot_overflow = 0;
  
  sei();

  DDRD=0b11111111;
  DDRC=0b11001111;
  DDRB=0b11111111;

  for(unsigned char i=0;i<4;i++) temp_time[i]=time[i];
  
  Wire.begin(9); 
  Wire.onReceive(receiveEvent); 
}

void receiveEvent(int bytes) {
  int action = Wire.read();

  switch(action){

    case 0:{ //new text
      int j=0;
      colon = false;
      while(Wire.available()){ 
        char a = Wire.read();
        if(j == 2 && a == 30){
          colon = true;
        }else{
          time[j] = a;
          j++;
        }
      }
      break;
    }
    
    
    case 1:{ //switch led
      char ledSettings = Wire.read();

      switch(ledSettings>>1){
        case 1:{
          if(ledSettings & 0x01) // is lbs 1 - new state
            sbi(PORTB,1);
          else
            cbi(PORTB,1);
          break;
        }
        case 2:{
          if(ledSettings & 0x01)
            sbi(PORTD,3);
          else
            cbi(PORTD,3);
          break;
        }
        case 3:{
          if(ledSettings & 0x01)
            sbi(PORTD,2);
          else
            cbi(PORTD,2);
          break;
        }
      }
      break;
    }
    case 2:{ //animation setting
      animation = Wire.read();
      break;
    }
    case 3:{ //colon blink setting
      colon_animation = Wire.read();
      break;
    }
  }
} 

void loop() {}
