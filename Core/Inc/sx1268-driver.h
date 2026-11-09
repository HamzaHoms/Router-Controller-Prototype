/*
 * sx1268-driver.h
 *
 *  Created on: Oct 27, 2026
 *      Author: Hamza
 */

#ifndef INC_SX1268_DRIVER_H_
#define INC_SX1268_DRIVER_H_

#include "stm32f1xx.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#define TX_DMA_SIZE 256
#define RX_DMA_SIZE 256

//spi timeout
#define SX1268_SPITO 200

//Register Addresses
#define LoRa_Sync_Word_Start 		0x0740

//command opcodes
#define NOP 						0x00
#define WriteBuffer 				0x0E
#define ReadBuffer					0x1E
#define GetStatus	 				0xC0
#define SetDioIrqParams	  			0x08
#define SetDIO2AsRfSwitchCtrl	 	0x9D
#define SetDIO3AsTCXOCtrl	 		0x97
#define CalibrateFunction	 		0x89
#define CalibrateImage				0x98
#define SetRfFrequency				0x86
#define SetPaConfig				0x95
#define SetModulationParams		0x8B
#define SetPacketParams			0x8C
#define SetCadParams				0x88
#define GetIrqStatus 				0x12
#define ClearIrqStatus				0x02
#define SetPacketType				0x8A
#define SetStandby					0x80
#define SetBufferBaseAddress			0x8F
#define GetRxBufferStatus			0x13
#define GetPacketStatus 				0x14
#define GetRssiInst					0x15
#define SetTx						0x83
#define SetRx						0x82
#define WriteRegister 				0x0D
#define ReadRegister 				0x1D
#define SetCAD					0xC5
#define SetTxParams				0x8E

//DIO pin macros
#define DIO1 0
#define DIO2 1
#define DIO3 2

//IRQ source bitmasks
#define IRQ_TXDONE			 (uint16_t)0x0001
#define IRQ_RXDONE 			 (uint16_t)0x0002
#define IRQ_PREAMBLEDETECTED   (uint16_t)0x0004
#define IRQ_SYNCWORDVALID 	 (uint16_t)0x0008
#define IRQ_HEADERVALID 		 (uint16_t)0x0010
#define IRQ_HEADERERR 		 (uint16_t)0x0020
#define IRQ_CRCERROR 			 (uint16_t)0x0040
#define IRQ_CADDONE 			 (uint16_t)0x0080
#define IRQ_CADDETECTED 		 (uint16_t)0x0100
#define IRQ_TIMEOUT			 (uint16_t)0x0200
#define IRQ_ALL 				 (uint16_t)0x03FF

//Power Amplifier Presets
  //Duty Cycle:
#define PA_DC_0 				0x00
#define PA_DC_1				0x01
#define PA_DC_2				0x02
#define PA_DC_3				0x03
#define PA_DC_4				0x04
#define PA_DC_MaxVal			PA_DC_4

  //hpMax
#define PA_HPmax_0			0x00
#define PA_HPmax_1			0x01
#define PA_HPmax_2			0x02
#define PA_HPmax_3			0x03
#define PA_HPmax_4			0x04
#define PA_HPmax_5			0x05
#define PA_HPmax_6			0x06
#define PA_HPmax_7			0x07
#define PA_HPmax_MaxVal		PA_HPmax_7

  //Default constant values for deviceSel and paLut
  // !!! DO NOT CHANGE THESE TWO !!!
#define PA_DevSel_Default 		0x00
#define PA_PaLut_Default		0x01

  // Power Amplifier Ramp Times
#define RAMP_10U    0x00
#define RAMP_20U    0x01
#define RAMP_40U    0x02
#define RAMP_80U    0x03
#define RAMP_200U   0x04
#define RAMP_800U   0x05
#define RAMP_1700U  0x06
#define RAMP_3400U  0x07

//Image Rejection Window
//This is the value that is added and subtracted to the used frequency to find the upper and lower limits of the image reject window
//The value is in MHz
#define ImjRejWind_Val 			0x08

