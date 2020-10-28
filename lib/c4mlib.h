
// build info: 
//   version  : 3.6.1
//   time     : 07/09/2020 08:50:47
//   device   : asam128
//   platform : avr
//   mcu      : atmega128
//   fcpu     : 11059200

// developed by MVMC-lab

// you can get lastest version at https://gitlab.com/MVMC-lab/c4mlib/c4mlib/tags

#ifndef C4MLIB_H
#define C4MLIB_H


#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>


#include "spi.cfg"
#include "newmode.cfg"
#include "adc.cfg"
#include "tim.cfg"
#include "pwm.cfg"
#include "asamodule.cfg"
#include "interrupt.cfg"
#include "remo_reg.cfg"
#include "twi.cfg"
#include "uart.cfg"
#include "ext.cfg"

#define F_CPU 11059200UL

/*-- macro section start -----------------------------------------------------*/
/**
 * @def SETBIT(ADDRESS, BIT)
 * @ingroup macro_macro
 * @brief 將 ADDRESS 指定的 BIT 設置為 1。
 */
#define SETBIT(ADDRESS, BIT) ((ADDRESS) |= (1 << BIT))

/**
 * @def CLRBIT(ADDRESS, BIT)
 * @ingroup macro_macro
 * @brief 將 ADDRESS 指定的 BIT 清除為 0。
 */
#define CLRBIT(ADDRESS, BIT) ((ADDRESS) &= ~(1 << BIT))

/**
 * @def CHKBIT(ADDRESS, BIT)
 * @ingroup macro_macro
 * @brief 檢查 ADDRESS 指定的 BIT 是 1 或 0。
 */
#define CHKBIT(ADDRESS, BIT) (((ADDRESS) & (1 << BIT)) == (1 << BIT))

/**
 * @def REGFPT(ADDRESS, MASK, SHIFT, DATA)
 * @ingroup macro_macro
 * @brief 依照指定的 MASK, SHIFT, DATA 去讀取暫存器 ADDRESS。
 */
#define REGFPT(ADDRESS, MASK, SHIFT, DATA) \
    ((ADDRESS) = (((ADDRESS) & (~MASK)) | (((DATA) << (SHIFT)) & (MASK))))

/**
 * @def REGFGT(ADDRESS, MASK, SHIFT, DATA_P)
 * @ingroup macro_macro
 * @brief 依照指定的 MASK, SHIFT, DATA_P 去寫入暫存器 ADDRESS。
 */
#define REGFGT(ADDRESS, MASK, SHIFT, DATA_P) \
    ((*((char *)DATA_P)) = (((ADDRESS) & (MASK)) >> (SHIFT)))

/**
 * @def REGPUT(REG_P, BYTES, DATA_P)
 * @ingroup macro_macro
 * @brief 依照指定BYTES數的 *DATA_P 去寫入暫存器 *REG_P 。
 */
#define REGPUT(REG_P, BYTES, DATA_P)                                        \
    do {                                                                    \
        for (signed char __putptr = BYTES - 1; __putptr >= 0; __putptr--) { \
            *((volatile char *)REG_P + __putptr) =                          \
                *((volatile char *)DATA_P + __putptr);                      \
        }                                                                   \
    } while (0)

/**
 * @def REGGET(REG_P, BYTES, DATA_P)
 * @ingroup macro_macro
 * @brief 依照指定BYTES數從暫存器 *REG_P 取得數值存入DATA_P。
 */
#define REGGET(REG_P, BYTES, DATA_P)                                        \
    do {                                                                    \
        for (signed char __getptr = BYTES - 1; __getptr >= 0; __getptr--) { \
            *((volatile char *)DATA_P + __getptr) =                         \
                *((volatile char *)REG_P + __getptr);                       \
        }                                                                   \
    } while (0)

/**
 * @def HIBYTE16(SOURCE)
 * @ingroup macro_macro
 * @brief
 */
#define HIBYTE16(SOURCE) (SOURCE >> 8)

/**
 * @def LOBYTE16(SOURCE)
 * @ingroup macro_macro
 * @brief
 */
#define LOBYTE16(SOURCE) (SOURCE & 0xff)

/**
 * @brief 無回傳、參數為(void *)型態之函式型態。
 */
typedef void (*Func_t)(void *);
/*-- macro section end -------------------------------------------------------*/

/*-- interrupt section start -------------------------------------------------*/
/**
 * @brief FuncBlock 工作管理結構
 * @ingroup interrupt_struct
 *
 * 此結構被應用在硬體中斷、IFD中斷中，為一項工作。
 * 結構中包含該工作是否開啟、觸發時要執行哪個函式、以及該函式的傳參。
 */
typedef struct {
    volatile uint8_t enable;  ///< 禁致能
    Func_t func_p;            ///< 觸發時執行函式
    void* funcPara_p;         ///< 觸發時執行函式之傳參
} FuncBlockStr_t;

/**
 * @brief IFD工作結構
 * @ingroup interrupt_struct
 *
 * 負責管理一項工作的觸發週期、相位、計數等參數。
 * 為IFD管理器底下使用的結構之一。
 */
typedef struct {
    volatile uint16_t cycle;    ///< 計數觸發週期。
    volatile uint16_t phase;    ///< 計數觸發相位。
    volatile uint16_t counter;  ///< 計數器，計數器固定頻率上數。
    volatile uint8_t enable;  ///< 禁致能控制，決定本逾時ISR是否致能。
    volatile Func_t func_p;  ///< 執行函式指標，為無回傳、傳參為void*函式。
    void* funcPara_p;  ///< 中斷中執行函式專用結構化資料住址指標。
} IntFreqDivISR_t;

/**
 * @brief IFD管理器結構
 * @ingroup interrupt_struct
 *
 * 提供給中斷除頻功能，Interrupt Frequence Diveder(IFD)相關函式使用的結構。
 * 負責管理登記好的IFD工作。
 */
typedef struct {
    uint8_t total;  ///< 紀錄已註冊IFD工作數量
    volatile IntFreqDivISR_t fb[MAX_IFD_FUNCNUM];  ///< 紀錄所有已註冊IFD工作
} IntFreqDivStr_t;

/**
 * @brief 初始化IFD管理器
 *
 * @param IntFreqDivStr_p IFD管理器結構指標
 */
void IntFreqDiv_net(IntFreqDivStr_t* IntFreqDivStr_p);

/**
 * @brief 註冊一項工作到IFD管理器中。
 * @ingroup interrupt_func
 *
 * @param IntFreqDivStr_p IFD管理器的指標。
 * @param FbFunc_p 要註冊的函式，為無回傳、傳參為void*函式。
 * @param FbPara_p 要註冊函式的傳參。
 * @param cycle 循環週期。
 * @param phase 觸發相位，循環週期中的第幾次計數觸發。
 * @return uint8_t IFD工作編號。
 *
 * 此函式會在IFD管理器建立一項IFD工作，並會回傳其在管理器中的工作編號。
 * IFD工作預設為關閉，可以使用 IntFreqDiv_en 開啟。當 IntFreqDiv_step
 * 執行一次時，IFD工作的計數器便會上數，並依據參數設定的循環週期、
 * 觸發相位來決定要不要觸發並執行IFD工作。
 */
uint8_t IntFreqDiv_reg(IntFreqDivStr_t* IntFreqDivStr_p, Func_t FbFunc_p,
                       void* FbPara_p, uint16_t cycle, uint16_t phase);

/**
 * @brief 啟用/關閉指定的IFD工作。
 * @ingroup interrupt_func
 *
 * @param IntFreqDivStr_p IFD管理器的指標。
 * @param Fb_Id 要啟用的IFD工作編號。
 * @param enable 是否啟用，1:啟用、0:關閉。
 *
 * 此函式可以控制一項IFD工作要不要啟用，依照工作編號去開關IFD管理器中相對應編號
 * 的IFD工作。啟用後IFD工作中的計數器才會開始計數，也才能被觸發。
 *
 * 若輸入的編號還沒有被註冊，將不會有任何動作。
 */
void IntFreqDiv_en(IntFreqDivStr_t* IntFreqDivStr_p, uint8_t Fb_Id,
                   uint8_t enable);

/**
 * @brief 透過IFD管理器計數一次。
 * @ingroup interrupt_func
 *
 * @param IntFreqDivStr_p IFD管理器的指標。
 *
 * 此函式會去計數IFD管理器供登記並啟用的IFD工作，若計數大小與觸發相位相等時，
 * 便會執行IFD工作。
 *
 * 此函式可以被註冊到硬體中斷中，如此一來，中斷觸發時便會計數並觸發工作。也可
 * 以透過手動呼叫此函式來計數。
 */
void IntFreqDiv_step(IntFreqDivStr_t* IntFreqDivStr_p);
/*-- interrupt section end ---------------------------------------------------*/

/*-- hardware section start --------------------------------------------------*/
/**
 * @brief tim flag put 函式
 *
 * @ingroup hw_tim_func
 * @param REG_p 暫存器實際位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 LSByte 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char TIM_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief tim flag get 函式
 *
 * @ingroup hw_tim_func
 * @param REG_p 暫存器實際位址。
 * @param Mask   遮罩。
 * @param Shift  向右位移。
 * @param Data_p 資料指標。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 LSByte 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char TIM_fgt(volatile unsigned char *REG_p, char Mask, char Shift, void *Data_p);

/**
 * @brief tim put 函式
 *
 * @ingroup hw_tim_func
 * @param REG_p 暫存器實際位址。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char TIM_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief tim get 函式
 *
 * @ingroup hw_tim_func
 * @param REG_p 暫存器實際位址。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char TIM_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief 與 TIM_fpt 相同。
 */
char PWM_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief 與 TIM_fgt 相同。
 */
char PWM_fgt(volatile unsigned char *REG_p, char Mask, char Shift, void *Data_p);

/**
 * @brief 與 TIM_put 相同。
 */
char PWM_put(volatile unsigned char *REG_p, char Bytes, void* Data_p);

/**
 * @brief 與 TIM_get 相同。
 */
char PWM_get(volatile unsigned char *REG_p, char Bytes, void* Data_p);

/**
 * @brief dio flag put 函式
 *
 * @ingroup hw_dio_func
 * @param REG_p 暫存器編號。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 *   - 6：警告操作方式與輸出入設定不同，但依然會執行操作。
 */
char DIO_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief dio flag get 函式
 *
 * @ingroup hw_dio_func
 * @param REG_p 暫存器編號。
 * @param Mask   遮罩。
 * @param Shift  向右位移。
 * @param Data_p 資料指標。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 *   - 6：警告操作方式與輸出入設定不同，但依然會執行操作。
 */
char DIO_fgt(volatile unsigned char *REG_p, char Mask, char Shift,
             void *Data_p);

/**
 * @brief dio put 函式
 *
 * @ingroup hw_dio_func
 * @param REG_p 暫存器編號。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 *   - 6：警告操作方式與輸出入設定不同，但依然會執行操作。
 */
char DIO_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief dio get 函式
 *
 * @ingroup hw_dio_func
 * @param REG_p 暫存器編號。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 *   - 6：警告操作方式與輸出入設定不同，但依然會執行操作。
 */
char DIO_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief 與 DIO_fpt 相同。
 */
char EXT_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief 與 DIO_fgt 相同。
 */
char EXT_fgt(volatile unsigned char *REG_p, char Mask, char Shift, void *Data_p);

/**
 * @brief adc flag put 函式
 *
 * @ingroup hw_adc_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 *REG_p 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */

char ADC_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief adc flag get 函式
 *
 * @ingroup hw_adc_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 *REG_p 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char ADC_fgt(volatile unsigned char *REG_p, char Mask, char Shift,
             void *Data_p);

/**
 * @brief adc put 函式
 *
 * @ingroup hw_adc_func
 * @param REG_p 暫存器位址。
 * @param Bytes  資料大小。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 *REG_p 錯誤。
 *   - 3：參數 Bytes 錯誤。
 */
char ADC_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief adc get 函式
 *
 * @ingroup hw_adc_func
 * @param REG_p 暫存器位址。
 * @param Bytes  資料大小。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 *REG_p 錯誤。
 *   - 3：參數 Bytes 錯誤。
 */
char ADC_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief adc set 函式
 *
 * @ingroup hw_adc_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 *REG_p 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char ADC_set(volatile unsigned char *REG_p, char Mask, char Shift, char Data);


