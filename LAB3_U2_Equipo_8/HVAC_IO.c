 // FileName:        HVAC_IO.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Funciones de control de HW a través de estados.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"

static int iteraciones = 0;
/* Variables sobre las cuales se maneja el sistema. */
int Persiana_1A, Persiana_2A, Persiana_1B, Persiana_2B, Delay_Per1=0, Delay_Per2, i, j, k;//Estados de persianas

char state[MAX_MSG_SIZE];  // Lecturas de POT1
char state2[MAX_MSG_SIZE]; // Lecturas de POT2
char state3[MAX_MSG_SIZE]; // Lecturas de POT3
char state4[MAX_MSG_SIZE]; // Lecturas de PERSIANA1
char state5[MAX_MSG_SIZE]; // Lecturas de PERSIANA2

void System_Inicio (void);

bool event = FALSE;            // Evento I/O que fuerza impresión inmediata.

bool sys_flag, sys_turn;
bool light_flag = FALSE, light_turn = FALSE;
bool Lumin1, Lumin2, Lumin3;
bool Per_UD_1 = FALSE, Per_UD_2 = FALSE, ACT_STATE_P1 = FALSE, ACT_STATE_P2 = FALSE, PREV_STATE_P1, PREV_STATE_P2;

float Pot_1, Pot_2, Pot_3=0.0; //Valores de los POTs y sus canales.

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

void System_Inicio (void){
    GPIO_setOutput(BSP_LED1_PORT, BSP_LED1, 1);
    GPIO_setOutput(BSP_LED2_PORT, BSP_LED2, 0);
    GPIO_setOutput(BSP_LED3_PORT, BSP_LED3, 0);
    GPIO_setOutput(BSP_LED4_PORT, BSP_LED4, 0);
    UART_putsf(MAIN_UART, "\n SISTEMA: APAGADO \n\r");
    sys_turn = FALSE;
    sys_flag = FALSE;
}

void Sys_ON(void){
    sys_turn = (!sys_turn);
    if(sys_turn){
        sys_flag = TRUE;
        GPIO_setOutput(BSP_LED4_PORT, BSP_LED4, 1); //Se activa el LED Azul
        UART_putsf(MAIN_UART,"\n Sistema: ENCENDIDO \n\r");
    }else{
        sys_turn = TRUE;
        GPIO_clear_interrupt_flag(P1,B1);
        UART_putsf(MAIN_UART, "\n SI DESEA TERMINAR LA APLICACIÓN, ENTONCES VUELVA APRESIONAR EL SWITCH\n");
        for(k=0;k<5;k++){
            for(i=0;i<2500000;i++);
            if(!GPIO_getInputPinValue(SYS_PORT ,BIT(SYS_PIN_ON))){
                System_Inicio();
            }
        }
    }
}

void Button_SYS(void){ //Funcion que activa el sistema, utilizando un botón conectado al puerto 6.0 como entrada
    GPIO_clear_interrupt_flag(P1,B1); // Limpia la bandera de la interrupción.
    GPIO_clear_interrupt_flag(P1,B4); // Limpia la bandera de la interrupción.

    if(!GPIO_getInputPinValue(SYS_PORT ,BIT(SYS_PIN_ON))){ // Si se trata del botón 1.1, llamará a la función para trabajar con la persiana 1
        for(i=0;i<1000000;i++);
        Sys_ON();
        event=TRUE;
    }
    else if(!GPIO_getInputPinValue(LIGHT_PORT ,BIT(LIGHT_PIN_ON))){ // Si se trata del botón 1.4 1.1, llamará a la función para trabajar con la persiana 1.
        for(j=0;j<1000000;j++);
        LIGHT_ON();
        event=TRUE;
    }
    return;
}

void LIGHT_ON(void){
    light_flag = (!light_flag);
}

/*******************************************************************************************/

/**********************************************************************************
 * Function: INT_SWI
 * Preconditions: Interrupción habilitada, registrada e inicialización de módulos.
 * Overview: Función que es llamada cuando se genera
 *           la interrupción del botón SW1 o SW2.
 * Input: None.
 * Output: None.
 **********************************************************************************/