//LoRa Modulation Parameter Presets
  //Spread Factor
#define LoRa_SF5 				0x05
#define LoRa_SF6 				0x06
#define LoRa_SF7 				0x07
#define LoRa_SF8 				0x08
#define LoRa_SF9 				0x09
#define LoRa_SF10 				0x0A
#define LoRa_SF11				0x0B
#define LoRa_SF12				0x0C
#define LoRa_SF_Min			LoRa_SF5
#define LoRa_SF_Max			LoRa_SF12

  //Bandwidth
#define LoRa_BW_7				0x00
#define LoRa_BW_10			0x08
#define LoRa_BW_15			0x01
#define LoRa_BW_20			0x09
#define LoRa_BW_31			0x02
#define LoRa_BW_41			0x0A
#define LoRa_BW_62			0x03
#define LoRa_BW_125			0x04
#define LoRa_BW_250			0x05
#define LoRa_BW_500			0x06
#define LoRa_BW_Undefined		0xFF

  //Coding Rate
#define LoRa_CR_4_5			0x01
#define LoRa_CR_4_6			0x02
#define LoRa_CR_4_7			0x03
#define LoRa_CR_4_8			0x04
#define LoRa_CR_4_5_LI			0x05
#define LoRa_CR_4_6_LI			0x06
#define LoRa_CR_4_8_LI			0x07

//LoRa Packet Parameter Presets
  //Header Type
#define LoRa_VariableLenHeader	0x00
#define LoRa_FixedLenHeader 	0x01

  //CRC
#define LoRa_CRC_OFF			0x00
#define LoRa_CRC_ON			0x01

  //IQ Invert
#define LoRa_IQ_Standard 		0x00
#define LoRa_IQ_Inverted 		0x01

//CAD Parameter Presets
  //CAD Symbol Number
#define CAD_ON_1_SYMB 		0x00
#define CAD_ON_2_SYMB 		0x01
#define CAD_ON_4_SYMB 		0x02
#define CAD_ON_8_SYMB 		0x03
#define CAD_ON_16_SYMB 		0x04

//TCXO Voltage Presets
#define TCXO_1v6 				0x00
#define TCXO_1v7 				0x01
#define TCXO_1v8 				0x02
#define TCXO_2v2 				0x03
#define TCXO_2v4 				0x04
#define TCXO_2v7 				0x05
#define TCXO_3v0 				0x06
#define TCXO_3v3 				0x07


//Access macros for deeply embedded struct elements and LUTs
#define SX68_LORA_(dev_pointer ,element) dev_pointer->CONTROL.LoRa_CONFIG.element
#define SX68_OPMODE_(dev_pointer) dev_pointer->CONTROL._dev_mode
#define SX68_RFTYPE_(dev_pointer) dev_pointer->CONTROL._dev_rf_type
#define SX68_BUS_STS_(dev_pointer) dev_pointer->GPIO._bus_state


typedef enum SX1268_Status{
  SX1268_OK = 0,
  SX1268_ERROR = 1,
  SX1268_RX_DONE = 2,
  SX1268_CMD_TIMEOUT = 3,
  SX1268_CMD_ERROR = 4,
  SX1268_DEVICE_ERROR = 5,
  SX1268_TX_DONE = 6,
  SX1268_BUSY = 7
} SX1268_Status;

typedef enum SX1268_Mode{
  MODE_SLEEP = 0,
  MODE_STDBY_RC = 2,
  MODE_STDBY_XOSC = 3,
  MODE_FS = 4,
  MODE_RX = 5,
  MODE_TX = 6
}SX1268_Mode;

typedef enum SX1268_Type{
  TYPE_NONE = 0x0F,
  TYPE_LoRa = 0x01,
  TYPE_FSK = 0x00,
  TYPE_FH_LRSS = 0x03
}SX1268_Type;

typedef struct GPIO_t
{
  GPIO_TypeDef* _port;
  uint16_t _pin;
} GPIO_t;



