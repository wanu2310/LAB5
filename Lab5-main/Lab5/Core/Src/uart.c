/*
 * uart.c
 *
 *  Created on: Nov 30, 2023
 *      Author: 84869
 */
#include "uart.h"


uint8_t temp=0;
uint8_t buffer[MAX_BUFFER_SIZE];
uint8_t index_buffer=0;
uint8_t buffer_count=0;
uint8_t buffer_flag=0;

uint8_t data_receive[MAX_BUFFER_SIZE];      //Data received from UART
uint8_t data_receive_idx=0;					//Received data index
uint8_t data[MAX_BUFFER_SIZE];				//Data
uint8_t data_for_transmit[MAX_BUFFER_SIZE];		//Data structure
uint8_t data_size=0;						//Data size



enum DataReceive{
	WAIT_FOR_BEGIN_CHAR,	//Wait for '!'
	WRITING_DATA
};
enum DataReceive data_parser=WAIT_FOR_BEGIN_CHAR;


enum DataProcessing{
	WAIT_FOR_RTS,   //Wait for !RTS#
	DATA_TRANSMIT,
	WAIT_FOR_OK		//Wait for !OK#
};
enum DataProcessing state=WAIT_FOR_RTS;



void command_parser_fsm(){
	switch(data_parser){
	case WAIT_FOR_BEGIN_CHAR:
		if (temp=='!'){
			data_parser=WRITING_DATA;
			data_receive_idx=0;
			memset(data_receive,0,MAX_BUFFER_SIZE);
		}
		break;
	case WRITING_DATA:
		if (temp=='#'){
			data_parser=WAIT_FOR_BEGIN_CHAR;
			memcpy(data,data_receive,MAX_BUFFER_SIZE);
		} else{
			data_receive[data_receive_idx++]=temp;  //Writing data from temp to data_receive
		}
		break;
	default:
		break;
	}
}

void uart_communication_fsm(){
	switch (state){
	case WAIT_FOR_RTS:
		if(strcmp((char*)data,"RTS")==0){
			state=DATA_TRANSMIT;
			data_size=sprintf((char*)data_for_transmit,"!ADC=%d#", (int) HAL_ADC_GetValue(&hadc1));
		}
		break;
	case DATA_TRANSMIT:
		HAL_UART_Transmit(&huart2, data_for_transmit, sizeof(data_for_transmit), 1000);
		state=WAIT_FOR_OK;
		setTimer0(3000);
		break;
	case WAIT_FOR_OK:
		if (strcmp((char*)data,"OK")==0){
			state=WAIT_FOR_RTS;
			memset(data,0,MAX_BUFFER_SIZE);
		}
		if (getTimer0Flag()){
			state=DATA_TRANSMIT;
		}
		break;
	default:
		break;
	}
}




