/**
 * Auteur 
 * Maxime Champagne
 * 25 avril 2022
 * 
 * Modifi� par Adriana Giselle Bardales Lazo
 * 
 * Description
 * Cet programme fait un g�n�rateur de forme d'onde � l'aide d'un potentiom�tre num�rique
 *
 * SPI/main.c
 * 
*/
#include <stdio.h>
#include "mcc_generated_files/mcc.h"

#define MAX 60
uint8_t const sin[MAX] ={
              254,254,252,249,244,238,231,222,213,202,
              191,179,167,154,141,127,114,101,88,76,
              64,53,42,33,24,17,11,6,3,1,
              0,1,3,6,11,17,24,33,42,53,
              64,76,88,101,114,128,141,154,167,179,
              191,202,213,222,231,238,244,249,252,254};

uint8_t const car[MAX] ={
             0,0,0,0,0,0,0,0,0,0,
			  0,0,0,0,0,0,0,0,0,0,
			  0,0,0,0,0,0,0,0,0,0,
			  255,255,255,255,255,255,255,255,255,255,
			  255,255,255,255,255,255,255,255,255,255,
			  255,255,255,255,255,255,255,255,255,255};

uint8_t const tri[MAX] ={
            9,17,26,34,43,51,60,68,77,85,
			 94,102,111,119,128,136,145,153,162,170,
			 179,187,196,204,213,221,230,238,247,255,
			 247,238,230,221,213,204,196,187,179,170,
			 162,153,145,136,128,119,111,102,94,86,
			 77,68,60,51,43,34,26,17,9,0};

void out_dig(uint8_t x);
void sinus_60(void);
void carre_60(void);
void tri_60(void);

void myTimer1_ISR(void);
void envoiPoint(void);

int nb_point = 0;
uint16_t calcul;
int flag;
int fre;
//void myTimer1_ISR(void);

/*
                         Main application
 */
void main(void)
{
    uint8_t valeur, lecture;
    float tension;
    
    SYSTEM_Initialize();
    
    INTERRUPT_GlobalInterruptEnable();
    
    INTERRUPT_PeripheralInterruptEnable();
    
    TMR1_SetInterruptHandler(myTimer1_ISR);

    SSPCON1bits.SSPEN = 1;
    IO_RA5_SetHigh();
    
    char data;
    uint16_t symbole;
    int nb_step = 20;
    
    while (1)
    {
        
       /*
        //Code de test pour valider le fonctionnement du potentiom�tre
        {
            printf("\n\rEntrer une valeur entre 0 et 255, suivie de [Enter]");
            valeur = 0;
            do
            {
                do
                {
                        lecture = EUSART1_Read();                 
                }
                while (((lecture < '0') || (lecture > '9')) && (lecture != 0x0d));
                if ((lecture >= '0') && (lecture <= '9')) 
                {
                    valeur = 10 * valeur + lecture - '0';
                    putchar(lecture);
                }
            }
        
            while ((lecture != 0x0d) && (valeur < 26)); //    while ((lecture != 0x0d) && (valeur < 26));   tension = (float)5* valeur /256;
            tension = (float)5* valeur /256;
            printf("\n\rValeur = %u tension = %3.2f ", valeur, tension);
           out_dig(valeur);    // envoi sur potentiometre 
        } 
       */

        if (EUSART1_is_rx_ready()){
            data = EUSART1_Read();
        } 
        
        myTimer1_ISR(); //foction qui gere le temps entre deux points
        
        if (flag == 1){           
            envoiPoint(); //foction qui envoi les points
            printf("hello");
            flag = 0;
        }
        
        else {
            
            switch(data){

                case 's':
                    sinus_60();
                    printf("\n\rType d'onde: Sinusoidal" "\n\rFrequence: %d",fre);
                    break;

                case 'c':
                    carre_60();
                    printf("\n\rType d'onde: Carre" "\n\rFrequence: %d",fre);
                    break;

                case 't':
                    tri_60();
                    printf("\n\rType d'onde: Triangulaire" "\n\rFrequence: %d",fre);
                    break;
                    
                case '+':
                    nb_step += 20; // incremente de 20hz
                    if (nb_step >= 100){//si frequence plus grand que 100 Hz
                        fre = nb_step;//on stock la valeur de f sur fre
                        printf("Frequence MAX");
                        nb_step = 1666;
                    }
                    
                    nb_step = 1 / (nb_step * 60 * 0,00000001);
                    calcul = 65536 - nb_step;
                    break;
                    
                case '-':
                    nb_step -= 20; // decremente de 20hz
                    if (nb_step <= 20){ //si frequence plus petit que 20 Hz
                        fre = nb_step;
                        printf("Frequence MIN");
                        nb_step = 8333;
                     }
                    
                    nb_step = 1 / (nb_step * 60 * 0,00000001);
                    calcul = 65536 - nb_step;
                    break;

                default :
                    printf("\n\r.");
                    break;
            }
        }
        

    }
    

           
}

//---------------------------------------------------------------
// Routine d'interruption du Timer1
//---------------------------------------------------------------
void myTimer1_ISR(void){
    //static uint8_t i; 
    TMR1_WriteTimer(calcul);
    
//    i++;
//    if (i==MAX){
//        i=0;t
//    }
    flag = 1;
}
    
//----------------------------------------------------------------
// Transmission au pot. d'une onde comprenant 60 points par cycle.
//----------------------------------------------------------------
void sinus_60(void) {
    uint8_t i;
    //for (i=0;i<MAX;i++) {
    out_dig(sin[nb_point]);   
}

void carre_60(void) {
    uint8_t i;
   // for (i=0;i<MAX;i++) {
    out_dig(car[nb_point]);         
}

void tri_60(void) {
    uint8_t i;
    //for (i=0;i<MAX;i++) {
    out_dig(tri[nb_point]);      
}

//----------------------------------------------------------------
//  Transmission d'une donnee a la sortie du pot. numerique
//----------------------------------------------------------------
void out_dig(uint8_t x){
	IO_RA5_SetLow();   // selection du potentiometre
	SPI_ExchangeByte(0x11);  // ecriture, pot. 0
	SPI_ExchangeByte(x);
	IO_RA5_SetHigh();
	//__delay_ms(1);
}
//----------------------------------------------------------------
//  Variation de la fr�quence de 20Hz � 100Hz
//----------------------------------------------------------------
void envoiPoint(void){
    nb_point += 1;
    
    if (nb_point == 60){
        nb_point = 0;
    }

}
