/*
  USB Host PS4 Dualshock Controller
  HEX interface 
*/

#define PS4_DATA 20  // 20 chars sent - PS4(3) + Data(14) + CRC(1) + CR/LF(2)

unsigned char crc_val;
unsigned char tdata[PS4_DATA];
unsigned char str_start = 0;
unsigned int  ps4_i;
unsigned char ps4_ok = 0;  // valid data received

// data storage structure to store incomming PS4 values
// This is an expanded structure to make it easier to access the data
struct {
  unsigned char  l_joystick_x;
  unsigned char  l_joystick_y;
  unsigned char  r_joystick_x;
  unsigned char  r_joystick_y;
  unsigned char  accel_x;
  unsigned char  accel_y; 
  unsigned char  l2; 
  unsigned char  r2;   
  
  unsigned char  button_left;
  unsigned char  button_down;
  unsigned char  button_right;
  unsigned char  button_up;  
  unsigned char  button_square;
  unsigned char  button_x;
  unsigned char  button_circle;
  unsigned char  button_triangle;   
  
  unsigned char  button_l1;
  unsigned char  button_r1;
  unsigned char  button_l2;
  unsigned char  button_r2;  
  unsigned char  button_share;  
  unsigned char  button_options;
  unsigned char  button_l3;
  unsigned char  button_r3;
  
  unsigned char  button_ps4;  
  unsigned char  button_tpad;    
   
  unsigned char  tpad_x;  
  unsigned char  tpad_y;    
  unsigned char  battery;
} ps4;

// Function Prototypes
unsigned char crc8(unsigned char *data, unsigned char len);
void get_ps4(void);

// Get PS4 Hex Serial Data and Decode
void get_ps4(void){
  // if there's any serial available, read it:
  while (Serial.available() > 0) {
     byte c = Serial.read();  //gets one byte from serial buffer
     // Looking for start of input string CR LF P S 3 
     if(str_start<3){
        switch(c){
           case 'P':
              str_start++; 
              break;
           case 'S':
              if(str_start==1){
                 str_start++; 
              }  
              break;
           case '4':
              if(str_start==2){
                 str_start++; 
                 tdata[0]='P';
                 tdata[1]='S';
                 tdata[2]='4';
                 ps4_i=3;
              }  
              break;  
           default:
              str_start=0;     
              break;
        }           
     }
     else{
        // Add value to tdata
        tdata[ps4_i++] = c;
        if(ps4_i>=PS4_DATA) {
          // Complete string received - process data
          str_start=0;  
          // Calculate check digit
          crc_val=crc8(tdata,PS4_DATA-3);
          if(crc_val==tdata[PS4_DATA-3]){
             // CRC Match - Data Valid
             // Set values
             ps4.l_joystick_x=tdata[3];
             ps4.l_joystick_y=tdata[4];
             ps4.r_joystick_x=tdata[5];
             ps4.r_joystick_y=tdata[6];
             ps4.accel_x=tdata[7];
             ps4.accel_y=tdata[8]; 
             ps4.l2=tdata[9]; 
             ps4.r2=tdata[10]; 
             
             if(((tdata[11]&0x0F)==5) || ((tdata[11]&0x0F)==6) || ((tdata[11]&0x0F)==7)) ps4.button_left=1;  else ps4.button_left=0;	//W
             if(((tdata[11]&0x0F)==3) || ((tdata[11]&0x0F)==4) || ((tdata[11]&0x0F)==5)) ps4.button_down=1;  else ps4.button_down=0;	//S
             if(((tdata[11]&0x0F)==1) || ((tdata[11]&0x0F)==2) || ((tdata[11]&0x0F)==3)) ps4.button_right=1; else ps4.button_right=0;	//E
             if(((tdata[11]&0x0F)==0) || ((tdata[11]&0x0F)==1) || ((tdata[11]&0x0F)==7)) ps4.button_up=1;    else ps4.button_up=0;	//N
             ps4.button_square  =((tdata[11]&0b00010000)>>4); // SQUARE
             ps4.button_x       =((tdata[11]&0b00100000)>>5); // X
             ps4.button_circle  =((tdata[11]&0b01000000)>>6); // CIRCLE
             ps4.button_triangle=((tdata[11]&0b10000000)>>7); // TRIANGLE

             ps4.button_l1     =((tdata[12]&0b00000001));    // L1
             ps4.button_r1     =((tdata[12]&0b00000010)>>1); // R1
             ps4.button_l2     =((tdata[12]&0b00000100)>>2); // L2
             ps4.button_r2     =((tdata[12]&0b00001000)>>3); // R2
             ps4.button_share  =((tdata[12]&0b00010000)>>4); // SHARE
             ps4.button_options=((tdata[12]&0b00100000)>>5); // OPTIONS
             ps4.button_l3     =((tdata[12]&0b01000000)>>6); // L3
             ps4.button_r3     =((tdata[12]&0b10000000)>>7); // R3

             ps4.button_ps4    =((tdata[13]&0b00000001));    // PS4
             ps4.button_tpad   =((tdata[13]&0b00000010)>>1); // TPAD
                
             ps4.tpad_x        =tdata[14]; 
             ps4.tpad_y        =tdata[15];      
             ps4.battery       =tdata[16];       
             ps4_ok = 1;     
          }   
          else{
             Serial.println ('CRC Fail'); 
          }           
        }
     }       
  }
}

// CRC-8 Check Digit
unsigned char crc8(unsigned char *data, unsigned char len) {
   unsigned char crc = 0x00;
   unsigned char extract, i, sum;
   while(len--){ 
      extract = *data++;
      for (i = 8; i; i--) {
         sum = (crc ^ extract) & 0x01;
         crc >>= 1;
         if (sum) {
           crc ^= 0x8C;
         }  
         extract >>= 1;
      }
   }  
   return crc;
}



