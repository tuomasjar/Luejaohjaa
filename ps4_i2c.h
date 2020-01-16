/*
  USB Host PS4 Dualshock Controller
  I2C interface 
*/

#define PS4_I2C_LEN 14

unsigned char ps4_ic2[PS4_I2C_LEN];
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
void get_ps4(void);
void decode_ps4(void);

void get_ps4(void)
{
  // Get data from PS4 DualShock Controller via I2C
  // We only want single byte values (0 to 255)
  unsigned char i2c_index=0;  
  Wire.beginTransmission(I2C_ADDRESS);  // transmit to device
  Wire.write(0);                        // Start receiving data from register 0
  Wire.endTransmission();               // end transmission
   
  Wire.requestFrom(I2C_ADDRESS, PS4_I2C_LEN);    // request PS4_I2C_LEN bytes from PS4 
  if(Wire.available()==0)ps4_ok = 0;
  else ps4_ok=1;
  while(Wire.available())
  { 
    ps4_ic2[i2c_index++] = Wire.read();          // receive a byte and increment index     
  }  
  if(ps4_ok)decode_ps4();                         // Decode PS4 data
  Serial.println(String(ps4_ok));
  }

// Decode PS4 Data and move to struct
void decode_ps4(void){

   ps4.l_joystick_x = ps4_ic2[0];
   ps4.l_joystick_y = ps4_ic2[1];
   ps4.r_joystick_x = ps4_ic2[2];
   ps4.r_joystick_y = ps4_ic2[3];
   ps4.accel_x      = ps4_ic2[4];
   ps4.accel_y      = ps4_ic2[5]; 
   ps4.l2           = ps4_ic2[6]; 
   ps4.r2           = ps4_ic2[7]; 
             
   if(((ps4_ic2[8]&0x0F)==5) || ((ps4_ic2[8]&0x0F)==6) || ((ps4_ic2[8]&0x0F)==7)) ps4.button_left=1;  else ps4.button_left=0;	//W
   if(((ps4_ic2[8]&0x0F)==3) || ((ps4_ic2[8]&0x0F)==4) || ((ps4_ic2[8]&0x0F)==5)) ps4.button_down=1;  else ps4.button_down=0;	//S
   if(((ps4_ic2[8]&0x0F)==1) || ((ps4_ic2[8]&0x0F)==2) || ((ps4_ic2[8]&0x0F)==3)) ps4.button_right=1; else ps4.button_right=0;	//E
   if(((ps4_ic2[8]&0x0F)==0) || ((ps4_ic2[8]&0x0F)==1) || ((ps4_ic2[8]&0x0F)==7)) ps4.button_up=1;    else ps4.button_up=0;	//N
   ps4.button_square  =((ps4_ic2[8]&0b00010000)>>4); // SQUARE
   ps4.button_x       =((ps4_ic2[8]&0b00100000)>>5); // X
   ps4.button_circle  =((ps4_ic2[8]&0b01000000)>>6); // CIRCLE
   ps4.button_triangle=((ps4_ic2[8]&0b10000000)>>7); // TRIANGLE

   ps4.button_l1     =((ps4_ic2[9]&0b00000001));    // L1
   ps4.button_r1     =((ps4_ic2[9]&0b00000010)>>1); // R1
   ps4.button_l2     =((ps4_ic2[9]&0b00000100)>>2); // L2
   ps4.button_r2     =((ps4_ic2[9]&0b00001000)>>3); // R2
   ps4.button_share  =((ps4_ic2[9]&0b00010000)>>4); // SHARE
   ps4.button_options=((ps4_ic2[9]&0b00100000)>>5); // OPTIONS
   ps4.button_l3     =((ps4_ic2[9]&0b01000000)>>6); // L3
   ps4.button_r3     =((ps4_ic2[9]&0b10000000)>>7); // R3

   ps4.button_ps4    =((ps4_ic2[10]&0b00000001));    // PS4
   ps4.button_tpad   =((ps4_ic2[10]&0b00000010)>>1); // TPAD
                
   ps4.tpad_x        =ps4_ic2[11]; 
   ps4.tpad_y        =ps4_ic2[12];      
   ps4.battery       =ps4_ic2[13];       

}
