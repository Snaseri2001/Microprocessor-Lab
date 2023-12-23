/*
 * program.c
 *
 *  Created on: Dec 12, 2023
 *      Author: fathi
 */


#include "main.h"

typedef struct pin_type{
    GPIO_TypeDef *port;
    uint16_t pin;
} ;

typedef struct seven_segment_type{
    pin_type digit_activators[4];
    pin_type BCD_input[4];
    uint32_t digits[4];
    uint32_t number;
    uint32_t state;
};

extern UART_HandleTypeDef huart1 ;
extern RTC_DateTypeDef mydate ;
extern RTC_TimeTypeDef mytime ;
extern RTC_HandleTypeDef hrtc;


int dimstep = 0;
int warn_num = 0;
int  tereshhold ;
int  state ;
seven_segment_type  seven_segment = {.digit_activators={{.port=GPIOD, .pin=GPIO_PIN_4},
                                                       {.port=GPIOD, .pin=GPIO_PIN_5},
                                                       {.port=GPIOD, .pin=GPIO_PIN_6},
                                                       {.port=GPIOD, .pin=GPIO_PIN_7}},
        .BCD_input={{.port=GPIOD, .pin=GPIO_PIN_0},
                    {.port=GPIOD, .pin=GPIO_PIN_1},
                    {.port=GPIOD, .pin=GPIO_PIN_2},
                    {.port=GPIOD, .pin=GPIO_PIN_3}},
        .digits={0, 0, 0, 0},
        .number = 0,
		.state=0
};