typedef struct SX1268
{

  //struct that contains the different gpio pins and drivers related to the sx1268
  //_hspi : is a pointer to the spi handler that the sx1268 is on
  //_cs_gpio : is the gpio port and pin of the chip select or nss pin that the sx1268 is on
  //_nrst_gpio : is the gpio port and pin of the line that the reset pin of the sx1268 is on
  //_bind_gpio : is the gpio port and pin of the line that the busy indicator pin of the sx1268 is on
  //_bus_state : is the current occupation state of the bus, is set and reset by the DMA using functions
  // which are the ReadBuffer and WriteBuffer functions, but the CmdTXRX functions poll for the bus to be empty
  // CmdTXRX functions are blocking so bus locking is not necessary for them, but they still need to poll the bus in case of an active DMA
  //
  //note: _hspi, _cs_gpio, and _bind_gpio HAVE to be set the correct pins before the device can be used
  struct {
    SPI_HandleTypeDef* _hspi;
    GPIO_t _cs_gpio;
    GPIO_t _nrst_gpio;
    GPIO_t _bind_gpio;
    volatile HAL_StatusTypeDef _bus_state;
  } GPIO;

  //struct that contains the gpio definitions of the dio lines of the sx1268
  //_diox_gpio : the gpio port and pin of the line that that specific dio pin of the sx1268 is connected to
  //note: that atleast one dio pin must be set as an irq source for all interrupts for the driver to work
  struct {
    GPIO_t _dio1_gpio;
    GPIO_t _dio2_gpio;
    GPIO_t _dio3_gpio;
  } DIO;

  //struct that contains the definitions related to the irqs of the device
  //_irq_flags : a 16-bit variable where each bit is equal to a specific flag as defined by the sx1268 datasheet on page 84/117 on table 13-29
  //_irqx_flag : an array of boolean flags that are set when the corresponding irq from diox pin is set
  struct {
    uint16_t _irq_flags;
    volatile uint8_t _irqx_flag[3];
  } IRQ;

  //struct that contains the definitions related to the tx and rx buffers of the dma line and their statuses
  //_tx/rx_dma_buf : the definition of the memory buffer that will be used for dma
  //_tx/rx_buf_offset : the index at which the tx/rx chunk starts in the SX1268's memory
  //_tx/rx_pckt_size : the size of the to-be-sent/last-received packet that is sitting in the dma buffers
  //_tx/rx_buf_sts : the status of that specific buffer, writing vs reading vs empty, so that the data in it isn't corrupted.
  struct {
    uint8_t _tx_dma_buf[TX_DMA_SIZE];
    uint8_t _rx_dma_buf[RX_DMA_SIZE];

    uint8_t _tx_buf_offset;
    uint8_t _rx_buf_offset;

    uint8_t _tx_pckt_size;
    uint8_t _rx_pckt_size;

    volatile SX1268_Status _tx_buf_sts;
    volatile SX1268_Status _rx_buf_sts;
  } DATA;

  //struct that contains some variables used for getting the status of the device
  //_dev_sts : shows the overall status of the device , for example: ok, busy, error, etc.
  //_dev_mode : shows the current mode of operation of the device, for example: stdy_rc, tx, rx, fs, etc.
  //_dev_rf_type : shows the current type of modulation and packets that is currently active, for example: LoRa, GFSK, LR_FHSS, etc.
  //for the operational modes chech section {9.} of the datasheet starting at page 59/117.
  struct {
    SX1268_Status _cmd_sts;
    SX1268_Mode _dev_mode;
    SX1268_Type _dev_rf_type;

    uint8_t _pa_hpmax;
    uint8_t _pa_dutycycle;
    uint32_t _rf_frequency;

    struct{
      uint8_t _sf;
      uint8_t _bw;
      uint8_t _cr;
      uint8_t _ldro;

      uint16_t _preamble_len;
      uint8_t _header_type;
      uint8_t _payload_len;
      uint8_t _crc_en;
      uint8_t _iq_inv;
    } LoRa_CONFIG;

  } CONTROL;

} SX1268;


//Utility functions and Lookup tables

