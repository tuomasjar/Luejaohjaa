/****************************
 * 
 * Brokk Ohjausjärjestelmä
 * 
 * Tekijä: Tuomas Järvinen
 * 
 * Delete Finland Oy
 * 
 ****************************/

#include <Wire.h>
#include <EEPROM.h>

struct arvot{
  uint8_t high[12];
  uint8_t low[12];
};

#define baudRate 115200
#define I2C_ADDRESS       41    // I2C Address of USB Host
#include "ps4_i2c.h"            // Include ps4_i2c.h if using I2C 

uint8_t sylinteri[15];
uint8_t ramppi = 1;
unsigned long time; 
struct arvot hieno;
int eeAddr = 0;
uint8_t nopeus=0;
bool alavaunu=false;
bool startti=false;
bool highpower = false;
long sulkuaika;

void setup()  { 

   Wire.begin();
    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(3, OUTPUT); 
    pinMode(2, OUTPUT);

    pinMode(35, OUTPUT);
    pinMode(34, OUTPUT);
    pinMode(33, OUTPUT);
    pinMode(32, OUTPUT);
    pinMode(44, OUTPUT);
   Serial.begin(baudRate);
    EEPROM.get(eeAddr,hieno);

   digitalWrite(35, LOW); //Ylläpitovirta
   digitalWrite(34, LOW); //Kuolleen miehen kytkin
   digitalWrite(33, LOW); //LOW = Ylä/ HIGH=alavaunu
   digitalWrite(32, LOW); //1-/1&2-sylinteri
   analogWrite(44, LOW); //paineenlisäys venttiili
   sulkuaika=millis();
  }
  
