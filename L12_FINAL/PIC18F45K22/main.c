#include <xc.h>
#define _XTAL_FREQ 8000000  

#include <string.h>
#include "config.h"
#include "GLCD.h"
#include "splash.h"
#include <stdlib.h>

#define amunt PORTAbits.RA1
#define esquerra PORTAbits.RA3
#define dreta PORTAbits.RA2
#define abaix PORTAbits.RA4
#define START PORTAbits.RA5

//VARIABLES

//variables teclat
char UP = 0, DOWN = 0, RIGHT = 0, LEFT = 0;
//flag botons
char flag_up = 0, flag_down = 0, flag_right = 0, flag_left = 0;
//cronometre
char minuts = 0, segons = 0, decimes = 0;

int vides = 3;
int nivell = 1;

//flags dels nivells
char n1 = 0, n2 = 0, n3 = 0, n4 = 0, n_ant = 0, flag_ant = 0;

//flag jugant o no
int playing = 0;

//coordenades x i y de la nau
int pos_nau = 5;
int pos2_nau = 7;

//flag te benzina
char te_benzina = 0;

//coordenades x i y de la benzina
int xbenzina = 5;
int ybenzina = 20;

//coordenades x i y de la base
int xbase = 7;
int ybase = 20;

//flag del xoc
int xocat = 0;

//flag per mostrar l'enemic
char enemic1 = 0, enemic2 = 0, enemic3 = 0;

//coordenades enemic
int x1enemic = 1;
int y1enemic = 0;
//coordenades enemic
int x2enemic = 3;
int y2enemic = 0;
//coordenades enemic
int x3enemic = 2;
int y3enemic = 20;

//coordenades a modificar
int x1, x2, x3, y1, y2, y3;

//carrega del nivell de 48 a 22
int carrega = 48;

int punts = 0;

int roda = 0;

//flag final del lloc
char flag_fi = 0;

int duty_anterior = 48;

//objectes del joc
const char space = ' ';
const char cor = '"';
const char nau = '#';
const char base = '$';
const char benzina = '%';
const char enemic = '&';

//comandes del teclat
char comand;


// Escriu text
void writeTxt(byte page, byte y, char * s) {
   int i = 0;
   while (*s != '\n' && *s != '\0') {
     putchGLCD(page, y+i, *(s++));
     ++i;
   };
}

//CONFIGURACIÓ

void Config_PIC_Global() {
   
   //configuracio PIC
   ANSELA = 0x01;   
   ANSELB = 0x00;  
   ANSELC = 0x00;     
   ANSELD = 0x00;

   PORTA = 0x00;   
   PORTB = 0x00;  
   PORTD = 0x00; 

   TRISA = 0xFF;   
   TRISB = 0x00;
   TRISD = 0x00;
   TRISC = 0b11000100;
   TRISCbits.RC2 = 1;

   //configuracio timers i CCP
   INTCON = 0xA0; 	//GIE = PEIE = 1	 
   RCON = 0x80;		//IPEN = 1

   T0CON = 0x81;	//TMR0ON = PRESCALER = 1
   TMR0H = 0x3C;	//2e-6 - 0.1/2e-6 = 15536 = 3CB0
   TMR0L = 0xB0;
   INTCON2 = 0x04;
   
	
   //configuracio pwm  
   TRISCbits.RC2=0;

   PIE1bits.TMR2IE=1; 	//enable tmr2
   T2CONbits.T2CKPS1=1;	//prescaler
   PR2=124;   		//PWM = (pr2+1)*4*tosc*prescaler
   
   CCP1CONbits.CCP1M3=1;
   CCP1CONbits.CCP1M2=1;
   CCPR1L=0;
   CCP1CONbits.DC1B1=0;
   CCP1CONbits.DC1B0=0;
   
   T2CONbits.TMR2ON=1;
   
   //configuracio llegir en serie
   OSCCONbits.IRCF = 0b101;
   TRISCbits.TRISC7 = 1;
   TRISCbits.TRISC6 = 1;
   
   SPBRG1 = 16; 
   SPBRGH1 = 0;
   
   TXSTA1bits.BRGH = 1;
   BAUDCON1bits.BRG16 = 1;
   RCSTA1bits.SPEN = 1;
   RCSTA1bits.CREN1 = 1;
   TXSTAbits.TXEN = 1;
   
   //configuracio AD
   ADCON0bits.CHS = 0b0000;     //AN0
   ADCON0bits.GO = 1;
      
   ADCON1bits.TRIGSEL = 0; 	//CCP5
   ADCON1bits.PVCFG = 0b00; 	
   ADCON1bits.NVCFG = 0b00; 	
   
   ADCON2bits.ADFM = 1;    	//Right
   ADCON2bits.ACQT= 0b001 ; 	//2 TAD
   ADCON2bits.ADCS = 0b001; 	//Fosc/8
   ADCON0bits.ADON = 1;         //enable ADC
   
   //configuracio GLCD
   GLCDinit();		   
   clearGLCD(0,7,0,127);     
   setStartLine(0); 
}

