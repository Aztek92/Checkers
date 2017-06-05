#include <targets/AT91SAM7.h>
#include "lcd.h"
#include "bmp.h"
#include "bmp1.h"
#include "bits.h"
#include "lcd_font.h"



#define LCD_RESET_LOW     PIOA_CODR   = BIT2
#define LCD_RESET_HIGH    PIOA_SODR   = BIT2

void Delaya (unsigned long a) { while (--a!=0); }

void Delay_ (unsigned long a) {

  volatile unsigned long d;
  d=a;
  while (--d!=0);
}

unsigned int i,j;


void InitLCD(void) 
{
  // Pin for backlight
  PIOB_SODR   = BIT20;    // Set PB20 to HIGH
  PIOB_OER    = BIT20;    // Configure PB20 as output
  // Reset pin
  PIOA_SODR   = BIT2;     // Set PA2 to HIGH
  PIOA_OER    = BIT2;     // Configure PA2 as output
  PIOA_PDR = BIT12 | BIT16 | BIT17 | BIT18;
  PIOA_ASR = BIT12 | BIT16 | BIT17 | BIT18;
  PIOA_BSR = 0;
  PMC_PCER = PMC_PCER_SPI0;
  SPI0_CR      = 0x81;               //SPI Enable, Sowtware reset
  SPI0_CR      = 0x01;               //SPI Enable
  SPI0_MR      = 0xE0019;            //Master mode, fixed select, disable decoder, FDIV=1 (MCK), PCS=1110
  SPI0_CSR0 = 0x01010C11;           //9bit, CPOL=1, ClockPhase=0, SCLK = 48Mhz/32*12 = 125kHz
}


void WriteSpiCommand(unsigned int data)
{
  data = (data & ~0x0100);
  // Wait for the transfer to complete
  while((SPI0_SR & SPI0_SR_TXEMPTY) == 0);
  SPI0_TDR = data;
}

void WriteSpiData(unsigned int data)
{
  data = (data | 0x0100);
  // Wait for the transfer to complete
  while((SPI0_SR & SPI0_SR_TXEMPTY) == 0);
  SPI0_TDR = data;
}


void Backlight(unsigned char state) 
{
  if(state == BKLGHT_LCD_ON)
    PIOB_SODR   = BIT20;    // Set PB20 to HIGH
  else
    PIOB_CODR   = BIT20;    // Set PB20 to LOW
}

void SetContrast(unsigned char contrast)
{
#ifdef GE12
  WriteSpiCommand(CONTRAST);
  WriteSpiData(0x20+contrast);    // contrast
#else
#endif
}

