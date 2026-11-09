/*
 * sx1268-driver.c
 *
 *  Created on: Oct 28, 2026
 *      Author: Hamza
 */


#include "sx1268-driver.h"

//Utility Functions and Lookup tables

uint32_t _SX68_LoRaBW_LUT
(
uint8_t bw_config //the configuration macro of the lora bandwidth setting
)
{
  switch(bw_config)
  {
    case LoRa_BW_7:
      return 7800;

    case LoRa_BW_10:
      return 10420;

    case LoRa_BW_15:
      return 15630;

    case LoRa_BW_20:
      return 20830;

    case LoRa_BW_31:
      return 31250;

    case LoRa_BW_41:
      return 41670;

    case LoRa_BW_62:
      return 62500;

    case LoRa_BW_125:
      return 125000;

    case LoRa_BW_250:
      return 250000;

    case LoRa_BW_500:
      return 500000;

    default:
      return LoRa_BW_Undefined;
  }
}


void _SX68_LoRaCadParams_LUT
(
uint8_t sf, //the spread factor input
uint8_t* peak, //pointer to the variable that will be populated by the ideal CadDetPeak value
uint8_t* min, //pointer to the variable that will be populated by the ideal CadDetMin value
uint8_t* symb_num //pointer to the variable that will be populated by the ideal CadSymbolNum value
)
{
  switch(sf)
  {
    case LoRa_SF5:
      *min = 10;
      *peak = 21;
      *symb_num = CAD_ON_2_SYMB;
      break;

    case LoRa_SF6:
      *min = 10;
      *peak = 21;
      *symb_num = CAD_ON_2_SYMB;
      break;

    case LoRa_SF7:
      *min = 10;
      *peak = 22;
      *symb_num = CAD_ON_2_SYMB;
      break;

    case LoRa_SF8:
      *min = 10;
      *peak = 22;
      *symb_num = CAD_ON_2_SYMB;
      break;

    case LoRa_SF9:
      *min = 10;
      *peak = 23;
      *symb_num = CAD_ON_4_SYMB;
      break;

    case LoRa_SF10:
      *min = 10;
      *peak = 24;
      *symb_num = CAD_ON_4_SYMB;
      break;

    case LoRa_SF11:
      *min = 10;
      *peak = 25;
      *symb_num = CAD_ON_4_SYMB;
      break;

    case LoRa_SF12:
      *min = 10;
      *peak = 28;
      *symb_num = CAD_ON_4_SYMB;
      break;
  }
}

//Lower Level Functions =========================================================================

void _SX68_SetCS
(
SX1268* dev, //the target device struct
uint8_t cs_state //the target state : GPIO_PIN_SET, GPIO_PIN_RESET
)
{
  HAL_GPIO_WritePin(dev->GPIO._cs_gpio._port, dev->GPIO._cs_gpio._pin, cs_state);
}


