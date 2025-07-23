#ifndef DRIVER_GPIO_H
#define DRIVER_GPIO_H

#include <QObject>

#define ESM6800_DEV_MAJOR				251
#define ESM6800_SYSINFO_MINOR			0					//ESM6800_sysinfo
#define ESM6800_ISA_MINOR				1					//ESM6800_isa
#define ESM6800_GPIO_MINOR				2					//ESM6800_gpio
#define ESM6800_KEYPAD_MINOR		    	3					//ESM6800_keypad
#define ESM6800_IRQ1_MINOR				4					//ESM6800_irq1
#define ESM6800_IRQ2_MINOR				5					//ESM6800_irq2
#define ESM6800_IRQ3_MINOR				6					//ESM6800_irq3
#define ESM6800_IRQ4_MINOR				7					//ESM6800_irq4

#define ESM6800_PWM1_MINOR				8					//ESM6800_pwm1(->GPIO12 of ESM6800)
#define ESM6800_PWM2_MINOR				9					//ESM6800_pwm2(->GPIO14 of ESM6800)
#define ESM6800_PWM3_MINOR				10					//ESM6800_pwm3(->GPIO15 of ESM6800)
#define ESM6800_PWM4_MINOR				11					//ESM6800_pwm4(->GPIOX9 of ESM6800)
#define ESM6800_LCD_MINOR				12					//ESM6800_lcd
#define ESM6800_SPI_MINOR				13					//ESM6800_spi(use hardware SPI1)
#define ESM6800_AD_MINOR			    	        14					//ESM6800_ad


#define ESM6800_BOARD_TYPE_ESM6800		0
#define ESM6800_BOARD_TYPE_EM9281		1
#define ESM6800_BOARD_TYPE_EM9380		2
#define ESM6800_BOARD_TYPE_EM9283		3
#define ESM6800_BOARD_TYPE_EM9287		7



/*
* Emlinix JUN-2-2010: double input parameters can be used in more than one driver
*/
struct double_pars
{
    unsigned int	par1;
    unsigned int	par2;
};


/*
* Emlinix JUN-3-2010: struct for lcd driver
*/
struct lcd_line
{
    unsigned int	type;			// = 0: point; = 1: line; = 2: bar
    unsigned int	x0;
    unsigned int	y0;
    unsigned int	x1;
    unsigned int	y1;
    unsigned int 	color;			// = 0: write "0";  = 1: write "1", = 2: xor operation
};

struct lcd_block
{
    unsigned int 	x0;
    unsigned int	y0;
    unsigned int	xsize;			// = 1 - 8; left alignment
    unsigned int	ysize;			// = 1 - 16;
    unsigned char	data[16];		// block data to be copied
};

struct spi_config
{
    unsigned int	uCPOL;			// clock polarity = 0: ; = 1:
    unsigned int	uCPHA;			// clock phase = 0: ; = 1:
    unsigned int	data_bit;		// = 8, 9, .. 16
    unsigned int	baudrate;		// < 100000000bps => 100Mbps
};

struct daq_info
{
    unsigned int	cmd;			// = 0, 1, 2, ....
    unsigned int	data;			// data value
    char			unit[16];			// return unit string: "Voltage", "Kalvin"
};


/* CS&ZHL FEB26-2014: add structure for ESM6800_pwm */
struct pwm_config_info
{
    /*unsigned int		cmd;			*/// = 0, 1, 2, ....
    unsigned int		freq; 			/* in Hz */
    unsigned int		duty;			/* in % */
    unsigned int		polarity;
    unsigned int		count;		/* LGZ APR-28-2016:  */
};

//LGZ_start April-22-2016
typedef struct cap_config_info
{
    unsigned int dwPolarity;
    unsigned int dwMaxFreq;
    unsigned long dwTimeUs;
    unsigned int dwCount;
    unsigned int dwPeriodNs;
    unsigned int dwWidthNs;
}CAP_INFO, *PCAP_INFO;

/*
* Emlinix FEB-15-2010: ioctl cmd code definitions:
*/
#define ESM6800_MAGIC								ESM6800_DEV_MAJOR

