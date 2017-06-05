/*
main funtion is pretty messy due to limitations of SAM7X256
*/

#include <targets/AT91SAM7.h>
#include "usart.h"
#include "lcd.h"
#include "boxes.h"
#include <stdbool.h>

#define AUDIO_OUT PIOB_SODR_P19
#define SW_1 PIOB_SODR_P24 //button 1
#define SW_2 PIOB_SODR_P25 //button 3
#define LCD_BACKLIGHT PIOB_SODR_P20 
#define SW_LEFT PIOA_SODR_P7 //left
#define SW_DOWN PIOA_SODR_P8 //down
#define SW_UP PIOA_SODR_P9 //up
#define SW_RIGHT PIOA_SODR_P14  //right
#define SW_CLICK PIOA_SODR_P15  //click

 #define MY 8 //margins
 #define MX 6 

void delay(int n)  __attribute__ ((section(".fast"))); //delay on proc
void delay(int n)
{
    volatile int m_delay;
    for (m_delay = 3000 * n; m_delay > 0; m_delay--) 
        __asm__("nop");
}

struct pawn { //structure of currently selected pawn
    int x; //coordinates
    int y;
    bool select; //is pawn selected?
    int type; //...if so, what type of pawn?
} pawn; //global initialization


int checkboard[8][8] = {0}; //declaration of checkboard array, will be used to save the status of the game
int whiteorblack = 1; //who has a turn now

int selected_x = 0; //coordinates of selection
int selected_y = 7;


int reset(){ //start/reset the game

    int j=0;
    int i=0;
    for(j=0;j<8;j++) {
        for(i=0;i<8;i++) {
            if     (i%2==0 && j%2 == 0 && j<3) checkboard[j][i] = 1; 
            else if(i%2==1 && j%2 == 1 && j<3) checkboard[j][i] = 1; 
            else if(i&2==0 && j%2 == 0 && j>4) checkboard[j][i] = 2; 
            else if(i%2==1 && j%2 == 1 && j>4) checkboard[j][i] = 2; 
            else checkboard[j][i] = 0;
            checkboard[6][0] = 2;
            checkboard[6][2] = 2;
            checkboard[6][4] = 2;
            checkboard[6][6] = 2; 
        }
    }
}


bool if_king() { //checks if pawn made it to last row, if so turns it into king
	if(pawn.type == 1 && pawn.y == 7) { pawn.type = 3; return true; } 
	if(pawn.type == 2 && pawn.y == 0) { pawn.type = 4; return true; } 
}


bool is_move_possible() { //checks if move is possible - what did you expect?
    int i=0, j=0; //auxiliary variables 
    if(pawn.select == true) { //is pawn selected?
      if(pawn.type == 1) { //checks different possibilites for each pawn type
        if(pawn.x == 0){
            if(checkboard[pawn.x+1][pawn.y+1] == 0) return true; //dont cross the borders
       		}
       		else if(pawn.x == 7) {
            	if(checkboard[pawn.x-1][pawn.y+1] == 0) return true;
            }
            else {
                if(checkboard[pawn.x+1][pawn.y+1] == 0) return true;
                if(checkboard[pawn.x-1][pawn.y+1] == 0) return true;
            }
        }

      else if(pawn.type == 2) {
        if(pawn.x == 0){
            if(checkboard[pawn.x+1][pawn.y-1] == 0) return true;
        	}
        	else if(pawn.x == 7){
                if(checkboard[pawn.x-1][pawn.y-1] == 0) return true;
            }
            else{
                if(checkboard[pawn.x+1][pawn.y-1] == 0) return true;
                if(checkboard[pawn.x-1][pawn.y-1] == 0) return true;
            }
      }

      else if(pawn.type == 3 || pawn.type == 4) { //loops for kings
		for(i=pawn.x+1, j=pawn.y+1; i<8 && j<8; i++,j++) {
          if(checkboard[i][j] == 0) return true;
          }
        for(i=pawn.x-1, j=pawn.y+1; i>0 && j<8; i--,j++) {
          if(checkboard[i][j] == 0) return true;
          }
        for(i=pawn.x+1, j=pawn.y-1; i<8 && j>0; i++,j--) {
          if(checkboard[i][j] == 0) return true;
          }
        for(i=pawn.x-1, j=pawn.y-1; i>0 && j>0; i--,j--) {
          if(checkboard[i][j] == 0) return true;
          }
      	}
      else return false;
    }
    else return false;
}