void loop()  { 

  //Hienosäätö 
  if(Serial.available()>0)
  {
    char c = Serial.read();
    if(c=='r')
    {
      EEPROM.get(eeAddr,hieno);
      Serial.print("s");
      Serial.print(" ");
      for(int i=0;i<10;i++)
      {
        Serial.print(hieno.low[i]);
        Serial.print(" ");
      }
      for(int i=0;i<10;i++)  
      {
        Serial.print(hieno.high[i]);
        Serial.print(" ");
      }   
      Serial.println("");
    }
    else if(c=='s')
    {
      for(int i=0;i<10;i++)
      {
        hieno.low[i]=Serial.parseInt();
      }
      for(int i=0;i<10;i++)
      {
        hieno.high[i]=Serial.parseInt();
      }
      EEPROM.put(eeAddr,hieno);
      Serial.println("U");
    }
    else Serial.println("N");
  }
  get_ps4(); //Lue ohjaimen tilanne

  if(ps4_ok==1) //jos luku onnistuu
  {
        if(ps4.button_tpad)
        {
           digitalWrite(35, LOW); //Ylläpitovirta
           digitalWrite(34, LOW); //Kuolleen miehen kytkin
           digitalWrite(33, LOW); //LOW = Ylä/ HIGH=alavaunu
           digitalWrite(32, LOW); //1-/1&2-sylinteri
           analogWrite(44, LOW); 
           alavaunu=false;
           highpower=false;
           startti=false;
           for(int i=2;i<14;i++)
           {
            analogWrite(i,0);
           }
            for(int i=0;i<10;i++)
           {
             sylinteri[i]=hieno.low[i];
           }
           delay(5000);
        }
        if(ps4.button_l1)  //Kuolleenmiehenkytkin/ohjausventtiili auki
        {
          digitalWrite(34,HIGH);
          sulkuaika=millis();
        }
        else 
        {
          digitalWrite(34,LOW);
        }
        
        if(ps4.button_options && (millis()-time)>500 && !ps4.button_l1 && !startti) //Koneen käynnistys ei saa painaa l1-nappia
        {
          digitalWrite(35,HIGH); //ylläpitovirta päälle
          startti=true;
          time=millis();
        }
        if(ps4.button_options && (millis()-time)>500 && startti) // sammuttaa koneen
        {
            digitalWrite(35,LOW); //ylläpitovirta pois
            digitalWrite(34,LOW); //sammuttaa ohjausventtiilin
            startti=false;
            time=millis();
        }

        if(ps4.button_l1 && ps4.button_r1) //1-2 sylinterin venttiilin aukaisu
        {
          digitalWrite(32,HIGH);
        }
        else 
        {
          digitalWrite(32,LOW);
        }

        if(ps4.button_x && (millis()-time)>500) //vähennä nopeutta
        {
          if(nopeus>=100)nopeus=100;
          else nopeus+=10;
          time=millis();
          Serial.print("n ");
          Serial.print(nopeus);
          Serial.println("");
        }
        if(ps4.button_triangle && (millis()-time)>500) //nosta nopeutta
        {
          if(nopeus<=0)nopeus=0;
          else nopeus-=10;
          time=millis();
          Serial.print("n ");
          Serial.print(nopeus);
          Serial.println("");
        }

        if(ps4.button_square && (millis()-time)>500) //ylä tai alavaunu
        {
          if(alavaunu)
          {
            alavaunu=false;
            digitalWrite(33, LOW);
          }
          else
          {
            alavaunu=true;
            digitalWrite(33,HIGH);
          }
          time=millis();
        }
        /****************************************************************************************
         * 
         * Ylävaunun ohjaus
         *  
         *****************************************************************************************/
        if(!alavaunu)
        {

          /**************************************************************
           * 
           * Oikea joystick oikea-vasen
           * vaunun kääntö
           * 
           *************************************************************/
         if(ps4.button_l1 && ps4.r_joystick_x > 135) 
         {

            int haluttu0 = map(ps4.r_joystick_x, 128, 255, hieno.low[0], hieno.high[0]-nopeus);
            if(haluttu0 > sylinteri[0]) sylinteri[0]+=ramppi;
            if(haluttu0< sylinteri[0]) sylinteri[0]-= ramppi;
            analogWrite(9, sylinteri[0]);
         }
         else
         {
            analogWrite(9,0);
            sylinteri[0]=hieno.low[0];
         }
         
         if(ps4.button_l1 && ps4.r_joystick_x <120) 
         {
            
            int haluttu1 = map(ps4.r_joystick_x, 128, 0, hieno.low[0], hieno.high[0]-nopeus);
            if(haluttu1 > sylinteri[1]) sylinteri[1]+=ramppi;
            if(haluttu1< sylinteri[1]) sylinteri[1]-= ramppi;
            analogWrite(8, sylinteri[1]);
         }
         else
         {
            analogWrite(8,0);
            sylinteri[1]=hieno.low[0];
         }
         
         /****************************************************************
          * 
          * Vasen joystick alas-ylös
          * Kolmossylinteri
          * 
          ***************************************************************/
         if(ps4.button_l1 && ps4.l_joystick_y > 135)
         {
            
            int haluttu2 = map(ps4.l_joystick_y, 128, 255, hieno.low[1], hieno.high[1]-nopeus);
            if(haluttu2 > sylinteri[2]) sylinteri[2]+=ramppi;
            if(haluttu2< sylinteri[2]) sylinteri[2]-= ramppi;
            analogWrite(11, sylinteri[2]);
         }
         else
         {
            analogWrite(11,0);
            sylinteri[2]=hieno.low[1];
         }
         
         if(ps4.button_l1 && ps4.l_joystick_y <120)
         {
            
            int haluttu3 = map(ps4.l_joystick_y, 128, 0, hieno.low[1], hieno.high[1]-nopeus);
            if(haluttu3 > sylinteri[3]) sylinteri[3]+=ramppi;
            if(haluttu3< sylinteri[3]) sylinteri[3]-= ramppi;
            analogWrite(10, sylinteri[3]);
         }
          else
          {
            analogWrite(10,0);
            sylinteri[3]=hieno.low[1];
         }

         /********************************************************************
          * 
          * Vasen joystick oikea-vasen
          * Työkalukääntö
          * 
          *********************************************************************/
         
         if(ps4.button_l1 && ps4.l_joystick_x > 135) //vasen tatti oikealle
         {
            int haluttu4 = map(ps4.l_joystick_x, 128, 255, hieno.low[2], hieno.high[2]-nopeus);
            if(haluttu4 > sylinteri[4]) sylinteri[4]+=ramppi;
            if(haluttu4< sylinteri[4]) sylinteri[4]-= ramppi;
            analogWrite(12, sylinteri[4]);
         }
         else
         {
            analogWrite(12,0);
            sylinteri[4]=hieno.low[2];
         }
         
         if(ps4.button_l1 && ps4.l_joystick_x <120) //vasen tatti vasemmalle
         {
            int haluttu5 = map(ps4.l_joystick_x, 128, 0, hieno.low[2], hieno.high[2]-nopeus);
            if(haluttu5 > sylinteri[5]) sylinteri[5]+=ramppi;
            if(haluttu5< sylinteri[5]) sylinteri[5]-= ramppi;
            analogWrite(13, sylinteri[5]);
         }
          else
          {
            analogWrite(13,0);
            sylinteri[5]=hieno.low[2];
         }
          /*******************************************************************
           * Oikea joystick alas-ylös
           * 1- ja 2-sylinteri. 
           * Oikea takanappi aktivoi 1-2 venttiilin.
           * Muuten ohjaa 2-sylinteriä
           * 
           ******************************************************************/
         if(ps4.button_r1) //oikea takanappi
         {
         if(ps4.button_l1 && ps4.r_joystick_y > 135) 
         {
            int haluttu6 = map(ps4.r_joystick_y, 128, 255, hieno.low[3], hieno.high[3]-nopeus);
            if(haluttu6 > sylinteri[6]) sylinteri[6]+=ramppi;
            if(haluttu6< sylinteri[6]) sylinteri[6]-= ramppi;
            analogWrite(7, sylinteri[6]);
            highpower=true;
         }
         else
         {
            analogWrite(7,0);
            sylinteri[6]=hieno.low[3];
         }
         if(ps4.button_l1 && ps4.r_joystick_y <120) 
         {
            int haluttu7 = map(ps4.r_joystick_y, 128, 0, hieno.low[3], hieno.high[3]-nopeus);
             if(haluttu7 > sylinteri[7]) sylinteri[7]+=ramppi;
            if(haluttu7< sylinteri[7]) sylinteri[7]-= ramppi;
            analogWrite(6, sylinteri[7]);
            highpower=true;
         }
          else
          {
            analogWrite(6,0);
            sylinteri[7]=hieno.low[3];
         }
         }
         else //ei oikeaa takanappia
         {
         if(ps4.button_l1 && ps4.r_joystick_y > 135)
         {
            int haluttu6 = map(ps4.r_joystick_y, 128, 255, hieno.low[3], hieno.high[3]-nopeus);
            if(haluttu6 > sylinteri[6]) sylinteri[6]+=ramppi;
            if(haluttu6< sylinteri[6]) sylinteri[6]-= ramppi;
            analogWrite(6, sylinteri[6]);
            highpower=true;
         }
         else
         {
            analogWrite(6,0);
            sylinteri[6]=hieno.low[3];
         }
         if(ps4.button_l1 && ps4.r_joystick_y <120) 
         {
            int haluttu7 = map(ps4.r_joystick_y, 128, 0, hieno.low[3], hieno.high[3]-nopeus);
             if(haluttu7 > sylinteri[7]) sylinteri[7]+=ramppi;
            if(haluttu7< sylinteri[7]) sylinteri[7]-= ramppi;
            analogWrite(7, sylinteri[7]);
            highpower=true;
         }
          else
          {
            analogWrite(7,0);
            sylinteri[7]=hieno.low[3];
         }
         }
         /***************************************************************************
          * 
          * Liipasimet
          * Työkalujen käyttövoima
          * 
          ***************************************************************************/
          
         if(ps4.button_l1 && ps4.button_l2) //vasen alapainike
         {
            int haluttu8 = map(ps4.l2, 0, 255, hieno.low[4], hieno.high[4]-nopeus);
            if(haluttu8 > sylinteri[8]) sylinteri[8]+=ramppi;
            if(haluttu8< sylinteri[8]) sylinteri[8]-= ramppi;
            analogWrite(5, sylinteri[8]);
         }
         else
         {
            analogWrite(5,0);
            sylinteri[8]=hieno.low[4];
         }
         if(ps4.button_l1 && ps4.button_r2) //oikea alapainike
         {
            int haluttu9 = map(ps4.r2, 0, 255, hieno.low[4], hieno.high[4]-nopeus);
            if(haluttu9 > sylinteri[9]) sylinteri[9]+=ramppi;
            if(haluttu9< sylinteri[9]) sylinteri[9]-= ramppi;
            analogWrite(4, sylinteri[9]);
         }
         else
         {
            analogWrite(4,0);
            sylinteri[9]=hieno.low[4];
         }
        }

        
/****************************************************************************************************
 * 
 * Alavaunun ohjaus
 * 
 ****************************************************************************************************/
        
        if(alavaunu)
        {
          /***********************************************************************************
           *  
           *  Telat
           *  
           ***********************************************************************************/
            highpower=true;
          /**************************
           * 
           * Eteenpäin
           * 
           ***************************/
            
            if(ps4.l_joystick_y < 96)  
            {
              if(ps4.l_joystick_x > 160)
              {
                int kuusi = map(ps4.l_joystick_y, 128, 0, hieno.low[5],hieno.high[5]-nopeus);
                analogWrite(11,0);
                analogWrite(7,0);
                analogWrite(10,0);
                analogWrite(6,kuusi);
              }
              else if(ps4.l_joystick_x <96)
              {
                int kymppi = map(ps4.l_joystick_y, 128,255,hieno.low[5],hieno.high[5]-nopeus);
                analogWrite(11,0);
                analogWrite(7,0);
                analogWrite(10,kymppi);
                analogWrite(6,0);
              }
              else {
                int molemmat = map(ps4.l_joystick_y, 128,0,hieno.low[5],hieno.high[5]-nopeus);
                analogWrite(11,0);
                analogWrite(7,0);
                analogWrite(10,molemmat);
                analogWrite(6,molemmat);
              }
            }
            /*******************************************
             * 
             * Kääntö
             * 
             *******************************************/
              if(ps4.l_joystick_y > 96 && ps4.l_joystick_y < 160)
              {
                if(ps4.l_joystick_x < 96)
                {
                   int molemmat = map(ps4.l_joystick_x, 128, 0, hieno.low[5],hieno.high[5]-nopeus);
                   analogWrite(10,0);
                   analogWrite(7,0);
                   analogWrite(6, molemmat);
                   analogWrite(11, molemmat);
                }
                else if(ps4.l_joystick_x > 160)
                {
                  int molemmat = map(ps4.l_joystick_x, 128, 255, hieno.low[5],hieno.high[5]-nopeus);
                  analogWrite(11,0);
                  analogWrite(6,0);
                  analogWrite(10,molemmat);
                  analogWrite(7,molemmat);
                }
                else {
                analogWrite(11,0);
                analogWrite(10,0);
                analogWrite(7,0);
                analogWrite(6,0);
                }
              }
              /*******************************************
               * 
               * Taaksepäin
               * 
               ******************************************/
              if(ps4.l_joystick_y > 160)
              {
                 if(ps4.l_joystick_x > 160)
                 {
                  
                    int seiska = map(ps4.l_joystick_y,128,0,hieno.low[5],hieno.high[5]-nopeus);
                    analogWrite(10,0);
                    analogWrite(6,0);
                    analogWrite(11,0);
                    analogWrite(7,seiska);
                 }
                 else if(ps4.l_joystick_x < 96)
                  {
                  int ykstoist = map(ps4.l_joystick_y, 128,255,hieno.low[5],hieno.high[5]-nopeus);
                  analogWrite(10,0);
                    analogWrite(6,0);
                    analogWrite(11,ykstoist);
                    analogWrite(7,0);
                 }
                 else
                 {
                  int molemmat = map(ps4.l_joystick_y, 128,255,hieno.low[5],hieno.high[5]-nopeus);
                  analogWrite(6,0);
                  analogWrite(10,0);
                  analogWrite(7,molemmat);
                  analogWrite(11,molemmat);
                 }
              }
            
        /***********************************************************************************************
         * 
         * Tukijalat
         * 
         **********************************************************************************************/
                  
         if(ps4.button_l1 && ps4.r_joystick_x > 135) //oikea tatti oikealle
         {
            int haluttu4 = map(ps4.r_joystick_x, 128, 255, hieno.low[7], hieno.high[7]-nopeus);
            if(haluttu4 > sylinteri[4]) sylinteri[4]+=ramppi;
            if(haluttu4< sylinteri[4]) sylinteri[4]-= ramppi;
            analogWrite(9, sylinteri[4]);
            analogWrite(13,sylinteri[4]);
         }
         else
         {
            analogWrite(9,0);
            analogWrite(13,0);
            sylinteri[4]=hieno.low[7];
         }
         
         if(ps4.button_l1 && ps4.r_joystick_x <120) //oikea tatti vasemmalle
         {
            int haluttu5 = map(ps4.r_joystick_x, 128, 0, hieno.low[7], hieno.high[7]-nopeus);
            if(haluttu5 > sylinteri[5]) sylinteri[5]+=ramppi;
            if(haluttu5< sylinteri[5]) sylinteri[5]-= ramppi;
            analogWrite(8, sylinteri[5]);
            analogWrite(12,sylinteri[5]);
         }
          else
          {
            analogWrite(8,0);
            analogWrite(12,0);
            sylinteri[5]=hieno.low[7];
         }
        
        }
        /***************************************************************************************
         * 
         * Paineenlisäys venttiilin ohjaus
         *
         ***************************************************************************************/
         if(ps4.button_l1)
         {
           int pressure;
           if(highpower)pressure=100;
           else pressure=40;
           uint8_t paine[4]={0,0,0,0};
           int max=0;
           if(ps4.l_joystick_y >135)
           {
            paine[0]=map(ps4.l_joystick_y, 128,255,0,pressure); 
           }
           else if(ps4.l_joystick_y < 120) 
           {
            paine[0]=map(ps4.l_joystick_y, 128,0,0,pressure);
           }
           else paine[0]=0;
           
           if(ps4.l_joystick_x >135) {
            paine[1]=map(ps4.l_joystick_x, 128,255,0,pressure);
           }
           else if(ps4.l_joystick_x < 120) {
            paine[1]=map(ps4.l_joystick_x, 128,0,0,pressure);
           }
           else paine[1]=0;
           
           if(ps4.r_joystick_y >135) {
            paine[2]=map(ps4.r_joystick_y, 128,255,0,pressure);
           }
           else if(ps4.r_joystick_y < 120){
            paine[2]=map(ps4.r_joystick_y, 128,0,0,pressure);
           }
           else paine[2]=0;
           
           if(ps4.r_joystick_x >135) {
            paine[3]=map(ps4.r_joystick_x, 128,255,0,pressure);
           }
            else if(ps4.r_joystick_x < 120) {
            paine[3]=map(ps4.r_joystick_x, 128,0,0,pressure);
           }
           else paine[3]=0;

           for(int i = 0;i<4;i++)
           {
              if(paine[i]>max)
              {
                max=paine[i];
              }
           }
           analogWrite(44,max);
           highpower=false;
         }
         else
         {
          highpower=false;
          analogWrite(44,0);
         }
        
         ps4_ok=0;     
      }
      else {
        analogWrite(6, 0);
        analogWrite(7, 0);
        analogWrite(8, 0);
        analogWrite(9, 0);
        analogWrite(10, 0);
        analogWrite(11, 0);
        analogWrite(12, 0);
        analogWrite(13, 0);
        analogWrite(5, 0);  
        analogWrite(4,0);
        analogWrite(3,0);
        analogWrite(44,0);
        for(int i=0;i<10;i++)
        {
          sylinteri[i]=hieno.low[i];
        }
      }
      if(millis()-sulkuaika>60000) //Jos ei käytetä yli minuuttiin
      {
         analogWrite(6, 0);
        analogWrite(7, 0);
        analogWrite(8, 0);
        analogWrite(9, 0);
        analogWrite(10, 0);
        analogWrite(11, 0);
        analogWrite(12, 0);
        analogWrite(13, 0);
        analogWrite(5, 0);  
        analogWrite(4,0);
        analogWrite(3,0);
        analogWrite(44,0);
        for(int i=0;i<10;i++)
        {
          sylinteri[i]=hieno.low[i];
        }
        nopeus=0;
        alavaunu=false;
        highpower = false;        
      }

   }