#define ESM6800_SYSINFO_IOCTL_GET_DBGSL			_IOR(ESM6800_MAGIC,  0x00, unsigned int)
#define ESM6800_SYSINFO_IOCTL_GET_BOARDTYPE		_IOR(ESM6800_MAGIC,  0x01, unsigned int)
#define ESM6800_SYSINFO_IOCTL_GET_VID				_IOR(ESM6800_MAGIC,  0x02, unsigned int)
#define ESM6800_SYSINFO_IOCTL_GET_UID				_IOR(ESM6800_MAGIC,  0x03, unsigned int)
#define ESM6800_SYSINFO_IOCTL_GET_BOOTSTATUS		_IOR(ESM6800_MAGIC,  0x04, unsigned int)

#define ESM6800_GPIO_IOCTL_OUT_ENABLE				_IOW(ESM6800_MAGIC,  0x60, unsigned int)
#define ESM6800_GPIO_IOCTL_OUT_DISABLE			_IOW(ESM6800_MAGIC,  0x61, unsigned int)
#define ESM6800_GPIO_IOCTL_OUT_SET					_IOW(ESM6800_MAGIC,  0x62, unsigned int)
#define ESM6800_GPIO_IOCTL_OUT_CLEAR				_IOW(ESM6800_MAGIC,  0283, unsigned int)
#define ESM6800_GPIO_IOCTL_OPEN_DRAIN				_IOW(ESM6800_MAGIC,  0284, unsigned int)
#define ESM6800_GPIO_IOCTL_PIN_STATE				_IOR(ESM6800_MAGIC,  0285, unsigned int)

#define ESM6800_CAN_IOCTL_START_CHIP				_IO(ESM6800_MAGIC,   0xa0)
#define ESM6800_CAN_IOCTL_STOP_CHIP				_IO(ESM6800_MAGIC,   0xa1)
#define ESM6800_CAN_IOCTL_SET_BAUD				_IOW(ESM6800_MAGIC,  0xa2, unsigned int)
#define ESM6800_CAN_IOCTL_SET_FILTER				_IOW(ESM6800_MAGIC,  0xa3, struct accept_filter)
#define ESM6800_CAN_IOCTL_GET_ERRORCODE 			_IOR(ESM6800_MAGIC,  0xa4, unsigned int)
#define ESM6800_CAN_IOCTL_READ_REG				_IOWR(ESM6800_MAGIC, 0xa5, struct isa_io)
#define ESM6800_CAN_IOCTL_GET_STATISTICS			_IOR(ESM6800_MAGIC,  0xa6, struct drv_statistics)
#define ESM6800_CAN_IOCTL_SET_SELFTEST			_IOW(ESM6800_MAGIC,  0xa7, unsigned int)
#define ESM6800_CAN_IOCTL_GET_ECCREG 				_IOR(ESM6800_MAGIC,  0xa8, unsigned int)
//#define ESM6800_CAN_IOCTL_CLEAR_RXBUF				_IO(ESM6800_MAGIC,   0xa9)
//#define ESM6800_CAN_IOCTL_CLEAR_TXBUF				_IO(ESM6800_MAGIC,   0xaa)
//#define ESM6800_CAN_IOCTL_LAST_TIMESTAMP			_IOR(ESM6800_MAGIC,  0xab, struct timeval)

#define ESM6800_PWM_IOCTL_START					_IOW(ESM6800_MAGIC,  0xb0, struct double_pars)
#define ESM6800_PWM_IOCTL_STOP						_IO(ESM6800_MAGIC,   0xb1)
#define ESM6800_COUNT_IOCTL_START					_IOW(ESM6800_MAGIC,  0xb2, struct double_pars)
#define ESM6800_COUNT_IOCTL_STOP					_IO(ESM6800_MAGIC,   0xb3)

#define ESM6800_I2C_IOCTL_CONFIG					_IOW(ESM6800_MAGIC,  0xc0, struct i2c_config)
#define ESM6800_I2C_IOCTL_WRITE					_IOW(ESM6800_MAGIC,  0xc1, struct i2c_io)
#define ESM6800_I2C_IOCTL_READ						_IOWR(ESM6800_MAGIC, 0xc2, struct i2c_io)

