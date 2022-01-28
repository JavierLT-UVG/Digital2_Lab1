#include "ADC.h"


void config_adc(void)           // Configuración del ADC
{
    PIE1bits.ADIE = 1;          // Activar interrupción de ADC
    PIR1bits.ADIF = 0;          // Limpiar bandera de ADC
    
    ADCON1bits.ADFM = 0;        // Justificación a la izquierda
    ADCON1bits.VCFG0 = 0;       // Vss como referencia
    ADCON1bits.VCFG1 = 0;       // Vdd como referencia
    
    ADCON0bits.ADCS = 0b01;     // Fosc/8
    ADCON0bits.CHS = 5;         // Selección del canal 5
    ADCON0bits.ADON = 1;        // ADC encendido
    __delay_us(50);             // Delay de 50us
    return;
}