void seven_segment_display_decimal(uint32_t n) {
    if (n < 10) {
        HAL_GPIO_WritePin(seven_segment.BCD_input[0].port, seven_segment.BCD_input[0].pin,
                          (n & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(seven_segment.BCD_input[1].port, seven_segment.BCD_input[1].pin,
                          (n & 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(seven_segment.BCD_input[2].port, seven_segment.BCD_input[2].pin,
                          (n & 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(seven_segment.BCD_input[3].port, seven_segment.BCD_input[3].pin,
                          (n & 8) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void seven_segment_deactivate_digits(void) {
    for (int i = 0; i < 4; ++i) {
    	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(seven_segment.digit_activators[i].port, seven_segment.digit_activators[i].pin,
                          GPIO_PIN_SET);
    }
}

void seven_segment_activate_digit(uint32_t d) {
//    static uint32_t state = 0;
    static uint32_t last_time = 0;

    if (d < 4) {
        HAL_GPIO_WritePin(seven_segment.digit_activators[d].port, seven_segment.digit_activators[d].pin,
                          GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, 0);
        if (seven_segment.state == d){
        	if ((HAL_GetTick() - last_time) < 950){
				HAL_GPIO_WritePin(seven_segment.digit_activators[d].port, seven_segment.digit_activators[d].pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
			} else if  (HAL_GetTick() - last_time < 1000) {
				HAL_GPIO_WritePin(seven_segment.digit_activators[d].port, seven_segment.digit_activators[d].pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
			} else {
				last_time = HAL_GetTick();
			}
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, 1);
        }
    }
}

void seven_segment_set_num(uint32_t n) {
    if (n < 10000) {
        seven_segment.number = n;
        for (uint32_t i = 0; i < 4; ++i) {
            seven_segment.digits[3 - i] = n % 10;
            n /= 10;
        }
    }
}
void seven_segment_set_num_reverse(uint32_t n) {
    if (n < 10000) {
        seven_segment.number = n;
        for (uint32_t i = 0; i < 4; ++i) {
            seven_segment.digits[i] = n % 10;
            n /= 10;
        }
    }
}
void concat_uint32_array(uint32_t array[4]) {
    uint32_t result = 0;

    for (int i = 0; i < 4; i++) {
        result = (result << 8) | (array[i] & 0xFF);
    }

    seven_segment.number = result;
}


void seven_segment_refresh(void) {
    static uint32_t state = 0;
    static uint32_t last_time = 0;
    if (HAL_GetTick() - last_time > 5) {
        seven_segment_deactivate_digits();
		seven_segment_activate_digit(state);
        seven_segment_display_decimal(seven_segment.digits[state]);
        state = (state + 1) % 4;
        last_time = HAL_GetTick();
    }
}

void reset_pins_8_to_15() {
	    uint16_t pins = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
	                    GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	    updateStatus();
	    HAL_GPIO_WritePin(GPIOE, pins, GPIO_PIN_RESET);
}

void leds_update_start(uint32_t number) {
    switch (number) {
        case 1:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_SET);
            break;
        case 2:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET);
            break;
        case 3:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
            break;
        case 4:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
            break;
        case 5:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_SET);
            break;
        case 6:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET);
            break;
        case 7:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_SET);
            break;
        case 8:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_SET);
            break;
        default:
            // Handle invalid numbers or default case
            break;
    }
}

void turn_on_leds(uint32_t start, uint32_t length) {
    for (uint32_t i = start-1; i < start + length-1; i++) {
        // Set GPIO pin i to GPIO_PIN_SET
        HAL_GPIO_WritePin(GPIOE, (GPIO_PIN_8 << (i % 8)), GPIO_PIN_SET);
    }
}

void turn_on_leds_counterclockwise(uint32_t start, uint32_t length) {
    for (int i = start-1; i > start - length-1; i--) {
        // Calculate the index taking into account the wrap-around
//        int index = (i < 0) ? (i + 8) : i % 8;
//        if (i==0){
//        	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_SET);
//        }else if (i==-1){
//        	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_SET);
//        }else {
//        	HAL_GPIO_WritePin(GPIOE, (GPIO_PIN_8 << (index)), GPIO_PIN_SET);
//        }
    	HAL_GPIO_WritePin(GPIOE, (GPIO_PIN_8 << ((i < 0) ? (i + 8) : i % 8)), GPIO_PIN_SET);
    }
}


void programInit() {
    seven_segment_set_num(0000);
}


void programLoop() {
    seven_segment_refresh();
}

void setTIM8CCR1(int digit) {
    TIM8->CCR1 = digit * 100;
}
void setTIM8CCR2(int digit) {
    TIM8->CCR2 = digit * 100;
}
void setTIM8CCR3(int digit) {
    TIM8->CCR3 = digit * 100;
}
void setTIM8CCR4(int digit) {
    TIM8->CCR4 = digit * 100;
}

#define DEBOUNCE_DELAY_MS 1000

// Variables for debounce
int enableCCR1 = 0; // Flag for CCR1
int enableCCR2 = 0; // Flag for CCR2
int enableCCR3 = 0; // Flag for CCR3
int enableCCR4 = 0; // Flag for CCR4
static uint32_t last_interrupt_time = 0;


void updateStatus(){
	if (seven_segment.state == 0){
		if (enableCCR1)
			setTIM8CCR1(seven_segment.digits[0]);
		if (enableCCR2)
			setTIM8CCR2(seven_segment.digits[0]);
		if (enableCCR3)
			setTIM8CCR3(seven_segment.digits[0]);
		if (enableCCR4)
			setTIM8CCR4(seven_segment.digits[0]);
	} else if (seven_segment.state == 1) {
		if (seven_segment.digits[1] == 0){
			setTIM8CCR1(0); enableCCR1 = 0;
			setTIM8CCR2(0); enableCCR2 = 0;
			setTIM8CCR3(0); enableCCR3 = 0;
			setTIM8CCR4(0); enableCCR4 = 0;
		} else if (seven_segment.digits[1] == 1){
			setTIM8CCR1(seven_segment.digits[0]); enableCCR1 = 1;
			setTIM8CCR2(0); enableCCR2 = 0;
			setTIM8CCR3(0); enableCCR3 = 0;
			setTIM8CCR4(0); enableCCR4 = 0;
		} else if (seven_segment.digits[1] == 2){
			setTIM8CCR1(seven_segment.digits[0]); enableCCR1 = 1;
			setTIM8CCR2(0); enableCCR2 = 0;
			setTIM8CCR3(0); enableCCR3 = 0;
			setTIM8CCR4(seven_segment.digits[0]); enableCCR4 = 1;
		} else if (seven_segment.digits[1] == 3){
			setTIM8CCR1(seven_segment.digits[0]); enableCCR1 = 1;
			setTIM8CCR2(seven_segment.digits[0]); enableCCR2 = 1;
			setTIM8CCR3(0); enableCCR3 = 0;
			setTIM8CCR4(seven_segment.digits[0]); enableCCR4 = 1;
		} else if (seven_segment.digits[1] >= 4) {
			setTIM8CCR1(seven_segment.digits[0]); enableCCR1 = 1;
			setTIM8CCR2(seven_segment.digits[0]); enableCCR2 = 1;
			setTIM8CCR3(seven_segment.digits[0]); enableCCR3 = 1;
			setTIM8CCR4(seven_segment.digits[0]); enableCCR4 = 1;
		}
	}
}

void MinusDigitsUpdate(seven_segment_type *seven_segment) {
    int X;
    switch (seven_segment->state) {
        case 0:
            X = 9;
            break;
        case 1:
            X = 4;
            break;
        case 2:
            X = 3;
            break;
        // Add more cases as needed for different states
        default:
            // Set a default value for X or handle an error condition
            X = -1; // Default case: Error condition
            break;
    }
    seven_segment->digits[seven_segment->state] = (seven_segment->digits[seven_segment->state] == 0) ? X : (seven_segment->digits[seven_segment->state] - 1) % (X + 1);
    dimstep = seven_segment->digits[0];
    warn_num = seven_segment->digits[2];

}

void PlusDigitsUpdate(seven_segment_type *seven_segment) {
    int X;
    switch (seven_segment->state) {
        case 0:
            X = 10;
            break;
        case 1:
            X = 5;
            break;
        case 2:
            X = 4;
            break;
        // Add more cases as needed for different states
        default:
            // Set a default value for X or handle an error condition
            X = -1; // Default case: Error condition
            break;
    }
    seven_segment->digits[seven_segment->state] = (seven_segment->digits[seven_segment->state] + 1) % X;
    dimstep = seven_segment->digits[0];
    warn_num = seven_segment->digits[2];

}

char wave[4][10] = {
		"Normal" ,
		"Sin" ,
		"Triangle" ,
		"Square"
};


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    uint32_t current_time = HAL_GetTick();
    if ((current_time - last_interrupt_time) > DEBOUNCE_DELAY_MS) {
        last_interrupt_time = current_time;

        if (state == 0){
        	int x ;
        	x = seven_segment.digits[3]*1000 ;
        	x += seven_segment.digits[2]*100 ;
        	x += seven_segment.digits[1]*10 ;
        	x += seven_segment.digits[0] ;
        	tereshhold += x ;


        	seven_segment.digits[3] = 0 ;
        	seven_segment.digits[2] = 0 ;
        	seven_segment.digits[1] = 0 ;
        	seven_segment.digits[0] = 0 ;
        	state = 1 ;


        }else{
        	void seven_segment_set_num(uint32_t n) {
        	    if (n < 10000) {
        	        seven_segment.number = n;
        	        for (uint32_t i = 0; i < 4; ++i) {
        	            seven_segment.digits[3 - i] = n % 10;
        	            n /= 10;
        	        }
        	    }
        	}





        if (GPIO_Pin == GPIO_PIN_0) {
        	if (seven_segment.state != 3){
        		MinusDigitsUpdate(&seven_segment);
      		  char timestr[30] ;
      		  HAL_RTC_GetTime(&hrtc, &mytime, RTC_FORMAT_BIN);
      		  HAL_RTC_GetDate(&hrtc, &mydate, RTC_FORMAT_BIN) ;


      		  int ss = sprintf(timestr, "INFO [%2d:%2d:%2d] \n " , mytime.Hours , mytime.Minutes , mytime.Seconds) ;
      		  HAL_UART_Transmit(&huart1 ,&timestr,ss,1000);



        		 unsigned char data[100]="SALAM";
        		 uint8_t n = sprintf(data,"Digit { %d } decreased \n", seven_segment.state);
        		 HAL_UART_Transmit(&huart1 ,&data,n,1000);

        		uint32_t updatearray = {seven_segment.digits[0], seven_segment.digits[1], seven_segment.digits[2], seven_segment.digits[3]};
        		concat_uint32_array(updatearray);

        		 unsigned char data1[100]="SALAM";
        		 n = sprintf(data1,"Digit  changed \n");
        		 HAL_UART_Transmit(&huart1 ,&data1,n,1000);

        		 if(seven_segment.state == 2){
        			 unsigned char data1[100]="SALAM";
        			 uint8_t n = sprintf(data1,"Wave change to { %s } \n" , wave[seven_segment.digits[2]] );
        			 HAL_UART_Transmit(&huart1 ,&data1,n,1000);
        		 }

        		 if(seven_segment.state == 0){
        		        unsigned char data1[100]="SALAM";
        		         n = sprintf(data1,"Dimstep Decreased \n"  );
        		         HAL_UART_Transmit(&huart1 ,&data1,n,1000);
        		         		 }


        		updateStatus();
        	}
        } else if (GPIO_Pin == GPIO_PIN_1) {
        	if (seven_segment.state != 3){
				PlusDigitsUpdate(&seven_segment);

	    		  char timestr[30] ;
	    		  HAL_RTC_GetTime(&hrtc, &mytime, RTC_FORMAT_BIN);
	    		  HAL_RTC_GetDate(&hrtc, &mydate, RTC_FORMAT_BIN) ;


	    		  int ss = sprintf(timestr, "INFO [%2d:%2d:%2d] \n " , mytime.Hours , mytime.Minutes , mytime.Seconds) ;
	    		  HAL_UART_Transmit(&huart1 ,&timestr,ss,1000);



				 unsigned char data[100]="SALAM";
			     uint8_t n = sprintf(data,"Digit { %d } increased \n", seven_segment.state);
			     HAL_UART_Transmit(&huart1 ,&data,n,1000);


				uint32_t updatearray = {seven_segment.digits[0], seven_segment.digits[1], seven_segment.digits[2], seven_segment.digits[3]};
				concat_uint32_array(updatearray);


				 unsigned char data1[100]="SALAM";
			     int n1 = sprintf(data1,"Digit  changed \n");
				 HAL_UART_Transmit(&huart1 ,&data1,n1,1000);

				 if(seven_segment.state == 2){
				     unsigned char data1[100]="SALAM";
				     uint8_t n = sprintf(data1,"Wave change to { %s } \n" , wave[seven_segment.digits[2]] );
				     HAL_UART_Transmit(&huart1 ,&data1,n,1000);
				        		 }

				 if(seven_segment.state == 0){
				       unsigned char data1[100]="SALAM";
				       uint8_t n = sprintf(data1,"Dimstep Increased \n"  );
				       HAL_UART_Transmit(&huart1 ,&data1,n,1000);
				        		         		 }


				updateStatus();
        	}
        } else if (GPIO_Pin == GPIO_PIN_2) {
        	seven_segment.state = (seven_segment.state + 1) % 3;
        }



    }
    }
}