//a small utility function that acts as a look-up table for converting a bandwidth config into it's actual value in hertz
uint32_t _SX68_LoRaBW_LUT(
uint8_t bw_config //the configuration macro of the lora bandwidth setting
);

//a utility function that gives the best CadDetPeak, CadDetMin, and CadSymbolNum for the provided SF
//the values are pulled from table 4.1.1.1 "Best Settings" in AN1200.48 Application notes
void _SX68_LoRaCadParams_LUT(
uint8_t sf, //the spread factor input
uint8_t* peak, //pointer to the variable that will be populated by the ideal CadDetPeak value
uint8_t* min, //pointer to the variable that will be populated by the ideal CadDetMin value
uint8_t* symb_num //pointer to the variable that will be populated by the ideal CadSymbolNum value
);

//Lower Level Functions=========================================================================

//Sets the chip select line to the cs_state value
void _SX68_SetCS(
SX1268* dev, //the target device struct
uint8_t cs_state //the target state : GPIO_PIN_SET, GPIO_PIN_RESET
);

//resets the SX1268 with a 1ms wait time in reset state
void _SX68_Reset(
SX1268* dev
);

//takes the STATUS byte returned by the SX1268 during normal ops and splits it into a "last known mode" and "previous command status"
//sets the ' _cmd_sts' to the previous command status and '_dev_mode' to the last known mode
//also returns the '_cmd_sts' value
SX1268_Status _SX68_CheckStatus(
SX1268* dev,
uint8_t status
);

//reads the busy pin of the SX1268 and returns the value
GPIO_PinState _SX68_ReadBIND(
SX1268* dev //the target device struct
);

//the main transaction function, responsible for sending a command with opcode 'opcode', arguments 'args' of length 'arg_len'
//, and returns the expected 'ret_len' of return values into 'ret_val' buffer
//returns SX1268_OK if everything went well, returns SX1268_ERROR if an SPI error occured
// !!!DOES NOT CHECK THE STATUS BYTES, USE THE GUARDED VERSION FOR THAT!!!
SX1268_Status _SX68_CmdTXRX(
SX1268* dev, //the target device struct
uint8_t opcode, //the opcode of the command to be sent, {datasheet section 11.}
uint8_t* args, //the buffer of arguments to be sent alongside the command
uint8_t arg_len, //the number of bytes of the arguments in the
uint8_t* ret_val, //a buffer for the returned values from the command to be put into, will also be checked for status errors
uint8_t ret_len //the number of bytes of expected return values
);

//the guarded version of the command transaction function, calls the "GetStatus()" command after the main command is sent
//checks if the command succeeded or not
//use sparingly since it blocks the CPU for longer
SX1268_Status _SX68_CmdTXRX_G(
SX1268* dev, //the target device struct
uint8_t opcode, //the opcode of the command to be sent, {datasheet section 11.}
uint8_t* args, //the buffer of arguments to be sent alongside the command
uint8_t arg_len, //the number of bytes of the arguments in the
uint8_t* ret_val, //a buffer for the returned values from the command to be put into, will also be checked for status errors
uint8_t ret_val_len //the number of bytes of expected return values
);

//A function used for writing to the buffer of the SX1268, it writes 'len' bytes from 'buf_out' to the SX1268's buffer at 'offset'
SX1268_Status _SX68_WriteBuffer(
SX1268* dev, //the target device struct
uint8_t* buf_out, //pointer to the buffer where the data will be pulled from
uint8_t len, //the amount of data to be sent in bytes
uint8_t offset //the offset to start writing to on the sx1268's memory
);

//A function used for reading the buffer of the SX1268, it reads 'len' bytes, out of the SX1268's buffer at 'offset' and into 'buf_in'
SX1268_Status _SX68_ReadBuffer(
SX1268* dev, //the target device struct
uint8_t* buf_in, //pointer to the buffer where the data will be pulled in to
uint8_t len, //the amount of data to be pulled in bytes
uint8_t offset //the offset to start reading from on the sx1268's memory
);