#define ESM6800_LCD_IOCTL_TYPE						_IOW(ESM6800_MAGIC,  0xd0, unsigned int)
#define ESM6800_LCD_IOCTL_LINE						_IOW(ESM6800_MAGIC,  0xd1, struct lcd_line)
#define ESM6800_LCD_IOCTL_BLOCK					_IOW(ESM6800_MAGIC,  0xd2, struct lcd_block)
#define ESM6800_LCD_IOCTL_CLEAR					_IO(ESM6800_MAGIC,   0xd3)
#define ESM6800_LCD_IOCTL_UPDATE					_IO(ESM6800_MAGIC,   0xd4)

#define ESM6800_SPI_IOCTL_GET_CONFIG				_IOR(ESM6800_MAGIC,  0xe0, struct spi_config)
#define ESM6800_SPI_IOCTL_SET_CONFIG				_IOW(ESM6800_MAGIC,  0xe1, struct spi_config)

#define ESM6800_AUART_MAJOR	242
#define ESM6800_IOCTL_SET_RTS_PIN					_IOR(ESM6800_AUART_MAJOR,  0x00, unsigned int)


#define	GPIO0		(1 <<  0)
#define	GPIO1		(1 <<  1)
#define	GPIO2		(1 <<  2)
#define	GPIO3		(1 <<  3)
#define	GPIO4		(1 <<  4)
#define	GPIO5		(1 <<  5)
#define	GPIO6		(1 <<  6)
#define	GPIO7		(1 <<  7)
#define	GPIO8		(1 <<  8)
#define	GPIO9		(1 <<  9)
#define	GPIO10		(1 << 10)
#define	GPIO11		(1 << 11)
#define	GPIO12		(1 << 12)
#define	GPIO13		(1 << 13)
#define	GPIO14		(1 << 14)
#define	GPIO15		(1 << 15)
#define	GPIO16		(1 << 16)
#define	GPIO17		(1 << 17)
#define	GPIO18		(1 << 18)
#define	GPIO19		(1 << 19)
#define	GPIO20		(1 << 20)
#define	GPIO21		(1 << 21)
#define	GPIO22		(1 << 22)
#define	GPIO23		(1 << 23)
#define	GPIO24		(1 << 24)
#define	GPIO25		(1 << 25)
#define	GPIO26		(1 << 26)
#define	GPIO27		(1 << 27)
#define	GPIO28		(1 << 28)
#define	GPIO29		(1 << 29)
#define	GPIO30		(1 << 30)
#define	GPIO31		(1 << 31)
#define	GPIOX_FLAG	(1 << 31)

#define	ESM6800_GPIO_OUTPUT_ENABLE		0
#define	ESM6800_GPIO_OUTPUT_DISABLE		1
#define	ESM6800_GPIO_OUTPUT_SET			2
#define	ESM6800_GPIO_OUTPUT_CLEAR		3
#define	ESM6800_GPIO_INPUT_STATE		5

#define 	ESM6800_PWM_START					10
#define 	ESM6800_PWM_STOP					11
#define 	ESM6800_COUNT_START				12
#define 	ESM6800_COUNT_STOP					13


//--------------------------------------------------------------------------------------------------------
// CS&ZHL JAN-11-2013: definitions for AD in ESM6800
//
#define	ESM6800_DAQ_VOLTAGE_CH0			0
#define	ESM6800_DAQ_VOLTAGE_CH1			1
#define	ESM6800_DAQ_VDD_5V				2
#define	ESM6800_DAQ_VDDIO_3V3				3
#define	ESM6800_DAQ_VDDA_1V8				4
#define	ESM6800_DAQ_VDDD_1V45			5
#define	ESM6800_DAQ_CPU_TEMPERATURE		6
#define	ESM6800_DAQ_BOARD_TEMPERATURE	7


