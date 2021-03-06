/**************************************************************************//**
 * @file
 * @brief USB/USART0 bootloader.
 * @author Energy Micro AS
 * @version 1.02
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2011 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#include "em_device.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_usb.h"
#include "em_gpio.h"
#include "cdc.h"
#include "crc.h"
#include "flash.h"
#include "boot.h"
#include "autobaud.h"
#include "xmodem.h"
#include "bootldio.h"
#include "retargetdebug.h"
#include "ramfuncs.h"
#include "platform.h"

/*** Typedef's and defines. ***/

#define BULK_EP_SIZE    64      /* This is the max. ep size.              */

/** Version string, used when the user connects */
#define BOOTLOADER_VERSION_STRING "BOOTLOADER version 1.01, Chip ID "

#define BUTTON0_PIN      8
#define BUTTON0_PORT    gpioPortE

#define USER_PAGE_START 0x0FE00000
#define USER_PAGE_END   0x0FE00200
#define LOCK_PAGE_START 0x0FE04000
#define LOCK_PAGE_END   0x0FE04200

#define DEBUG_LOCK_WORD (0x0FE04000 + (127 * 4))

/*** Function prototypes. ***/

static void commandlineLoop(  void );
static void verify(           uint32_t start, uint32_t end );
static void Disconnect(       int predelay, int postdelay );
static void StartRTC( void );

/*** The descriptors for a USB CDC device. ***/
#include "descriptors.h"

/*** Variables ***/

/*** Strings ***/
static const uint8_t crcString[]     = "\r\nCRC: ";
static const uint8_t newLineString[] = "\r\n";
static const uint8_t readyString[]   = "\r\nReady\r\n";
static const uint8_t okString[]      = "\r\nOK\r\n";
static const uint8_t failString[]    = "\r\nFail\r\n";
static const uint8_t unknownString[] = "\r\n?\r\n";

/**************************************************************************//**
 * The main entry point.
 *****************************************************************************/
int main(void)
{
    int msElapsed, i;

    /* Enable peripheral clocks. */
    CMU->HFPERCLKDIV = CMU_HFPERCLKDIV_HFPERCLKEN;
    CMU->HFPERCLKEN0 = CMU_HFPERCLKEN0_GPIO | BOOTLOADER_USART_CLOCK |
                       AUTOBAUD_TIMER_CLOCK ;

    /* Enable DMA interface */
    CMU->HFCORECLKEN0 = CMU_HFCORECLKEN0_DMA;

    StartRTC();

    GPIO_PinModeSet(BUTTON0_PORT, BUTTON0_PIN, gpioModeInputPull, 1);

    GPIO_PinModeSet(LED0_PORT, LED0_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(LED1_PORT, LED1_PIN, gpioModePushPull, 0);

    int pin = GPIO_PinInGet(BUTTON0_PORT, BUTTON0_PIN);

    //Skip bootloader unless button is pressed (low)
    if (pin != 0) {

        GPIO_PinOutSet(LED0_PORT, LED0_PIN);

        if ( BOOT_checkFirmwareIsValid() ) {
            BOOT_boot();
        } else {
            GPIO_PinOutSet(LED1_PORT, LED1_PIN);

            while(1);
        }

    }

    GPIO_PinOutSet(LED1_PORT, LED1_PIN);

    //Bootloader logic follows
    NVIC_DisableIRQ(RTC_IRQn);
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
    USBTIMER_Init();

    SystemHFXOClockSet(48000000);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
    USBD_Init(&initstruct);

    AUTOBAUD_start();

    /* Wait 30 seconds for USART or USB connection */
    msElapsed = 0;
    while (msElapsed < 30000) {
        if (AUTOBAUD_completed())
            break;

        if (CDC_Configured) {
            break;
        }

        USBTIMER_DelayMs(100);
        msElapsed += 100;
    }
    AUTOBAUD_stop();

    if (msElapsed >= 30000) {
        Disconnect(0, 2000);
        SCB->AIRCR = 0x05FA0004;        /* Reset EFM32. */
    }

    /* Print a message to show that we are in bootloader mode */
    BOOTLDIO_printString("\r\n\r\n" BOOTLOADER_VERSION_STRING );

    /* Print the chip ID. This is useful for production tracking */
    BOOTLDIO_printHex(DEVINFO->UNIQUEH);
    BOOTLDIO_printHex(DEVINFO->UNIQUEL);
    BOOTLDIO_printString("\r\n");

    /* Figure out correct flash geometry. */
    FLASH_CalcPageSize();
    /* Initialize flash for writing */
    FLASH_init();

    /* Start executing command line */
    commandlineLoop();
}


/**************************************************************************//**
 * @brief
 *   The main command line loop. Placed in Ram so that it can still run after
 *   a destructive write operation.
 *   NOTE: __ramfunc is a IAR specific instruction to put code into RAM.
 *   This allows the bootloader to survive a destructive upload.
 *****************************************************************************/
static void commandlineLoop( void )
{
    uint8_t  c;

    /* The main command loop */
    while (1) {
        /* Retrieve new character */
        c = BOOTLDIO_rxByte();
        /* Echo */
        if (c != 0) {
            BOOTLDIO_txByte( c );
        }

        switch (c) {
        /* Bootloader version command */
        case 'i':
            /* Print version */
            BOOTLDIO_printString("\r\n\r\n" BOOTLOADER_VERSION_STRING );
            /* Print the chip ID */
            BOOTLDIO_printHex(DEVINFO->UNIQUEH);
            BOOTLDIO_printHex(DEVINFO->UNIQUEL);
            BOOTLDIO_printString("\r\n");
            break;

        /* Upload command */
        case 'u':
            BOOTLDIO_printString( readyString );
            XMODEM_download( BOOTLOADER_SIZE, flashSize );
            BOOTLDIO_printString( "Upload complete\r\n" );
            break;

        /* Destructive upload command */
        case 'd':
            BOOTLDIO_printString( readyString );
            XMODEM_download( 0, flashSize );
            BOOTLDIO_printString( "Upload complete\r\n" );
            break;

        /* Write to user page */
        case 't':
            BOOTLDIO_printString( readyString );
            XMODEM_download( USER_PAGE_START, USER_PAGE_END );
            BOOTLDIO_printString( "User page written\r\n" );
            break;

        /* Write to lock bits */
        case 'p':
            BOOTLDIO_printString( readyString );
            XMODEM_download( LOCK_PAGE_START, LOCK_PAGE_END );
            BOOTLDIO_printString( "Lock bits set\r\n" );
            break;

        /* Boot into new program */
        case 'b':
          BOOTLDIO_printString( "Booting application\r\n" );
            Disconnect( 5000, 2000 );
            BOOT_boot();
            break;

        /* Debug lock */
        case 'l':
#if defined( BL_DEBUG )
            /* We check if there is a debug session active in DHCSR. If there is we
             * abort the locking. This is because we wish to make sure that the debug
             * lock functionality works without a debugger attatched. */
            if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0x0) {
                USB_PUTS( "\r\n\r\n **** WARNING: DEBUG SESSION ACTIVE. NOT LOCKING!  **** \r\n\r\n" );
                BOOTLDIO_printString( "Debug active.\r\n" );
            } else {
                USB_PUTS( "Starting debug lock sequence.\r\n" );
#endif
                FLASH_writeWord( DEBUG_LOCK_WORD, 0x0 );
                if ( *(volatile uint32_t*)DEBUG_LOCK_WORD == 0x0 ) {
                    BOOTLDIO_printString( okString );
                } else {
                    BOOTLDIO_printString( failString );
                }
#if defined( BL_DEBUG )
                USB_PRINTF( "Debug lock word: 0x%x \r\n", *((uint32_t *) DEBUG_LOCK_WORD) );
            }
#endif
            break;

        /* Verify content by calculating CRC of entire flash */
        case 'v':
            verify( 0, flashSize );
            break;

        /* Verify content by calculating CRC of application area */
        case 'c':
            verify( BOOTLOADER_SIZE, flashSize );
            break;

        /* Verify content by calculating CRC of user page.*/
        case 'n':
            verify( USER_PAGE_START, USER_PAGE_END );
            break;

        /* Verify content by calculating CRC of lock page */
        case 'm':
            verify( LOCK_PAGE_START, LOCK_PAGE_END );
            break;

        /* Reset command */
        case 'r':
            Disconnect( 5000, 2000 );

            /* Write to the Application Interrupt/Reset Command Register to reset
             * the EFM32. See section 9.3.7 in the reference manual. */
            SCB->AIRCR = 0x05FA0004;
            break;

        /* Unknown command */
        case 0:
            /* Timeout waiting for RX - avoid printing the unknown string. */
            break;

        default:
            BOOTLDIO_printString( unknownString );
        }
    }
}