//SPLASH
void splash(){
   int p = 0;
   for (int i = 0; i < 8; ++i){
      for (int j = 0; j < 128; ++j){
	 int pixel = bitmap[p];
	 
	 writeByte(i, j, pixel);
	 ++p;
      }
   }
}


//VIRTUAL TERMINAL

void putc_usart1 (char xc){
   while (!PIR1bits.TX1IF);
   TXREG1 = xc;
}

void puts_usart1(char * cptr){
   while(*cptr != '\n' && *cptr != '\0') {
      putc_usart1 (*cptr++);
   }
}

//CRONOMETRE

//imprimir cronometre
void tictac() {
   //minuts
   if (minuts == 0) {
      putchGLCD(0,118,'0');
      putchGLCD(0,119,'0');
   }
   else if (minuts < 10) {
      putchGLCD(0,118,'0');
      writeNum(0,119,minuts);
   }
   else {
      writeNum(0,118,minuts);
   }
   putchGLCD(0,120,':');
   //segons
   if (segons == 0) {
      putchGLCD(0,121,'0');
      putchGLCD(0,122,'0');
   }
   else if (segons < 10) {
      putchGLCD(0,121,'0');
      writeNum(0,122,segons);
   }
   else {
      writeNum(0,121,segons);
   }
   putchGLCD(0,123,':');
   //decimes
   putchGLCD(0,125,'0');
   writeNum(0,124,decimes);
}

//cronomtre figura 
void update_cronometre() {
   tictac();
   //roda
   roda %= 4;
   if (roda == 1) putchGLCD(0,127,'/');
   else if (roda == 2)putchGLCD(0,127,'|');
   else if (roda == 3) putchGLCD(0,127,0x5C); /*\*/
   else if (roda == 4) putchGLCD(0,127,'-');
}


//CARREGA
void actualitza_barra(){
   for (int i = 48; i > carrega; --i) {	//superior
      for (int j = 113; j < 118; ++j) {	//inferior
	 SetDot(i,j);
      }
   }
   if (carrega <= 22) flag_fi = 1;	//maxim
}

void rectangle() {
   //columnes laterals
   for (int i = 111; i < 119; ++i) {
      SetDot(20,i);
      SetDot(50,i);
   }
   //files superiors
   for (int j = 20; j < 51; ++j) {
      SetDot(j,119);
      SetDot(j,111);
   }
}


//VIDES
void update_vides(){
   if (vides == 3) {
      putchGLCD(0, 0, cor);
      putchGLCD(0, 2, cor);
      putchGLCD(0, 4, cor);
   }
   else if (vides == 2){
      putchGLCD(0, 0, cor);
      putchGLCD(0, 2, cor);
      putchGLCD(0, 4, space);
   }
   else if(vides == 1){
      putchGLCD(0, 0, cor);
      putchGLCD(0, 2, space);
      putchGLCD(0, 4, space);
   }
   else if (vides == 0) {
      putchGLCD(0, 0, space);
      putchGLCD(0, 2, space);
      putchGLCD(0, 4, space);
   }
}

void aleatori() {
   xbenzina = rand()%7;
   ybenzina = rand()%20;
   if (xbenzina == 0) ++xbenzina;
}
void random() {
   if (enemic1) {
      x1enemic = rand()%7;
      y1enemic = rand()%20;
      if (x1enemic == 0) ++x1enemic;
      enemic1 = 0;
   }
   if (enemic2) {
      x2enemic = rand()%7;
      y2enemic = rand()%20;
      if (x2enemic == 0) ++x2enemic;
      enemic2 = 0;
   }
   if (enemic3) {
      x3enemic = rand()%7;
      y3enemic = rand()%20;
      if (x3enemic == 0) ++x3enemic;
      enemic3 = 0;
   }
}


//MOVIMENTS (NAU, BENZINA, ENEMICS)
//BENZINA
void agafa_benzina() {
   if (pos2_nau == xbenzina && pos_nau == ybenzina) {
      te_benzina = 1;
      putchGLCD(pos2_nau+1,pos_nau,benzina);
   }
}