void LCDSettings(void) 
{
#ifdef GE12
  ///// Initialization start /////
  // 2. Software Reset
  WriteSpiCommand(SOFTRST);
  Delaya(2000);
  // 3. Initial escape
  WriteSpiCommand(INITESC);
  Delaya(2000);
  ///// Initialization end /////
  ///// Display setting 1 /////
  // 1. Refresh set
  WriteSpiCommand(REFSET);
  WriteSpiData(0);
  WriteSpiCommand(DISPCTRL);
  WriteSpiData(128);     // Set the lenght of one selection term
  WriteSpiData(128);     // Set N inversion -> no N inversion
  WriteSpiData(134);     // Set frame frequence and bias rate -> 2 devision of frequency and 1/8 bias, 1/67 duty, 96x67 size
  WriteSpiData(84);      // Set duty parameter
  WriteSpiData(69);      // Set duty parameter
  WriteSpiData(82);      // Set duty parameter
  WriteSpiData(67);      // Set duty parameter
  // 3.1 Grey scale 0 position set - 15 parameters
  WriteSpiCommand(GRAYSCALE0);
  WriteSpiData(1);     // GCP1 - gray lavel to be output when the RAM data is "0001"
  WriteSpiData(2);     // GCP2 - gray lavel to be output when the RAM data is "0010"
  WriteSpiData(4);     // GCP3 - gray lavel to be output when the RAM data is "0011"
  WriteSpiData(8);     // GCP4 - gray lavel to be output when the RAM data is "0100"
  WriteSpiData(16);    // GCP5 - gray lavel to be output when the RAM data is "0101"
  WriteSpiData(30);    // GCP6 - gray lavel to be output when the RAM data is "0110"
  WriteSpiData(40);    // GCP7 - gray lavel to be output when the RAM data is "0111"
  WriteSpiData(50);    // GCP8 - gray lavel to be output when the RAM data is "1000"
  WriteSpiData(60);    // GCP9 - gray lavel to be output when the RAM data is "1001"
  WriteSpiData(70);    // GCP10 - gray lavel to be output when the RAM data is "1010"
  WriteSpiData(80);    // GCP11 - gray lavel to be output when the RAM data is "1011"
  WriteSpiData(90);    // GCP12 - gray lavel to be output when the RAM data is "1100"
  WriteSpiData(100);   // GCP13 - gray lavel to be output when the RAM data is "1101"
  WriteSpiData(110);   // GCP14 - gray lavel to be output when the RAM data is "1110"
  WriteSpiData(127);   // GCP15 - gray lavel to be output when the RAM data is "1111"
  // 4. Gamma curve set - select gray scale - GRAYSCALE 0 or GREYSCALE 1
  WriteSpiCommand(GAMMA);
  WriteSpiData(1);     // Select grey scale 0
  // 5. Command driver output
  WriteSpiCommand(COMMONDRV);
  WriteSpiData(0);     // Set COM1-COM41 side come first, normal mod
  // 6. Set Normal mode (my)
  WriteSpiCommand(NORMALMODE);
  // 7. Inversion off
  // WriteSpiCommand(INVERSIONOFF);
  // 8. Column address set
  WriteSpiCommand(COLADDRSET);
  WriteSpiData(0);
  WriteSpiData(131);
  // 9. Page address set
  WriteSpiCommand(PAGEADDRSET);
  WriteSpiData(0);
  WriteSpiData(131);
  // 10. Memory access controler
  WriteSpiCommand(ACCESSCTRL);
  WriteSpiData(0x40);     // horizontal
  //WriteSpiData(0x20);   // vertical
  ///// Display setting 1 end  /////
  ///// Power supply  ///////
  // 1. Power control
  WriteSpiCommand(PWRCTRL);
  WriteSpiData(4);     // Internal resistance, V1OUT -> high power mode, oscilator devision rate
  // 2. Sleep out
  WriteSpiCommand(SLEEPOUT);
  // 3. Voltage control - voltage control and write contrast define LCD electronic volume
  WriteSpiCommand(VOLTCTRL);
  //WriteSpiData(0x7f);    //  full voltage control
  //WriteSpiData(0x03);    //  must be "1"
  // 4. Write contrast
  WriteSpiCommand(CONTRAST);
  WriteSpiData(0x3b);    // contrast
  Delaya(2000);
  // 5. Temperature gradient
  WriteSpiCommand(TEMPGRADIENT);
  for(i=0; i<14; i++) {WriteSpiData(0);}
  // 6. Booster voltage ON
  WriteSpiCommand(BOOSTVON);
  // Finally - Display On
  WriteSpiCommand(DISPLAYON);	
#else
  // Hardware reset
  LCD_RESET_LOW;
  Delay_(1000);
  LCD_RESET_HIGH;
  Delay_(1000);
  // Display vontrol
  WriteSpiCommand(DISCTL);
//  WriteSpiData(0x03); // no division
//  WriteSpiData(0x23); // 160 line
//  WriteSpiData(0x02); // 2 highlighte line
  WriteSpiData(0x00); // default
  WriteSpiData(0x20); // (32 + 1) * 4 = 132 lines (of which 130 are visible)
  WriteSpiData(0x0a); // default
  // COM scan
  WriteSpiCommand(COMSCN);
  WriteSpiData(0x00);  // Scan 1-80
  // Internal oscilator ON
  WriteSpiCommand(OSCON);
  // wait aproximetly 100ms
  Delay_(10000);
  // Sleep out
  WriteSpiCommand(SLPOUT);
  // Voltage control
  WriteSpiCommand(VOLCTR);
  WriteSpiData(0x1F); // middle value of V1
  WriteSpiData(0x03); // middle value of resistance value
  // Temperature gradient
  WriteSpiCommand(TMPGRD);
  WriteSpiData(0x00); // default
  // Power control
  WriteSpiCommand(PWRCTR);
  WriteSpiData(0x0f);   // referance voltage regulator on, circuit voltage follower on, BOOST ON
  // Normal display
  WriteSpiCommand(DISNOR);
  // Inverse display
  WriteSpiCommand(DISINV);
  // Partial area off
  WriteSpiCommand(PTLOUT);
  // Data control
  WriteSpiCommand(DATCTL);
  WriteSpiData(0x01); // all inversions off, column direction
  WriteSpiData(0x03); // RGB sequence
  WriteSpiData(0x02); // Grayscale -> 16
  // Page Address set
  WriteSpiCommand(PASET);
  WriteSpiData(0);
  WriteSpiData(131);
  // Page Column set
  WriteSpiCommand(CASET);
  WriteSpiData(0);
  WriteSpiData(131);
#endif
WriteSpiCommand(DISON);
}


