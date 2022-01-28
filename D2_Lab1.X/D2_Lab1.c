/*
 * File:   D2_Lab1.c
 * Author: Francisco Javier López Turcios
 *
 * Created on 24 de enero de 2022, 10:05 PM
 */


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#include "Display_h.h"
#include "ADC.h"
#define _XTAL_FREQ 4000000


//============================================================================
//============================ VARIABLES GLOBALES ============================
//============================================================================
uint8_t estado;                 // Determina el estado de los transistores
uint8_t adresh_aux;            // Almacena el último valor de la conversión ADC

uint8_t dec;                    // Variables que almacenan dígitos del display
uint8_t uni;

uint8_t dec_d;                  // Variables que almacenan los dígitos después
uint8_t uni_d;                  // de haber sido modificados por la tabla

//============================================================================
//========================= DECLARACIÓN DE FUNCIONES =========================
//============================================================================
void config_io(void);
void config_reloj(void);
void config_int(void);
void config_tmr0(void);
void config_iocb(void);

//============================================================================
//============================== INTERRUPCIONES ==============================
//============================================================================
void __interrupt() isr (void)
{
    if(INTCONbits.RBIF)         // Interrupción de botones, antirrebotes
    {
        if(!PORTBbits.RB0)      // Si se presiona B0, incrementar Puerto A
        {
            PORTA++;
        }
        if(!PORTBbits.RB1)      // Si se presiona B1, incrementar Puerto A
        {
            PORTA--;
        }
        INTCONbits.RBIF = 0;    // Limpiar bandera de overflow
    }
    
    if(INTCONbits.T0IF)         // Interrupción de TMR0, multiplexeo
    {
        INTCONbits.T0IF = 0;    // Limpiar bandera
        PORTD = 0;              // Limpiar último transistor encendido 
        switch(estado)
        {
        case 0:
            estado = 1;
            PORTD = 0b01;       // Encender transistor 1
            PORTC = uni_d;      // Display recibe valor de unidades
            TMR0 = 131;         // Timer0 a 2ms
            break;
        case 1:
            estado = 0;
            PORTD = 0b10;       // Encender transistor 2
            PORTC = dec_d;      // Display recibe valor de decenas
            TMR0 = 131;         // Timer0 a 2ms
            break;
        default:
            estado = 0;
            TMR0 = 131;         // Timer0 a 2ms
            break;
        }
    }
    
    if(PIR1bits.ADIF)           // Si la bandera está encendida, entrar
    {
        adresh_aux = ADRESH;        
        PIR1bits.ADIF = 0;          // Limpiar bandera
    }
}

//============================================================================
//=================================== MAIN ===================================
//============================================================================
void main(void) {
    
    config_io();
    config_reloj();
    config_int();
    config_tmr0();
    config_iocb();
    config_adc();
    
    while(1)
    {
        if(!ADCON0bits.GO)              // Si la conversión ya terminó, entrar
        {
            __delay_us(50);             // Delay para no interrumpir conversión
            ADCON0bits.GO = 1;          // Iniciar nueva conversión
        }
        
        divisor(adresh_aux, &dec, &uni);
        dec_d = tabla(dec);
        uni_d = tabla(uni);
        
        if(adresh_aux > PORTA)
        {
            PORTBbits.RB7 = 1;
        }
        else
        {
            PORTBbits.RB7 = 0;
        }
    }
    return;
}

//============================================================================
//================================ FUNCIONES =================================
//============================================================================
void config_io(void)
{
    ANSEL = 0b00100000;         // AN5 encendido, los demás pines digitales
    ANSELH = 0;
    
    TRISA = 0;                  // Puerto A salida (LEDs)
    TRISBbits.TRISB0 = 1;       // Puerto B en 0 y 1 entradas (botones)
    TRISBbits.TRISB1 = 1;
    TRISBbits.TRISB7 = 0;       // Puerto B en 7 salida (LED)
    TRISC = 0;                  // Puerto C salida (display)
    TRISD = 0b00;               // Puerto D en 0 y 1 salida (transistores)
    TRISEbits.TRISE0 = 1;       // Puerto E en 0 entrada (pot, analógica)
    
    PORTA = 0;                  // Limpiar condiciones iniciales de los puertos
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    PORTE = 0;
    return;
}

void config_reloj(void)         // Configuración del oscilador
{
    OSCCONbits.IRCF2 = 1;       // 4MHz
    OSCCONbits.IRCF1 = 1;       // 
    OSCCONbits.IRCF0 = 0;       // 
    OSCCONbits.SCS = 1;         // Reloj interno
    return;
}

void config_int(void)
{
    INTCONbits.GIE  = 1;        // Activar interrupciones
    INTCONbits.PEIE = 1;        // Activar interrupciones periféricas
    INTCONbits.RBIE = 1;        // Activar interrupciones de PuertoB
    INTCONbits.T0IE = 1;        // Activar interrupciones de Timer0
    INTCONbits.RBIF = 0;        // Apagar bandera de overflow de PuertoB
    INTCONbits.T0IF = 0;        // Apagar bandera de overflow de Timer0
    return;
}

void config_tmr0(void)
{
    OPTION_REGbits.T0CS = 0;    // Reloj interno seleccionado
    OPTION_REGbits.T0SE = 0;    // Flancos positivos
    OPTION_REGbits.PSA = 0;     // Prescaler a Timer0
    OPTION_REGbits.PS2 = 0;     // Prescaler (011 = 1:16)
    OPTION_REGbits.PS1 = 1;     
    OPTION_REGbits.PS0 = 1;     
    TMR0 = 131;                 // Preload para 2 ms
    return;
}

void config_iocb(void)
{
    OPTION_REGbits.nRBPU = 0;   // Encender configuración de weak pullups
    WPUB    =   0b00000011;     // Encender 2 pullups del puerto b (botones)
    IOCB    =   0b00000011;     // Encender interrupt on change de los pullups
    return;
}