/**
 * @brief spi flag put 函式
 *
 * @ingroup hw_spi_func
 * @param REG_p 暫存器實際位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 LSByte 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char SPI_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief spi flag get 函式
 *
 * @ingroup hw_spi_func
 * @param REG_p 暫存器實際位址。
 * @param Mask   遮罩。
 * @param Shift  向右位移。
 * @param Data_p 資料指標。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 LSByte 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char SPI_fgt(volatile unsigned char *REG_p, char Mask, char Shift, void *Data_p);

/**
 * @brief spi put 函式
 *
 * @ingroup hw_spi_func
 * @param REG_p 暫存器實際位址。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char SPI_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief spi get 函式
 *
 * @ingroup hw_spi_func
 * @param REG_p 暫存器實際位址。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char SPI_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);

#include <avr/interrupt.h>
#undef ISR
#ifdef __cplusplus
#    define ISR(vector, ...)                                                 \
        extern "C" void vector##_routine(void) __attribute__((__INTR_ATTRS)) \
            __VA_ARGS__;                                                     \
        void vector##_routine(void)
#else
#    define ISR(vector, ...)                                                   \
        void vector##_routine(void) __attribute__((__INTR_ATTRS)) __VA_ARGS__; \
        void vector##_routine(void)
#endif

/**
 * @brief EEPROM設定函式
 *
 * @ingroup eeprom_func
 * @param Address 要寫入的EEPROM位址。
 * @param Bytes   資料大小。
 * @param Data_p  資料指標。
 */
void EEPROM_set(int Address, char Bytes, void *Data_p);

/**
 * @brief EEPROM get 函式
 *
 * @ingroup eeprom_func
 * @param Address 要讀取的EEPROM位址。
 * @param Bytes   資料大小。
 * @param Data_p  資料指標。
 */
void EEPROM_get(int Address, char Bytes, void *Data_p);

/**
 * @brief twi flag put 函式
 *
 * @ingroup hw_twi_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 */
char TWI_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief twi flag get 函式
 *
 * @ingroup hw_twi_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 */
char TWI_fgt(volatile unsigned char *REG_p, char Mask, char Shift,
             void *Data_p);

/**
 * @brief twi put 函式
 *
 * @ingroup hw_twi_func
 * @param REG_p 暫存器位址。
 * @param Bytes  資料大小。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 3：參數 Bytes 錯誤。
 */
char TWI_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief twi get 函式
 *
 * @ingroup hw_twi_func
 * @param REG_p 暫存器位址。
 * @param Bytes  資料大小。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 3：參數 Bytes 錯誤。
 */
char TWI_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief twi set 函式
 *
 * @ingroup hw_twi_func
 * @param REG_p 暫存器位址。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 */