void LCDWrite130x130bmp(void) 
{
#ifdef GE12
  // Display OFF
  WriteSpiCommand(DISPLAYOFF);
  // WRITE MEMORY
  WriteSpiCommand(MEMWRITE);
  for(j=0; j<sizeof(bmp); j++) {WriteSpiData(bmp[j]);}
  // Display On
  WriteSpiCommand(DISPLAYON);	
#else
  WriteSpiCommand(DATCTL);
  WriteSpiData(0x00); // all inversions off, column direction
  WriteSpiData(0x03); // RGB sequence
  WriteSpiData(0x02); // Grayscale -> 16
  // WRITE MEMORY
  WriteSpiCommand(RAMWR);
  for(j=0; j<25740; j++) {WriteSpiData(bmp[j]); }
  // wait aproximetly 100ms
  Delay_(10000);
  // Display On
  WriteSpiCommand(DISON);	
  WriteSpiCommand(DATCTL);
  WriteSpiData(0x01); // all inversions off, column direction
  WriteSpiData(0x00); // RGB sequence
  WriteSpiData(0x02); // Grayscale -> 16
#endif
}

void LCDWrite16x16bmp(int x, int y) 
{
#ifdef GE12
  // Display OFF
  WriteSpiCommand(DISPLAYOFF);
  // WRITE MEMORY
  WriteSpiCommand(MEMWRITE);
  for(j=0; j<sizeof(bmp1); j++) {WriteSpiData(bmp1[j]);}
  // Display On
  WriteSpiCommand(DISPLAYON);	
#else
  WriteSpiCommand(DATCTL);
  WriteSpiData(0x00); // all inversions off, column direction
  WriteSpiData(0x03); // RGB sequence
  WriteSpiData(0x02); // Grayscale -> 16
  // Row address set (command 0x2B)
  WriteSpiCommand(PASET);
  WriteSpiData(x);
  WriteSpiData(x + 16 - 1);
  // Column address set (command 0x2A)
  WriteSpiCommand(CASET);
  WriteSpiData(y);
  WriteSpiData(y + 16 - 1);
  // WRITE MEMORY
  WriteSpiCommand(RAMWR);
  for(j=0; j<350; j++) {WriteSpiData(bmp1[j]); }
  // wait aproximetly 100ms
  Delay_(10000);
  // Display On
  WriteSpiCommand(DISON);	
  WriteSpiCommand(DATCTL);
  WriteSpiData(0x01); // all inversions off, column direction
  WriteSpiData(0x00); // RGB sequence
  WriteSpiData(0x02); // Grayscale -> 16
#endif
}

void LCDWrite15x15bmp(int x, int y, char * bitmap) 
{
#ifdef GE12
  // Display OFF
  WriteSpiCommand(DISPLAYOFF);
  // WRITE MEMORY
  WriteSpiCommand(MEMWRITE);
  for(j=0; j<sizeof(bitmap); j++) {WriteSpiData(bitmap[j]);}
  // Display On
  WriteSpiCommand(DISPLAYON);	
#else
  WriteSpiCommand(DATCTL);
  WriteSpiData(0x00); // all inversions off, column direction
  WriteSpiData(0x03); // RGB sequence
  WriteSpiData(0x02); // Grayscale -> 16
  // Row address set (command 0x2B)
  WriteSpiCommand(PASET);
  WriteSpiData(x);
  WriteSpiData(x + 15 -1);
  // Column address set (command 0x2A)
  WriteSpiCommand(CASET);
  WriteSpiData(y);
  WriteSpiData(y + 15 -1);
  // WRITE MEMORY
  WriteSpiCommand(RAMWR);
  for(j=0; j<350; j++) {WriteSpiData(bitmap[j]); }
  // wait aproximetly 100ms
  Delay_(10000);
  // Display On
  WriteSpiCommand(DISON);	
  WriteSpiCommand(DATCTL);
  WriteSpiData(0x01); // all inversions off, column direction
  WriteSpiData(0x00); // RGB sequence
  WriteSpiData(0x02); // Grayscale -> 16
#endif
}

