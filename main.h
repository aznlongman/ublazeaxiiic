#include "xparameters.h"
#include "xintc.h"
#include "xgpio.h"
#include "sleep.h"
#include "xil_exception.h"
#include "xdebug.h"
#include "xuartlite.h"
#include "xuartlite_l.h"
#include "xstatus.h"
#include "seven_segment_led.h"

#include "xiic_l.h"
#include "xiic.h"


#define KEY_DEV_ID       XPAR_AXI_GPIO_1_DEVICE_ID  //按键 AXI GPIO ID
#define LED_DEV_ID       XPAR_AXI_GPIO_0_DEVICE_ID  //LED AXI GPIO ID
#define INTC_DEVICE_ID   XPAR_INTC_0_DEVICE_ID      //中断控制器ID
#define EXCEPTION_ID     XIL_EXCEPTION_ID_INT       //中断异常ID
#define AXI_GPIO_INTR_ID XPAR_INTC_0_GPIO_1_VEC_ID  //AXI GPIO中断ID

#define UART_DEVICE_ID XPAR_UARTLITE_0_DEVICE_ID //串口器件 ID
#define UART_INTR_ID XPAR_INTC_0_UARTLITE_0_VEC_ID //串口中断 ID

#define RX_NOEMPTY XUL_SR_RX_FIFO_VALID_DATA // 接收 FIFO 非空

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_BASE_ADDRESS	XPAR_IIC_0_BASEADDR
#define IIC_DEVICE_ID XPAR_IIC_0_DEVICE_ID

#define SR XIIC_SR_REG_OFFSET
#define RX_FIFO XIIC_DRR_REG_OFFSET
#define TX_FIFO XIIC_DTR_REG_OFFSET

/*
 * The following constant defines the address of the IIC
 * temperature sensor device on the IIC bus.  Note that since
 * the address is only 7 bits, this  constant is the address divided by 2.
 */
//#define TEMP_SENSOR_ONCHIP_ADDRESS  0x18 /* The actual address is 0x30 */
//#define TEMP_SENSOR_AMBIENT_ADDRESS 0x4B /* The actual address is 0x96 */

#define TEMP_SENSOR_ONCHIP_ADDRESS  0x4B /* The actual address is 0x48 */
#define TEMP_SENSOR_AMBIENT_ADDRESS 0x0B /* The actual address is 0x96 */


void interrupt_KEY_LED_test(void);
void GpioHandler(void *CallbackRef);
void LED_Init(void);
void KEY_Init(void);
void Interrupt_Init(void);
void uart_handler(void *CallbackRef);
void UART_Init(void);
void Seven_Segment_Custom_IP_Test(void);

int Initialization_IIC(void);
static void RecvHandler(XIic *InstancePtr);
static void StatusHandler(void *CallbackRef, int Status);
static void SendHandler(XIic *InstancePtr);
static int SetupInterruptSystem(XIic *IicPtr);