void _SX68_Reset
(
SX1268* dev
)
{
  HAL_GPIO_WritePin(dev->GPIO._nrst_gpio._port, dev->GPIO._nrst_gpio._pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(dev->GPIO._nrst_gpio._port, dev->GPIO._nrst_gpio._pin, GPIO_PIN_SET);
}


SX1268_Status _SX68_CheckStatus
(
SX1268* dev,
uint8_t status
)
{
  uint8_t dev_mode = status >> 4; //extract bits 4:6
  uint8_t cmd_sts = (status & 0b00001110) >> 1; //extract bits 1:3

  dev->CONTROL._dev_mode = dev_mode;
  dev->CONTROL._cmd_sts = cmd_sts;

  return cmd_sts;
}


GPIO_PinState _SX68_ReadBIND
(
SX1268* dev //the target device struct
)
{
  return HAL_GPIO_ReadPin(dev->GPIO._bind_gpio._port, dev->GPIO._bind_gpio._pin);
}


SX1268_Status _SX68_CmdTXRX
(
SX1268* dev, //the target device struct
uint8_t opcode, //the opcode of the command to be sent, {datasheet section 11.}
uint8_t* args, //the buffer of arguments to be sent alongside the command
uint8_t arg_len, //the number of bytes of the arguments in the
uint8_t* ret_val, //a buffer for the returned values from the command to be put into, will also be checked for status errors
uint8_t ret_len //the number of bytes of expected return values
)
{
  // 1) Check that a valid argument length or return length has been passed
  if(arg_len > 9 || ret_len > 6) return SX1268_ERROR;

  // 2) Poll for busy line
  uint32_t tk1 = HAL_GetTick();

  while(_SX68_ReadBIND(dev) != GPIO_PIN_RESET || SX68_BUS_STS_(dev) != HAL_OK)
  {
      if(HAL_GetTick() - tk1 > SX1268_SPITO) return SX1268_CMD_TIMEOUT;
  }

  // 3) Prepare the buffers
  uint8_t ftx_chunk[12];

  ftx_chunk[0] = opcode; //set byte 0 to the opcode
  if(arg_len != 0 && args != NULL) memcpy(ftx_chunk+1, args, arg_len); //add the arguments if there are any expected
  if(ret_len != 0 && ret_val != NULL) ftx_chunk[arg_len+1] = NOP; //add a NOP if there is an expected return value

  uint8_t ftrans_len = (args==NULL? 0 : arg_len) + (ret_val==NULL? 1 : ret_len);


  // 4) Execute the first chuck transaction
  _SX68_SetCS(dev, GPIO_PIN_RESET);

  HAL_StatusTypeDef hal_sts = HAL_SPI_Transmit(dev->GPIO._hspi, ftx_chunk, ftrans_len, SX1268_SPITO);
  if(hal_sts != HAL_OK)
  {
      _SX68_SetCS(dev, GPIO_PIN_SET);
      return SX1268_ERROR;
  }

  // 5) Execute the second chunk transaction, aka read any expected return values
  if(ret_len != 0)
  {
      hal_sts = HAL_SPI_Receive(dev->GPIO._hspi, ret_val, ret_len, SX1268_SPITO);

      if(hal_sts != HAL_OK)
      {
          _SX68_SetCS(dev, GPIO_PIN_SET);
          return SX1268_ERROR;
      }
  }

  _SX68_SetCS(dev, GPIO_PIN_SET);
  return SX1268_OK;
}


SX1268_Status _SX68_CmdTXRX_G
(
SX1268* dev, //the target device struct
uint8_t opcode, //the opcode of the command to be sent, {datasheet section 11.}
uint8_t* args, //the buffer of arguments to be sent alongside the command
uint8_t arg_len, //the number of bytes of the arguments in the
uint8_t* ret_val, //a buffer for the returned values from the command to be put into, will also be checked for status errors
uint8_t ret_len //the number of bytes of expected return values
)
{
  // 1) Check that a valid argument length or return length has been passed
  if(arg_len > 9 || ret_len > 6) return SX1268_ERROR;

  // 2) Poll for busy line
  uint32_t tk1 = HAL_GetTick();

  while(_SX68_ReadBIND(dev) != GPIO_PIN_RESET || SX68_BUS_STS_(dev) != HAL_OK)
  {
      if(HAL_GetTick() - tk1 > SX1268_SPITO) return SX1268_CMD_TIMEOUT;
  }

  // 3) Prepare the buffers
  uint8_t ftx_chunk[12];

  ftx_chunk[0] = opcode; //set byte 0 to the opcode
  if(arg_len != 0 && args != NULL) memcpy(ftx_chunk+1, args, arg_len); //add the arguments if there are any expected
  if(ret_len != 0 && ret_val != NULL) ftx_chunk[arg_len+1] = NOP; //add a NOP if there is an expected return value

  uint8_t ftrans_len = (args==NULL? 0 : arg_len) + (ret_val==NULL? 1 : 2);

  // 4) Execute the first chunk transaction
  _SX68_SetCS(dev, GPIO_PIN_RESET);

  HAL_StatusTypeDef hal_sts = HAL_SPI_Transmit(dev->GPIO._hspi, ftx_chunk, ftrans_len, SX1268_SPITO);
  if(hal_sts != HAL_OK)
  {
      _SX68_SetCS(dev, GPIO_PIN_SET);
      return SX1268_ERROR;
  }

  // 5) Execute the second chunk transaction, aka read any expected return values
  if(ret_len != 0)
  {
      hal_sts = HAL_SPI_Receive(dev->GPIO._hspi, ret_val, ret_len, SX1268_SPITO);

      if(hal_sts != HAL_OK)
      {
          _SX68_SetCS(dev, GPIO_PIN_SET);
          return SX1268_ERROR;
      }
  }

  // 6) End the initial transaction and free the chip
  _SX68_SetCS(dev, GPIO_PIN_SET);

  // 7) poll for the chip to finish writing
  tk1 = HAL_GetTick();

  while(_SX68_ReadBIND(dev) != GPIO_PIN_RESET)
  {
      if(HAL_GetTick() - tk1 > SX1268_SPITO) return SX1268_CMD_TIMEOUT;
  }

  // 8) Prepare the status read buffers
  uint8_t sts_byte_tx[2];
  uint8_t sts_byte_rx[2];

  sts_byte_tx[0] = GetStatus;
  sts_byte_tx[1] = NOP;

  // 9) Begin the status read transaction
  _SX68_SetCS(dev, GPIO_PIN_RESET);

  hal_sts = HAL_SPI_TransmitReceive(dev->GPIO._hspi, sts_byte_tx, sts_byte_rx, 2, SX1268_SPITO);
  _SX68_SetCS(dev, GPIO_PIN_SET);

  if(hal_sts != HAL_OK)
  {
      return SX1268_ERROR;
  }

  // 10) Finally check the status and return ok or error based on it.
  // the status value can be read from the dev.CONTROL._cmd_sts variable
  // the status value is not returned directly to keep the error checking as easy as "if(return != SX1268_OK) check_error"
  SX1268_Status rf_sts = _SX68_CheckStatus(dev, sts_byte_rx[1]);

  switch(rf_sts)
  {
    case SX1268_RX_DONE:
    case SX1268_TX_DONE:
    case SX1268_OK:
    case 0x01:
      return SX1268_OK;

    default:
      return SX1268_ERROR;
  }
}


SX1268_Status _SX68_WriteBuffer
(
SX1268* dev, //the target device struct
uint8_t* buf_out, //pointer to the buffer where the data will be pulled from
uint8_t len, //the amount of data to be sent in bytes
uint8_t offset //the offset to start writing to on the sx1268's memory
)
{
  // 1) Poll for busy line
  uint32_t tk1 = HAL_GetTick();

  while(_SX68_ReadBIND(dev) != GPIO_PIN_RESET || SX68_BUS_STS_(dev) != HAL_OK)
  {
      if(HAL_GetTick() - tk1 > SX1268_SPITO) return SX1268_CMD_TIMEOUT;
  }

  SX68_BUS_STS_(dev) = HAL_BUSY;

  // 2) Initialize and prep the initial command transaction buffers
  uint8_t tx_chunk[2];
  uint8_t rx_chunk[2];

  tx_chunk[0] = WriteBuffer;
  tx_chunk[1] = offset;

  // 3) Execute the WriteBuffer command transaction
  _SX68_SetCS(dev, GPIO_PIN_RESET);

  HAL_StatusTypeDef hal_sts = HAL_SPI_TransmitReceive(dev->GPIO._hspi, tx_chunk, rx_chunk, 2, SX1268_SPITO);
  if(hal_sts != HAL_OK)
    {
      _SX68_SetCS(dev, GPIO_PIN_SET);
      SX68_BUS_STS_(dev) = HAL_OK;
      return SX1268_ERROR;
    }

  // 4) Send the buffer data

  hal_sts = HAL_SPI_Transmit_DMA(dev->GPIO._hspi, buf_out, len);
  if(hal_sts != HAL_OK)
    {
      _SX68_SetCS(dev, GPIO_PIN_SET);
      SX68_BUS_STS_(dev) = HAL_OK;
      return SX1268_ERROR;
    }

  return SX1268_OK;
}


SX1268_Status _SX68_ReadBuffer
(
SX1268* dev, //the target device struct
uint8_t* buf_in, //pointer to the buffer where the data will be pulled in to
uint8_t len, //the amount of data to be pulled in bytes
uint8_t offset //the offset to start reading from on the sx1268's memory
)
{
  // 1) Poll for busy line
  uint32_t tk1 = HAL_GetTick();

  while(_SX68_ReadBIND(dev) != GPIO_PIN_RESET || SX68_BUS_STS_(dev) != HAL_OK)
  {
      if(HAL_GetTick() - tk1 > SX1268_SPITO) return SX1268_CMD_TIMEOUT;
  }

  dev->GPIO._bus_state = HAL_BUSY;

  // 2) Initialize and prep the initial command transaction buffers
  uint8_t tx_chunk[3];
  uint8_t rx_chunk[3];

  tx_chunk[0] = ReadBuffer;
  tx_chunk[1] = offset;
  tx_chunk[2] = NOP;

  // 3) Begin the command transaction
  _SX68_SetCS(dev, GPIO_PIN_RESET);

  HAL_StatusTypeDef hal_sts = HAL_SPI_TransmitReceive(dev->GPIO._hspi, tx_chunk, rx_chunk, 3, SX1268_SPITO);
  if(hal_sts != HAL_OK)
    {
      _SX68_SetCS(dev, GPIO_PIN_SET);
      dev->GPIO._bus_state = HAL_OK;
      return SX1268_ERROR;
    }

  // 4) Read the data in the buffer
  hal_sts = HAL_SPI_Receive_DMA(dev->GPIO._hspi, buf_in, len);
  if(hal_sts != HAL_OK)
    {
      _SX68_SetCS(dev, GPIO_PIN_SET);
      dev->GPIO._bus_state = HAL_OK;
      return SX1268_ERROR;
    }

  return SX1268_OK;
}


SX1268_Status _SX68_WriteRegister
(
SX1268* dev, //the target device struct
uint16_t reg_adr, //address of the target register
uint8_t* data, //the data to be written
uint8_t data_size //the size of the data to be written
)
{
  if(data_size>8 || data == NULL) return SX1268_ERROR;
  uint16_t trans_len = data_size + 2;

  uint8_t trans_reg[10];
  trans_reg[0] = (uint8_t)(reg_adr>>8);
  trans_reg[1] = (uint8_t)(reg_adr);
  memcpy(trans_reg+2, data, data_size);

  return _SX68_CmdTXRX(dev, WriteRegister, trans_reg, trans_len, NULL, 0);
}

SX1268_Status _SX68_ReadRegister
(
SX1268* dev, //the target device struct
uint16_t reg_adr, //address of the target register
uint8_t* data, //where the data will be read into
uint8_t data_size //the size of the data to be read
)
{
  if(data_size>8 || data == NULL) return SX1268_ERROR;

  uint8_t reg_adr_byte[2];
  reg_adr_byte[0] = (uint8_t)(reg_adr >> 8);
  reg_adr_byte[1] = (uint8_t)(reg_adr);

  return _SX68_CmdTXRX(dev, ReadRegister, reg_adr_byte, 2, data, data_size);
}

//Interfacing Functions ==========================================================================

SX1268_Status SX68_Init
(
SX1268* dev, //the target device struct
SPI_HandleTypeDef* hspi, //the hal spi handler that will be used
GPIO_t cs_gpio, //spi chips select / nss gpio port and pin
GPIO_t nrst_gpio, //sx1268 chip reset line gpio port and pin
GPIO_t bind_gpio //sx1268 busy indicator line gpio port and pin
)
{
  dev->GPIO._hspi = hspi;
  dev->GPIO._cs_gpio = cs_gpio;
  dev->GPIO._nrst_gpio = nrst_gpio;
  dev->GPIO._bind_gpio = bind_gpio;
  dev->CONTROL._pa_dutycycle = 0;
  dev->CONTROL._pa_hpmax = 0;
  dev->CONTROL._rf_frequency = 0;

  __HAL_SPI_ENABLE(dev->GPIO._hspi);

  _SX68_SetCS(dev, GPIO_PIN_SET);

  _SX68_Reset(dev);

  uint32_t tk1 = HAL_GetTick();

  while(_SX68_ReadBIND(dev) != GPIO_PIN_RESET)
  {
      if(HAL_GetTick() - tk1 > SX1268_SPITO) return SX1268_CMD_TIMEOUT;
  }

  uint8_t calib_arg = 0b00111111;

  return _SX68_CmdTXRX_G(dev, CalibrateFunction, &calib_arg, 1, NULL, 0);
}


SX1268_Status SX68_GetIRQStatus
(
SX1268* dev //the target device struct
)
{
  uint8_t irq_byte[2];

  SX1268_Status rf_sts = _SX68_CmdTXRX(dev, GetIrqStatus, NULL, 0, irq_byte, 2);
  if(rf_sts != SX1268_OK) return SX1268_ERROR;

  dev->IRQ._irq_flags = (((uint16_t)(irq_byte[0])) << 8) | (uint16_t)(irq_byte[1]);

  return SX1268_OK;
}


SX1268_Status SX68_ClearIRQStatus
(
SX1268* dev //the target device struct
)
{
  uint8_t clear_byte[2] = {0x02, 0x01};

  return _SX68_CmdTXRX(dev, ClearIrqStatus, clear_byte, 2, NULL, 0);
}


void SX68_ClearDIOStatus
(
SX1268* dev, //the target device struct
uint8_t diox //the DIO IRQ flag to be cleared
)
{
  if(diox > 2) return;

  dev->IRQ._irqx_flag[diox] = 0;
}


inline uint8_t SX68_GetDIOStatus
(
SX1268* dev, //the target device struct
uint8_t diox //the DIO IRQ to be returned
)
{
  if(diox>DIO3) return 0;

  return (uint8_t)(dev->IRQ._irqx_flag[diox]);
}


SX1268_Status SX68_SetOPType
(
SX1268* dev, //the target device struct
SX1268_Type type //LoRa vs FSK !!! FSK not supported currently !!!
)
{
  uint8_t type_byte = type;

  SX1268_Status rf_sts = _SX68_CmdTXRX_G(dev, SetPacketType, &type_byte, 1, NULL, 0);
  if(rf_sts != SX1268_OK) return SX1268_ERROR;

  SX68_RFTYPE_(dev) = type;

  return SX1268_OK;
}


SX1268_Status SX68_SetStandby
(
SX1268* dev, //the target device struct
SX1268_Mode stdby_type //stdby_RC vs stdby_xosc
)
{
  if(stdby_type != MODE_STDBY_RC && stdby_type != MODE_STDBY_XOSC) return SX1268_ERROR;

  uint8_t type_byte = stdby_type-2;

  return _SX68_CmdTXRX(dev, SetStandby, &type_byte, 1, NULL, 0);
}


SX1268_Status SX68_SetBufferBase
(
SX1268* dev, //the target device struct
uint8_t tx_offset_ptr, //where in the buffer the TX chunk begins
uint8_t rx_offset_ptr //where in the buffer the RX chunk begins
)
{
  uint8_t offset_byte[2];
  offset_byte[0] = tx_offset_ptr;
  offset_byte[1] = rx_offset_ptr;

  SX1268_Status rf_sts = _SX68_CmdTXRX_G(dev, SetBufferBaseAddress, offset_byte, 2, NULL, 0);
  if(rf_sts != SX1268_OK) return SX1268_ERROR;

  dev->DATA._tx_buf_offset = tx_offset_ptr;
  dev->DATA._rx_buf_offset = rx_offset_ptr;

  return SX1268_OK;
}


SX1268_Status SX68_WriteTXPacket
(
SX1268* dev, //the target device struct
uint8_t* packet_buf, //pointer to the buffer where the packet resides
uint8_t packet_size //the size of the packet
)
{
  if(dev->DATA._tx_buf_sts != SX1268_OK) return SX1268_BUSY;

  dev->DATA._tx_buf_sts = SX1268_BUSY;

  memcpy(dev->DATA._tx_dma_buf, packet_buf, packet_size);

  dev->DATA._tx_pckt_size = packet_size;

  return _SX68_WriteBuffer(dev, dev->DATA._tx_dma_buf, packet_size, dev->DATA._tx_buf_offset);
}


SX1268_Status SX68_ReadRXPacket
(
SX1268* dev //the target device struct
)
{
  if(dev->DATA._rx_buf_sts != SX1268_OK) return SX1268_BUSY;

  dev->DATA._rx_buf_sts = SX1268_BUSY;

  uint8_t buf_sts_byte[2];

  SX1268_Status rf_sts = _SX68_CmdTXRX_G(dev, GetRxBufferStatus, NULL, 0, buf_sts_byte, 2);
  if(rf_sts != SX1268_OK) return SX1268_ERROR;

  dev->DATA._rx_pckt_size = buf_sts_byte[0];

  return _SX68_ReadBuffer(dev, dev->DATA._rx_dma_buf, dev->DATA._rx_pckt_size, buf_sts_byte[1]);
}


SX1268_Status SX68_GetPacket
(
SX1268* dev, //the target device struct
uint8_t* packet_buf, //pointer to the memory where the packet will be stored into
uint8_t buf_size //size of 'packet_buf', note that this is simply for overflow protection, not the actual number of bytes read
)
{
  if(dev->DATA._rx_buf_sts == SX1268_BUSY) return SX1268_BUSY;
  else if(dev->DATA._rx_buf_sts != SX1268_RX_DONE) return SX1268_ERROR;
  else if(buf_size < dev->DATA._rx_pckt_size) return SX1268_ERROR;

  memcpy(packet_buf, dev->DATA._rx_dma_buf, dev->DATA._rx_pckt_size);

  dev->DATA._rx_buf_sts = SX1268_OK;

  return SX1268_OK;
}


SX1268_Status SX68_GetPacketRFStatus
(
SX1268* dev, //the target device struct
uint8_t* rssi_avg, //pointer to the memory where the average rssi value will be put
uint8_t* snr, //pointer to the memory where the average signal-to-noise ratio will be put
uint8_t* rssi_est //pointer to the memory where the estimated final rssi value will be put
)
{
  uint8_t pckt_sts_byte[3];

  SX1268_Status rf_sts = _SX68_CmdTXRX(dev, GetPacketStatus, NULL, 0, pckt_sts_byte, 3);
  if(rf_sts != SX1268_OK) return SX1268_ERROR;

  *rssi_avg = pckt_sts_byte[0];
  *snr = pckt_sts_byte[1];
  *rssi_est = pckt_sts_byte[2];

  return SX1268_OK;
}


SX1268_Status SX68_GetChannelActivity
(
SX1268* dev, //the target device struct
int* signal_str //pointer to the variable where the strength of the signal currently on air in dBm will be stored
)
{
  uint8_t rssi_raw_byte;

  SX1268_Status rf_sts = _SX68_CmdTXRX(dev, GetRssiInst, NULL, 0, &rssi_raw_byte, 1);
  if(rf_sts != SX1268_OK) return SX1268_ERROR;

  *signal_str =  -rssi_raw_byte/2;

  return SX1268_OK;
}


SX1268_Status SX68_Transmit
(
SX1268* dev, //the target device struct
uint32_t timeout //max time the packet transmission is expected to last, will generate a timeout irq if the tx doesn't complete in the alloted time
)
{
  uint32_t to_ticks = timeout * (timeout==0xFFFFFF? 1 : 64);

  uint8_t to_byte[3];
  to_byte[0] = (uint8_t)(to_ticks >> 16);
  to_byte[1] = (uint8_t)(to_ticks >> 8);
  to_byte[2] = (uint8_t)(to_ticks);

  return _SX68_CmdTXRX_G(dev, SetTx, to_byte, 3, NULL, 0);
}


SX1268_Status SX68_BeginRX(
SX1268* dev, //the target device struct
uint32_t timeout //max time the sx1268 spends in RX mode waiting for a packet
//0 means the device stays in RX indefinitely until a packet is received then returns from RX mode
//0xFFFFFF means the device stays in RX indefinitely no matter how many packets are received
)
{
  uint32_t to_ticks = timeout * (timeout==0xFFFFFF? 1 : 64);

  uint8_t to_byte[3];
  to_byte[0] = (uint8_t)(to_ticks >> 16);
  to_byte[1] = (uint8_t)(to_ticks >> 8);
  to_byte[2] = (uint8_t)(to_ticks);

  return _SX68_CmdTXRX_G(dev, SetRx, to_byte, 3, NULL, 0);
}


SX1268_Status SX68_BeginCAD
(
SX1268* dev //the target device struct
)
{
  return _SX68_CmdTXRX_G(dev, SetCAD, NULL, 0, NULL, 0);
}


//IRQ Implementation Functions===================================================================

void SX68_DIO1_IRQimpl
(
SX1268* dev //the target device struct
)
{
  dev->IRQ._irqx_flag[DIO1] = 1;
}


void SX68_DIO2_IRQimpl
(
SX1268* dev //the target device struct
)
{
  dev->IRQ._irqx_flag[DIO2] = 1;
}


void SX68_DIO3_IRQimpl
(
SX1268* dev //the target device struct
)
{
  dev->IRQ._irqx_flag[DIO3] = 1;
}


void SX68_DMA_TXCplt_IRQimpl(
SX1268* dev //the target device struct
)
{
  _SX68_SetCS(dev, GPIO_PIN_SET);

  dev->DATA._tx_buf_sts = SX1268_OK;

  dev->GPIO._bus_state = HAL_OK;
}


void SX68_DMA_RXCplt_IRQimpl
(
SX1268* dev //the target device struct
)
{
  _SX68_SetCS(dev, GPIO_PIN_SET);

  dev->DATA._rx_buf_sts = SX1268_RX_DONE;

  dev->GPIO._bus_state = HAL_OK;
}


//Configuration Functions========================================================================

SX1268_Status SX68_SetIRQMask
(
SX1268* dev, //the target device struct
uint16_t dio1_mask, //the IRQ sources that are going to be set on the irqMask and DIO1Mask bytes
uint16_t dio2_mask, //the IRQ sources that are going to be set on the irqMask and DIO2Mask bytes
uint16_t dio3_mask //the IRQ sources that are going to be set on the irqMask and DIO2Mask bytes
)
{
  uint16_t irq_mask = dio1_mask | dio2_mask | dio3_mask;

  uint8_t params_buf[8];

  params_buf[0] = (uint8_t)(irq_mask>>8);
  params_buf[1] = (uint8_t)(irq_mask);

  params_buf[2] = (uint8_t)(dio1_mask>>8);
  params_buf[3] = (uint8_t)(dio1_mask);

  params_buf[4] = (uint8_t)(dio2_mask>>8);
  params_buf[5] = (uint8_t)(dio2_mask);

  params_buf[6] = (uint8_t)(dio3_mask>>8);
  params_buf[7] = (uint8_t)(dio3_mask);

  return _SX68_CmdTXRX_G(dev, SetDioIrqParams, params_buf, 8, NULL, 0);
}


SX1268_Status SX68_SetDIO2RFSwitch
(
SX1268* dev, //the target device struct
uint8_t enable //0 to disable dio2 as rf switch, 1 to enable it
)
{
  if(enable > 1) return SX1268_ERROR;
  return _SX68_CmdTXRX_G(dev, SetDIO2AsRfSwitchCtrl, &enable, 1, NULL, 0);
}


SX1268_Status SX68_SetDIO3TCXO(
SX1268* dev, //the target device struct
uint8_t tcxo_voltage, //the voltage of the tcxo output
uint32_t timeout_delay // the time window the tcxo has to spool up before a XOSC_START_ERR is flagged, max value is as a 3-byte uint
)
{
  if(tcxo_voltage > TCXO_3v3) return SX1268_ERROR;

  timeout_delay &= (uint32_t)(0xFFFFFF);

  uint8_t tcxo_byte[4];
  tcxo_byte[0] = tcxo_voltage;
  tcxo_byte[1] = (uint8_t)(timeout_delay >> 16);
  tcxo_byte[2] = (uint8_t)(timeout_delay >> 8);
  tcxo_byte[3] = (uint8_t)(timeout_delay);

  return _SX68_CmdTXRX_G(dev, SetDIO3AsTCXOCtrl, tcxo_byte, 4, NULL, 0);
}


void SX68_SetDIOxGPIO
(
SX1268* dev, //the target device struct
uint8_t diox, //which dio gpio to set
GPIO_t dio_gpio //diox gpio port and pin
)
{
  switch(diox)
  {
    case DIO1:
      dev->DIO._dio1_gpio = dio_gpio;
      break;

    case DIO2:
      dev->DIO._dio2_gpio = dio_gpio;
      break;

    case DIO3:
      dev->DIO._dio3_gpio = dio_gpio;
      break;
  }
}


SX1268_Status SX68_ConfigRF
(
SX1268* dev, //the target device struct
uint32_t freq, //the target frequency in Hz
uint8_t pa_dutycycle, //the duty cycle the power amplifier uses
uint8_t pa_hpmax //the max power setting of the amplifier, must not be higher than 7
//the the max power and dutycycle should be chosen with the matching network taken into account
)
{
  // 1) Guard check incase the user passed a max power setting that is too high
  if(pa_hpmax > PA_HPmax_MaxVal || pa_dutycycle > PA_DC_MaxVal) return SX1268_ERROR;

  // 2) Prepare the  image calibration settings for the set frequency section:
  uint8_t calib_arg = 0b01000000;
  SX1268_Status rf_sts = _SX68_CmdTXRX_G(dev, CalibrateFunction, &calib_arg, 1, NULL, 0);
  if(rf_sts != SX1268_OK) return SX1268_ERROR;

  uint8_t rejimg_limits[2];
  rejimg_limits[0] = (freq / 1000000 - ImjRejWind_Val + 2)/4;
  rejimg_limits[1] = (freq / 1000000 + ImjRejWind_Val + 2)/4;
  //these calculate the top and bottom of the frequency band of the image rejection using the formula :
  //frequency limits = (target frequency +- rejection window half + 2) / 4, where the +-8 gives a 16MHz rejection band,
  //and the division by 4 is expected by the sx1268
  //the + 2 is to round to the neareset possible integer to ensure the window is not offset

  // 3) Execute the image calibration command
  rf_sts = _SX68_CmdTXRX_G(dev, CalibrateImage, rejimg_limits, 2, NULL, 0);
  if(rf_sts != SX1268_OK) return SX1268_ERROR;

  // 4) Prepare the frequency value into a byte buffer
  uint32_t freq_conf = (uint32_t)(((uint64_t)(freq) * (uint64_t)(33554432)/*2^25*/) / (uint64_t)(32000000)/*clock frequency of sx1268*/);
  uint8_t freq_conf_byte[4];
  freq_conf_byte[0] = (uint8_t)(freq_conf >> 24);
  freq_conf_byte[1] = (uint8_t)(freq_conf >> 16);
  freq_conf_byte[2] = (uint8_t)(freq_conf >> 8);
  freq_conf_byte[3] = (uint8_t)(freq_conf);

  // 5) Send the frequency setting command
  rf_sts = _SX68_CmdTXRX_G(dev, SetRfFrequency, freq_conf_byte, 4, NULL, 0);
  if(rf_sts != SX1268_OK) return SX1268_ERROR;

  //Finally cache the frequency in the local struct once it is successfully set
  dev->CONTROL._rf_frequency = freq;

  // 6) Prepare the byte buffer that will be sent as the power amplifier config

  uint8_t pa_conf_byte[4];
  pa_conf_byte[0] = pa_dutycycle;
  pa_conf_byte[1] = pa_hpmax;
  pa_conf_byte[2] = PA_DevSel_Default;
  pa_conf_byte[3] = PA_PaLut_Default;

  // 7) Execute the power amplifier config setting command
  rf_sts = _SX68_CmdTXRX_G(dev, SetPaConfig, pa_conf_byte, 4, NULL, 0);
  if(rf_sts != SX1268_OK) return SX1268_ERROR;

  // Finally cache the power amplifier configuration in the local struct once it is successfully set
  dev->CONTROL._pa_dutycycle = pa_dutycycle;
  dev->CONTROL._pa_hpmax = pa_hpmax;

  return SX1268_OK;
}


SX1268_Status SX68_SetTXParameters
(
SX1268* dev, //the target device struct
int8_t tx_power, //the power that will be used during the upcoming transmits
uint8_t ramp_time //the time it will take for the PA to ramp up to TX power
)
{
  if(tx_power < -9 || tx_power > 22) return SX1268_ERROR;

  uint8_t tx_param_byte[2];
  tx_param_byte[0] = (uint8_t)(tx_power);
  tx_param_byte[1] = ramp_time;

  return _SX68_CmdTXRX_G(dev, SetTxParams, tx_param_byte, 2, NULL, 0);
}


SX1268_Status SX68_LoRa_ModConfig
(
SX1268* dev, //the target device struct
uint8_t sf, //spread factor that will be used for all coming LoRa transactions
uint8_t bw, //bandwidth that will be used for all coming LoRa transactions
uint8_t cr //coding rate that will be used for all coming LoRa transactions
//LDRO setting is dynamically set using the sf and bw values that are passed
//Use the provided macros for setting these parameters
)
{
  if(SX68_RFTYPE_(dev) != TYPE_LoRa) return SX1268_ERROR;

  uint32_t num_bw = _SX68_LoRaBW_LUT(bw);

  if(num_bw == LoRa_BW_Undefined) return SX1268_ERROR;

  if( (((1<<sf) * 1000)/num_bw) /*find time on air for a single symbol in ms*/ > 16 /*max ToA without ldro*/ ) SX68_LORA_(dev, _ldro) = 1;
  uint8_t ldro = 0;

  uint8_t mod_params_byte[4];
  mod_params_byte[0] = sf;
  mod_params_byte[1] = bw;
  mod_params_byte[2] = cr;
  mod_params_byte[3] = ldro;

  SX1268_Status rf_sts = _SX68_CmdTXRX_G(dev, SetModulationParams, mod_params_byte, 4, NULL, 0);
  if(rf_sts != SX1268_OK) return SX1268_ERROR;

  SX68_LORA_(dev, _sf) = sf;
  SX68_LORA_(dev, _bw) = bw;
  SX68_LORA_(dev, _cr) = cr;
  SX68_LORA_(dev, _ldro) = ldro;

  return SX1268_OK;
}


SX1268_Status SX68_LoRa_Params
(
SX1268* dev, //the target device struct
uint16_t preamble_len, //the number of symbols used for preamble
uint8_t header_type, //header for variable packet size type, or for fixed packet size type
uint8_t crc_en, //enable or disable the crc
uint8_t iq_inv //enable or disable inverted IQ
//Payload size can be set using SX68_LoRa_PayloadLength function or by passing it into the payload_len argument in SX68_WriteTXPacket
//Use the provided macros for setting these parameters
)
{
  if(SX68_RFTYPE_(dev) != TYPE_LoRa) return SX1268_ERROR;

  uint8_t packet_params_byte[6];
  packet_params_byte[0] = (uint8_t)(preamble_len>>8);
  packet_params_byte[1] = (uint8_t)(preamble_len);
  packet_params_byte[2] = header_type;
  packet_params_byte[3] = 0;
  packet_params_byte[4] = crc_en;
  packet_params_byte[5] = iq_inv;

   SX1268_Status rf_sts = _SX68_CmdTXRX_G(dev, SetPacketParams, packet_params_byte, 6, NULL, 0);

   if(rf_sts != SX1268_OK)
     {
       return SX1268_ERROR;
     }

   SX68_LORA_(dev, _preamble_len) = preamble_len;
   SX68_LORA_(dev, _header_type) = header_type;
   SX68_LORA_(dev, _payload_len) = 0;
   SX68_LORA_(dev, _crc_en) = crc_en;
   SX68_LORA_(dev, _iq_inv) = iq_inv;

   return SX1268_OK;
}


SX1268_Status SX68_LoRa_PayloadLength
(
SX1268* dev, //the target device struct
uint8_t payload_len //the length of the next payload if using variable header or the fixed payload length if using fixed header
)
{
  if(SX68_RFTYPE_(dev) != TYPE_LoRa) return SX1268_ERROR;

  uint8_t packet_params_byte[6];
  packet_params_byte[0] = (uint8_t)(SX68_LORA_(dev, _preamble_len)>>8);
  packet_params_byte[1] = (uint8_t)(SX68_LORA_(dev, _preamble_len));
  packet_params_byte[2] = SX68_LORA_(dev, _header_type);
  packet_params_byte[3] = payload_len;
  packet_params_byte[4] = SX68_LORA_(dev, _crc_en);
  packet_params_byte[5] = SX68_LORA_(dev, _iq_inv);

  SX1268_Status rf_sts = _SX68_CmdTXRX(dev, SetPacketParams, packet_params_byte, 6, NULL, 0);

  if(rf_sts != SX1268_OK)
    {
      return SX1268_ERROR;
    }

  SX68_LORA_(dev, _payload_len) = payload_len;

  return SX1268_OK;
}


SX1268_Status SX68_LoRa_SyncWord
(
SX1268* dev, //the target device struct
uint16_t sync_word //the sync word to be placed in the syncWord registers
)
{
  if(SX68_RFTYPE_(dev) != TYPE_LoRa) return SX1268_ERROR;

  uint8_t sync_word_byte[2];
  sync_word_byte[0] = (uint8_t)(sync_word>>8);
  sync_word_byte[1] = (uint8_t)(sync_word);

  return _SX68_WriteRegister(dev, LoRa_Sync_Word_Start, sync_word_byte, 2);
}


SX1268_Status SX68_LoRa_RXCadParameters
(
SX1268* dev, //the target device struct
uint8_t exit_mode, //the mode the sx1268 switches to after activity detection
uint32_t exit_timeout //the max time the sx1268 spends in RX mode once it enters it after activity detection if that is set in 'exit_mode'
)
{
  // 1) check the devices is in LoRa mode
  if(SX68_RFTYPE_(dev) != TYPE_LoRa) return SX1268_ERROR;
  uint32_t to_ms = exit_timeout * 64;

  // 2) initialize the internal variables with the minimum ideal values
  uint8_t cad_det_min = 10;
  uint8_t cad_det_peak = 21;
  uint8_t cad_symbol_num = CAD_ON_2_SYMB;

  //populate them with the ideal values for the current spread factor from the lookup table
  _SX68_LoRaCadParams_LUT(SX68_LORA_(dev, _sf), &cad_det_min, &cad_det_peak, &cad_symbol_num);

  // 3) initialize the byte buffer for the arguments and execute the command
  uint8_t cad_params_byte[7];
  cad_params_byte[0] = cad_symbol_num;
  cad_params_byte[1] = cad_det_peak;
  cad_params_byte[2] = cad_det_min;
  cad_params_byte[3] = exit_mode;
  cad_params_byte[4] = (uint8_t)(to_ms >> 16);
  cad_params_byte[5] = (uint8_t)(to_ms >> 8);
  cad_params_byte[6] = (uint8_t)(to_ms);

  return _SX68_CmdTXRX_G(dev, SetCadParams, cad_params_byte, 7, NULL, 0);
}
