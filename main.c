#include "main.h"

static XIntc    Intc;                              //中断控制器实例
static XGpio    KEY_Gpio;                          //GPIO中断实例 按键
static XGpio    LED_Gpio;                          //GPIO实例
static XUartLite Uart;							   //串口实例

int led_value;          //LED值
int key_value;          //按键值
int Intr_times = 0;     //有效中断计数
int key_intr_flag = 0;  //中断标志

XIic Iic;
volatile u8 TransmitComplete;
volatile u8 ReceiveComplete;

int main(){

	xil_printf("This is a microblaze system!\r\n");

    LED_Init();
    KEY_Init();
    UART_Init();
    Interrupt_Init();
    Seven_Segment_Custom_IP_Test();

    Initialization_IIC();

	//xil_printf("data = %x\r\n", rec_byte);
	//xil_printf("bytecount = %d\r\n", ByteCount);

    while(1){
    	interrupt_KEY_LED_test();
    }
    return 0;
}

void Seven_Segment_Custom_IP_Test(void){
	 SEVEN_SEGMENT_LED_mWriteReg(XPAR_SEVEN_SEGMENT_LED_0_S0_AXI_BASEADDR, SEVEN_SEGMENT_LED_S0_AXI_SLV_REG0_OFFSET, 0x00000000);
	 SEVEN_SEGMENT_LED_mWriteReg(XPAR_SEVEN_SEGMENT_LED_0_S0_AXI_BASEADDR, SEVEN_SEGMENT_LED_S0_AXI_SLV_REG1_OFFSET, 0x00000000);
	 SEVEN_SEGMENT_LED_mWriteReg(XPAR_SEVEN_SEGMENT_LED_0_S0_AXI_BASEADDR, SEVEN_SEGMENT_LED_S0_AXI_SLV_REG2_OFFSET, 0x00000000);
	 SEVEN_SEGMENT_LED_mWriteReg(XPAR_SEVEN_SEGMENT_LED_0_S0_AXI_BASEADDR, SEVEN_SEGMENT_LED_S0_AXI_SLV_REG3_OFFSET, 0x0000000F);
}

void UART_Init(void){
	//初始化串口设备
	XUartLite_Initialize(&Uart , UART_DEVICE_ID);
	//初始化中断控制器
	XIntc_Initialize(&Intc, INTC_DEVICE_ID);
	//关联处理函数
	XIntc_Connect(&Intc, UART_INTR_ID,(XInterruptHandler)uart_handler,&Uart);
	//使能串口
	XUartLite_EnableInterrupt(&Uart);
	//打开中断控制器
	XIntc_Start(&Intc, XIN_REAL_MODE);
	//使能中断控制器
	XIntc_Enable(&Intc,UART_INTR_ID);
	//设置并打开中断异常处理功能
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
		(Xil_ExceptionHandler)XIntc_InterruptHandler , &Intc);
	Xil_ExceptionEnable();
}

void Interrupt_Init(void){
	//初始化中断控制器
	XIntc_Initialize(&Intc, INTC_DEVICE_ID);
	//关联中断ID和中断服务函数
	XIntc_Connect(&Intc,AXI_GPIO_INTR_ID,(Xil_ExceptionHandler)GpioHandler,&KEY_Gpio );
	//使能中断
	XGpio_InterruptEnable(&KEY_Gpio, 1);
	//使能全局中断
	XGpio_InterruptGlobalEnable(&KEY_Gpio);
	//在中断控制器上启用中断向量
	XIntc_Enable(&Intc,AXI_GPIO_INTR_ID);
	//启动中断控制器
	XIntc_Start(&Intc, XIN_REAL_MODE);
	//设置并打开中断异常处理
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(EXCEPTION_ID,
			(Xil_ExceptionHandler)XIntc_InterruptHandler,&Intc);
	Xil_ExceptionEnable();

}

void uart_handler(void *CallbackRef)//中断处理函数
{
	u8 Read_data;
	u32 isr_status;
	XUartLite *InstancePtr= (XUartLite *)CallbackRef;

	//读取状态寄存器
	isr_status = XUartLite_ReadReg(InstancePtr->RegBaseAddress ,
			XUL_STATUS_REG_OFFSET);
	if(isr_status & RX_NOEMPTY){ //接收 FIFO 中有数据
		//读取数据
		Read_data=XUartLite_ReadReg(InstancePtr->RegBaseAddress ,
				XUL_RX_FIFO_OFFSET);
		//发送数据
		XUartLite_WriteReg(InstancePtr->RegBaseAddress ,
				XUL_TX_FIFO_OFFSET, Read_data);
	}
}