bool is_capture_possible() { //checks if is it possible to capture pawn
    int i=0, j=0; //auxiliary variables
    if(pawn.select == true) {
      if(pawn.type == 1) { //checks different possibilites for each pawn type
        if(pawn.x == 0){
            if((checkboard[pawn.x+1][pawn.y+1] == 2 || checkboard[pawn.x+1][pawn.y+1] == 4) && checkboard[pawn.x+2][pawn.y+2] == 0) return true;
       		}
       		else if(pawn.x == 7) {
            	if((checkboard[pawn.x-1][pawn.y+1] == 2 || checkboard[pawn.x-1][pawn.y+1] == 4) && checkboard[pawn.x-2][pawn.y+2] == 0) return true;
            }
            else {
                if((checkboard[pawn.x+1][pawn.y+1] == 2 || checkboard[pawn.x+1][pawn.y+1] == 4) && checkboard[pawn.x+2][pawn.y+2] == 0) return true;
                if((checkboard[pawn.x-1][pawn.y+1] == 2 || checkboard[pawn.x-1][pawn.y+1] == 4) && checkboard[pawn.x-2][pawn.y+2] == 0) return true;
            }
        }
        
	      else if(pawn.type == 2) {
	        if(pawn.x == 0){
	            if((checkboard[pawn.x+1][pawn.y-1] == 1 || checkboard[pawn.x+1][pawn.y-1] == 2) && checkboard[pawn.x+2][pawn.y-2] == 0) return true;
	        	}
	        	else if(pawn.x == 7){
	                if((checkboard[pawn.x-1][pawn.y-1] == 1 || checkboard[pawn.x-1][pawn.y-1] == 2) && checkboard[pawn.x-2][pawn.y-2] == 0) return true;
	            }
	            else{
	                if((checkboard[pawn.x+1][pawn.y-1] == 1 || checkboard[pawn.x+1][pawn.y-1] == 2) && checkboard[pawn.x+2][pawn.y-2] == 0) return true;
	                if((checkboard[pawn.x-1][pawn.y-1] == 1 || checkboard[pawn.x-1][pawn.y-1] == 2) && checkboard[pawn.x-2][pawn.y-2] == 0) return true;
	            }
	      }

	      else if(pawn.type == 3) { //again, loops for kings
			for(i=pawn.x+1, j=pawn.y+1; i<8 && j<8; i++,j++) {
	          if((checkboard[i][j] == 2 || checkboard[i][j] == 4) && checkboard[i+1][j+1] == 0) return true;
	          }
	        for(i=pawn.x-1, j=pawn.y+1; i>0 && j<8; i--,j++) {
	          if((checkboard[i][j] == 2 || checkboard[i][j] == 4) && checkboard[i-1][j+1] == 0) return true;
	          }
	        for(i=pawn.x+1, j=pawn.y-1; i<8 && j>0; i++,j--) {
	          if((checkboard[i][j] == 2 || checkboard[i][j] == 4) && checkboard[i+1][j-1] == 0) return true;
	          }
	        for(i=pawn.x-1, j=pawn.y-1; i>0 && j>0; i--,j--) {
	          if((checkboard[i][j] == 2 || checkboard[i][j] == 4) && checkboard[i-1][j-1] == 0) return true;
	          }
	      	}

			else if(pawn.type == 4) {
			for(i=pawn.x+1, j=pawn.y+1; i<8 && j<8; i++,j++) {
	          if((checkboard[i][j] == 1 || checkboard[i][j] == 3) && checkboard[i+1][j+1] == 0) return true;
	          }
	        for(i=pawn.x-1, j=pawn.y+1; i>0 && j<8; i--,j++) {
	          if((checkboard[i][j] == 1 || checkboard[i][j] == 3) && checkboard[i-1][j+1] == 0) return true;
	          }
	        for(i=pawn.x+1, j=pawn.y-1; i<8 && j>0; i++,j--) {
	          if((checkboard[i][j] == 1 || checkboard[i][j] == 3) && checkboard[i+1][j-1] == 0) return true;
	          }
	        for(i=pawn.x-1, j=pawn.y-1; i>0 && j>0; i--,j--) {
	          if((checkboard[i][j] == 1 || checkboard[i][j] == 3) && checkboard[i-1][j-1] == 0) return true;
	          }
	      	}

      else return false;
    }
    else return false;
}

