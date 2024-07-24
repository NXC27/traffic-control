#include<stm32f401xe.h>

typedef enum {OFF = 0, RED = 1, YELLOW = 2, GREEN = 3} light;

int flag = 1;
int store_flag = 0;
int status = 0;

int red_time = 15;
int count = 15;

void gpio_config()
{
	RCC->AHB1ENR |= (1<<0|1<<1|1<<2);
	//port C pin[8:0] output mode
	GPIOC->MODER |= (1<<0|1<<2|1<<4|1<<6|1<<8|1<<10|1<<12|1<<14|1<<16);
	GPIOC->MODER &=~(1<<1|1<<3|1<<5|1<<7|1<<9|1<<11|1<<13|1<<15|1<<17);

	//port A pin[8:0] output mode
	GPIOA->MODER |= (1<<0|1<<2|1<<4|1<<6|1<<8|1<<10|1<<12|1<<14|1<<16);
	GPIOA->MODER &=~(1<<1|1<<3|1<<5|1<<7|1<<9|1<<11|1<<13|1<<15|1<<17);

	GPIOB->MODER |= (1<<26|1<<28|1<<30);
	GPIOB->MODER &=~(1<<27|1<<29|1<<31);

	GPIOB->MODER |= (1<<16|1<<18|1<<20);
	GPIOB->MODER &=~(1<<17|1<<19|1<<21);
}
void TIM2_init()
{
	__disable_irq();
	RCC->APB1ENR |= (1<<0);
	TIM2->PSC = 16000-1;	//1000Hz
	TIM2->ARR = 100-1;		//100ms
	TIM2->CNT = 0;
	TIM2->DIER |= (1<<0);
	TIM2->CR1 |= (1<<0);
	NVIC_SetPriority(TIM2_IRQn,5);
	NVIC_EnableIRQ(TIM2_IRQn);
	__enable_irq();
}
void Ext_init_PB0()
{
	//Interrupt initilization
	__disable_irq();
	RCC->APB2ENR|=(1<<14);		//Enable the clock for syscfg
	GPIOB->MODER&=~(0x3<<0);	//Make PB0 input

	GPIOB->PUPDR|=(1<<0);		// Make PB0 pullup
	GPIOB->PUPDR&=~(1<<1);		//

	SYSCFG->EXTICR[0]|=(1<<0);	//Select PB0 external interrupt pin
	EXTI->IMR|=(1<<0);			//unmask PB0
	EXTI->FTSR|=(1<<0);
	NVIC_SetPriority(EXTI0_IRQn,1); // set priority to interrupt small number high priority
	NVIC_EnableIRQ(EXTI0_IRQn);
	__enable_irq();
}
void Ext_init_PB1()
{
 //Init external interrupt
  __disable_irq();   //disable global interrupt
  GPIOB->MODER &= ~(1<<2); //make PB1 input
  GPIOB->MODER &= ~(1<<3);

  GPIOB->PUPDR |= (1<<2);  //make PB1 pull-up
  GPIOB->PUPDR &= ~(1<<3);

  SYSCFG->EXTICR[0] |= (1<<4); //select PB1 to external interrupt  1/4 = 0
  EXTI->IMR |= (1<<1);  //unmask PB1
  EXTI->FTSR |= (1<<1);  // enable falling edge
  NVIC_SetPriority(EXTI1_IRQn,1); // set priority to interrupt small number high priority
  NVIC_EnableIRQ(EXTI1_IRQn); //enable EXTI4
  __enable_irq();   //enable global interrupt
}
void Ext_init_PB2()
{
 //Init external interrupt
  __disable_irq();   //disable global interrupt
  GPIOB->MODER &= ~(1<<4); //make PB2 input
  GPIOB->MODER &= ~(1<<5);

  GPIOB->PUPDR |= (1<<4);  //make PB2 pull-up
  GPIOB->PUPDR &= ~(1<<5);

  SYSCFG->EXTICR[0] |= (1<<8); //select PB2 to external interrupt  2/4 = 0
  EXTI->IMR |= (1<<1);  //unmask
  EXTI->FTSR |= (1<<1);  // enable falling edge
  NVIC_SetPriority(EXTI2_IRQn,1); // set priority to interrupt small number high priority
  NVIC_EnableIRQ(EXTI2_IRQn); //enable EXTI4
  __enable_irq();   //enable global interrupt
}
void Ext_init_PB3()
{
 //Init external interrupt
  __disable_irq();   //disable global interrupt
  GPIOB->MODER &= ~(1<<6); //make PB2 input
  GPIOB->MODER &= ~(1<<7);

  GPIOB->PUPDR |= (1<<6);  //make PB2 pull-up
  GPIOB->PUPDR &= ~(1<<7);

  SYSCFG->EXTICR[0] |= (1<<12); //select PB3 to external interrupt  3/4 = 0
  EXTI->IMR |= (1<<1);  //unmask
  EXTI->FTSR |= (1<<1);  // enable falling edge
  NVIC_SetPriority(EXTI3_IRQn,1); // set priority to interrupt small number high priority
  NVIC_EnableIRQ(EXTI3_IRQn); //enable EXTI4
  __enable_irq();   //enable global interrupt
}
void TIM2_IRQHandler()
{
	if (TIM2->SR & (1<<0))
	{
		count --;
	}
	TIM2->SR &=~(1<<0);
}
void EXTI0_IRQHandler (void)
{
	if(EXTI->PR>>0&1)
	{
		//stop the current green lane
		//swap
		count = 3;
	}
	EXTI->PR|=(1<<0);
}
void EXTI1_IRQHandler (void)
{
	if(EXTI->PR>>1&1)
	{
		//all red
		if (status == 0)
		{
			count = 3;
			store_flag = flag;
		}
		status ++;
		if (status > 1)
		{
			flag = store_flag;
			status = 0;
		}
	}
	EXTI->PR|=(1<<1);
}
void EXTI2_IRQHandler (void)
{
	if(EXTI->PR>>1&1)
	{
		//increase delay time
		count += 5;
		red_time += 5;
	}
	EXTI->PR|=(1<<1);
}
void EXTI3_IRQHandler (void)
{
	if(EXTI->PR>>1&1)
	{
		//decrease delay time
		if (red_time > 10)
		{
			count -= 5;
			red_time -= 5;
		}
	}
	EXTI->PR|=(1<<1);
}
void control_traffic_light_vertical(light light_state)
{
	switch(light_state)
	{
		case OFF:
			GPIOB->ODR &=~(1<<8);
			GPIOB->ODR &=~(1<<9);
			GPIOB->ODR &=~(1<<10);	//all off
			break;
		case RED:
			GPIOB->ODR |= (1<<8);	//red on
			GPIOB->ODR &=~(1<<9);
			GPIOB->ODR &=~(1<<10);
			break;
		case YELLOW:
			GPIOB->ODR &=~(1<<8);
			GPIOB->ODR |= (1<<9);	//yellow on
			GPIOB->ODR &=~(1<<10);
			break;
		case GREEN:
			GPIOB->ODR &=~(1<<8);
			GPIOB->ODR &=~(1<<9);
			GPIOB->ODR |= (1<<10);	//green on
			break;
	}
}
void control_traffic_light_horizontal(light light_state)
{
	switch(light_state)
	{
		case OFF:
			GPIOB->ODR &=~(1<<13);
			GPIOB->ODR &=~(1<<14);
			GPIOB->ODR &=~(1<<15);	//all off
			break;
		case RED:
			GPIOB->ODR |= (1<<13);	//red on
			GPIOB->ODR &=~(1<<14);
			GPIOB->ODR &=~(1<<15);
			break;
		case YELLOW:
			GPIOB->ODR &=~(1<<13);
			GPIOB->ODR |= (1<<14);	//yellow on
			GPIOB->ODR &=~(1<<15);
			break;
		case GREEN:
			GPIOB->ODR &=~(1<<13);
			GPIOB->ODR &=~(1<<14);
			GPIOB->ODR |= (1<<15);	//green on
			break;
	}
}
int main()
{
	gpio_config();
	TIM2_init();
	Ext_init_PB0();
	Ext_init_PB1();
	Ext_init_PB2();
	Ext_init_PB3();

	int first_digit_h;
	int second_digit_h;
	int first_digit_v;
	int second_digit_v;
	int arr_7seg[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

	int set_timer_h;
	int set_timer_v;
	int delay_ms = 12*1275;
	//reset
	control_traffic_light_vertical(RED);
	while (1)
	{
		if (flag == 1)
		{
			if (count > 3){
				control_traffic_light_horizontal(GREEN);
				set_timer_h = count - 3;
				set_timer_v = count;
			}
			else if (count <= 3 && count > 0)
			{
				control_traffic_light_horizontal(YELLOW);
				set_timer_h = count;
				set_timer_v = count;
			}
			else if (count == 0)
			{
				control_traffic_light_horizontal(RED);
				count = red_time;
				if (status)
				{
					flag = 2;
				}
				else
				{
					flag = 0;
				}
			}
		}
		else if (flag == 0)
		{
			if (count > 3){
				control_traffic_light_vertical(GREEN);
				set_timer_v = count - 3;
				set_timer_h = count;
			}
			else if (count <= 3 && count > 0)
			{
				control_traffic_light_vertical(YELLOW);
				set_timer_v = count;
				set_timer_h = count;
			}
			else if (count == 0)
			{
				control_traffic_light_vertical(RED);
				count = red_time;
				if (status)
				{
					flag = 2;
				}
				else
				{
					flag = 1;
				}
			}
		}
		else if (flag == 2)
		{
			set_timer_h = 55;
			set_timer_v = 55;
			control_traffic_light_horizontal(RED);
			control_traffic_light_vertical(RED);
		}
		first_digit_h = set_timer_h/10;
		second_digit_h = set_timer_h%10;
		first_digit_v = set_timer_v/10;
		second_digit_v = set_timer_v%10;
		GPIOA->ODR = (0x100+arr_7seg[first_digit_v]);
		GPIOC->ODR = (0x100+arr_7seg[first_digit_h]);
		for(volatile int i=0; i <delay_ms; i++);
		for(volatile int i=0; i <delay_ms; i++);
		GPIOA->ODR = (0x080+arr_7seg[second_digit_v]);
		GPIOC->ODR = (0x080+arr_7seg[second_digit_h]);
		for(volatile int i=0; i <delay_ms; i++);
		for(volatile int i=0; i <delay_ms; i++);
	}

	return 0;
}