void deixa_benzina() {
   if (te_benzina && pos2_nau+1 == xbase && pos_nau == ybase) {
      carrega -= nivell;
      ++punts;
      actualitza_barra();
      te_benzina = 0;
      aleatori();
      putchGLCD(xbenzina,ybenzina,benzina);
      if (!flag_ant) {
	flag_ant = 1;
	n_ant = nivell; 
      }
   }
   else if (!te_benzina) putchGLCD(xbenzina,ybenzina,benzina);
}

//MOVIMENT ENEMICS
//xoc dels enemics amb la nau
void colisio() {
   if ((pos2_nau == x1enemic && pos_nau == y1enemic) || (te_benzina && pos2_nau+1 == x1enemic && pos_nau == y1enemic)) enemic1 = 1;
   if ((pos2_nau == x2enemic && pos_nau == y2enemic) || (te_benzina && pos2_nau+1 == x2enemic && pos_nau == y2enemic)) enemic2 = 1;
   if ((pos2_nau == x3enemic && pos_nau == y3enemic) || (te_benzina && pos2_nau+1 == x3enemic && pos_nau == y3enemic)) enemic3 = 1;
   if (enemic1 || enemic2 || enemic3) {
      xocat = 1;
      
      if (te_benzina) {
	 te_benzina = 0;
	 aleatori();
	 clearGLCD(pos2_nau+1,pos2_nau+2,pos_nau*5,pos_nau*5+5);
      }
      --vides;
      update_vides();
   }
}
//reaparicio dels enemics
void spawn() {
   if (xocat) {
      xocat = 0;
      random();
      if (enemic1) putchGLCD(x1enemic,y1enemic,enemic);
      if (enemic2) putchGLCD(x2enemic,y2enemic,enemic);
      if (enemic3) putchGLCD(x3enemic,y3enemic,enemic);
   }
   else if (!xocat) {
      putchGLCD(x1enemic,y1enemic,enemic);
      putchGLCD(x2enemic,y2enemic,enemic);
      putchGLCD(x3enemic,y3enemic,enemic);
   }
}

//moviment dels enemics
void move_enemics() {
   
   //posicio
   x1 = x1enemic;
   y1 = y1enemic;
   x2 = x2enemic;
   y2 = y2enemic;
   x3 = x3enemic;
   y3 = y3enemic;
   //elimina anterior
   clearGLCD(x1enemic,x1enemic+1,y1enemic*5,y1enemic*5+5);
   clearGLCD(x2enemic,x2enemic+1,y2enemic*5,y2enemic*5+5);
   clearGLCD(x3enemic,x3enemic+1,y3enemic*5,y3enemic*5+5);
   //moviment nou
   ++y1;
   ++y2;
   --y3;
   x1++;
   x2++;
   x3++;
   //limits del joc
   if (x1 <= 1) x1 = 1;
   if (x2 <= 1) x2 = 1;
   if (x3 <= 1) x3 = 1;
   if (x1 > 7) x1 = 1;
   if (x2 > 7) x2 = 1;
   if (x3 > 7) x3 = 1;
   
   if (y1 <= 1) y1 = 20;
   if (y2 <= 1) y2 = 20;
   if (y3 <= 1) y3 = 20;
   if (y1 > 20) y1 = 1;
   if (y2 > 20) y2 = 1;
   if (y3 > 20) y3 = 1;
   //posicio
   x1enemic = x1;
   y1enemic = y1;
   x2enemic = x2;
   y2enemic = y2;
   x3enemic = x3;
   y3enemic = y3;
   
   putchGLCD(x1,y1,enemic);
   putchGLCD(x2,y2,enemic);
   putchGLCD(x3,y3,enemic);
}