//Note: Both Buffer functions read and write to the same buffer, so they could over-write each other or over-write a received packet if not careful

SX1268_Status _SX68_WriteRegister(
SX1268* dev, //the target device struct
uint16_t reg_adr, //address of the target register
uint8_t* data, //the data to be written
uint8_t data_size //the size of the data to be written
);

SX1268_Status _SX68_ReadRegister(
SX1268* dev, //the target device struct
uint16_t reg_adr, //address of the target register
uint8_t* data, //where the data will be read into
uint8_t data_size //the size of the data to be read
);

//Interfacing Functions==========================================================================


  //Operational Functions=========================================================================

//This function is used to initialize the SX1268, it initializes the struct with the relevant data
//It restarts the chip and holds it in the restart for 1ms
//it re-calibrates the chip

//it returns SX1268_OK if done ok
//it returns SX1268_ERROR in two cases:
// 1) the chip was held in the busy state for more than SX1268_SPITO ms
// 2) the re-calibration function failed
SX1268_Status SX68_Init(
SX1268* dev, //the target device struct
SPI_HandleTypeDef* hspi, //the hal spi handler that will be used
GPIO_t cs_gpio, //spi chips select / nss gpio port and pin
GPIO_t nrst_gpio, //sx1268 chip reset line gpio port and pin
GPIO_t bind_gpio //sx1268 busy indicator line gpio port and pin
);

//updates the _irq_flags mask in the IRQ struct of the SX1268 struct with the latest IRQ values from the SX1268
//does not automatically clear the IRQ masks of the SX1268, SX68_ClearIRQStatus must be called for that
//this function is most useful to be called when an interrupt from one of the diox pins is detected
SX1268_Status SX68_GetIRQStatus(
SX1268* dev //the target device struct
);

//clears the IRQ flags inside the SX1268, this must be called so that subsequent IRQs can be triggered
SX1268_Status SX68_ClearIRQStatus(
SX1268* dev //the target device struct
);

//clears the _irq_flagx boolean in the IRQ struct of the SX1268 struct
//this should be done before clearing the IRQ status of the flags on the SX1268 so that interrupts can correctly set the boolean flags
void SX68_ClearDIOStatus(
SX1268* dev, //the target device struct
uint8_t diox //the DIO IRQ flag to be cleared
);

//returns the value of the specific diox
//returns 0 for no irq, 1 for irq set
uint8_t SX68_GetDIOStatus(
SX1268* dev, //the target device struct
uint8_t diox //the DIO IRQ to be returned
);

//Note: With these 3 IRQ functions, the correct sequence of events fpr handling IRQs is as follows :
// 1) An IRQ is detected on the configured DIOx pin
// 2) the relevant IRQimpl function sets the _irq_flagx boolean flag
// 3) the interrupt exits
// 4) in the main loop or elsewhere, the user checks that the boolean flag is set
// 5) if the flag is set, the user calls SX68_GetIRQStatus to update the flags, and executes logic based on which interrupt was detected
// 6) the user clears the _irq_flagx boolean flag with the SX68_ClearDIOStatus function
// 7) finally, the user clears the SX1268's internal flags with the SX68_ClearIRQStatus function

//changes the operational RF type of the SX1268, this must be done while the SX1268 is in standby mode
SX1268_Status SX68_SetOPType(
SX1268* dev, //the target device struct
SX1268_Type type //LoRa vs FSK !!! FSK not supported currently !!!
);

//sets the SX1268 in standby mode
SX1268_Status SX68_SetStandby(
SX1268* dev, //the target device struct
SX1268_Mode stdby_type //stdby_RC vs stdby_xosc
);

//sets where the TX and RX chunks of the buffer begin in, this can be used to split the buffer into an RX section and TX section
//do not expect it to guard the buffer, if the RX packet exceeds the RX chunk size it will overflow into the TX section
SX1268_Status SX68_SetBufferBase(
SX1268* dev, //the target device struct
uint8_t tx_offset_ptr, //index in the buffer the TX chunk begins at
uint8_t rx_offset_ptr //index in the buffer the RX chunk begins at
);