/**************************************************************************//**
 * @brief
 *   Helper function to print flash write verification using CRC
 * @param start
 *   The start of the block to calculate CRC of.
 * @param end
 *   The end of the block. This byte is not included in the checksum.
 *****************************************************************************/
static void verify(uint32_t start, uint32_t end)
{
    BOOTLDIO_printString(crcString);
    BOOTLDIO_printHex(CRC_calc((void *) start, (void *) end));
    BOOTLDIO_printString(newLineString);
}


/**************************************************************************//**
 * Disconnect USB link with optional delays.
 *****************************************************************************/
static void Disconnect( int predelay, int postdelay )
{
    if ( predelay ) {
        /* Allow time to do a disconnect in a terminal program. */
        USBTIMER_DelayMs( predelay );
    }

    USBD_Disconnect();

    if ( postdelay ) {
        /*
         * Stay disconnected long enough to let host OS tear down the
         * USB CDC driver.
         */
        USBTIMER_DelayMs( postdelay );
    }
}

/**************************************************************************//**
 * @brief RTC IRQ Handler
 *****************************************************************************/
void RTC_IRQHandler( void )
{
    /* Clear interrupt flag */
    RTC->IFC = RTC_IFC_COMP1 | RTC_IFC_COMP0 | RTC_IFC_OF;
}

/**************************************************************************//**
 * Initialize and start RTC.
 *****************************************************************************/
static void StartRTC( void )
{
    /* Enable LE */
    CMU->HFCORECLKEN0 |= CMU_HFCORECLKEN0_LE;

    /* Enable LFRCO for RTC */
    CMU->OSCENCMD = CMU_OSCENCMD_LFRCOEN;
    /* Setup LFA to use LFRCRO */
    CMU->LFCLKSEL = CMU_LFCLKSEL_LFA_LFRCO;
    /* Enable RTC */
    CMU->LFACLKEN0 = CMU_LFACLKEN0_RTC;

    /* Clear interrupt flags */
    RTC->IFC = RTC_IFC_COMP1 | RTC_IFC_COMP0 | RTC_IFC_OF;
    /* 250 ms wakeup time */
    RTC->COMP0 = ( PIN_LOOP_INTERVAL * SystemLFRCOClockGet() ) / 1000;
    /* Enable Interrupts on COMP0 */
    RTC->IEN = RTC_IEN_COMP0;
    /* Enable RTC interrupts */
    NVIC_EnableIRQ(RTC_IRQn);
    /* Enable RTC */
    RTC->CTRL = RTC_CTRL_COMP0TOP | RTC_CTRL_DEBUGRUN | RTC_CTRL_EN;
}