char TWI_set(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief UART flag put函式
 *
 * @ingroup hw_uart_func
 * @param REG_p 暫存器編號。
 * @param Mask   遮罩。
 * @param Shift  向左位移。
 * @param Data   寫入資料。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char UART_fpt(volatile unsigned char *REG_p, char Mask, char Shift, char Data);

/**
 * @brief ASA UART flag get函式
 *
 * @ingroup hw_uart_func
 * @param REG_p 暫存器編號。
 * @param Mask   遮罩。
 * @param Shift  向右位移。
 * @param Data_p 資料指標。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 2：參數 REG_p 錯誤。
 *   - 4：參數 Mask 錯誤。
 *   - 5：參數 Shift 超出範圍。
 */
char UART_fgt(volatile unsigned char *REG_p, char Mask, char Shift,
              void *Data_p);

/**
 * @brief ASA UART put函式
 *
 * @ingroup hw_uart_func
 * @param REG_p 暫存器編號。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char UART_put(volatile unsigned char *REG_p, char Bytes, void *Data_p);

/**
 * @brief ASA UART get函式
 *
 * @ingroup hw_uart_func
 * @param REG_p 暫存器編號。
 * @param Bytes  資料大小。
 * @param Data_p 資料指標。
 * @return char  錯誤代碼：
 *   - 0：成功無誤。
 *   - 3：參數 Bytes 錯誤。
 */
char UART_get(volatile unsigned char *REG_p, char Bytes, void *Data_p);
/*-- hardware section end ----------------------------------------------------*/

/*-- device section start ----------------------------------------------------*/
/**
 * @brief 進行硬體初始化的動作，包含asabus、stdio初始化。
 * @ingroup device_func
 *
 * 針對不同的硬體平台，將硬體初始化，包含下列動作：
 *   1. STDIO 及硬體初始化：請參照 C4M_STDIO_init。
 *   2. ASABUS ID 硬體初始化：請參照 ASABUS_ID_init。
 *   3. ASABUS SPI 硬體初始化：請參照 ASABUS_SPI_init。
 *   4. ASABUS UART 硬體初始化：請參照 ASABUS_UART_init。
 *   5. EEPROM 初始化及讀取裝置ID。
 *   6. 外掛函式系統的初始化。
 */
void C4M_DEVICE_set(void);
/*-- device section end ------------------------------------------------------*/

/*-- stdio section start -----------------------------------------------------*/
/**
 * @brief 進行標準IO進行初始化的動作。
 * @ingroup stdio_func
 *
 * 依據各個硬體平台去連結不同的通訊方式到 standard IO 中，
 * 便有printf、scanf等的標準IO操作函式可以使用。
 */
void C4M_STDIO_init(void);
/*-- stdio section end -------------------------------------------------------*/

/*-- asabus section start ----------------------------------------------------*/
/**
 * @brief 初始化ASABUS上的ID腳位。
 * @ingroup asabus_func
 *
 * ASABUS 上的 ID 腳位共有三隻，分別為ADDR0、ADDR1、ADDR2。
 * 呼叫此函式將會對前述三隻腳位進行初始化，設定為輸出。
 *
 * 以下是常見的板子上的ADDR腳位
 * asam128_v2:
 *  - ADDR0 : PF5
 *  - ADDR1 : PF6
 *  - ADDR2 : PF7
 *
 * 注意：在呼叫 C4M_DEVIDE_set 的時候也會執行此函式。
 */
void ASABUS_ID_init(void);

/**
 * @brief 設定ASABUS上使用的ID，0~7。
 * @ingroup asabus_func
 *
 * @param id 要設定的 ID 編號，0~7。
 *
 * 可以設定的ID為0~7，轉換成2進位每個bit分別對應到 ADDR0、ADDR1、ADDR2。
 * 若該bit被設定為1，則會令對應腳位輸出 1(logic high)。如果傳入 id
 * 編號非0~7，則不會進行設定的動作。
 *
 * 以下是常見的板子上的ADDR腳位<br>
 *  - asam128_v2:
 *    - ADDR0 : PF5
 *    - ADDR1 : PF6
 *    - ADDR2 : PF7
 */
void ASABUS_ID_set(char id);

/**
 * @brief 初始化ASABUS上的UART硬體。
 * @ingroup asabus_func
 *
 * 針對 ASABUS 上所使用的 UART 硬體進行初始化，會設定鮑率為38400、
 * 停止位元為1、傳輸大小為8位元。
 *
 * 注意：在呼叫 C4M_DEVIDE_set 的時候也會執行此函式。
 * 
 * 以下是常見的板子使用的uart硬體編號：<br>
 *  - asam128_v2: uart0
 */
void ASABUS_UART_init(void);

/**
 * @brief 將資料透過 ASABUS UART 傳送。
 * @ingroup asabus_func
 *
 * 等待並接收 ASABUS UART 來的資料，沒有逾時機制，
 * 所以若硬體錯誤將不會離開此函式。
 */
void ASABUS_UART_transmit(char data);

/**
 * @brief 等待並接收 ASABUS UART 來的資料。
 * @ingroup asabus_func
 *
 * @return char 接收回來的資料。
 *
 * 沒有逾時機制，所以若硬體錯誤將不會離開此函式。
 * 所以若硬體錯誤將不會離開此函式。
 */
char ASABUS_UART_receive(void);

/**
 * @brief 初始化ASABUS上的SPI硬體。
 * @ingroup asabus_func
 *
 * 針對 ASABUS 上所使用的 SPI 硬體進行初始化，設定為master模式，
 * SCK頻率則會因硬體而異。
 *
 * 注意：在呼叫 C4M_DEVIDE_set 的時候也會執行此函式。
 *
 * 以下是常見的板子使用的spi及初始化參數：<br>
 *  - asam128_v2: spi
 *    - SCK 頻率: fosc/64 = 11059200/64 = 172.8 kHz。
 *    - CPOL = 0，前緣為上升緣。
 *    - CPHA = 0，取樣時間為前緣。
 */
void ASABUS_SPI_init(void);

/**
 * @brief 與 ASABUS SPI 交換資料。
 * @ingroup asabus_func
 *
 * @param data 要交換的資料。
 * @return char 交換回來的資料。
 *
 * 沒有逾時機制，所以若硬體錯誤將不會離開此函式。
 * 
 * 以下是常見的板子使用的uart硬體編號：<br>
 *  - asam128_v2: spi
 */
char ASABUS_SPI_swap(char data);
/*-- asabus section end ------------------------------------------------------*/

/*-- asahmi section start ----------------------------------------------------*/
// HMI macro
#define HMI_TYPE_I8 \
    0  ///< 資料型態編號 0，int8_t、char                @ingroup asahmi_macro
#define HMI_TYPE_I16 \
    1  ///< 資料型態編號 1，int16_t、int                @ingroup asahmi_macro
#define HMI_TYPE_I32 \
    2  ///< 資料型態編號 2，int32_t、long int           @ingroup asahmi_macro
#define HMI_TYPE_I64 \
    3  ///< 資料型態編號 3，int64_t                     @ingroup asahmi_macro
#define HMI_TYPE_UI8 \
    4  ///< 資料型態編號 4，uint8_t、unsigned char      @ingroup asahmi_macro
#define HMI_TYPE_UI16 \
    5  ///< 資料型態編號 5，uint16_t、unsigned int      @ingroup asahmi_macro
#define HMI_TYPE_UI32 \
    6  ///< 資料型態編號 6，uint32_t、unsigned long int @ingroup asahmi_macro
#define HMI_TYPE_UI64 \
    7  ///< 資料型態編號 7，uint64_t                    @ingroup asahmi_macro
#define HMI_TYPE_F32 \
    8  ///< 資料型態編號 8，float、double               @ingroup asahmi_macro
#define HMI_TYPE_F64 \
    9  ///< 資料型態編號 9，AVR不支援64位元浮點數        @ingroup asahmi_macro

// HMI declaration
/**
 * @brief 發送陣列(1D)到HMI。
 *
 * @ingroup asahmi_func
 * @param Type 陣列的資料型態編號，詳見資料型態對應編號表。
 * @param Num 陣列的個數。
 * @param Data_p 存放陣列的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char 錯誤代碼：：
 *   - 0：成功無誤。
 *   - 1：資料總大小超過30000，請自行切分，並發送。
 *
 * 透過開發版上與PC的接口發送一維陣列資料，發送以 Data_p 為記憶體開頭的陣列資料
 * ，通常為一維的C陣列，會讀取 Num * 形態大小 Bytes的記憶體，若存取記憶體大小超
 * 過Data_p大小，則會發生記憶體非法操作，進而產生錯誤，所以務必確認參數 Num
 * 與實際陣列的個數吻和。
 */
char HMI_put_array(char Type, char Num, void *Data_p);

/**
 * @brief 從HMI接收陣列(1D)。
 *
 * @ingroup asahmi_func
 * @param Type 陣列的資料型態編號，詳見資料型態對應編號表。
 * @param Num 陣列的資料個數。
 * @param Data_p 陣列的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char 錯誤代碼：：
 *   - 0：成功無誤。
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。
 *   - 2：封包類型對應錯誤，接收到的資料封包非陣列封包，請檢察通訊雙方流程是否對應。
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。
 *   - 4：資料型態編號對應錯誤，發送端與接收端的資料型態編號不吻合。
 *   - 5：陣列個數對應錯誤，發送端與接收端的資料個數不吻合。
 *
 * 透過開發版上與PC的接口接收一維陣列資料，放入記憶體位置 Data_p，並會檢查封包類
 * 型、資料型態、及數量等參數。若回傳帶碼不為0，則代表接收失敗，並不會對Data_p
 * 進行寫入。若回傳帶碼為0，會把資料放入Data_p中，請確保Data_p的指標型態與要接
 * 收的資料一致、或足夠容納傳送而來資資料。若Data_p可用記憶體大小小於送來資料大
 * 小，則會發生記憶體非法操作，進而產生錯誤。
 */
char HMI_get_array(char Type, char Num, void *Data_p);

/**
 * @brief 發送矩陣(2D)到HMI。
 *
 * @ingroup asahmi_func
 * @param Type 矩陣的資料型態編號，詳見資料型態對應編號表。
 * @param Dim1 矩陣的維度1大小。
 * @param Dim2 矩陣的維度2大小。
 * @param Data_p 矩陣的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：資料總大小超過30000，請自行切分，並發送。
 *
 * 透過開發版上與PC的接口發送二維矩陣資料，發送以 Data_p 為記憶體開頭的矩陣資料
 * ，通常為二維的C陣列，會讀取 Dim1 * Dim2 * 形態大小 Bytes的記憶體，若存取記
 * 憶體大小超過Data_p大小，則會發生記憶體非法操作，進而產生錯誤，所以務必確認參
 * 數Dim1、Dim2與實際矩陣的個數吻和。
 */
char HMI_put_matrix(char Type, char Dim1, char Dim2, void *Data_p);

/**
 * @brief 從HMI接收矩陣(2D)。
 *
 * @ingroup asahmi_func
 * @param Type 矩陣的資料型態編號，詳見資料型態對應編號表。
 * @param Dim1 矩陣的維度1大小。
 * @param Dim2 矩陣的維度2大小。
 * @param Data_p 存放矩陣的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。
 *   - 2：封包類型對應錯誤，接收到的資料封包非矩陣封包，請檢察通訊雙方流程是否對應。
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。
 *   - 4：資料型態編號對應錯誤，發送端與接收端的資料型態編號不吻合。
 *   - 5：矩陣維度一對應錯誤，發送端與接收端的維度一大小不吻合。
 *   - 6：矩陣維度二對應錯誤，發送端與接收端的維度二大小不吻合。
 *
 * 透過開發版上與PC的接口接收二維矩陣資料，放入記憶體位置 Data_p，並會檢查封包類
 * 型、資料型態、及數量等參數。若回傳帶碼不為0，則代表接收失敗，並不會對Data_p
 * 進行寫入。若回傳帶碼為0，會把資料放入Data_p中，請確保Data_p的指標型態與要接
 * 收的資料一致、或足夠容納傳送而來資資料。若Data_p可用記憶體大小小於送來資料大
 * 小，則會發生記憶體非法操作，進而產生錯誤。
 */
char HMI_get_matrix(char Type, char Dim1, char Dim2, void *Data_p);

/**
 * @brief 發送結構到HMI。
 *
 * @ingroup asahmi_func
 * @param FormatString 代表此結構格式的格式字串，詳見FormatString 格式字串。
 * @param Bytes 結構的大小(bytes)。
 * @param Data_p 結構的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char  錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：資料總大小超過30000，請自行切分，並發送。
 *
 * 透過開發版上與PC的接口發送結構資料，發送以 Data_p 為記憶體開頭的結構資料，
 * 會讀取 Bytes 大小的記憶體，若存取記體大小超過Data_p大小，則會發生記憶體非
 * 法操作，進而產生錯誤，所以務必確認參數Bytes與實際結構的大小吻和。
 */
char HMI_put_struct(const char *FormatString, int Bytes, void *Data_p);

/**
 * @brief 從HMI接收結構。
 *
 * @ingroup asahmi_func
 * @param FormatString 代表此結構格式的格式字串，詳見FormatString 格式字串。
 * @param Bytes 結構的大小(bytes)。
 * @param Data_p 存放結構的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。
 *   - 2：封包類型對應錯誤，接收到的資料封包非矩陣封包，請檢察通訊雙方流程是否對應。
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。
 *   - 4：結構格式字串對應錯誤，發送端與接收端的結構格式字串不吻合。
 *   - 5：結構大小對應錯誤，發送端與接收端的結構大小不吻合。
 *
 * 透過開發版上與PC的接口發送結構資料，發送以 Data_p 為記憶體開頭的結構資料，
 * 會讀取 Bytes 大小的記憶體，若存取記體大小超過Data_p大小，則會發生記憶體非
 * 法操作，進而產生錯誤，所以務必確認參數Bytes與實際結構的大小吻和。
 */
char HMI_get_struct(const char *FormatString, int Bytes, void *Data_p);

/**
 * @brief 主動同步後，發送陣列(1D)到HMI。
 *
 * @ingroup asahmi_func
 * @param Type 陣列的資料型態編號，詳見資料型態對應編號表。
 * @param Num 陣列的個數。
 * @param Data_p 存放陣列的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：資料總大小超過30000，請自行切分，並發送。
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。
 * 透過開發版上與PC的接口發送一維陣列資料，發送以 Data_p 為記憶體開頭的陣列資料
 * ，通常為一維的C陣列，會讀取 Num * 形態大小 Bytes的記憶體，若存取記憶體大小超
 * 過Data_p大小，則會發生記憶體非法操作，進而產生錯誤，所以務必確認參數 Num
 * 與實際陣列的個數吻和。
 */
char HMI_snput_array(char Type, char Num, void *Data_p);

/**
 * @brief 主動同步後，從HMI接收陣列(1D)。
 *
 * @ingroup asahmi_func
 * @param Type 陣列的資料型態編號，詳見資料型態對應編號表。
 * @param Num 陣列的資料個數。
 * @param Data_p 陣列的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。
 *   - 2：封包類型對應錯誤，接收到的資料封包非陣列封包，請檢察通訊雙方流程是否對應。
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。
 *   - 4：資料型態編號對應錯誤，發送端與接收端的資料型態編號不吻合。
 *   - 5：陣列個數對應錯誤，發送端與接收端的資料個數不吻合。
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。<br>
 * 透過開發版上與PC的接口接收一維陣列資料，放入記憶體位置 Data_p，並會檢查封包類
 * 型、資料型態、及數量等參數。若回傳帶碼不為0，則代表接收失敗，並不會對Data_p
 * 進行寫入。若回傳帶碼為0，會把資料放入Data_p中，請確保Data_p的指標型態與要接
 * 收的資料一致、或足夠容納傳送而來資資料。若Data_p可用記憶體大小小於送來資料大
 * 小，則會發生記憶體非法操作，進而產生錯誤。
 */
char HMI_snget_array(char Type, char Num, void *Data_p);

/**
 * @brief 主動同步後，發送矩陣(2D)到HMI。
 *
 * @ingroup asahmi_func
 * @param Type 矩陣的資料型態編號，詳見資料型態對應編號表。
 * @param Dim1 矩陣的維度1大小。
 * @param Dim2 矩陣的維度2大小。
 * @param Data_p 矩陣的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 1：資料總大小超過30000，請自行切分，並發送。
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。<br>
 * 透過開發版上與PC的接口發送二維矩陣資料，發送以 Data_p 為記憶體開頭的矩陣資料
 * ，通常為二維的C陣列，會讀取 Dim1 * Dim2 * 形態大小 Bytes的記憶體，若存取記
 * 憶體大小超過Data_p大小，則會發生記憶體非法操作，進而產生錯誤，所以務必確認參
 * 數Dim1、Dim2與實際矩陣的個數吻和。
 */
char HMI_snput_matrix(char Type, char Dim1, char Dim2, void *Data_p);

/**
 * @brief 主動同步後，從HMI接收矩陣(2D)。
 *
 * @ingroup asahmi_func
 * @param Type 矩陣的資料型態編號，詳見資料型態對應編號表。
 * @param Dim1 矩陣的維度1大小。
 * @param Dim2 矩陣的維度2大小。
 * @param Data_p 存放矩陣的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。
 *   - 2：封包類型對應錯誤，接收到的資料封包非矩陣封包，請檢察通訊雙方流程是否對應。
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。
 *   - 4：資料型態編號對應錯誤，發送端與接收端的資料型態編號不吻合。
 *   - 5：矩陣維度一對應錯誤，發送端與接收端的維度一大小不吻合。
 *   - 6：矩陣維度二對應錯誤，發送端與接收端的維度二大小不吻合。
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。<br>
 * 透過開發版上與PC的接口接收二維矩陣資料，放入記憶體位置 Data_p，並會檢查封包類
 * 型、資料型態、及數量等參數。若回傳帶碼不為0，則代表接收失敗，並不會對Data_p
 * 進行寫入。若回傳帶碼為0，會把資料放入Data_p中，請確保Data_p的指標型態與要接
 * 收的資料一致、或足夠容納傳送而來資資料。若Data_p可用記憶體大小小於送來資料大
 * 小，則會發生記憶體非法操作，進而產生錯誤。
 */
char HMI_snget_matrix(char Type, char Dim1, char Dim2, void *Data_p);

/**
 * @brief 主動同步後，發送結構到HMI。
 *
 * @ingroup asahmi_func
 * @param FormatString 代表此結構格式的格式字串，詳見FormatString 格式字串。
 * @param Bytes 結構的大小(bytes)。
 * @param Data_p 結構的起始記憶體位置，會依序讀取此記憶體後續的資料，並送出。
 * @return char  錯誤代碼： <br>
 *   - 0：成功無誤。<br>
 *   - 1：資料總大小超過30000，請自行切分，並發送。<br>
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。<br>
 * 透過開發版上與PC的接口發送結構資料，發送以 Data_p 為記憶體開頭的結構資料，
 * 會讀取 Bytes 大小的記憶體，若存取記體大小超過Data_p大小，則會發生記憶體非
 * 法操作，進而產生錯誤，所以務必確認參數Bytes與實際結構的大小吻和。
 */
char HMI_snput_struct(const char *FormatString, int Bytes, void *Data_p);

/**
 * @brief 主動同步後，從HMI接收結構。
 *
 * @ingroup asahmi_func
 * @param FormatString 代表此結構格式的格式字串，詳見FormatString 格式字串。
 * @param Bytes 結構的大小(bytes)。
 * @param Data_p 存放結構的起始記憶體位置，會依序寫入此記憶體後續的資料。
 * @return char  錯誤代碼： <br>
 *   - 0：成功無誤。<br>
 *   - 1：封包頭錯誤，請檢察通訊雙方流程是否對應。<br>
 *   - 2：封包類型對應錯誤，接收到的資料封包非矩陣封包，請檢察通訊雙方流程是否對應。<br>
 *   - 3：封包檢查碼錯誤，請檢查通訊線材品質，並重新發送一次。<br>
 *   - 4：結構格式字串對應錯誤，發送端與接收端的結構格式字串不吻合。<br>
 *   - 5：結構大小對應錯誤，發送端與接收端的結構大小不吻合。<br>
 *   - 7：同步失敗。
 *
 * 發送同步請求與HMI端進行同步動作，同步成功後開始從HMI接收結構，若同步失敗則不
 * 進行後續動作。<br>
 * 透過開發版上與PC的接口發送結構資料，發送以 Data_p 為記憶體開頭的結構資料，
 * 會讀取 Bytes 大小的記憶體，若存取記體大小超過Data_p大小，則會發生記憶體非
 * 法操作，進而產生錯誤，所以務必確認參數Bytes與實際結構的大小吻和。
 */
char HMI_snget_struct(const char *FormatString, int Bytes, void *Data_p);
/*-- asahmi section end ------------------------------------------------------*/

/*-- asatwi section start ----------------------------------------------------*/
/**
 * @brief ASA TWI Master 多位元組傳送函式。
 *
 * @ingroup asatwi_func
 * @param mode      TWI通訊模式，目前支援：1、2、3、4、5、6。
 * @param SLA       Slave(僕)裝置的TWI ID。
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Bytes     待送資料位元組數。
 * @param Data_p    待送資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *
 * ASA TWI Master Transmit 函式，依照功能分為6種Mode，通訊方式如下：
 *  - mode 1：
 *      TWI通訊第一 Bytes 為 [RegAdd | Data_p[Bytes-1]] ，並由高至低傳輸，
 *      使用者須自行將Data_p的最高位元向右位移RegAdd bits數。
 *      此mode中的RegAdd為控制旗標(control flag)。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *
 *  - mode 2：
 *      TWI通訊第一 Bytes 為 [RegAdd | Data_p[0]] ，並由低至高傳輸，
 *      使用者須自行將Data_p的最高位元向右位移RegAdd bits數。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *
 *  - mode 3：TWI通訊不指定RegAdd，並由高至低傳輸。
 *  - mode 4：TWI通訊不指定RegAdd，並由低至高傳輸。
 *  - mode 5：TWI通訊指定RegAdd，並由高至低傳輸。
 *  - mode 6：TWI通訊指定RegAdd，並由低至高傳輸。
 */
char TWIM_trm(char mode, char SLA, char RegAdd, char Bytes, uint8_t *Data_p, uint16_t WaitTick);

/**
 * @brief ASA TWI Master 多位元組接收函式。
 *
 * @ingroup asatwi_func
 * @param mode      TWI通訊模式，目前支援：1、2、3、4、5、6。
 * @param SLA       Slave(僕)裝置的TWI ID。
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Bytes     待收資料位元組數。
 * @param Data_p    待收資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *
 * ASA TWI Master receive 函式，依照功能分為6種Mode，通訊方式如下：
 *  - mode 1：TWI通訊第一Bytes為[RegAdd | Data_p[Bytes-1]]，並由高至低接收。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - mode 2：TWI通訊第一Bytes為[RegAdd | Data_p[0]]，並由低至高接收。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - mode 3：TWI通訊不指定RegAdd，並由高至低接收。
 *  - mode 4：TWI通訊不指定RegAdd，並由低至高接收。
 *  - mode 5：TWI通訊指定RegAdd，並由高至低接收。
 *  - mode 6：TWI通訊指定RegAdd，並由低至高接收。
 */
char TWIM_rec(char mode, char SLA, char RegAdd, char Bytes, uint8_t *Data_p,
              uint16_t WaitTick);

/**
 * @brief ASA TWI Master 旗標式傳送函式。
 *
 * @ingroup asatwi_func
 * @param mode      TWI通訊模式，目前支援：3、5。
 * @param SLA       Slave(僕)裝置的TWI ID。
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Mask      位元組遮罩。
 * @param Shift     待送旗標向左位移。
 * @param Data_p    待送資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * TWI Master(主)旗標式資料傳輸是由TWIM_trm()與TWIM_rec()實現，
 * 依照功能將只支援指定Mode，通訊方式如下：
 *  - mode 3：
 *      使用TWIM_rec() mode 3讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用TWIM_trm() mode 3傳輸資料。
 *
 *  - mode 5：
 *      使用TWIM_rec() mode 5讀取指定RegAdd中的資料，將資料左移後，
 *      用遮罩取資料，最後使用TWIM_trm() mode 5傳輸資料到指定RegAdd。
 */
char TWIM_ftm(char mode, char SLA, char RegAdd, char Mask, char Shift,
              uint8_t *Data_p, uint16_t WaitTick);

/**
 * @brief ASA TWI Master 旗標式接收函式。
 *
 * @ingroup asatwi_func
 * @param mode      TWI通訊模式，目前支援：3、5。
 * @param SLA       Slave(僕)裝置的TWI ID。
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Mask      位元組遮罩。
 * @param Shift     接收旗標向右位移。
 * @param Data_p    待收資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * TWI Master(主)旗標式資料傳輸是由TWIM_rec()實現，
 * 依照功能將只支援指定Mode，通訊方式如下：
 *  - mode 3：使用TWIM_rec() mode 3讀取資料，將資料左移後，用遮罩取資料。
 *  - mode 5：
 *      使用TWIM_rec() mode 5讀取指定RegAdd中的資料，將資料左移後，
 *      用遮罩取資料。
 */
char TWIM_frc(char mode, char SLA, char RegAdd, char Mask, char Shift,
              uint8_t *Data_p, uint16_t WaitTick);

/**
 * @brief TWI Slave Mode 1 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 *
 * TWI Slave(僕) Mode 1 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 1，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的第一個Register(Reg_ID = 2)，由高至低傳輸。
 *  - TWI Slave Recive：
 *      將Master接收到的資料儲存到RemoRW_reg所註冊的第一個
 *      Register(Reg_ID = 2)，由高至低接收。
 *
 * @warning 此mode不支援廣播功能
 */
void ASA_TWIS1_step(void);

/**
 * @brief TWI Slave Mode 2 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 *
 * TWI Slave(僕) Mode 2 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 2，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的第一個Register(Reg_ID = 2)，由低至高傳輸。
 * 
 *  - TWI Slave Recive：
 *      將Master接收到的資料儲存到RemoRW_reg所註冊的第一個
 *      Register(Reg_ID = 2)，由低至高接收。
 *
 * @warning 此mode不支援廣播功能
 */
void ASA_TWIS2_step(void);

/**
 * @brief TWI Slave Mode 3 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 *
 * TWI Slave(僕) Mode 3 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 3，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的第一個Register(Reg_ID = 2)，由高至低傳輸。
 * 
 *  - TWI Slave Recive：
 *      將Master接收到的資料儲存到RemoRW_reg所註冊的第一個
 *      Register(Reg_ID = 2)，由高至低接收。
 *
 * @warning 此mode不支援廣播功能
 */
void ASA_TWIS3_step(void);

/**
 * @brief TWI Slave Mode 4 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 *
 * TWI Slave(僕) Mode 4 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 4，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的第一個Register(Reg_ID = 2)，由低至高傳輸。
 * 
 *  - TWI Slave Recive：
 *      將Master接收到的資料儲存到RemoRW_reg所註冊的第一個，
 *      Register(Reg_ID = 2)，由低至高接收。
 *  
 * @warning 此mode不支援廣播功能
 */
void ASA_TWIS4_step(void);

/**
 * @brief TWI Slave Mode 5 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 * TWI Slave(僕) Mode 5 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 5，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的指定Register，由高至低傳輸。
 * 
 *  - TWI Slave Recive：
 *      將 Master 接收到的資料儲存到 RemoRW_reg 所註冊的指定 
 *      Register，由高至低接收。
 */
void ASA_TWIS5_step(void);

/**
 * @brief TWI Slave Mode 6 串列埠中斷執行片段
 *
 * @ingroup asatwi_func
 * TWI Slave(僕) Mode 6 透過串列埠中斷來送收資料，其對應TWI Master(主) Mode 6，
 * 通訊方式如下：
 *  - TWI Slave Transmit：
 *      將RemoRW_reg所註冊的指定Register，由低至高傳輸。
 * 
 *  - TWI Slave Recive：
 *      將Master接收到的資料儲存到RemoRW_reg所註冊的指定
 *      Register，由低至高接收。
 */
void ASA_TWIS6_step(void);

#define TIMEOUTSETTING 500000  ///< TWI逾時設定   @ingroup asatwi_macro
#define CF_BIT 3               ///< TWI Contral Flag bit數設定 @ingroup asatwi_macro
#define CF_MASK 0xE0           ///< TWI Contral Flag 遮罩 @ingroup asatwi_macro
#define TWAR_MASK 0xFE         ///< TWI TWAR 遮罩 @ingroup asatwi_macro
#define TIMEOUT_FLAG 0X01      ///< TWI 逾時期標 @ingroup asatwi_macro
#define REG_MAX_COUNT 20       ///< TWI 暫存計數器最大值 @ingroup asatwi_macro
#define BUFF_MAX_SZ 32         ///< TWI BUFFER最大值 @ingroup asatwi_macro

/**
 * @brief TWI Hardware 傳輸
 *
 * @ingroup     asatwi_func
 * @param reg   傳輸資料。
 * @return  uint8_t 錯誤代碼：
 *                   - 1：Timeout
 *                   - 其餘編號：參考TWI狀態列表。
 *
 * 將欲傳送資料存入TWI Data Register (TWDR) 中，並對TWI Configer Register
 * (TWCR)中的TWINT和TWEN寫入。
 */
uint8_t TWI_Reg_tram(uint8_t reg);

/**
 * @brief TWI Hardware 接收 ACK回覆
 *
 * @ingroup     asatwi_func
 * @param data  指標指向資料。
 * @return  uint8_t 錯誤代碼：
 *                   - 1：Timeout
 *                   - 其餘編號：參考TWI狀態列表。
 *
 * 對TWI Configer Register (TWCR)中的TWINT、TWEN和TWEA寫入，並將TWI Data
 * Register (TWDR) 接收到的資料存入指定位址指標中。
 */
uint8_t TWI_Reg_rec(uint8_t *data);
/**
 * @brief TWI Hardware 接收 NACK回覆
 *
 * @ingroup     asatwi_func
 * @param data  指標指向資料。
 * @return  uint8_t 錯誤代碼：
 *                   - 1：Timeout
 *                   - 其餘編號：參考TWI狀態列表。
 *
 * 對TWI Configer Register (TWCR)中的TWINT和TWEN寫入，並將TWI Data
 * Register (TWDR) 接收到的資料存入指定位址指標中。
 */
uint8_t TWI_Reg_recNack(uint8_t *data);
/**
 * @brief TWI Hardware Acknowledge
 *
 * @ingroup asatwi_func
 * @param ack_p Acknowledge Flag
 *
 * 決定TWI Configer Register 中的Acknowledge bit(TWEA)是否要enable。
 */
void TWICom_ACKCom(uint8_t ack_p);
/**
 * @brief TWI Hardware Stop
 *
 * @ingroup asatwi_func
 * @param stop_p Stop Flag
 * 
 * 決定TWI Configer Register 中的Stop bit(TWSTO)是否要enable。
 */
void TWICom_Stop(uint8_t stop_p);
/**
 * @brief TWI Hardware Start
 *
 * @ingroup asatwi_func
 * @param _start start Flag
 *
 * 決定TWI Configer Register 中的Start bit(TWSTA)是否要enable。
 */
uint8_t TWICom_Start(uint8_t _start);
/*-- asatwi section end ------------------------------------------------------*/

/*-- asauart section start ---------------------------------------------------*/
/**
 * @brief ASA UART Master 多位元組傳送函式。
 *
 * @ingroup asauart_func
 *
 * @param Mode      UART 通訊模式，目前支援0, 1, 2, 3。
 * @param UartID    目標裝置的UART ID。
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Bytes     待送資料位元組數。
 * @param Data_p    待送資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * ASA 作為 Master端 透過UART通訊傳送資料。
 * Uart Master端 傳送給 Slave端，依照封包不同，分作11種 Mode ，如以下所示：
 *  - Mode 0：
 *      封包組成 為 [Header]、[UID]、[RegAdd]、[Data]、[Rec(Header)]。
 *      先後傳送資料 [ASAUART_CMD_HEADER (0xAA)]、[UID]、[RegAdd]，隨後再根據
 *      資料筆數[Bytes]由低到高丟出資料，傳完資料後會傳送最後一筆接收的資料
 *      [checksum] 給Slave端驗證，Slave會回傳當作 [ASAUART_RSP_HEADER(0xAB)]
 *      成功資訊或錯誤資訊(0x06)。
 *  - Mode 1：
 *      封包組成 為 [ Data+CF(Control Flag) ]。
 *      傳送第一筆資料為 [RegAdd | Data_p[0]]，剩餘的傳輸資料由低到高傳送。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - Mode 2：
 *      封包組成 為 [ CF(Control Flag)+Data ]。
 *      傳送第一筆資料為 [RegAdd |Data_p[Bytes-1]]，剩餘的傳輸資料由高到低傳送。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - Mode 3：
 *      封包組成 為 [Data]，單純送收資料。
 *      注意：RegAdd 2 專門用來傳送資料，RegAdd 3 專門用來接收資料。
 *      由低到高傳送資料。
 *  - Mode 4：
 *      封包組成 為 [Data]，單純送收資料。
 *      注意：RegAdd 2 專門用來傳送資料，RegAdd 3 專門用來接收資料。
 *      由高到低傳送資料。
 *  - Mode 5：
 *      封包組成 為 [RegAdd(8bit)]、[Data]。
 *      注意：RegAdd 2、3 專門用來傳送資料，RegAdd 4、5 專門用來接收資料。
 *      先傳送一筆資料為 [RegAdd]，接著由低到高傳送資料 [Data]。
 *  - Mode 6：
 *      封包組成 為 [RegAdd(8bit)]、[Data]。
 *      注意：RegAdd 2、3 專門用來傳送資料，RegAdd 4、5 專門用來接收資料。
 *      先傳送一筆資料 [RegAdd]，接著由高到低傳送資料 [Data]。
 *  - Mode 7：
 *      封包組成 為 [ W (1bit，0)+RegAdd(7bit) ]、[Data]。
 *      先傳送一筆資料 [RegAdd]，接著由低到高傳送資料 [Data]。
 *  - Mode 8：
 *      封包組成 為 [ W (1bit，0)+RegAdd(7bit) ] 、[Data]。
 *      先傳送一筆資料 [RegAdd]，接著由高到低傳送資料 [Data]。
 *  - Mode 9：
 *      封包組成 為 [ RegAdd(7bit)+W (1bit，0) ]、[Data]。
 *      先傳送一筆資料 [RegAdd<<1]，接著由低到高傳送資料 [Data]。
 *  - Mode 10：
 *      封包組成 為 [ RegAdd(7bit)+W (1bit，0) ]、[Data]。
 *      先傳送一筆資料 [RegAdd<<1]，接著由高到低傳送資料 [Data]。
 */
char UARTM_trm(char Mode, char UartID, char RegAdd, char Bytes, void *Data_p,uint16_t WaitTick);

/**
 * @brief ASA UART Master 多位元組接收函式。
 *
 * @ingroup asauart_func
 *
 * @param Mode      UART 通訊模式，目前支援0, 1, 2, 3。
 * @param UartID    UART僕ID：   目標裝置的UART ID
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Bytes     待收資料位元組數。
 * @param Data_p    待收資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * Uart Master端 傳送給 Slave端，依照封包不同，分作11種 Mode ，如以下所示：
 *  - Mode 0：
 *      封包組成 為 [Header]、[UID]、[RegAdd]、[Data]、[Rec(Header)]。
 *      先後傳送資料 [ASAUART_CMD_HEADER (0xAA)]、[UID]、[RegAdd]、[checksum]
 *      後，讀取Slave端回傳的[ASAUART_RSP_HEADER]，確認無誤後 再根據資料筆數
 *      [Bytes] 由低到高接收資料，接收資料完後會接收Slave傳送的最後一筆資料
 *      [checksum] 進行比對，無誤後將資料存取起來。
 *  - Mode 1：
 *      封包組成 為 [ Data+CF(Control Flag) ]。
 *      注意：限定搭配 UARTM_trm()後 使用。
 *      由低到高接收資料。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - Mode 2：
 *      封包組成 為 [ CF(Control Flag)+Data ]。
 *      注意：限定搭配 UARTM_trm()後 使用。
 *      由高到低接收資料。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *  - Mode 3：
 *      封包組成 為 [Data]，單純送收資料。
 *      注意：RegAdd 2 專門用來傳送資料，RegAdd 3 專門用來接收資料。
 *      由低到高接收資料。
 *  - Mode 4：
 *      封包組成 為 [Data]，單純送收資料。
 *      注意：RegAdd 2 專門用來傳送資料，RegAdd 3 專門用來接收資料。
 *      由高到低接收資料。
 *  - Mode 5：
 *      封包組成 為 [RegAdd(8bit)]、[Data]。
 *      注意：RegAdd 2、3 專門用來傳送資料，RegAdd 4、5
 *      專門用來接收資料。先傳送一筆資料為 [RegAdd] ，
 *      接著由低到高接收資料 [Data]。
 *  - Mode 6：
 *      封包組成 為 [RegAdd(8bit)]、[Data]。
 *      注意：RegAdd 2、3 專門用來傳送資料，RegAdd 4、5
 *      專門用來接收資料。先傳送一筆資料 [RegAdd]，
 *      接著由高到低接收資料 [Data]。
 *  - Mode 7：
 *      封包組成 為 [ R (1bit，1)+RegAdd(7bit) ]、[Data]。
 *      先傳送一筆資料 RegAdd佔最低位元、R位元佔最高位元
 *      [0x80 | RegAdd]，接著由低到高接收資料 [Data]。
 *  - Mode 8：
 *      封包組成 為 [ R (1bit，1)+RegAdd(7bit) ]、[Data]。
 *      先傳送一筆資料 RegAdd佔最低位元、R位元佔最高位元
 *      [0x80 | RegAdd]，接著由高到低接收資料 [Data]。
 *  - Mode 9：
 *      封包組成 為 [ RegAdd(7bit)+R (1bit，1) ]、[Data]。
 *      先傳送一筆資料 RegAdd佔最高位元、R位元佔最低位元
 *      [RegAdd<<1 | 0x01]，接著由低到高接收資料 [Data]。
 *  - Mode 10：
 *      封包組成 為 [ RegAdd(7bit)+R (1bit，1) ]、[Data]。
 *      先傳送一筆資料 RegAdd佔最高位元、R位元佔最低位元
 *      [RegAdd<<1 | 0x01]，接著由高到低接收資料 [Data]。
 */
char UARTM_rec(char Mode, char UartID, char RegAdd, char Bytes, void *Data_p,uint16_t WaitTick);

/**
 * @brief ASA UART Master 旗標式傳送函式。
 *
 * @ingroup asauart_func
 *
 * @param Mode      UART 通訊模式，目前支援0, 3, 5, 7, 9。
 * @param UartID    UART僕ID：      目標的裝置UART ID
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Mask      位元組遮罩。
 * @param Shift     待送旗標向左位移。
 * @param Data_p    待送資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * Uart Master 旗標式資料傳輸是由UARTM_trm()與UARTM_rec()實現。
 * 依照功能將只支援指定Mode，通訊方式如下：
 *  - Mode 0： 使用UARTM_rec() Mode 0 讀取指定UID、RegAdd中的資料，
 *    將資料左移後，用遮罩取資料，最後使用UARTM_trm() Mode 0
 *    傳輸資料到指定RegAdd。
 *  - Mode 3： 使用UARTM_rec() Mode 3 讀取資料，將資料左移後，
 *    用遮罩取資料，最後使用UARTM_trm() Mode 3 傳輸資料。
 *  - Mode 5： 使用UARTM_rec() Mode 5 讀取指定RegAdd中的資料，
 *    將資料左移後，用遮罩取資料，最後使用UARTM_trm() Mode 5
 *    傳輸資料到指定RegAdd。
 *  - Mode 7： 使用UARTM_rec() Mode 7 讀取指定RegAdd中的資料，
 *    將資料左移後，用遮罩取資料，最後使用UARTM_trm() Mode 7
 *    傳輸資料到指定RegAdd。
 *  - Mode 9： 使用UARTM_rec() Mode 9 讀取指定RegAdd中的資料，
 *    將資料左移後，用遮罩取資料，最後使用UARTM_trm() Mode 9
 *    傳輸資料到指定RegAdd。
 *
 */
char UARTM_ftm(char Mode, char UartID, char RegAdd, char Mask, char Shift,
               char *Data_p, uint16_t WaitTick);

/**
 * @brief ASA UART Master 旗標式接收函式。
 *
 * @ingroup asauart_func
 *
 * @param Mode      UART 通訊模式，目前支援0, 3, 5 ,7 ,9。
 * @param UartID    UART僕ID：      目標的裝置UART ID
 * @param RegAdd    遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Mask      位元組遮罩。
 * @param Shift     接收旗標向右位移。
 * @param Data_p    待收資料指標。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @return  char    錯誤代碼：
 *                   - 0： 通訊成功。
 *                   - 1： Timeout。
 *                   - 4： SLA錯誤。
 *                   - 5： mode選擇錯誤。
 *
 * Uart Master 旗標式資料接收是由TWIM_rec()實現。
 * 依照功能將只支援指定Mode，通訊方式如下：
 *  - Mode 0：
 *      使用UARTM_rec() Mode 0 讀取指定UID、RegAdd中的資料，
 *      將資料左移後，用遮罩取資料。
 *  - Mode 3：
 *      使用UARTM_rec() Mode 3 讀取資料，將資料左移後，
 *      用遮罩取資料。
 *  - Mode 5：
 *      使用UARTM_rec() Mode 5 讀取指定RegAdd中的資料，
 *      將資料左移後，用遮罩取資料。
 *  - Mode 7：
 *      使用UARTM_rec() Mode 7 讀取指定RegAdd中的資料，
 *      將資料左移後，用遮罩取資料。
 *  - Mode 9：
 *      使用UARTM_rec() Mode 9 讀取指定RegAdd中的資料，
 *      將資料左移後，用遮罩取資料。
 */
char UARTM_frc(char Mode, char UartID, char RegAdd, char Mask, char Shift,
               char *Data_p, uint16_t WaitTick);

/**
 * @brief Slave端 Mode 0 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 *
 * @warning 必須先將UART Interrupt致能，並注意是否已於 RemoReg_init()
 * 內實作將此函式連接至uart_hal.c內的UARTS_inst.rx_compelete_cb。
 */
void ASA_UARTS0_rx_step(void);

/**
 * @brief Slave端 Mode 1 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 *
 */
void ASA_UARTS1_rx_step(void);

/**
 * @brief Slave端 Mode 2 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */

void ASA_UARTS2_rx_step(void);

/**
 * @brief Slave端 Mode 3 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS3_rx_step(void);

/**
 * @brief Slave端 Mode 4 串列埠 Rx中斷 執行片段。
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS4_rx_step(void);

/**
 * @brief Slave端 Mode 5 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS5_rx_step(void);

/**
 * @brief Slave端 Mode 6 串列埠 Rx中斷 執行片段。
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS6_rx_step(void);

/**
 * @brief Slave端 Mode 7 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS7_rx_step(void);

/**
 * @brief Slave端 Mode 8 串列埠 Rx中斷 執行片段。
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS8_rx_step(void);

/**
 * @brief Slave端 Mode 9 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS9_rx(void);

/**
 * @brief Slave端 Mode 10 串列埠 Rx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS10_rx_step(void);

/**
 * @brief Slave端 Mode 0 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS0_tx_step(void);

/**
 * @brief Slave端 Mode 1 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS1_tx_step(void);

/**
 * @brief Slave端 Mode 2 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS2_tx_step(void);

/**
 * @brief Slave端 Mode 3 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS3_tx_step(void);

/**
 * @brief Slave端 Mode 4 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS4_tx_step(void);

/**
 * @brief Slave端 Mode 5 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS5_tx_step(void);

/**
 * @brief Slave端 Mode 6 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS6_tx_step(void);

/**
 * @brief Slave端 Mode 7 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS7_tx_step(void);

/**
 * @brief Slave端 Mode 8 串列埠 Tx中斷 執行片段。。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。。
 */
void ASA_UARTS8_tx_step(void);

/**
 * @brief Slave端 Mode 9 串列埠 Tx中斷 執行片段。
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS9_tx_step(void);

/**
 * @brief Slave端 Mode 10 串列埠 Tx中斷 執行片段。
 * @ingroup asauart_func
 *
 * 進行UART封包解包，解包狀態機轉移與執行皆於呼叫階段執行。
 */
void ASA_UARTS10_tx_step(void);
/*-- asauart section end -----------------------------------------------------*/

/*-- asaspi section start ----------------------------------------------------*/
/**
 * @brief ASA SPI Master 多位元組傳送函式。
 *
 * @ingroup asaspi_func
 * @param mode    SPI通訊模式，目前支援：0~10。
 * @param ASAID   ASA介面卡的ID編號。
 * @param RegAdd  遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Bytes   資料筆數。
 * @param WaitTick  位元組間延遲時間，單位為 1us。
 * @param *Data_p 待送資料指標。
 * @return char   錯誤代碼：
 *                 - 0：成功無誤。
 *                 - 5：模式選擇錯誤。
 *                 - 其他：錯誤。
 *
 * SPI Master傳輸給Slave裝置，依照功用分為10種mode，傳送方式如下：
 *  - mode 0 ：
 *      具check的SPI通訊方式，第一筆傳送給Slave端為[W | RegAdd]，
 *      之後開始傳送資料Data_p[0]直到Data[Bytes-1]，每次傳送給Slave端，
 *      Slave端會回傳上一筆資料給Master端，Master端會進行檢查，如果不一樣，
 *      再傳送最後一筆資料給Slave的時候，Slave會回傳非0的錯誤資訊。
 *
 *  - mode 1 ：
 *      SPI通訊的第一筆為[RegAdd | Data_p[Bytes-1]]，
 *      剩餘的傳輸資料由低到高傳輸。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *
 *  - mode 2 ：
 *      SPI通訊的第一筆為[RegAdd | Data_p[0]]，
 *      剩餘的傳輸資料由高到低傳輸。
 *      * 此 mode 中 RegAdd 為控制旗標(control flag)。
 *
 *  - mode 3 ：SPI通訊的傳輸資料由低到高傳輸，純送資料，未送RegAdd。
 *  - mode 4 ：SPI通訊的傳輸資料由高到低傳輸，純送資料，未送RegAdd。
 *  - mode 5 ：SPI通訊的第一筆為[RegAdd]，傳輸資料由低到高傳輸。
 *  - mode 6 ：SPI通訊的第一筆為[RegAdd]，傳輸資料由高到低傳輸。
 *  - mode 7 ：SPI通訊的第一筆為[W | RegAdd]，傳輸資料由低到高傳輸。
 *  - mode 8 ：SPI通訊的第一筆為[W | RegAdd]，傳輸資料由高到低傳輸。
 *  - mode 9 ：SPI通訊的第一筆為[(RegAdd<<1) | W]，傳輸資料由低到高傳輸。
 *  - mode 10：SPI通訊的第一筆為[(RegAdd<<1) | W]，傳輸資料由高到低傳輸。
 *  - mode 100以上，為外掛式SPI Master(主)傳輸資料。
 */
char ASA_SPIM_trm(char mode, char ASAID, char RegAdd, char Bytes, void *Data_p,
                  uint16_t WaitTick);

/**
 * @brief ASA SPI Master 多位元組接收函式。
 *
 * @ingroup asaspi_func
 * @param mode    SPI通訊模式，目前支援：0、3~10。
 * @param ASAID   ASA介面卡的ID編號。
 * @param RegAdd  控制旗標(control flag)或遠端讀寫暫存器的ID.
 * @param Bytes   資料筆數。
 * @param WaitTick 位元組間延遲時間，單位為 1us。
 * @param *Data_p 待收資料指標。
 * @return char   錯誤代碼：
 *                 - 0：成功無誤。
 *                 - 5：模式選擇錯誤。
 *                 - 其他：錯誤。
 *
 * SPI Master從Slave裝置接收資料，依照功用分為10種mode，接收方式如下：
 *  - mode 0 ：
 *      具check的SPI通訊方式，第一筆傳送給Slave端為[RegAdd]，
 *      之後開始由低到高接收資料依序儲存到 *Data_p，
 *      接收完成後會傳送最後一筆接收的資料給Slave端驗證，
 *      Slave會回傳正確或錯誤資訊。
 *
 *  - mode 1 ：回傳5，表示模式選擇錯誤。
 *  - mode 2 ：回傳5，表示模式選擇錯誤。
 *  - mode 2 ：回傳5，表示模式選擇錯誤。
 *  - mode 3 ：SPI通訊的第一筆為[0x00]]，純接資料，由低到高依序存入*Data_p。
 *  - mode 4 ：SPI通訊的第一筆為[0x00]]，純接資料，由高到低依序存入*Data_p。
 *  - mode 5 ：SPI通訊的，和Slave交換資料由低到高傳輸。
 *  - mode 6 ：SPI通訊的，和Slave交換資料由高到低傳輸。
 *  - mode 7 ：SPI通訊的第一筆為[R | RegAdd]，之後和Slave交換資料由低到高傳輸。
 *  - mode 8 ：SPI通訊的第一筆為[R | RegAdd]，之後和Slave交換資料由高到低傳輸。
 *  - mode 9 ：
 *      SPI通訊和Slave交換資料第一筆為[(RegAdd<<1) | R]，
 *      之後交換資料由低到高傳輸。
 *  - mode 10：
 *      SPI通訊和Slave交換資料第一筆為[(RegAdd<<1) | R]，
 *      之後交換資料由高到低傳輸。
 *  - mode 100以上，為外掛式SPI Master(主)接收資料。
 */
char ASA_SPIM_rec(char mode, char ASAID, char RegAdd, char Bytes, void *Data_p,
                  uint16_t WaitTick);

/**
 * @brief ASA SPI Master 旗標式接收函式。
 *
 * @ingroup asaspi_func
 * @param mode     SPI通訊模式，目前支援：0、3~10。
 * @param ASAID    ASA介面卡的ID編號。
 * @param RegAdd   遠端讀寫暫存器(Register)的位址或控制旗標(control flag)。
 * @param Mask     位元組遮罩。
 * @param Shift    接收旗標向右位移。
 * @param WaitTick 位元組間延遲時間，單位為 1us。
 * @param *Data_p  待收資料指標。
 * @return char    錯誤代碼：
 *                  - 0：成功無誤。
 *                  - 5：模式選擇錯誤。
 *                  - 其他：錯誤。
 *
 * SPI Master從Slave裝置接收資料，依照功用分為10種mode，接收方式如下：
 *  - mode 0 ：
 *    具check的SPI通訊方式，先使用ASA_SPIM_rec mode 0 讀取資料，
 *    將資料左移後，用遮罩取資料，最後放到Data_p裡面。
 *
 *  - mode 1 ：回傳5，表示模式選擇錯誤。
 *  - mode 2 ：回傳5，表示模式選擇錯誤。
 *  - mode 3 ：
 *      使用ASA_SPIM_rec mode 3 讀取資料，將資料左移後，用遮罩取資料，
 *      最後放到Data_p裡面。
 *  - mode 4 ：回傳5，表示模式選擇錯誤。
 *  - mode 5 ：
 *      使用ASA_SPIM_rec mode 5 讀取資料，將資料左移後，用遮罩取資料，
 *      最後放到Data_p裡面。
 *  - mode 6 ：
 *      使用ASA_SPIM_rec mode 6 讀取資料，將資料左移後，用遮罩取資料，
 *      最後放到Data_p裡面。
 *  - mode 7 ：
 *      使用ASA_SPIM_rec mode 7 讀取資料，將資料左移後，用遮罩取資料，
 *      最後放到Data_p裡面。
 *  - mode 8 ：回傳5，表示模式選擇錯誤。
 *  - mode 9 ：
 *      使用ASA_SPIM_rec mode 9 讀取資料，將資料左移後，用遮罩取資料，
 *      最後放到Data_p裡面。
 *  - mode 10：回傳5，表示模式選擇錯誤。
 *  - mode 100以上，為外掛式SPI Master(主)旗標式資料接收。
 */
char ASA_SPIM_frc(char mode, char ASAID, char RegAdd, char Mask, char Shift,
                  char *Data_p, uint16_t WaitTick);

/**
 * @brief SPI Master(主)旗標式資料傳輸。
 *
 * @ingroup asaspi_func
 * @param mode     SPI通訊模式，目前支援：0、3~10。
 * @param ASAID    ASA介面卡的ID編號。
 * @param RegAdd   控制旗標(control flag)或遠端讀寫暫存器的ID.
 * @param Mask     位元組遮罩。
 * @param Shift    發送資料向左位移。
 * @param WaitTick 位元組間延遲時間，單位為 1us。
 * @param *Data_p  待送資料指標。
 * @return char    錯誤代碼。
 *                  - 0：成功無誤。
 *                  - 5：模式選擇錯誤。
 *                  - 其他：錯誤。
 *
 * SPI Master從Slave裝置接收資料，依照功用分為10種mode，接收方式如下：
 *  - mode 0 ：
 *      具check的SPI通訊方式，先使用ASA_SPIM_rec mode 0讀取資料，
 *      將資料左移後，用遮罩取資料，最後使用ASA_SPIM_trm mode 0傳輸資料。
 *  - mode 1 ：回傳5，表示模式選擇錯誤。
 *  - mode 2 ：回傳5，表示模式選擇錯誤。
 *  - mode 3 ：
 *      使用ASA_SPIM_rec mode 3讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用ASA_SPIM_trm mode 3傳輸資料。
 *  - mode 4 ：回傳5，表示模式選擇錯誤。
 *  - mode 5 ：
 *      使用ASA_SPIM_rec mode 5讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用ASA_SPIM_trm mode 5傳輸資料。
 *  - mode 6 ：
 *      使用ASA_SPIM_rec mode 6讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用ASA_SPIM_trm mode 6傳輸資料。
 *  - mode 7 ：
 *      使用ASA_SPIM_rec mode 7讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用ASA_SPIM_trm mode 7傳輸資料。
 *  - mode 8 ：回傳5，表示模式選擇錯誤。
 *  - mode 9 ：
 *      使用ASA_SPIM_rec mode 9讀取資料，將資料左移後，用遮罩取資料，
 *      最後使用ASA_SPIM_trm mode 9傳輸資料。
 *  - mode 10：回傳5，表示模式選擇錯誤。
 *  - mode 100以上，為外掛式SPI Master(主)旗標式資料傳輸。
 */
char ASA_SPIM_ftm(char mode, char ASAID, char RRegAdd, char WRegAdd, char Mask, char Shift,
                  char *Data_p, uint16_t WaitTick);

/**
 * @brief SPI Master(主)向記憶體(flash)傳輸資料。
 *
 * @ingroup asaspi_func
 * @param mode      SPI通訊模式。
 * @param ASAID     ASA介面卡的ID編號。
 * @param RegAdd    命令或指令.
 * @param AddBytes  記憶體位置的資料筆數。
 * @param MemAdd_p  指標指向記憶體位置。
 * @param DataBytes 資料筆數。
 * @param *Data_p   指標指向資料。
 * @return char     錯誤代碼：
 *                   - 0：成功無誤。
 *                   - 5：模式選擇錯誤。
 *
 * SPI Master(主)向記憶體(flash)傳輸資料，傳輸模式如下：
 *  - mode 5 ：
 *      SPI通訊的第一筆為[RegAdd]，之後由低到高傳送 AddBytes 筆
 *      MemAdd_p (記憶體位置)資料，最後由低到高傳送DataBytes筆Data_p的資料。
 * 
 *  - mode 6 ：
 *      SPI通訊的第一筆為[RegAdd]，之後由高到低傳送 AddBytes 筆
 *      MemAdd_p (記憶體位置)資料，最後由高到低傳送DataBytes筆Data_p的資料。
 * 
 *  - mode x ：回傳5，表示模式選擇錯誤。
 */
char SPIM_Mem_trm(char mode, char ASAID, char RegAdd, char AddBytes,
                  void *MemAdd_p, char DataBytes, void *Data_p);

/**
 * @brief SPI Master(主)向記憶體(flash)接收資料。
 *
 * @ingroup asaspi_func
 * @param mode      SPI通訊模式。
 * @param ASAID     ASA介面卡的ID編號。
 * @param RegAdd    命令或指令.
 * @param AddBytes  記憶體位置的資料筆數。
 * @param MemAdd_p  指標指向記憶體位置。
 * @param DataBytes 資料筆數。
 * @param *Data_p   指標指向資料。
 * @return char     錯誤代碼：
 *                   - 0：成功無誤。
 *                   - 5：模式選擇錯誤。
 *
 * SPI Master(主)向記憶體(flash)傳輸資料，傳輸模式如下：
 *  - mode 5 ：
 *      SPI通訊的第一筆為[RegAdd]，之後由低到高傳送 AddBytes 筆
 *      MemAdd_p (記憶體位置)資料，最後由低到高接收DataBytes筆資料至Data_p。
 * 
 *  - mode 6 ：
 *      SPI通訊的第一筆為[RegAdd]，之後由高到低傳送 AddBytes 筆
 *      MemAdd_p (記憶體位置)資料，最後由高到低接收DataBytes筆資料至Data_p。
 * 
 *  - mode x ：回傳5，表示模式選擇錯誤。
 */
char SPIM_Mem_rec(char mode, char ASAID, char RegAdd, char AddBytes,
                  void *MemAdd_p, char DataBytes, void *Data_p);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 0。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 0傳輸給Slave裝置，具檢
 * 查機制，當Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會
 * 由Master發送命令[R/W + RegAdd]，決定是要Read或Write Slave端的哪個暫
 * 存器，之後根據Master端傳送的資料，接收或傳送給Master。
 */
void ASA_SPIS0_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 1。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 1傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[3bits 控制旗標 + RegAdd]，之後對Slave端註冊的RegAdd寫入資
 * 料。
 */
void ASA_SPIS1_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 2。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 2傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[RegAdd + 3bits 控制旗標]，之後對Slave端註冊的RegAdd寫入資
 * 料。
 */
void ASA_SPIS2_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 3。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，使用者須先設定是要Read還是Write，之後
 * 使用Mode 3傳輸或接收Slave裝置，當Master cs(chip select)拉低觸發中斷
 * ，會執行此函式，之後由低到高傳輸或接收Slave資料。
 */
void ASA_SPIS3_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 4。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，使用者須先設定是要Read還是Write，之後
 * 使用Mode 4傳輸或接收Slave裝置，當Master cs(chip select)拉低觸發中斷
 * ，會執行此函式，之後由高到低傳輸或接收Slave資料。
 */
void ASA_SPIS4_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 7。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 7傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[R/W + RegAdd]，之後對Slave端註冊的RegAdd由低到高寫入或讀取
 * 資料。
 */
void ASA_SPIS7_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 8。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 7傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[R/W + RegAdd]，之後對Slave端註冊的RegAdd由高到低寫入或讀取
 * 資料。
 */
void ASA_SPIS8_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 9。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 7傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[RegAdd + R/W]，之後對Slave端註冊的RegAdd由低到高寫入或讀取
 * 資料。
 */
void ASA_SPIS9_step(void);

/**
 * @brief SPI Slave(僕)中斷狀態機處理函式，支援SPI Master(主) mode 10。
 *
 * @ingroup asaspi_func
 *
 * SPI Slave(僕)狀態機處理的函式，Master使用Mode 7傳輸給Slave裝置，當
 * Master cs(chip select)拉低觸發中斷，會執行此函式，第一筆會由Master
 * 發送命令[RegAdd + R/W]，之後對Slave端註冊的RegAdd由高到低寫入或讀取
 * 資料。
 */
void ASA_SPIS10_step(void);
/*-- asaspi section end ------------------------------------------------------*/

/*-- time section start ------------------------------------------------------*/
#define MAX_TIMEOUT_ISR 20

typedef void (*ISRFunc)(void);
typedef struct {
    volatile uint16_t
        time_limit;  // 時間限制        當計數器上數到該數值時觸發逾時中斷
    volatile uint16_t counter;  // 計數器          計數器固定頻率上數
    volatile uint8_t* p_postSlot;  // POST欄住址 一指標指向POST欄住址，為POST &
                                   // SLOT機制的子元件
    volatile uint8_t enable;  // 禁致能控制      決定本逾時ISR是否致能
    ISRFunc p_ISRFunc;        // 逾時中斷函式    逾時中斷函式指標
} TimeoutISR_t;

typedef struct {
    uint8_t total;  // 紀錄已有多少逾時中斷已註冊
    volatile TimeoutISR_t
        timeoutISR_inst[MAX_TIMEOUT_ISR];  // 紀錄所有已註冊的逾時中斷資料結構
} TimerCntStrType;
volatile TimerCntStrType TimerCntStr_inst;
/*-- time section end --------------------------------------------------------*/

/*-- asakb00 section start ---------------------------------------------------*/
/**
 * @brief ASA KB00 參數結構。
 * @ingroup asakb00_struct
 */
typedef struct {
    uint8_t mode;        ///< 控制鍵號模式、ASCII模式
    uint8_t keymap[16];  ///< 控制ASCII模式下每個按鍵的回傳值
} AsaKb00Para_t;

/**
 * @brief 呼叫本函式可以執行 ASA_KB00 介面卡工作模式的設定。
 *
 * @ingroup asakb00_func
 * @param ASA_ID ASA介面卡的ID編號。
 * @param LSByte 欲操作暫存器編號。
 * @param Mask  遮罩。
 * @param Shift 向左位移。
 * @param Data  寫入資料。
 * @param Str_p ASA KB00 參數結構。
 * @return char 錯誤代碼：
 *   - 0 : 成功無誤。
 *   - 4 : ASA_ID 錯誤，應如旋鈕所指向編號，小於7。
 *   - 7 : LSByte 錯誤，應介於 200 ~ 216。
 *   - 9 : Shift 錯誤，應小於 7。
 *
 * 呼叫本函式可以執行 ASA_KB00 介面卡工作模式的設定，主要有以下兩個功能：
 * 1. 選擇盤輸出鍵號或者對應的 ASCII 碼。
 *    - LSByte 應為 200。
 *    - Mask 應為 0x01。
 *    - Shift 應為 0。
 *    - Data 應為 0、1 ，代表鍵號模式、ASCII模式。
 * 2. 更新盤各按鍵對應的 ASCII 碼。
 *    - LSByte 應為 201 ~ 216，分別代表鍵號1~16的 ASCII 碼。
 *    - Mask 應為 0xff。
 *    - Shift 應為 0。
 *    - Data 要更新的 ASCII 碼。
 */
char ASA_KB00_set(char ASA_ID, char LSByte, char Mask, char Shift, char Data,
                  AsaKb00Para_t *Str_p);

/**
 * @brief 呼叫本函式可以 讀取盤輸入 或 查詢盤 ASCII 碼。
 *
 * @ingroup asakb00_func
 * @param ASA_ID  ASA介面卡的ID編號。
 * @param LSByte 欲操作暫存器編號。
 * @param Bytes 資料大小，即為*Data_p的大小。
 * @param *Data_p  指標指向資料。
 * @param Str_p ASA KB00 參數結構。
 * @return char
 *   - 0 : 成功無誤。
 *   - 4 : ASA_ID 錯誤，應如旋鈕所指向編號，小於7。
 *   - 7 : LSByte 錯誤，應介於 200 ~ 216。
 *   - 8 : Bytes 錯誤。
 *
 * 此函式為c4mlib函式庫內部使用，不開放給使用者使用。
 *
 * 呼叫本函式可以主要有以下兩個功能：
 * 1. 讀取盤輸入：由 ASA_KB00 介面卡讀取鍵入的數字，若無輸入，則會回傳 NULL 的
 * ASCII 碼(0)。
 *    - LSByte 應為 100。
 *    - Bytes 應為 1。
 *    - Data_p 應為 1 byte 大小記憶體位置(char、uint8_t)。
 * 2. 查詢盤 ASCII 碼：讀取目前設定的盤對應之 ASCII 碼。
 *    - LSByte 應為 201 ~ 216，分別代表鍵號1~16。
 *    - Bytes 應為 1。
 *    - Data_p 應為 1 byte 大小記憶體位置(char、uint8_t)。
 */
char ASA_KB00_get(char ASA_ID, char LSByte, char Bytes, void *Data_p,
                  AsaKb00Para_t *Str_p);
/*-- asakb00 section end -----------------------------------------------------*/

/*-- asa7s00 section start ---------------------------------------------------*/
/**
 * @brief ASA 7S00 參數結構。
 *
 * @ingroup asa7s00_struct
 *
 * ASA 7S00 有四個七節管，由左到右分別為編號0、1、2、3七節管。
 *
 * 其中 FFFlags 每個bit分別代表以下意思，其中閃爍旗標0、1代表不閃爍、閃爍
 * 浮點數旗標0、1代表該七節管小點不顯示、顯示：
 *  - bit 0 : 編號 3 七節管的閃爍旗標
 *  - bit 1 : 編號 2 七節管的閃爍旗標
 *  - bit 2 : 編號 1 七節管的閃爍旗標
 *  - bit 3 : 編號 0 七節管的閃爍旗標
 *  - bit 4 : 編號 3 七節管的浮點數旗標
 *  - bit 5 : 編號 2 七節管的浮點數旗標
 *  - bit 6 : 編號 1 七節管的浮點數旗標
 *  - bit 7 : 編號 0 七節管的浮點數旗標
 *
 * _7S00_put_reg 則代表編號0、1、2、3七節管要顯示的ascii。
 * 支援的ascii有數字0~9、英文A~F。
 */
typedef struct {
    uint8_t FFFlags;           ///< 浮點數旗標，閃爍旗標 (float and flash flag)
    uint8_t ASCII_list[4];     ///< not used.
    uint8_t _7S00_put_reg[4];  ///< 編號0、1、2、3七節管的ASCII碼。
} Asa7s00Para_t;

/**
 * @brief 經由ASA Bus設定7s00的小數點及位元的顯示。
 *
 * @ingroup asa7s00_func
 * @param ASA_ID ASA介面卡的ID編號。
 * @param LSByte 控制旗標(control flag)或遠端讀寫暫存器的ID。
 * @param Mask  遮罩。
 * @param Shift 向左位移。
 * @param Data  寫入資料。
 * @param Str_p ASA 7S00 參數結構。
 * @return char 錯誤代碼：
 *   - 0：成功無誤。
 *   - 4：ID錯誤，無法找到該裝置。
 *   - 5：欲顯示之數值無法表示。
 *   - 7：LSByte錯誤。
 *   - 9：Shift錯誤。
 *
 * 透過ASA Bus的傳輸，將7s00的ASA ID、LSByte=200、Mask、Shift、Data以及7s00的參
 * 數結構輸進此函數中，即可完成7s00四個位數七節管的小數點和閃爍位元的顯示設定。
 * 7s00的LSByte為固定的200，若輸入非200的值則會回傳7的error code。
 */
char ASA_7S00_set(char ASA_ID, char LSByte, char Mask, char Shift, char Data,
                  Asa7s00Para_t* Str_p);

/**
 * @brief 經由ASA Bus輸出要顯示在7s00的數值。
 *
 * @ingroup asa7s00_func
 * @param ASA_ID ASA介面卡的ID編號。
 * @param LSByte 控制旗標(control flag)或遠端讀寫暫存器的ID。
 * @param Bytes 資料大小，即為*Data_p的大小。
 * @param Data_p  指標指向資料。
 * @param Str_p ASA 7S00 參數結構。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 4：ID錯誤，無法找到該裝置。
 *   - 5：欲顯示之數值無法表示。
 *   - 7：LSByte錯誤。
 *   - 8：輸入Byte錯誤。
 * 
 * 透過ASA Bus的傳輸，將7s00的ASA ID、LSByte=200、Byte、*Data_p以及7s00的參
 * 數結構輸進此函數中，7s00及會按照使用者的輸入*Data_p和LSByte、Byte決定輸出的形式。
 * 假如Byte為1且LSByte為0~3，此函式會指定7s00某一位元輸出*Data_p之ASCII碼。
 * 假如Byte為4且LSByte為0，此函式會使7s00輸出至四位元的七節管，且為*Data_p之ASCII碼。
 */                 
char ASA_7S00_put(char ASA_ID, char LSByte, char Bytes, void* Data_p,
                  Asa7s00Para_t* Str_p);

/**
 * @brief 經由ASA Bus取得顯示在7s00的數值。
 *
 * @ingroup asa7s00_func
 * @param ASA_ID ASA介面卡的ID編號。
 * @param LSByte 控制旗標(control flag)或遠端讀寫暫存器的ID。
 * @param Bytes 資料大小，即為*Data_p的大小。
 * @param Data_p  指標指向取得的資料。
 * @param Str_p ASA 7S00 參數結構。
 * @return char 錯誤代碼： 
 *   - 0：成功無誤。
 *   - 2：ID錯誤，無法找到該裝置。
 *   - 3：輸入Byte大小超過1。
 *   - 4：ID錯誤，無法找到該裝置。
 *   - 5：欲顯示之數值無法表示。
 *   - 7：LSByte錯誤。
 *   - 8：輸入Byte錯誤。
 *
 * 透過ASA Bus的傳輸，將7s00的ASA ID、LSByte=200、Byte、*Data_p以及7s00的參
 * 數結構輸進此函數中，函式就會將7s00上一時刻的輸出讀回，並存進*Data_p之中。
 */                  
char ASA_7S00_get(char ASA_ID, char LSByte, char Bytes, void* Data_p,
                  Asa7s00Para_t* Str_p);
/*-- asa7s00 section end -----------------------------------------------------*/

/*-- asastp00 section start --------------------------------------------------*/
/**
 * @brief 發送資料至stp00
 * @ingroup asastp00_func
 * @param ASA_ID ASA裝置上旋鈕數值
 * @param RegAdd 暫存器位址代號
 *   - 1: 速度設定暫存器。
 *   - 2: 步進步數暫存器。
 *   - 3: 降速除頻值暫存器。
 * @param Bytes 欲發送資料大小 (只能為2bytes)
 * @param Data_p 欲發送資料的變數位址
 * @param Str_p 配合外掛函式群傳參規格
 * @return char 錯誤代碼：
 *   - 0: 正常執行
 *   - 4: ASA_ID錯誤，確認是否小於7
 *   - 7: RegAdd錯誤，確認是否為1~3
 *   - 8: Bytes錯誤，確認是否為2
 *
 * 此函式為c4mlib函式庫內部使用，不開放給使用者使用。
 *
 * 透過ASA_ID選擇ASABUS上指定的裝置。
 * 將Data_p的內容透過spi通訊傳至stp00上指定的暫存器中。
 */
char ASA_STP00_trm(char ASA_ID, char RegAdd, char Bytes, void* Data_p,
                   void* Str_p);

/**
 * @brief 配合外掛函式群規格
 *
 * @ingroup asastp00_func
 * @param ASA_ID 配合外掛函式群傳參規格
 * @param RegAdd 配合外掛函式群傳參規格
 * @param Bytes 配合外掛函式群傳參規格
 * @param Data_p 配合外掛函式群傳參規格
 * @param Str_p 配合外掛函式群傳參規格
 * @return char 錯誤代碼：
 *   - 0: 正常執行
 *
 * 此函式為c4mlib函式庫內部使用，不開放給使用者使用。
 *
 * 內容未實作
 */
char ASA_STP00_rec(char ASA_ID, char RegAdd, char Bytes, void* Data_p,
                   void* Str_p);

/**
 * @brief 配合外掛函式群規格
 *
 * @ingroup asastp00_func
 * @param ASA_ID 配合外掛函式群傳參規格
 * @param RegAdd 配合外掛函式群傳參規格
 * @param Mask 配合外掛函式群傳參規格
 * @param Shift 配合外掛函式群傳參規格
 * @param Data_p 配合外掛函式群傳參規格
 * @param Str_p 配合外掛函式群傳參規格
 * @return char char 錯誤代碼：
 *   - 0: 正常執行
 *
 * 此函式為c4mlib函式庫內部使用，不開放給使用者使用。
 *
 * 內容未實作
 */
char ASA_STP00_frc(char ASA_ID, char RegAdd, char Mask, char Shift,
                   void* Data_p, void* Str_p);

/**
 * @brief 配合外掛函式群規格
 *
 * @ingroup asastp00_func
 * @param ASA_ID 配合外掛函式群傳參規格
 * @param RegAdd 配合外掛函式群傳參規格
 * @param Mask 配合外掛函式群傳參規格
 * @param Shift 配合外掛函式群傳參規格
 * @param Data_p 配合外掛函式群傳參規格
 * @param Str_p 配合外掛函式群傳參規格
 * @return char char 錯誤代碼：
 *   - 0: 正常執行
 *
 * 此函式為c4mlib函式庫內部使用，不開放給使用者使用。
 *
 * 內容未實作
 */
char ASA_STP00_ftm(char ASA_ID, char RegAdd, char Mask, char Shift,
                   void* Data_p, void* Str_p);
/*-- asastp00 section end ----------------------------------------------------*/

/*-- rtpio section start -----------------------------------------------------*/
/**
 * @brief RealTimeUpCount結構原型
 * 
 * @ingroup rtupcount_struct
 * @param Fb_Id 中斷中功能方塊名單編號。
 * @param TrigCount 觸發次數計數值。
 * 
 * 計數值紀錄執行step函式次數，用以追蹤次數。
 */
typedef struct {
    uint8_t Fb_Id;              ///<中斷中功能方塊名單編號。
    volatile uint8_t TrigCount; ///<觸發次數計數值。
} RealTimeUpCountStr_t;

/**
 * @brief 初始化結構鏈結
 * 
 * @ingroup rtupcount_func
 * @param RealTimeUpCountStr_p  RealTimeUpCountStr結構指標
 * @return uint8_t 錯誤代碼<br>
 *     0: 正常執行
 */
uint8_t RealTimeUpCount_net(RealTimeUpCountStr_t* RealTimeUpCountStr_p);

/**
 * @brief 將觸發計數+1。
 *
 * @ingroup rtupcount_func
 * @param RealTimeUpCountStr_p 要執行的結構指標。
 *
 * 執行觸發次數值+1，可登錄在中斷服務常式中執行。
 */
void RealTimeUpCount_step(RealTimeUpCountStr_t* RealTimeUpCountStr_p);


/**
 * @defgroup rtpio_macro
 * @defgroup rtpio_func
 * @defgroup rtpio_struct
 */

/* Public Section Start */
#define RTPIO_MAX_DATA_LENGTH \
    2  ///<硬體暫存器位元最大大小  @ingroup rtpio_macro

/**
 * @brief RealTimePort結構原型
 *
 * @ingroup rtpio_struct
 *
 * 存取硬體暫存器位址和其大小後續讀寫用。
 * 資料緩衝暫存區則供資料存放，當執行step函式時將資料讀寫暫存區。
 * 計數值則紀錄執行step函式次數，用以追蹤次數。
 */
typedef struct {
    volatile uint8_t* Reg_p;  ///< 存放硬體暫存器指標。
    uint8_t Bytes;            ///< 硬體暫存器大小 (最多2 Bytes) 。
    uint8_t Fb_Id;            ///< 中斷中功能方塊名單編號。
    volatile uint8_t Buff[RTPIO_MAX_DATA_LENGTH];  ///< 資料暫存緩衝區
    volatile uint8_t TrigCount;                    ///< 觸發次數計數值
} RealTimePortStr_t;

/**
 * @brief RealTimeFlag結構原型
 *
 * @ingroup rtpio_struct
 *
 * 存取硬體暫存器位址和其大小後續讀寫用。
 * 資料緩衝暫存區則供資料存放，當執行step函式時將旗標資料讀寫暫存區。
 * 計數值則紀錄執行step函式次數，用以追蹤次數。
 */
typedef struct {
    volatile uint8_t* Reg_p;      ///< 存放硬體暫存器指標。
    uint8_t Fb_Id;                ///< 中斷中功能方塊名單編號。
    uint8_t Mask;                 ///< 旗標讀寫遮罩。
    uint8_t Shift;                ///< 旗標讀寫平移位元。
    volatile uint8_t FlagsValue;  ///< 資料暫存緩衝區。
    volatile uint8_t TrigCount;   ///< 觸發次數計數值。
} RealTimeFlagStr_t;

/**
 * @brief 鏈結結構實體與硬體暫存器
 *
 * @ingroup rtpio_func
 * @param Str_p RealTimePort結構指標。
 * @param Reg_p 硬體暫存器指標。
 * @param Bytes 暫存器位元大小(最多2bytes)。
 * @return uint8_t 錯誤代碼<br>
 *          0: 正常執行。<br>
 *          1: Byte錯誤(不是1~2)。
 *
 * 輸出入埠暫存器鏈結驅動函式會執行實體輸出入埠暫存指標鏈結到資料結構的工作，以備中斷時讀寫。
 */
uint8_t RealTimePort_net(RealTimePortStr_t* Str_p, uint8_t* Reg_p,
                         uint8_t Bytes);

/**
 * @brief 執行一次暫存器讀取，並將觸發計數+1。
 *
 * @ingroup rtpio_func
 * @param RealTimePortStr_p 要執行的結構指標。
 *
 * 執行硬體暫存器輸入埠之讀取並轉存至資料結構內的資料暫存區，並將觸發次數值加1。配合其資料結構實體，可登錄在中斷服務常式中執行。
 */
void RealTimePortIn_step(RealTimePortStr_t* RealTimePortStr_p);

/**
 * @brief 執行一次暫存器寫入，並將觸發計數+1。
 *
 * @ingroup rtpio_func
 * @param RealTimePortStr_p 要執行的結構指標。
 *
 * 執行資料結構內的資料暫存區之讀取並轉存至硬體暫存器輸出埠，並將觸發次數值加1。配合其資料結構實體，可登錄在中斷服務常式中執行。
 */
void RealTimePortOut_step(RealTimePortStr_t* RealTimePortStr_p);

/**
 * @brief 鏈結結構實體與硬體暫存器
 *
 * @ingroup rtpio_func
 * @param Str_p RealTimePort結構指標。
 * @param Reg_p 硬體暫存器指標。
 * @param Mask  旗標資料讀寫遮罩(0x00~0xFF)。
 * @param Shift 旗標資料讀寫最低位元(0x00~0x08)。
 * @return uint8_t 錯誤代碼<br>
 *          0: 正常執行。<br>
 *
 * 旗標輸出入埠暫存器鏈結驅動函式會執行硬體輸出入埠暫存指標鏈結到資料結構的工作，以備中斷時讀寫。
 */
uint8_t RealTimeFlag_net(RealTimeFlagStr_t* Str_p, uint8_t* Reg_p, uint8_t Mask,
                         uint8_t Shift);

/**
 * @brief 執行一次旗標讀取，並將觸發計數+1。
 *
 * @ingroup rtpio_func
 * @param RealTimeFlagStr_p 要執行的結構指標。
 *
 * 執行硬體暫存器輸入埠之讀取並轉存至資料結構內的資料暫存區，並將觸發次數值加1。配合其資料結構實體，可登錄在中斷服務常式中執行。
 */
void RealTimeFlagIn_step(RealTimeFlagStr_t* RealTimeFlagStr_p);

/**
 * @brief 執行一次旗標寫入，並將觸發計數+1。
 *
 * @ingroup rtpio_func
 * @param RealTimeFlagStr_p 要執行的結構指標。
 *
 * 執行資料結構內的資料暫存區之讀取並轉存至硬體暫存器輸出埠，
 * 並將觸發次數值加1。配合其資料結構實體，可登錄在中斷服務常式中執行。
 */
void RealTimeFlagOut_step(RealTimeFlagStr_t* RealTimeFlagStr_p);
/*-- rtpio section end -------------------------------------------------------*/

/*-- databuffer section start ------------------------------------------------*/
#define NETRBF(task, num, buffer_p)                                            \
    task.RBF[num] = buffer_p  ///< 供管理結構體鏈結到串列通訊工作的結構體

/**
 * @brief 供使用者創建並連結實體空間至緩衝器結構體中
 * @ingroup rtpio_macro
 */
#define REMOBUFF_LAY(RBSTR, DDEPTH, VDEPTH)                                    \
    RemoBFStr_t RBSTR = {0};                                                   \
    {                                                                          \
        static uint8_t RBSTR_DLIST[DDEPTH]    = {0};                           \
        static RemoVari_t RBSTR_VLIST[VDEPTH] = {0};                           \
        RBSTR.DList_p                         = RBSTR_DLIST;                   \
        RBSTR.VList_p                         = RBSTR_VLIST;                   \
    }                                                                          \
    RBSTR.DataDepth = DDEPTH;                                                  \
    RBSTR.VariDepth = VDEPTH;

/**
 * @brief 提供remo變數位址連結及資訊儲存
 * @ingroup remobf_struct
 */
typedef struct {
    uint8_t Bytes;  ///< 變數位元數
    void* Vari_p;   ///< 變數位址
} RemoVari_t;

/**
 * @brief 提供remobf結構原形
 * @ingroup remobf_struct
 */
typedef struct {
    uint8_t State;        ///< Buffer狀態
    uint8_t DataIndex;    ///< 資料讀取索引
    uint8_t VariIndex;    ///< 變數讀取索引
    uint8_t ByteIndex;    ///< 位元讀取索引
    uint8_t DataTotal;    ///< 資料總位元數
    uint8_t VariTotal;    ///< 變數總數
    uint8_t DataDepth;    ///< Buffer最大總位元數,
    uint8_t VariDepth;    ///< Remo變數最大總數量,
    RemoVari_t* VList_p;  ///< 變數儲存空間指標
    uint8_t* DList_p;     ///< Buffer空間指標
} RemoBFStr_t;

/**
 * @brief 將要提供遠端讀寫的變數大小及位址寫入結構體中並回傳遠端讀寫編號
 *
 * @ingroup remobf_func
 * @param Str_p remobf結構指標
 * @param Data_p 變數位址
 * @param Bytes 變數位元數
 * @return uint8_t 遠端讀寫編號(0~254)
 */
uint8_t RemoBF_reg(RemoBFStr_t* Str_p, void* Data_p, uint8_t Bytes);

/**
 * @brief 將buffer內的資料寫入至對應的讀寫變數位址中
 *
 * @ingroup remobf_func
 * @param Str_p remobf結構指標
 * @param RegId 對應變數的編號 0~254:對應編號 255:所有已註冊變數
 * @return uint8_t 錯誤代碼
 * - 0: 正常
 * - 1: State錯誤Buffer尚未填滿
 */
uint8_t RemoBF_put(RemoBFStr_t* Str_p, uint8_t RegId);

/**
 * @brief 將1byte的資料填入至buffer中
 *
 * @ingroup remobf_func
 * @param Str_p remobf結構指標
 * @param RegId 對應變數的編號 0~254:對應編號 255:所有已註冊變數
 * @param Data 欲寫入之資料值
 * @return int8_t 回傳剩餘Bytes數
 */
int8_t RemoBF_temp(RemoBFStr_t* Str_p, uint8_t RegId, uint8_t Data);

/**
 * @brief 取得1byte對應編碼變數中的值
 *
 * @ingroup remobf_func
 * @param Str_p remobf結構指標
 * @param RegId 對應變數的編號 0~254:對應編號 255:所有已註冊變數
 * @param Data_p 放入取得值的位址
 * @return int8_t 回傳剩餘Bytes數
 */
int8_t RemoBF_get(RemoBFStr_t* Str_p, uint8_t RegId, void* Data_p);

/**
 * @brief 歸零remobf結構體中的資料索引
 *
 * @ingroup remobf_func
 * @param Str_p Str_p remobf結構指標
 * @return uint8_t 錯誤代碼 0: 正常
 */
uint8_t RemoBF_clr(RemoBFStr_t* Str_p);

/**
 * @brief   供管理結構體鏈結到前後級工作之緩衝區欄
 * @ingroup databuffer_macro
 */
#define NETFBF(task_p, num, buffer_p) task_p->FBF[num] = buffer_p

/**
 * @brief 供暫存資料之矩陣，鍊結至管理先進先出之結構體，以及初始化先進先出結構體
 * @param Str 定義批次緩衝暫存器建構
 * @param List[MAXNUM] 定義資料陣列用矩陣
 * @ingroup databuffer_macro
 */
#define FIFOBUFF_LAY(FBSTR, MAXNUM)                                            \
    FifoBFStr_t FBSTR = {0};                                                   \
    FBSTR.Depth       = MAXNUM;                                                \
    {                                                                          \
        static uint8_t List[MAXNUM] = {0};                                     \
        FBSTR.List_p                = List;                                    \
    }

/**
 * @brief   定義先進先出結構體以管理先進先出緩衝區狀態之結構
 * @ingroup databuffer_struct
 */
typedef struct {
    uint8_t PIndex;  ///< FIFO Buffer 的寫入索引
    uint8_t GIndex;  ///< FIFO Buffer 的讀取索引
    uint8_t Total;   ///< FIFO Buffer 中目前已存入的資料數
    uint8_t Depth;   ///< FIFO Buffer 中可存入的資料入，即為大小
    void* List_p;    ///< FIFO Buffer 中的資料起始記憶體位置
} FifoBFStr_t;

/**
 * @ingroup databuffer_func
 *
 * @brief 資料寫入函式，供前級工作呼叫以存入新資料。
 *
 * @param *FBF_p     欲存入資料的批次緩衝區指標。
 * @param *Data_p    提供存入資料的變數或矩陣指標。
 * @param Bytes     資料位元組數。
 * @return
 * 尚餘緩衝空間容量位元組數n，若n大於等於0代表剩餘n位元組可存入，否則為不足n位元組。
 */
int8_t FifoBF_put(FifoBFStr_t* FBF_p, void* Data_p, uint8_t Bytes);

/**
 * @ingroup databuffer_func
 *
 * @brief 資料讀出函式，供前級工作呼叫以讀出新資料。
 *
 * @param *FBF_p     欲讀出資料的批次緩衝區指標。
 * @param *Data_p    提供讀出資料的變數或矩陣指標。
 * @param Bytes     資料位元組數
 * @return
 * 尚餘緩衝空間容量位元組數n，若n大於等於0代表剩餘n位元組可讀出，否則為不足n位元組。
 */
int8_t FifoBF_get(FifoBFStr_t* FBF_p, void* Data_p, uint8_t Bytes);

/**
 * @ingroup databuffer_func
 *
 * @brief 供前級工作呼叫以清空緩衝區。
 *
 * @param *FBF_p     欲清空批次緩衝區指標。
 * @return 緩衝區可存入之資料位元數
 */
uint8_t FifoBF_clr(FifoBFStr_t* FBF_p);
/*-- databuffer section end --------------------------------------------------*/


#endif // C4MLIB_H