//writes a packet into the TX chunk of the SX1268's buffer
//pay attention to the 'payloadLen' you set in the packet parameters and make sure it matches the length of the packet you're writing
//this function first copies the provided packet into the struct's local DMA memory
SX1268_Status SX68_WriteTXPacket(
SX1268* dev, //the target device struct
uint8_t* packet_buf, //pointer to the buffer where the packet resides
uint8_t packet_size //the size of the packet
);

//reads a packet from the RX chunk of the SX1268's buffer into the structs DMA memory
//packet size and buffer start are determined internally
SX1268_Status SX68_ReadRXPacket(
SX1268* dev //the target device struct
);

//copies the packet from the DMA buffer into the 'packet_buf' variable
//size is determined internally
//the intermediate DMA buffer is used for flow control
SX1268_Status SX68_GetPacket(
SX1268* dev, //the target device struct
uint8_t* packet_buf, //pointer to the memory where the packet will be stored into
uint8_t buf_size //size of 'packet_buf', note that this is simply for overflow protection, not the actual number of bytes read
);

//gives the values of the rssi and snr of the last packet received
SX1268_Status SX68_GetPacketRFStatus(
SX1268* dev, //the target device struct
uint8_t* rssi_avg, //pointer to the memory where the average rssi value will be put
uint8_t* snr, //pointer to the memory where the average signal-to-noise ratio will be put
uint8_t* rssi_est //pointer to the memory where the estimated final rssi value will be put
);

//gives the current rssi value detected on the air
//useful for making sure the channel is empty before sending
SX1268_Status SX68_GetChannelActivity(
SX1268* dev, //the target device struct
int* signal_str //pointer to the variable where the strength of the signal currently on air in dBm will be stored
);

//sets the SX1268 in TX mode to transmit the packet that was written using SX68_WriteTXPacket
//also uses 'payloadLen' to determine how many bytes from the TX buffer to send
SX1268_Status SX68_Transmit(
SX1268* dev, //the target device struct
uint32_t timeout //max time the packet transmission is expected to last, will generate a timeout irq if the tx doesn't complete in the alloted time
);

//sets the SX1268 in RX mode to receive packets
SX1268_Status SX68_BeginRX(
SX1268* dev, //the target device struct
uint32_t timeout //max time the sx1268 spends in RX mode waiting for a packet
//0 means the device stays in RX indefinitely until a packet is received then returns from RX mode
//0xFFFFFF means the device stays in RX indefinitely no matter how many packets are received
);

//sets the SX1268 in CAD mode with the parameters defined in SX68_LoRa_RXCadParameters
SX1268_Status SX68_BeginCAD(
SX1268* dev //the target device struct
);


  //IRQ Implementation Functions==============================================================

//implementation functions that should be placed into the relevant EXTI callback for each DIO's pin
//these functions should be called once their relevant DIO interrupt is triggered
//they simply set the corresponding _irqx_flag in the IRQ struct in the SX1268 struct
void SX68_DIO1_IRQimpl(
SX1268* dev //the target device struct
);

void SX68_DIO2_IRQimpl(
SX1268* dev //the target device struct
);

void SX68_DIO3_IRQimpl(
SX1268* dev //the target device struct
);


void SX68_DMA_TXCplt_IRQimpl(
SX1268* dev //the target device struct
);

void SX68_DMA_RXCplt_IRQimpl(
SX1268* dev //the target device struct
);
  //Configuration Functions========================================================================

//Note : All configuration functions require the SX1268 to be in stdby mode for them to work

//sets which IRQs trigger which dio pin, internally ORs the three masks to achieve the total IRQ bitmask
SX1268_Status SX68_SetIRQMask(
SX1268* dev, //the target device struct
uint16_t dio1_mask, //the IRQ sources that are going to be set on the irqMask and DIO1Mask bytes
uint16_t dio2_mask, //the IRQ sources that are going to be set on the irqMask and DIO2Mask bytes
uint16_t dio3_mask  //the IRQ sources that are going to be set on the irqMask and DIO3Mask bytes
);

