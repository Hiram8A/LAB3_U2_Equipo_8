/****************************************
 // FileName:        HVAC_IO.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V12.0.0 TI
 // Project version: LAB_2_U2_Equipo_8 v2.0
 // Company:         TECNM - CAMPUS CHIHUAHUA
 // Description:     Funciones de control de iluminación
 // Authors:         Hiram Ochoa Sáenz
  *                  Manuel Alejandro Quiroz Gallegos
  *                  Luis Octavio Méndez Valles
 // Professor:       Alfredo Chacón
 // Subject:         Arquitectura de Prog. Para Hardware
 // Updated:         10/11/2022
 ****************************************/

#include "HVAC.h"

/* Variables sobre las cuales se maneja el sistema. */
int DELAY_P1 = 0, DELAY_P2 = 0, ITERATIONS = 0, i, j;//Retrasos
float Pot_1, Pot_2, Pot_3; //Potenciometros

//Lecturas de potenciómetros y persianas
char state1[MAX_MSG_SIZE];
char state2[MAX_MSG_SIZE];
char state3[MAX_MSG_SIZE];
char state4[MAX_MSG_SIZE];
char state5[MAX_MSG_SIZE];

//Banderas para la ejecución del sistema
bool event = FALSE; //Evento; impresión inmediata
bool sys_flag, sys_turn;
bool light_flag = FALSE;
bool LIGHT_1, LIGHT_2, LIGHT_3;
bool Per_UD_1 = FALSE, Per_UD_2 = FALSE, ACT_STATE_P1 = FALSE, ACT_STATE_P2 = FALSE, PREV_STATE_P1, PREV_STATE_P2;

/* **** SE DECLARARON LAS VARIABLES Y FUNCIONES PARA REALIZAR EL DALAY CON EL TIMER ******** */
extern void Timer32_INT1 (void); // Función de interrupción.
extern void Delay_ms (uint32_t time); // Función de delay.

/*FUNCTION******************************************************************************
*
* Function Name    : System_InicialiceTIMER
* Returned Value   : None.
* Comments         :
*    Controla los preparativos para poder usar TIMER32
*
*END***********************************************************************************/
void System_InicialiceTIMER (void)
{
    T32_Init1();
    Int_registerInterrupt(INT_T32_INT1, Timer32_INT1);
    Int_enableInterrupt(INT_T32_INT1);
}

/*FUNCTION******************************************************************************
*
* Function Name    : System_Inicio
* Returned Value   : None.
* Comments         :
*    Imprime e indica el estado inicial del sistema (apagado).
*
*END***********************************************************************************/
void System_Inicio (void){
    GPIO_setOutput(BSP_LED1_PORT, BSP_LED1, 1);
    GPIO_setOutput(BSP_LED2_PORT, BSP_LED2, 0);
    GPIO_setOutput(BSP_LED3_PORT, BSP_LED3, 0);
    GPIO_setOutput(BSP_LED4_PORT, BSP_LED4, 0);
    UART_putsf(MAIN_UART, "\n SISTEMA: APAGADO \n\r");
    sys_turn = FALSE;
    sys_flag = FALSE;
}

/*FUNCTION******************************************************************************
*
* Function Name    : Sys_ON
* Returned Value   : None.
* Comments         :
*    Determina e imprime el estado del sistema.
*
*END***********************************************************************************/
void Sys_ON(void){
    sys_turn = (!sys_turn);
    if(sys_turn) //Enciende el sistema
    {
        sys_flag = TRUE;
        GPIO_setOutput(BSP_LED3_PORT, BSP_LED3, 1);
        UART_putsf(MAIN_UART,"\n Sistema: ENCENDIDO \n\r");
    }
    else //Pregunta si se presiona mientras esta en ejecución
    {
        sys_turn = TRUE;
        GPIO_clear_interrupt_flag(P1,B1);
        UART_putsf(MAIN_UART, "\n SI DESEA TERMINAR LA APLICACIÓN, ENTONCES VUELVA APRESIONAR EL SWITCH \n\r");
        for(i=0;i<3500000;i++)
        {
            if(!GPIO_getInputPinValue(SYS_PORT ,BIT(SYS_PIN_ON))) //Apaga el sistema
            {
                for(j=0;j<1000000;j++);
                System_Inicio();
            }
        }
    }
}

/**********************************************************************************
 * Function: Button_SYS
 * Preconditions: Interrupción habilitada, registrada e inicialización de módulos.
 * Overview: Función que es llamada cuando se genera
 *           la interrupción del botón SW1 o SW2.
 * Input: None.
 * Output: None.
 **********************************************************************************/