void INT_SWI(void)
{
    GPIO_clear_interrupt_flag(P2,B6); // Limpia la bandera de la interrupción.
    GPIO_clear_interrupt_flag(P2,B7); // Limpia la bandera de la interrupción.

    if(!GPIO_getInputPinValue(PERSIANAS_PORT, BIT(PERSIANA_1))){ // Si se trata del botón 1.1, llamará a la función para trabajar con la persiana 1
        for(i=0;i<1000000;i++);
        Per_UD_1 = (!Per_UD_1);
        ACT_STATE_P1 = Per_UD_1;
        event=TRUE;
    }
    else if(!GPIO_getInputPinValue(PERSIANAS_PORT, BIT(PERSIANA_2))){ // Si se trata del botón 1.4 1.1, llamará a la función para trabajar con la persiana 1.
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
    //Int_enableInterrupt(INT_PORT1);
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceADC
* Returned Value   : None.
* Comments         :
*    Inicializa las configuraciones deseadas para
*    el módulo general ADC y dos de sus canales; uno para la temperatura, otro para
*    el heartbeat.
*
*END***********************************************************************************/
void HVAC_InicialiceADC(void)
{
    // Iniciando ADC y canales.
    ADC_Initialize(ADC_14bitResolution, ADC_CLKDiv8);
    ADC_SetConvertionMode(ADC_SequenceOfChannels);
    ADC_ConfigurePinChannel(LIGHT_CH1, POT_1, ADC_VCC_VSS);   // Pin AN0 para potenciómetro.
    ADC_ConfigurePinChannel(LIGHT_CH2, POT_2, ADC_VCC_VSS);   // Pin AN1 para potenciómetro.
    ADC_ConfigurePinChannel(LIGHT_CH3, POT_3, ADC_VCC_VSS);   // Pin AN6 para potenciómetro.
    ADC_SetEndOfSequenceChannel(LIGHT_CH3);                     // Termina en el AN1, canal último.
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
    ADC_trigger(); while(ADC_is_busy()); //ADC_MSP432.h Se activa el ADC y pone al ADC en busy
    Pot_1 = ADC_result(LIGHT_CH1); //El resultado del convertidor ADC se guarda en su
    Pot_2 = ADC_result(LIGHT_CH2); //memoria y se registra en el canal y guarda
    Pot_3 = ADC_result(LIGHT_CH3); //en la variable.
    if(light_flag == TRUE){ // Observa entradas.
        Lumin1 = TRUE;
        Lumin2 = TRUE;
        Lumin3 = TRUE;
    }else{
        Lumin1 = FALSE;
        Lumin2 = FALSE;
        Lumin3 = FALSE;
    }
}
                                                                    //Este es solo un default en el caso de que
                                                                    // el sistema no encuentre ningun estado de los 3
                                                                    // activo (deberia ser un error debido a que debe
                                                                    // haber solo un botón activado siempre).

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_PrintState
* Returned Value   : None.
* Comments         :
*    Imprime via UART la situación actual del sistema en términos de temperaturas
*    actual y deseada, estado del abanico, del sistema y estado de las entradas.
*    Imprime cada cierto número de iteraciones y justo despues de recibir un cambio
*    en las entradas, produciéndose un inicio en las iteraciones.
*END***********************************************************************************/
void HVAC_PrintState(void)
{
    iteraciones++;
    if((iteraciones >= ITERATIONS_TO_PRINT || event == TRUE) && sys_flag == TRUE){
        iteraciones=0;
        event= FALSE;
        switch (Lumin1){ //Mostrará en la consola el valor del potenciometro 1 si esta activado, sino mostrará que esta apagado.
            case TRUE: sprintf(state, "LUM 1: %.0f ", (Pot_1/ 16383.0*10));
            break;
            case FALSE: sprintf(state,"LUM 1:OFF ");
            break;
            default:break;
        }
        switch (Lumin2){ //Mostrará en la consola el valor del potenciometro 2 si esta activado, sino mostrará que esta apagado.
            case TRUE: sprintf(state2,"LUM 2: %.0f ", (Pot_2/ 16383.0*10));
            break;
            case FALSE: sprintf(state2,"LUM 2:OFF, ");
            break;
            default:break;
        }
        switch (Lumin3){ //Mostrará en la consola el valor del potenciometro 3 si esta activado, sino mostrará que esta apagado.
            case TRUE: sprintf(state3,"LUM 3: %.0f ", (Pot_3/ 16383.0*10));
            break;
            case FALSE: sprintf(state3,"LUM 3:OFF, ");
            break;
            default:
            break;
        }
        if(PREV_STATE_P1 != ACT_STATE_P1){ //Si los botones de las persianas fueron presionados,
            Delay_Per1++; //aumentará el contrador que funcionará como delay.
        }

        if(Delay_Per1>0 && Delay_Per1<46){ //Si se usa un contador, donde llegue antes de 30 iteraciones, es lo equivalente a 10 segundos.

            if(Per_UD_1)
                sprintf(state4,"PER1: UP ");
            else
                sprintf(state4,"PER1: DOWN ");
        }else if (Per_UD_1){
            PREV_STATE_P1 = Per_UD_1;
            Delay_Per1=0;
            sprintf(state4,"PER1: OPEN ");
        }else{
            PREV_STATE_P1 = Per_UD_1;
            Delay_Per1=0;
            sprintf(state4,"PER1: CLOSED ");
        }

        if(PREV_STATE_P2 != ACT_STATE_P2){ //Si los botones de las persianas fueron presionados,
            Delay_Per2++; //aumentará el contrador que funcionará como delay.
        }

        if(Delay_Per2>0 && Delay_Per2<46){ //Si se usa un contador, donde llegue antes de 30 iteraciones, es lo equivalente a 10 segundos.

            if(Per_UD_2)
                sprintf(state5,"PER2: UP \r\n");
            else
                sprintf(state5,"PER2: DOWN \r\n");

        }else if (Per_UD_2){
            PREV_STATE_P2 = Per_UD_2;
            Delay_Per2=0;
            sprintf(state5,"PER2: OPEN \r\n");
        }else{
            PREV_STATE_P2 = Per_UD_2;
            Delay_Per2=0;
            sprintf(state5,"PER2: CLOSED \r\n");
        }
        Delay_ms(100);
        UART_putsf(MAIN_UART,state); //Imprime los resultados guardados en las cadenas asignadas.
        UART_putsf(MAIN_UART,state2);
        UART_putsf(MAIN_UART,state3);
        UART_putsf(MAIN_UART,state4);
        UART_putsf(MAIN_UART,state5);
    }
}