void LED_Init(void){
	//AXI_GPIO器件初始化
	XGpio_Initialize(&LED_Gpio, LED_DEV_ID);
	//为指定的GPIO信道设置所有独立信号的输入/输出方向
	XGpio_SetDataDirection(&LED_Gpio, 1, 0);
	//设置LED初始值
	XGpio_DiscreteWrite(&LED_Gpio, 1, 0xffff);
}

void KEY_Init(void){
	//AXI_GPIO器件初始化
	XGpio_Initialize(&KEY_Gpio, KEY_DEV_ID);
	//设置LED初始值
	XGpio_SetDataDirection(&KEY_Gpio, 1, 1);
}

void interrupt_KEY_LED_test(void){
    if(key_intr_flag){        //检测中断标志信号有效
        key_value = XGpio_DiscreteRead(&KEY_Gpio, 1);  //读取按键值
        if(key_value == 0){             //检测按键是否按下
            if(Intr_times == 0)         //根据有效中断数点亮LED
                led_value = 0x01;
            else if(Intr_times == 1)
                led_value = 0x02;
            else if(Intr_times == 2)
                led_value = 0x04;
            else
                led_value = 0x08;
        //按键按下后点亮对应LED灯
        XGpio_DiscreteWrite(&LED_Gpio, 1, led_value);
        xil_printf("i = %d\r\n",Intr_times);  //打印当前的Intr_times
        Intr_times = (Intr_times + 1)%4;    //将计数值约束在0到3之间
        //延迟1秒
        sleep(1);
        }
    key_intr_flag = 0;              //中断标志清零
    }
}

void GpioHandler(void *CallbackRef){
    XGpio *GpioPtr = (XGpio *)CallbackRef;
        key_intr_flag = 1;                   //接收到中断，标志信号拉高
        XGpio_InterruptDisable(GpioPtr, 1);  //关闭中断
        XGpio_InterruptClear(GpioPtr, 1);    //清除中断
        XGpio_InterruptEnable(GpioPtr, 1);   //使能中断
}

int Initialization_IIC(void) {

	int Status;
	XIic_Config *ConfigPtr;
	ConfigPtr = XIic_LookupConfig(IIC_DEVICE_ID);

	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XIic_CfgInitialize(&Iic, ConfigPtr, ConfigPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIic_SetRecvHandler(&Iic,&Iic, (XIic_Handler) RecvHandler);
	XIic_SetStatusHandler(&Iic,&Iic, (XIic_StatusHandler) StatusHandler);
	XIic_SetSendHandler(&Iic,&Iic, (XIic_Handler) SendHandler);
	Status = SetupInterruptSystem(&Iic);

	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}
	Status = XIic_SetAddress(&Iic, XII_ADDR_TO_SEND_TYPE, TEMP_SENSOR_ONCHIP_ADDRESS);
	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

static void RecvHandler(XIic *InstancePtr)
{
        ReceiveComplete = 0;
}

static void StatusHandler(void *CallbackRef, int Status)
{
}
static void SendHandler(XIic *InstancePtr)
{
        TransmitComplete = 0;
}

static int SetupInterruptSystem(XIic *IicPtr)
{
        int Status;
        Status = XIntc_Initialize(&InterruptController,INTC_DEVICE_ID);
        if (Status != XST_SUCCESS){
                return XST_FAILURE;
        }
        Status = XIntc_Connect(&InterruptController, INTC_IIC_INTERRUPT_ID,
                        (XInterruptHandler)XIic_InterruptHandler, IicPtr);
        if (Status != XST_SUCCESS){
                return XST_FAILURE;
        }
        Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
        if (Status != XST_SUCCESS){
                return XST_FAILURE;
        }
        XIntc_Enable(&InterruptController, INTC_IIC_INTERRUPT_ID);
        if (Status != XST_SUCCESS){
                        return XST_FAILURE;
        }
        Xil_ExceptionInit();
        Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                        (Xil_ExceptionHandler)XIntc_InterruptHandler,
                        &InterruptController);
        Xil_ExceptionEnable();
        return XST_SUCCESS;
}