//MOVIMENT NAU
void move_nau() { 
   putchGLCD(pos2_nau,pos_nau,nau);
   
   //AMUNT
   if ((amunt || UP) && !flag_up) { 
      if (!UP) puts_usart1("Up\r");
	 
      --pos2_nau;
      
      if (pos2_nau < 1) pos2_nau = 1;	//limit superior
      
      clearGLCD(pos2_nau,pos2_nau+1,pos_nau*5,pos_nau*5+5);
      putchGLCD(pos2_nau,pos_nau,nau);
      
      if (te_benzina) {
	 clearGLCD(pos2_nau+1,pos2_nau+2,pos_nau*5,pos_nau*5+5);
	 putchGLCD(pos2_nau+1,pos_nau,benzina);
      }
      UP = flag_up = 0;
   }
   else if (!amunt && flag_up) flag_up = 1;	//flancs
      
   //ABAIX
   if ((abaix || DOWN) && !flag_down) { 
      if (!DOWN) puts_usart1("Down\r");
      
      if (pos2_nau != 7) clearGLCD(pos2_nau,pos2_nau+1,pos_nau*5,pos_nau*5+5);
      else clearGLCD(pos2_nau,pos2_nau,pos_nau*5,pos_nau*5+5);
      
      ++pos2_nau;
      
      if (pos2_nau > 7) pos2_nau = 7;
      
      clearGLCD(pos2_nau,pos2_nau-1,pos_nau*5,pos_nau*5+5);
      putchGLCD(pos2_nau,pos_nau,nau);
      
      if (pos2_nau+1 <= 7 && te_benzina) {
	 clearGLCD(pos2_nau+1,pos2_nau+2,pos_nau*5,pos_nau*5+5);
	 putchGLCD(pos2_nau+1,pos_nau,benzina); 
      }
      DOWN = flag_down = 0;
   }
   else if (!abaix && flag_down) flag_down = 1;	//flancs
      
   //ESQUERRA
   if ((esquerra || LEFT) && !flag_left) { 
      if (!LEFT) puts_usart1("Left\r");
	 
      if (pos2_nau != 7) clearGLCD(pos2_nau,pos2_nau+1,pos_nau*5,pos_nau*5+5);
      else clearGLCD(pos2_nau,pos2_nau,pos_nau*5,pos_nau*5+5);
	 
      --pos_nau;
      
      if (pos_nau < 0) pos_nau = 0;
      
      
      putchGLCD(pos2_nau,pos_nau,nau);
      
      if (pos2_nau+1 <= 7 && te_benzina) {
	 putchGLCD(pos2_nau+1,pos_nau,benzina); 
      }
      LEFT = flag_left = 0;
   } 
   else if (!esquerra && flag_left) flag_left = 1;	//flancs
      
   //DRETA
   if ((dreta || RIGHT) && !flag_right) { 
      if (!RIGHT) puts_usart1("Right\r");
      
      if (pos2_nau != 7) clearGLCD(pos2_nau,pos2_nau+1,pos_nau*5,pos_nau*5+5);
      else clearGLCD(pos2_nau,pos2_nau,pos_nau*5,pos_nau*5+5);
      
      ++pos_nau;
      
      if (pos_nau > 20) pos_nau = 20;
      
      putchGLCD(pos2_nau,pos_nau,nau);
      
      if (pos2_nau+1 <= 7 && te_benzina) {
	 putchGLCD(pos2_nau+1,pos_nau,benzina); 
      }
      RIGHT = flag_right = 0;
   }
   else if (!dreta && flag_right) flag_right = 1;	//flancs
}

//DIFICULTAT
void dificultat() {
   ADCON0bits.GO = 1;
   while (ADCON0bits.GO) {}
   int despres = (ADRESH << 8) | ADRESL;
   
   //1023/4 = 255
   if (playing > 0) {
      if (despres >= 0 && despres < 256) {
	 nivell = 5;         	
	 writeTxt(0,6,"BORING!");
      }
      else if (despres > 255 && despres < 511) {
	 nivell = 4;     		
	 writeTxt(0,6,"LIVELY!");
      }
      else if (despres > 510 && despres < 766) {
	 nivell = 3;     		
	 writeTxt(0,6,"BRUTAL!");
      }
      else {
	 nivell = 2;		
	 writeTxt(0,6,"EXTREME");
      }
   }
}

//INTERRUPCIÓ
void interrupt rsi() {
   //TMR0
   if (TMR0IE && TMR0IF) {
      TMR0IF = 0;
      if (playing) {
	 //CRONOMETRE
	 TMR0H = 0x3C;
	 TMR0L = 0xB0;
	 ++roda;
	 ++decimes;
	 n1 = n2 = n3 = n4 = 0;
	 if (decimes == 10) {
	    decimes = 0;
	    segons++;
	    
	    if (segons%5 == 0) n1 = 1;
	    if (segons%4 == 0) n2 = 1;
	    if (segons%3 == 0) n3 = 1;
	    if (segons%2 == 0) n4 = 1;
	       
	    if (segons == 59) {
	       segons = 0;
	       minuts++;
	       if (minuts == 59) {
		  minuts = 0;
		  minuts = segons = decimes = 0;
	       }
	    }
	 }

      }
   }
   //TMR2
   if (TMR2IE && TMR2IF) {
      TMR2IF = 0;
      if (duty_anterior > carrega) ++CCPR1L;
      duty_anterior = carrega;
   }
}