void if_finish() { //checks if game is over
    int player = 0, i=0, j=0;
    for(i=0;i<8;i++){ //checks whole checkboard
        for(j=0;j<8;j++){
            if(checkboard[i][j] != 0)
                player = checkboard[i][j]; //remembers the player standing on checkboard
            if((player == 1 || player == 3) && (checkboard[i][j] == 2 && checkboard[i][j] == 4)) //if finds pawn of opponent, break
                break;
            if((player == 2 || player == 4) && (checkboard[i][j] == 1 && checkboard[i][j] == 3))
                break;
       
    }
    if(player == 1 || player == 3)
        LCDPutStr("White wins",37,5,LARGE,WHITE,BLACK) ; //notification who won
    else
        LCDPutStr("Black wins",37,5,LARGE,WHITE,BLACK) ;
	}
} 

int main() {
    int i=0; //auxiliary variables
    int j=0;
    pawn.select = false; //at the start of the game nothing is selected

    PMC_PCER = PMC_PCER_PIOB; //input/output devices
    PIOB_OER = LCD_BACKLIGHT; //backlight
    PIOB_PER = LCD_BACKLIGHT; //backlight 
    PMC_PCER = PMC_PCER_PIOA; //interrupt handeling
    PMC_PCER = PMC_PCER_PIOB; //buttons
    PMC_PCER = PMC_PCER_PIOA | PMC_PCER_PIOB;

    InitLCD(); //initialization of LCD
    LCDSettings(); //loads default LCD settings
    LCDClearScreen(); //clear the screen
    PIOB_SODR |= LCD_BACKLIGHT; //turns on backlight
    
    LCDWrite130x130bmp(); //draw the checkboard
    reset(); //start new game

    //draw pawns
    for(i=0;i<8;i++){
      for(j=0;j<8;j++){
          if(checkboard[i][j] == 1)
              LCDWrite15x15bmp(MY+(15*i), MX+(15*j), bpi); //draws correct image in the right place
          if(checkboard[i][j] == 2)
              LCDWrite15x15bmp(MY+(15*i), MX+(15*j), cpi);
      }
    }
  

    while(1) { //main loop of the program
      if ((PIOA_PDSR & SW_DOWN) == 0) { //joystick up
            if(selected_y<7){ //dont cross the checkboard borders
                switch(checkboard[selected_y][selected_x]){ //which box to draw after change
                    case 0:  
						if((selected_y+selected_x)%2 == 0) //checks if there should be white or black box (in empty cases)
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpo);
						else
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpo);
                      	break;
                    case 1:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) { //checks if pawn is selected, if so - dont change it
                      	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpiz);
                        } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpi); //if not - lift the selection
                      	break;
                    case 2:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) { //same goes for different situations
                      	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpiz);
                        } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpi);
                     	break;
					case 3:
						if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
	                    LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bdz);
	                    } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bd);
	                    break;
                    case 4:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
	                    LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cdz);
	                	} else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cd);
	                    break;
                }

            selected_y++; //incrementation of selection coordinates
            delay(250); //necessary delay

            	switch(checkboard[selected_y][selected_x]){ //draw next box as selected
	                case 0:
						if((selected_y+selected_x)%2 == 0)
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpoz);
						else
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpoz);
                      	break;
	                case 1:
	                   	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpiz);
	                  	break;
	                case 2:
			            LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpiz);
			            break;
					case 3:
	                  	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bdz);
	                  	break;
	                case 4:
	                  	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cdz);
	                  	break;
            }
            delay(500); //delay to be actually able to control the selection
            }
          }

      //same goes for different directions of joystick
      if ((PIOA_PDSR & SW_RIGHT) == 0) {
          if(selected_x<7){
              switch(checkboard[selected_y][selected_x]){
                    case 0:  
						if((selected_y+selected_x)%2 == 0)
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpo);
						else
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpo);
                      	break;
                    case 1:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
                      	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpiz);
                        } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpi);
                      	break;
                    case 2:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
                      	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpiz);
                        } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpi);
                     	break;
					case 3:
						if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
	                    LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bdz);
	                    } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bd);
	                    break;
                    case 4:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
	                    LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cdz);
	                	} else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cd);
	                    break;
                }
            selected_x++;
            delay(250);

            switch(checkboard[selected_y][selected_x]){
	                case 0:
						if((selected_y+selected_x)%2 == 0)
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpoz);
						else
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpoz);
                      	break;
	                case 1:
	                   	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpiz);
	                  	break;
	                case 2:
			            LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpiz);
			            break;
					case 3:
	                  	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bdz);
	                  	break;
	                case 4:
	                  	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cdz);
	                  	break;
            }

            delay(500);

            }
          }
      if ((PIOA_PDSR & SW_LEFT) == 0) {
          if(selected_x>0){
          	switch(checkboard[selected_y][selected_x]){
                    case 0:  
						if((selected_y+selected_x)%2 == 0)
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpo);
						else
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpo);
                      	break;
                    case 1:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
                      	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpiz);
                        } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpi);
                      	break;
                    case 2:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
                      	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpiz);
                        } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpi);
                     	break;
					case 3:
						if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
	                    LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bdz);
	                    } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bd);
	                    break;
                    case 4:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
	                    LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cdz);
	                	} else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cd);
	                    break;
                }
            selected_x--;
            delay(250);
            switch(checkboard[selected_y][selected_x]){
	                case 0:
						if((selected_y+selected_x)%2 == 0)
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpoz);
						else
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpoz);
                      	break;
	                case 1:
	                   	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpiz);
	                  	break;
	                case 2:
			            LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpiz);
			            break;
					case 3:
	                  	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bdz);
	                  	break;
	                case 4:
	                  	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cdz);
	                  	break;
            }

            delay(500);

            }         	
          }
      if ((PIOA_PDSR & SW_UP) == 0){
          if(selected_y>0){
          	switch(checkboard[selected_y][selected_x]){
                    case 0:  
						if((selected_y+selected_x)%2 == 0)
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpo);
						else
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpo);
                      	break;
                    case 1:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
                      	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpiz);
                        } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpi);
                      	break;
                    case 2:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
                      	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpiz);
                        } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpi);
                     	break;
					case 3:
						if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
	                    LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bdz);
	                    } else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bd);
	                    break;
                    case 4:
                    	if(pawn.select == true && pawn.x == selected_x && pawn.y == selected_y) {
	                    LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cdz);
	                	} else LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cd);
	                    break;
                }
            selected_y--;
            delay(250);
            switch(checkboard[selected_y][selected_x]){
	                case 0:
						if((selected_y+selected_x)%2 == 0)
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpoz);
						else
						LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpoz);
                      	break;
	                case 1:
	                   	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpiz);
	                  	break;
	                case 2:
			            LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpiz);
			            break;
					case 3:
	                  	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bdz);
	                  	break;
	                case 4:
	                  	LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cdz);
	                  	break;
            }
            delay(500);
            }
          }

      if ((PIOB_PDSR & SW_1) == 0){ //pushing the button, imporant event in whole game
      	if(pawn.select == false) { //checks if pawn is selected, if not, it lets player to choose pawn
	      	if((whiteorblack == 1 && (checkboard[selected_y][selected_x] == 1 || checkboard[selected_y][selected_x] == 3))
	      		|| ((whiteorblack == 2 && (checkboard[selected_y][selected_x] == 2 || checkboard[selected_y][selected_x] == 4) )) ) { //is it player's pawn?
	           pawn.x = selected_x; //saves coordinates of selected pawn
	           pawn.y = selected_y;
	           pawn.select = true; //pawn is selected
	           if(checkboard[pawn.y][pawn.x] == 1) pawn.type = 1; //sets the type of pawn
	           else if(checkboard[pawn.y][pawn.x] == 2) pawn.type = 2;
	           else if(checkboard[pawn.y][pawn.x] == 3) pawn.type = 3;
	           else if(checkboard[pawn.y][pawn.x] == 4) pawn.type = 4;
	        }
    	}
    	if(pawn.select == true) { //pawn is selected, so it checks current box if is it possible to move or capture
    		if(is_capture_possible()) { //can it capture?
    			LCDWrite15x15bmp(MY+(pawn.y*15), MX+(15*pawn.x), cpo); //if so, draws blank box in current position
    			checkboard[pawn.y][pawn.x] = 0; //in checkboard array, current box is now empty (0)
    			checkboard[(selected_y+pawn.y)/2][(selected_x+pawn.x)/2] = 0; //in the box where was the opponent type there is blank box as well
    			LCDWrite15x15bmp(MY+(((selected_y+pawn.y)/2)*15), MX+(15*((selected_y+pawn.y)/2)), cpo); //draws blank box in that place
    			if(pawn.type == 1) { LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpiz); checkboard[selected_y][selected_x] = 1; } //draws pawn in new place and updates the array
    			else if(pawn.type == 2) { LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpiz); checkboard[selected_y][selected_x] = 2; }
    			else if(pawn.type == 3) { LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bdz); checkboard[selected_y][selected_x] = 3; }
    			else if(pawn.type == 4) { LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cdz); checkboard[selected_y][selected_x] = 4; }
    		}

    		else if(is_move_possible()) { //checks if is it possible to move to selected box
    			LCDWrite15x15bmp(MY+(pawn.y*15), MX+(15*pawn.x), cpo); //draws blank box in previous position
    			checkboard[pawn.y][pawn.x] = 0; //updates game's array
    			if(pawn.type == 1) { LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bpiz); checkboard[selected_y][selected_x] = 1; } //draws pawn in new position
    			else if(pawn.type == 2) { LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cpiz); checkboard[selected_y][selected_x] = 2; }
    			else if(pawn.type == 3) { LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bdz); checkboard[selected_y][selected_x] = 3; }
    			else if(pawn.type == 4) { LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cdz); checkboard[selected_y][selected_x] = 4; }
    		}

    		if(if_king()) { //checks if pawn made it to last row, if so it changes the type of pawn
    			if(pawn.type == 1) { LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), bdz); checkboard[selected_y][selected_x] = 3; }
    			if(pawn.type == 2) { LCDWrite15x15bmp(MY+(selected_y*15), MX+(15*selected_x), cdz); checkboard[selected_y][selected_x] = 4; }
    		}

    		if(!is_capture_possible()) { //checks if capture is still possible, if not, next player's turn
	    		if(whiteorblack == 1) whiteorblack = 2; //changes turn
	    		else whiteorblack = 1;
    		}

    		if_finish(); //is game over? 

    	}
    }
      


      if(PIOB_PDSR & SW_2){ //cancel the selection
            pawn.select = false; //pawn is no longer selected
            switch(pawn.type){
                case 1:
                    LCDWrite15x15bmp(MY+(pawn.y*15), MX+(15*pawn.x), bpi); //draws not selected version of pawn in the position of (previous) selected pawn
                    break;
                case 2:
                    LCDWrite15x15bmp(MY+(pawn.y*15), MX+(15*pawn.x), cpi);
                    break;
                case 3:
                    LCDWrite15x15bmp(MY+(pawn.y*15), MX+(15*pawn.x), bd);
                    break;
                case 4:
                    LCDWrite15x15bmp(MY+(pawn.y*15), MX+(15*pawn.x), cd);
                    break;
            }
           
          }

    }
}