/* app flags definition */
#define APP_FLAG_DEBUG_MODE				0x00000001
#define APP_FLAG_V3_MODE					0x00000002
#define APP_FLAG_CFG0						0x00000004
#define APP_FLAG_CFG1						0x00000008
#define APP_FLAG_LSTARTUP_END				0x80000000
#define APP_FLAG_DEFAULT_CFG				(APP_FLAG_CFG1 | APP_FLAG_CFG0)
#define APP_FLAG_INIT_MODE					(APP_FLAG_DEBUG_MODE | APP_FLAG_V3_MODE | APP_FLAG_DEFAULT_CFG)


/*CS&ZHL  JUN-05-2013: definitions for ISO7816 mode & RTS_TOGGLE */

#define ESM6800_ISO7816_T0				(1 << 27)				// D27 of termios.c_cflag
#define ESM6800_ISO7816_T1				(1 << 26)				// D26 of termios.c_cflag
#define ESM6800_TTYS1_RTS_ENABLE		(1 << 25)				// D25 of termios.c_cflag
#define ESM6800_TTYS1_DTR_ENABLE		(1 << 24)				// D24 of termios.c_cflag
#define ESM6800_ISO7816_RXD_RESET		(1 << 23)				// D23 of termios.c_cflag
#define ESM6800_RTS_CONTROL_TOGGLE	(1 << 15)				// D15 of termios.c_cflag
#define ESM6800_ISO7816_MODE			(ESM6800_ISO7816_T0 | ESM6800_ISO7816_T1)

struct isa_mode
{
    unsigned int cmd;				/* = 0, 1, 2, 3, 4 */
    unsigned int data_reg;			/* offset of data register */
    unsigned int data_len;			/* len of data in byte */
    unsigned int eoi_reg;			/* offset of EOI register, INVALID = -1 */
    unsigned int eoi_val;				/* value of EOI */
};

struct isa_transfer
{
    const void	*tx_buf;				/* != NULL: write, only one pointer allowed to be valid */
    void		*rx_buf;				/* != NULL: read, only one pointer allowed to be valid */
    unsigned	len;					/* length in byte */
    unsigned	width;				/* data width, = 16: -> 16-bit, otherwise -> 8-bit */
    unsigned	offset;				/* offset = 0x00 .. 0xFF */
    unsigned	inc;					/* = 0: fixed offset, = 1: offset+1 after r/w */
};

#define ESM6800_ISA_IOCTL_START					_IOW(ESM6800_MAGIC,  0x20, struct isa_mode)
#define ESM6800_ISA_IOCTL_STOP					_IO(ESM6800_MAGIC,  0x21)

/* CS&ZHL DEC-19-2014: try to use spi-style isa read/write sequence */
/* not all platforms use <asm-generic/ioctl.h> or _IOC_TYPECHECK() ... */
#define ISA_MSGSIZE(N) \
    ((((N)*(sizeof (struct isa_transfer))) < (1 << _IOC_SIZEBITS)) \
        ? ((N)*(sizeof (struct isa_transfer))) : 0)
#define ISA_IOC_MESSAGE(N) 						_IOW(ESM6800_MAGIC, 0x22, char[ISA_MSGSIZE(N)])

#define ESM6800_IRQ_IOCTL_GET_COUNT				_IOR(ESM6800_MAGIC,  0x80, unsigned int)
#define ESM6800_IOCTL_SET_IDLE						_IOR(EM6800_MAGIC,  0x81, unsigned int)
#define ESM6800_IOCTL_GET_IDLESTATE				_IOR(ESM6800_MAGIC,  0x82, unsigned int)

/* ESM6800 ISA Block R/W commands */
#define ISA_POLL_RW					(1 << 0)
#define ISA_POLL_RW16				(1 << 1)
#define ISA_IRQ_RD					(1 << 2)
#define ISA_IRQ_WE					(1 << 3)
#define ISA_IRQ_RD16					(1 << 4)
#define ISA_IRQ_WE16					(1 << 5)
#define ISA_DMA_RD					(1 << 6)
#define ISA_DMA_RD16					(1 << 7)


class Driver_GPIO : public QObject
{
    Q_OBJECT
public:
    explicit Driver_GPIO(QObject *parent = nullptr);

signals:

};

#endif // DRIVER_GPIO_H