void Button_SYS(void) //Funcion que llama a las funciones que activan el sistema y las luces, utilizando botones conectados al puerto 1 como entrada
{
    GPIO_clear_interrupt_flag(P1,B1); //Limpia la bandera de la interrupción.
    GPIO_clear_interrupt_flag(P1,B4); //Limpia la bandera de la interrupción.

    if(!GPIO_getInputPinValue(SYS_PORT ,BIT(SYS_PIN_ON))) //Si se pulsa el boton 1.1, llama a la funcion que activa el sistema
    {
        for(i=0;i<1000000;i++);
        Sys_ON();
        event=TRUE;
    }
    else if(!GPIO_getInputPinValue(LIGHT_PORT ,BIT(LIGHT_PIN_ON))) //Si se pulsa el boton 1.4, llama a la funcion que activa las luces
    {
        for(j=0;j<1000000;j++);
        light_flag = (!light_flag);
        event=TRUE;
    }
    return;
}

/**********************************************************************************
 * Function: INT_SWI
 * Preconditions: Interrupción habilitada, registrada e inicialización de módulos.
 * Overview: Función que es llamada cuando se genera
 *           la interrupción del botón SW3 o SW4.
 * Input: None.
 * Output: None.
 **********************************************************************************/
void INT_SWI(void) //Funcion que activa las persianas utilizando botones conectados al puerto 2 como entrada
{
    GPIO_clear_interrupt_flag(P2,B6); // Limpia la bandera de la interrupción.
    GPIO_clear_interrupt_flag(P2,B7); // Limpia la bandera de la interrupción.

    if(!GPIO_getInputPinValue(PERSIANAS_PORT, BIT(PERSIANA_1))) //Si se pulsa el boton 2.6, activa la persiana 1
    {
        for(i=0;i<1000000;i++);
        Per_UD_1 = (!Per_UD_1);
        ACT_STATE_P1 = Per_UD_1;
        event=TRUE;
    }
    else if(!GPIO_getInputPinValue(PERSIANAS_PORT, BIT(PERSIANA_2))) //Si se pulsa el boton 2.7, activa la persiana 2
    {
        for(j=0;j<1000000;j++);
        Per_UD_2 = (!Per_UD_2);
        ACT_STATE_P2 = Per_UD_2;
        event=TRUE;
    }
    return;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceIO
* Returned Value   : None.
* Comments         :
*    Controla los preparativos para poder usar las entradas y salidas GPIO.
*
*END***********************************************************************************/
void HVAC_InicialiceIO(void)
{
    // Para entradas y salidas ya definidas en la tarjeta.
    GPIO_init_board();

    // Modo de interrupción de los botones principales.
    GPIO_interruptEdgeSelect(PERSIANAS_PORT, BIT(PERSIANA_1), GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(PERSIANAS_PORT, BIT(PERSIANA_2), GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(SYS_PORT, BIT(SYS_PIN_ON), GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(LIGHT_PORT ,BIT(LIGHT_PIN_ON), GPIO_HIGH_TO_LOW_TRANSITION);

    // Preparativos de interrupción.
    GPIO_clear_interrupt_flag(P1,B1);
    GPIO_clear_interrupt_flag(P1,B4);
    GPIO_clear_interrupt_flag(P2,B6);
    GPIO_clear_interrupt_flag(P2,B7);
    GPIO_enable_bit_interrupt(P1,B1);
    GPIO_enable_bit_interrupt(P1,B4);
    GPIO_enable_bit_interrupt(P2,B6);
    GPIO_enable_bit_interrupt(P2,B7);

    // Se necesitan más entradas, se usarán las siguientes:
    GPIO_setBitIO(SYS_PORT, SYS_PIN_ON, ENTRADA);
    GPIO_setBitIO(PERSIANAS_PORT,PERSIANA_1, ENTRADA);
    GPIO_setBitIO(PERSIANAS_PORT,PERSIANA_2, ENTRADA);
    GPIO_setBitIO(LIGHT_PORT ,LIGHT_PIN_ON, ENTRADA);

    /* Uso del módulo Interrupt para generar la interrupción general y registro de esta en una función
    *  que se llame cuando la interrupción se active.                                                   */
    Int_registerInterrupt(INT_PORT2, INT_SWI);
    Int_enableInterrupt(INT_PORT2);
    Int_registerInterrupt(INT_PORT1, Button_SYS);
    Int_enableInterrupt(INT_PORT1);
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceADC
* Returned Value   : None.
* Comments         :
*    Inicializa las configuraciones deseadas para
*    el módulo general ADC y tres de sus canales, uno para cada potenciometro.
*
*END***********************************************************************************/
void HVAC_InicialiceADC(void)
{
    // Iniciando ADC y canales.
    ADC_Initialize(ADC_14bitResolution, ADC_CLKDiv8);
    ADC_SetConvertionMode(ADC_SequenceOfChannels);
    ADC_ConfigurePinChannel(LIGHT_CH1, POT_1, ADC_VCC_VSS);   // Pin AN9 para potenciómetro.
    ADC_ConfigurePinChannel(LIGHT_CH2, POT_2, ADC_VCC_VSS);   // Pin AN8 para potenciómetro.
    ADC_ConfigurePinChannel(LIGHT_CH3, POT_3, ADC_VCC_VSS);   // Pin AN1 para potenciómetro.
    ADC_SetEndOfSequenceChannel(LIGHT_CH3);                   // Determina el final de la secuencia de canales.
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceUART
* Returned Value   : None.
* Comments         :
*    Inicializa las configuraciones deseadas para
*    configurar el modulo UART (comunicación asíncrona).
*
*END***********************************************************************************/
void HVAC_InicialiceUART (void)
{
    UART_init();
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarEntradas
* Returned Value   : None.
* Comments         :
*    Actualiza los variables indicadores de las entradas sobre las cuales surgirán
*    las salidas.
*
*END***********************************************************************************/
void HVAC_ActualizarEntradas(void)
{
    ADC_trigger(); while(ADC_is_busy()); // Comienza la conversión a ADC
    Pot_1 = ADC_result(LIGHT_CH1); // Guarda el valor de la conversión en las variables de los potenciómetros
    Pot_2 = ADC_result(LIGHT_CH2);
    Pot_3 = ADC_result(LIGHT_CH3);
    if(light_flag == TRUE){ // Enciende o apaga las luces
        LIGHT_1 = TRUE;
        LIGHT_2 = TRUE;
        LIGHT_3 = TRUE;
    }else{
        LIGHT_1 = FALSE;
        LIGHT_2 = FALSE;
        LIGHT_3 = FALSE;
    }
}

void LIGHTS(void)
{
    switch (LIGHT_1)  // Almacena los lumenes de la luz 1 en la cadena state1 de estar encendido, si no, OFF
    {
        case TRUE: sprintf(state1,"LUZ_1= %.0f ", (Pot_1/ 16383.0*10));break;
        case FALSE: sprintf(state1,"LUZ_1= OFF ");break;
    }
    switch (LIGHT_2)  // Almacena los lumenes de la luz 2 en la cadena state2 de estar encendido, si no, OFF
    {
        case TRUE: sprintf(state2,"LUZ_2= %.0f ", (Pot_2/ 16383.0*10));break;
        case FALSE: sprintf(state2,"LUZ_2= OFF, ");break;
    }
    switch (LIGHT_3)  // Almacena los lumenes de la luz 3 en la cadena state3 de estar encendido, si no, OFF
    {
        case TRUE: sprintf(state3,"LUZ_3= %.0f ", (Pot_3/ 16383.0*10));break;
        case FALSE: sprintf(state3,"LUZ_3= OFF, ");break;
    }
}

void PERSIANAS(void)
{
    if(PREV_STATE_P1 != ACT_STATE_P1) //Si ocurre un evento en la persiana 1, activa un contador que funciona como retardo
        DELAY_P1++;

    if(DELAY_P1>0 && DELAY_P1<10) //Almacena el estado cambiante de la persiana 1 durante 5 segundos
    {
        if(Per_UD_1)
            sprintf(state4,"P1: UP ");
        else
            sprintf(state4,"P1: DOWN ");
    }
    else if (Per_UD_1) //Almacena el estado final de la persiana 1
    {
        PREV_STATE_P1 = Per_UD_1;
        DELAY_P1=0;
        sprintf(state4,"P1: OPEN ");
    }
    else
    {
        PREV_STATE_P1 = Per_UD_1;
        DELAY_P1=0;
        sprintf(state4,"P1: CLOSED ");
    }

    if(PREV_STATE_P2 != ACT_STATE_P2) //Si ocurre un evento en la persiana 2, activa un contador que funciona como retardo
        DELAY_P2++;

    if(DELAY_P2>0 && DELAY_P2<10) //Almacena el estado cambiante de la persiana 2 durante 5 segundos
    {
        if(Per_UD_2)
            sprintf(state5,"P2: UP \r\n");
        else
            sprintf(state5,"P2: DOWN \r\n");
    }
    else if (Per_UD_2) //Almacena el estado final de la persiana 2
    {
        PREV_STATE_P2 = Per_UD_2;
        DELAY_P2=0;
        sprintf(state5,"P2: OPEN \r\n");
    }
    else
    {
        PREV_STATE_P2 = Per_UD_2;
        DELAY_P2=0;
        sprintf(state5,"P2: CLOSED \r\n");
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_PrintState
* Returned Value   : None.
* Comments         :
*    Imprime via UART la situación actual del sistema en términos de estados de las luces,
*    lumenes y estados de las persianas.
*    Imprime cada cierto número de iteraciones y justo despues de recibir un cambio
*    en las entradas, produciéndose un inicio en las iteraciones.
*END***********************************************************************************/
void HVAC_PrintState(void)
{
    ITERATIONS++;
    if((ITERATIONS > ITERATIONS_TO_PRINT || event) && sys_flag) // Entra si se cumplen las iteraciones necesarias o
    {                                                           // si ocurre un evento, mientras el sistema esté encendido
        ITERATIONS = 0;
        event = FALSE;
        LIGHTS();
        PERSIANAS();

        //Imprime los estados resultantes
        UART_putsf(MAIN_UART,state1);
        UART_putsf(MAIN_UART,state2);
        UART_putsf(MAIN_UART,state3);
        UART_putsf(MAIN_UART,state4);
        UART_putsf(MAIN_UART,state5);

        Delay_ms(500);
    }
}