void LCDWriteChar(char c)
{

WriteSpiCommand(RAMWR);


for(j=0; j<1000; j++) {WriteSpiData(0xF0); WriteSpiData(0x00);}

}



void LCDPutChar(char c, int x, int y, int size, int fColor, int bColor) 
{
extern const unsigned char FONT6x8[97][8];
extern const unsigned char FONT8x8[97][8];
extern const unsigned char FONT8x16[97][16];
int i,j;
unsigned int nCols;
unsigned int nRows;
unsigned int nBytes;
unsigned char PixelRow;
unsigned char Mask;
unsigned int Word0;
unsigned int Word1;
unsigned char *pFont;
unsigned char *pChar;
unsigned char *FontTable[] = {(unsigned char *)FONT6x8, (unsigned char *)FONT8x8,
(unsigned char *)FONT8x16};
// get pointer to the beginning of the selected font table
pFont = (unsigned char *)FontTable[size];
// get the nColumns, nRows and nBytes
nCols = *pFont;
nRows = *(pFont + 1);
nBytes = *(pFont + 2);
// get pointer to the last byte of the desired character
pChar = pFont + (nBytes * (c - 0x1F)) + nBytes - 1;
// Row address set (command 0x2B)
WriteSpiCommand(PASET);
WriteSpiData(x);
WriteSpiData(x + nRows - 1);
// Column address set (command 0x2A)
WriteSpiCommand(CASET);
WriteSpiData(y);
WriteSpiData(y + nCols - 1);
// WRITE MEMORY
WriteSpiCommand(RAMWR);
// loop on each row, working backwards from the bottom to the top
for (i = nRows - 1; i >= 0; i--) {
// copy pixel row from font table and then decrement row
PixelRow = *pChar--;
// loop on each pixel in the row (left to right)
// Note: we do two pixels each loop
Mask = 0x80;
for (j = 0; j < nCols; j += 2) {
// if pixel bit set, use foreground color; else use the background color
// now get the pixel color for two successive pixels
if ((PixelRow & Mask) == 0)
Word0 = bColor;
else
Word0 = fColor;
Mask = Mask >> 1;
if ((PixelRow & Mask) == 0)
Word1 = bColor;
else
Word1 = fColor;
Mask = Mask >> 1;
// use this information to output three data bytes
WriteSpiData((Word0 >> 4) & 0xFF);
WriteSpiData(((Word0 & 0xF) << 4) | ((Word1 >> 8) & 0xF));
WriteSpiData(Word1 & 0xFF);
}
}
// terminate the Write Memory command
WriteSpiCommand(NOP);
}


void LCDPutStr(char *pString, int x, int y, int Size, int fColor, int bColor) 
{
// loop until null-terminator is seen
while (*pString != 0x00) {
// draw the character
LCDPutChar(*pString++, x, y, Size, fColor, bColor);
// advance the y position
if (Size == SMALL)
y = y + 6;
else if (Size == MEDIUM)
y = y + 8;
else
y = y + 8;
// bail out if y exceeds 131
if (y > 131) break;
}
}


void LCDClearScreen(void) {
long i; // loop counter
// Row address set (command 0x2B)
WriteSpiCommand(PASET);
WriteSpiData(0);
WriteSpiData(131);
// Column address set (command 0x2A)
WriteSpiCommand(CASET);
WriteSpiData(0);
WriteSpiData(131);
// set the display memory to BLACK
WriteSpiCommand(RAMWR);
for(i = 0; i < ((131 * 131) / 2); i++) {
WriteSpiData((BLACK >> 4) & 0xFF);
WriteSpiData(((BLACK & 0xF) << 4) | ((BLACK >> 8) & 0xF));
WriteSpiData(BLACK & 0xFF);
}
}