//REINICIAR
void reinici() {
   clearGLCD(0,7,0,127);
   playing = 1;
   vides = 3;
   punts = 0;
   flag_fi = 0;
   flag_ant = 0;
   te_benzina = 0;
   carrega = 48;
   CCPR1L = 0;
   minuts = segons = decimes = 0;
   putchGLCD(7,20,base);
}

//INICI
void inici() {
   int f = 0;
   while (!START) {
      if (!f) {
	 writeTxt(3,9,"JETPIC");
	 writeTxt(4,6,"PRESS START!");
	 writeTxt(5,9,"& & & ");
   
	 puts_usart1("    ////JETPIC\\\\\\\\ \rPROJECT MADE BY PAU CAMPILLO i Bibek\r");
	 puts_usart1("//Instruccions:\r");
	 puts_usart1("Moviment de la nau amb botons fisics\r");
	 puts_usart1("Moviment de la nau amb teclat:\r");
	 puts_usart1(" UP:    q\r DOWN:  a\r LEFT:  o\r RIGHT: p\r");
	 puts_usart1("PRESS START TO PLAY!\r");
	 f = 1;
      }
   }
}

//FINAL
void final() {
   playing = 0;
   CCPR1L = 0;
   clearGLCD(0,7,0,127);
   
   writeTxt(0,0,"# # # # GAME OVER # # # #");
   writeTxt(1,8,"& & & & &");
   //Puntuació
   //cas en que el nivell es modifiqui mentres es juga
   if (n_ant == nivell) {
      if (punts > 10) writeNum(5,10,punts);
      else writeNum(5,11,punts);
      putchGLCD(5,12,'/');
      if (nivell == 5) writeNum(5,13,6);
      else if (nivell == 4) writeNum(5,13,7);
      else if (nivell == 3) writeNum(5,13,9);
      else if (nivell == 2) writeNum(5,13,13);
   }
   //Derrota
   if (vides == 0 && carrega > 41) writeTxt(4,3,"SELECT EASY MODE :(");
   else if (vides == 0 && carrega > 35) writeTxt(4,6,"THAT'S ALL?");
   else if (vides == 0 && carrega > 20) writeTxt(4,8,"NICE TRY!");
   //Victoria
   if (vides == 3) writeTxt(4,4,"FLAWLESS VICTORY!");
   if (vides == 2) writeTxt(4,8,"GREAT JOB");
   if (vides == 1) writeTxt(4,9,"THRIFTY");
   
   if (flag_fi) writeTxt(7,7,"GAME SUCCEED");
   else writeTxt(7,7,"GAME FAILED");
   
   puts_usart1("\rPRESS START TO PLAY!\r");
   
   while (!START) {}
   
   reinici();
}

void main() {
   Config_PIC_Global();
   //splash
   splash();
   __delay_ms(1000);
   clearGLCD(0,7,0,127);
   //inici
   inici();
   if (START) playing = 1;
   clearGLCD(0,7,0,127);
   vides = 3;

   while(1) {
      //JOC
      if (playing > 0) {
	 //CRONOMETRE
	 update_cronometre();
	 
	 if (nivell == 5 && n1) move_enemics();
	 else if (nivell == 4 && n2) move_enemics();
	 else if (nivell == 3 && n3) move_enemics();
	 else if (nivell == 2 && n4) move_enemics();
	 
	 //NAU
	 agafa_benzina();
	 move_nau();
	 deixa_benzina();
	 
	 //ENEMICS
	 colisio();
	 spawn();
	 
	 //CARREGA
	 rectangle();
	 
	 //VIDES
	 update_vides();
	 putchGLCD(7,20,base);
	 
	 //DIFICULTAT
	 dificultat();   
	 
	 //TECLAT
	 // Recieve Interrupt flag == rep lletres per escriure
	 if (RC1IF) {
	    char c = RCREG1; 
	    if (c == '\r') {}
	    else {
	       comand = c;
	       if (comand == 'q')  UP = 1;
	       if (comand == 'a')  DOWN = 1;
	       if (comand == 'o')  LEFT = 1;
	       if (comand == 'p')  RIGHT = 1;
	    }
	 }
	 
	 //FINAL mort o carrega completa
	 if (!vides || flag_fi){
	    playing = 0;
	    final();
	 }
      }     
   }
}