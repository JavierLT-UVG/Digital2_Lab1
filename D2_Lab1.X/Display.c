#include "Display_h.h"


uint8_t tabla(uint8_t valor)    // Traduce números de 0-F a valores de display
{
    switch(valor)
    {
        case 0:
            return 0b00111111;
            break;
        case 1:
            return 0b00000110;
            break;
        case 2:
            return 0b01011011;
            break;
        case 3:
            return 0b01001111;
            break;
        case 4:
            return 0b01100110;
            break;
        case 5:
            return 0b01101101;
            break;
        case 6:
            return 0b01111101;
            break;
        case 7:
            return 0b00000111;
            break;
        case 8:
            return 0b01111111;
            break;
        case 9:
            return 0b01101111;
            break;
        case 10:                // A
            return 0b01110111;
            break;
        case 11:                // B
            return 0b01111100;
            break;
        case 12:                // C
            return 0b00111001;
            break;
        case 13:                // D
            return 0b01011110;
            break;
        case 14:                // E
            return 0b01111001;
            break;
        case 15:                // F
            return 0b01110001;
            break;
        default:                // Default es un punto
            return 0b10000000;  
            break;
    }
}

void divisor(uint16_t num, uint8_t *decena, uint8_t *unidad)
{
    *decena = num / 16;         // Decenas hexadecimales
    *unidad = num % 16;         // Unidades hexadecimales
    return;
}