//sets dio2 as RF switch, if this is set, dio2 will not be available for IRQs
SX1268_Status SX68_SetDIO2RFSwitch(
SX1268* dev, //the target device struct
uint8_t enable //0 to disable dio2 as rf switch, 1 to enable it
);

//sets dio3 as tcxo input, if it is used, dio3 will not be available for IRQs
SX1268_Status SX68_SetDIO3TCXO(
SX1268* dev, //the target device struct
uint8_t tcxo_voltage, //the voltage of the tcxo output
uint32_t timeout_delay // the time window the tcxo has to spool up before a XOSC_START_ERR is flagged
);

//sets the gpio port and pin of the specified diox
void SX68_SetDIOxGPIO(
SX1268* dev, //the target device struct
uint8_t diox, //which dio gpio to set
GPIO_t dio_gpio //diox gpio port and pin
);

//one of the main configuration functions, configures the RF frontend of the chip
//the configuration passed here is cached into the CONTROL struct of the SX1268
SX1268_Status SX68_ConfigRF(
SX1268* dev, //the target device struct
uint32_t freq, //the target frequency in Hz
uint8_t pa_dutycycle, //the duty cycle the power amplifier uses
uint8_t pa_hpmax //the max power setting of the amplifier, must not be higher than 7
//the max power and dutycycle should be chosen with the matching network taken into account
);

//this function configures the TX settings of the chip
SX1268_Status SX68_SetTXParameters(
SX1268* dev, //the target device struct
int8_t tx_power, //the power that will be used during the upcoming transmits
uint8_t ramp_time //the time it will take for the PA to ramp up to TX power
);

//one of the main configuration functions, configures LoRa modulation parameters
//requires the SX1268 to already be LoRa mode
//the configuration passed here is cached into the CONTROL struct of the SX1268
SX1268_Status SX68_LoRa_ModConfig(
SX1268* dev, //the target device struct
uint8_t sf, //spread factor that will be used for all coming LoRa transactions
uint8_t bw, //bandwidth that will be used for all coming LoRa transactions
uint8_t cr //coding rate that will be used for all coming LoRa transactions
//LDRO setting is dynamically set using the sf and bw values that are passed
//Use the provided macros for setting these parameters
);

//one of the main configuration functions, configures LoRa packet and transaction parameters
//the configuration passed here is cached into the CONTROL struct of the SX1268
SX1268_Status SX68_LoRa_Params(
SX1268* dev, //the target device struct
uint16_t preamble_len, //the number of symbols used for preamble
uint8_t header_type, //header for variable packet size type, or for fixed packet size type
uint8_t crc_en, //enable or disable the crc
uint8_t iq_inv //enable or disable inverted IQ
//Payload size can be set using SX68_LoRa_PayloadLength function or by passing it into the payload_len argument in SX68_WriteTXPacket
//Use the provided macros for setting these parameters
);

//sets the payload length in the LoRa packet parameters
//if the SX1268 is in fixed header mode, this length is used for all transactions
//if the SX1268 is instead in variable header mode, this length is used for TX and is added to the LoRa packet header
SX1268_Status SX68_LoRa_PayloadLength(
SX1268* dev, //the target device struct
uint8_t payload_len //the length of the next payload if using variable header or the fixed payload length if using fixed header
);

//sets the sync word of the LoRa modem, this is necessary to detect valid packets on the air
SX1268_Status SX68_LoRa_SyncWord(
SX1268* dev, //the target device struct
uint16_t sync_word //the sync word to be placed in the syncWord registers
);

//sets the Channel Activity Detection parameters
//the sensitivity parameters are calculated internally using the set spread factor
SX1268_Status SX68_LoRa_RXCadParameters(
SX1268* dev, //the target device struct
uint8_t exit_mode, //the mode the sx1268 switches to after activity detection
uint32_t exit_timeout //the max time the sx1268 spends in RX mode once it enters it after activity detection if that is set in 'exit_mode'
);

#endif /* INC_SX1268_DRIVER_H_ */
