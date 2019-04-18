/**
 * \file
 *
 * \brief Example of usage of the maXTouch component with USART
 *
 * This example shows how to receive touch data from a maXTouch device
 * using the maXTouch component, and display them in a terminal window by using
 * the USART driver.
 *
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

/**
 * \mainpage
 *
 * \section intro Introduction
 * This simple example reads data from the maXTouch device and sends it over
 * USART as ASCII formatted text.
 *
 * \section files Main files:
 * - example_usart.c: maXTouch component USART example file
 * - conf_mxt.h: configuration of the maXTouch component
 * - conf_board.h: configuration of board
 * - conf_clock.h: configuration of system clock
 * - conf_example.h: configuration of example
 * - conf_sleepmgr.h: configuration of sleep manager
 * - conf_twim.h: configuration of TWI driver
 * - conf_usart_serial.h: configuration of USART driver
 *
 * \section apiinfo maXTouch low level component API
 * The maXTouch component API can be found \ref mxt_group "here".
 *
 * \section deviceinfo Device Info
 * All UC3 and Xmega devices with a TWI module can be used with this component
 *
 * \section exampledescription Description of the example
 * This example will read data from the connected maXTouch explained board
 * over TWI. This data is then processed and sent over a USART data line
 * to the board controller. The board controller will create a USB CDC class
 * object on the host computer and repeat the incoming USART data from the
 * main controller to the host. On the host this object should appear as a
 * serial port object (COMx on windows, /dev/ttyxxx on your chosen Linux flavour).
 *
 * Connect a terminal application to the serial port object with the settings
 * Baud: 57600
 * Data bits: 8-bit
 * Stop bits: 1 bit
 * Parity: None
 *
 * \section compinfo Compilation Info
 * This software was written for the GNU GCC and IAR for AVR.
 * Other compilers may or may not work.
 *
 * \section contactinfo Contact Information
 * For further information, visit
 * <A href="http://www.atmel.com/">Atmel</A>.\n
 */

#include <asf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf_board.h"
#include "conf_example.h"
#include "conf_uart_serial.h"

#include "func.h"

#include "ciclo.h"

// #define pino_pio PIOA

#define MAX_ENTRIES 3
#define STRING_LENGTH 70
#define STRING_MENU_LENGTH 35

#define USART_TX_MAX_LENGTH 0xff

volatile unsigned char state = CHOOSE_STATE;
volatile unsigned char prev_state = -1;
/** \brief Touch event struct */

// struct botao *pbotoes;
/*

Callbacks das funcoes de callback 
Botoes do display

*/

//struct botao botoes[][] = {{botaoPlay, botaoRight, botaoLeft, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
//                           {botaoPlay, botaoRight, botaoLeft, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
//                           {botaoPlay, botaoRight, botaoLeft, NULL, NULL, NULL, NULL, NULL, NULL, NULL}};

void initMenuOrder() {
  c_rapido.previous = &c_centrifuga;
  c_rapido.next = &c_diario;

  c_diario.previous = &c_rapido;
  c_diario.next = &c_pesado;

  c_pesado.previous = &c_diario;
  c_pesado.next = &c_enxague;

  c_enxague.previous = &c_pesado;
  c_enxague.next = &c_centrifuga;

  c_centrifuga.previous = &c_enxague;
  c_centrifuga.next = &c_rapido;
}

void next_callback(void) {
  actual_cycle = actual_cycle->next;

  draw_now = true;
}

void back_callback(void) {
  actual_cycle = actual_cycle->previous;

  draw_now = true;
}

void play_callback(void) {
  state = RUN_STATE;
  draw_now = true;
}

void pause_callback(void) {
  state = CHOOSE_STATE;
  draw_now = true;
}

void clean_screen(void) {
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
  ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH, ILI9488_LCD_HEIGHT);
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
}

void draw_button(struct botao b[], uint N) {
  for (int i = 0; i < N; i++) {
    ili9488_draw_pixmap(b[i].x,
                        b[i].y,
                        b[i].image->width,
                        b[i].image->height,
                        b[i].image->data);
  }
}

void draw_menu(t_ciclo *c) {
  char buf[STRING_MENU_LENGTH];

  ili9488_draw_pixmap(10,
                      10,
                      (c->image)->width,
                      (c->image)->height,
                      (c->image)->data);

  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
  ili9488_draw_filled_rectangle(10, 138, 138, 168);
  ili9488_draw_filled_rectangle(145, 10, 480, 220);
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
  ili9488_draw_string(10,
                      145,
                      c->nome);

  sprintf(buf, "Temperatura %d C", (c->temp));
  ili9488_draw_string(155,
                      10,
                      buf);
  sprintf(buf, "Smart Bubbles %s", c->bubblesOn ? "On" : "Off");
  ili9488_draw_string(155,
                      30,
                      buf);

  sprintf(buf, "Heavy mode %s", c->heavy ? "On" : "Off");
  ili9488_draw_string(155,
                      50,
                      buf);
  sprintf(buf, "Tempo centr. %d minutos", c->centrifugacaoTempo);
  ili9488_draw_string(155,
                      80,
                      buf);
  sprintf(buf, "Qnt enx. %d", c->enxagueQnt);
  ili9488_draw_string(155,
                      100,
                      buf);

  sprintf(buf, "Tempo enx. %d minutos", c->enxagueTempo);
  ili9488_draw_string(155,
                      120,
                      buf);

  sprintf(buf, "Centr RMP %d", c->centrifugacaoRPM);
  ili9488_draw_string(155,
                      140,
                      buf);
  sprintf(buf, "Tempo total %d minutos", (c->enxagueTempo * c->enxagueQnt) + (c->centrifugacaoTempo));
  ili9488_draw_string(155,
                      180,
                      buf);
}

void draw(struct botao botoes[], int N) {

  if (draw_now) {

    switch (state) {

    case CHOOSE_STATE:
      if (state != prev_state) {
        clean_screen();
        prev_state = state;
        draw_button(botoes, N);
      }

      draw_menu(actual_cycle);
      draw_now = false;
      break;

    case RUN_STATE:
      if (state != prev_state) {
        clean_screen();
        prev_state = state;
        draw_button(botoes, N);
      }

      draw_now = false;
      break;

    default:
      break;
    }
  }
}

int processa_touch(struct botao b[], struct botao *rtn, uint N, uint x, uint y) {
  for (int i = 0; i < N; i++) {
    if ((x >= b[i].x) && (x <= (b[i].x + b[i].size_x)) && (y >= b[i].y) && (y <= (b[i].y + b[i].size_y))) {

      //("Encontrou botao %d \n\r", i);
      *rtn = b[i];
      return true;
    }
  }
  return false;
}

static void configure_lcd(void) {
  /* Initialize display parameter */
  g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
  g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
  g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
  g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

  /* Initialize LCD */
  ili9488_init(&g_ili9488_display_opt);
}

/**
 * \brief Set maXTouch configuration
 *
 * This function writes a set of predefined, optimal maXTouch configuration data
 * to the maXTouch Xplained Pro.
 *
 * \param device Pointer to mxt_device struct
 */
static void mxt_init(struct mxt_device *device) {
  enum status_code status;

  /* T8 configuration object data */
  uint8_t t8_object[] = {
      0x0d, 0x00, 0x05, 0x0a, 0x4b, 0x00, 0x00,
      0x00, 0x32, 0x19};

  /* T9 configuration object data */
  uint8_t t9_object[] = {
      0x8B, 0x00, 0x00, 0x0E, 0x08, 0x00, 0x80,
      0x32, 0x05, 0x02, 0x0A, 0x03, 0x03, 0x20,
      0x02, 0x0F, 0x0F, 0x0A, 0x00, 0x00, 0x00,
      0x00, 0x18, 0x18, 0x20, 0x20, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x02,
      0x02};

  /* T46 configuration object data */
  uint8_t t46_object[] = {
      0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x03,
      0x00, 0x00};

  /* T56 configuration object data */
  uint8_t t56_object[] = {
      0x02, 0x00, 0x01, 0x18, 0x1E, 0x1E, 0x1E,
      0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E,
      0x1E, 0x1E, 0x1E, 0x1E, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00};

  /* TWI configuration */
  twihs_master_options_t twi_opt = {
      .speed = MXT_TWI_SPEED,
      .chip = MAXTOUCH_TWI_ADDRESS,
  };

  status = (enum status_code)twihs_master_setup(MAXTOUCH_TWI_INTERFACE, &twi_opt);
  Assert(status == STATUS_OK);

  /* Initialize the maXTouch device */
  status = mxt_init_device(device, MAXTOUCH_TWI_INTERFACE,
                           MAXTOUCH_TWI_ADDRESS, MAXTOUCH_XPRO_CHG_PIO);
  Assert(status == STATUS_OK);

  /* Issue soft reset of maXTouch device by writing a non-zero value to
	 * the reset register */
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_COMMANDPROCESSOR_T6, 0) + MXT_GEN_COMMANDPROCESSOR_RESET, 0x01);

  /* Wait for the reset of the device to complete */
  delay_ms(MXT_RESET_TIME);

  /* Write data to configuration registers in T7 configuration object */
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_POWERCONFIG_T7, 0) + 0, 0x20);
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_POWERCONFIG_T7, 0) + 1, 0x10);
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_POWERCONFIG_T7, 0) + 2, 0x4b);
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_POWERCONFIG_T7, 0) + 3, 0x84);

  /* Write predefined configuration data to configuration objects */
  mxt_write_config_object(device, mxt_get_object_address(device, MXT_GEN_ACQUISITIONCONFIG_T8, 0), &t8_object);
  mxt_write_config_object(device, mxt_get_object_address(device, MXT_TOUCH_MULTITOUCHSCREEN_T9, 0), &t9_object);
  mxt_write_config_object(device, mxt_get_object_address(device, MXT_SPT_CTE_CONFIGURATION_T46, 0), &t46_object);
  mxt_write_config_object(device, mxt_get_object_address(device, MXT_PROCI_SHIELDLESS_T56, 0), &t56_object);

  /* Issue recalibration command to maXTouch device by writing a non-zero
	 * value to the calibrate register */
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_COMMANDPROCESSOR_T6, 0) + MXT_GEN_COMMANDPROCESSOR_CALIBRATE, 0x01);
}

void draw_screen(void) {
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
  ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH - 1, ILI9488_LCD_HEIGHT - 1);
}

uint32_t convert_axis_system_x(uint32_t touch_x) {
  // entrada: 4096 - 0 (sistema de coordenadas atual)
  // saida: 0 - 320
  return ILI9488_LCD_WIDTH - (ILI9488_LCD_WIDTH * touch_x / 4096);
}

uint32_t convert_axis_system_y(uint32_t touch_y) {
  // entrada: 0 - 4096 (sistema de coordenadas atual)
  // saida: 0 - 320
  return ILI9488_LCD_HEIGHT - (ILI9488_LCD_HEIGHT * touch_y / 4096);
}

void mxt_handler(struct mxt_device *device, struct botao botoes[], uint Nbotoes) {
  /* USART tx buffer initialized to 0 */
  char tx_buf[STRING_LENGTH * MAX_ENTRIES] = {0};
  uint8_t i = 0; /* Iterator */

  /* Temporary touch event data struct */
  struct mxt_touch_event touch_event;

  /* Collect touch events and put the data in a string,
	 * maximum 2 events at the time */
  do {
    /* Temporary buffer for each new touch event line */
    char buf[STRING_LENGTH];

    /* Read next next touch event in the queue, discard if read fails */
    if (mxt_read_touch_event(device, &touch_event) != STATUS_OK) {
      continue;
    }

    // eixos trocados (quando na vertical LCD)
    uint32_t conv_x = convert_axis_system_x(touch_event.x);
    uint32_t conv_y = convert_axis_system_y(touch_event.y);

    /* Format a new entry in the data string that will be sent over USART */
    sprintf(buf, "X:%3d Y:%3d \n", conv_x, conv_y);

    /* -----------------------------------------------------*/
    struct botao bAtual;
    //contribuicao de status
    if (processa_touch(botoes, &bAtual, Nbotoes, conv_x, conv_y) && touch_event.status < 60) {
      bAtual.p_handler();
    }
    //update_screen(conv_x, conv_y);
    /* -----------------------------------------------------*/

    /* Add the new string to the string buffer */
    strcat(tx_buf, buf);
    i++;

    /* Check if there is still messages in the queue and
		 * if we have reached the maximum numbers of events */
  } while ((mxt_is_message_pending(device)) & (i < MAX_ENTRIES));

/* If there is any entries in the buffer, send them over USART */
#ifndef test

  if (i > 0) {
    usart_serial_write_packet(USART_SERIAL_EXAMPLE, (uint8_t *)tx_buf, strlen(tx_buf));
  }
#endif
}

int main(void) {
  struct mxt_device device; /* Device data container */

  /* Initialize the USART configuration struct */
  const usart_serial_options_t usart_serial_options = {
      .baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
      .charlength = USART_SERIAL_CHAR_LENGTH,
      .paritytype = USART_SERIAL_PARITY,
      .stopbits = USART_SERIAL_STOP_BIT};

  sysclk_init(); /* Initialize system clocks */
  board_init();  /* Initialize board */
  configure_lcd();
  draw_screen();
  initMenuOrder();

  struct botao botoes[][10] = {
      {
          botaoLeft,
          botaoPlay,
          botaoRight,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
      },
      {

          botaoPause,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
      },
  };
  int botoes_num[] = {3, 1, 0};

  /* Initialize the mXT touch device */
  mxt_init(&device);

  /* Initialize stdio on USART */
  stdio_serial_init(USART_SERIAL_EXAMPLE, &usart_serial_options);

  printf("\n\rmaXTouch data USART transmitter\n\r");

  //pino led = {.id = 4, .id_pio = ID_PIOA, .mask = 1 << 4, .pio = PIOA};

  ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH, ILI9488_LCD_HEIGHT);

  //draw_button(botoes[state], botoes_num[state]);

  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
  //   ili9488_draw_string(botaoRight.x + botaoRight.image->width + 10,
  //                       botaoRight.y + botaoRight.image->height / 2,
  //                       "Local");

  /* -----------------------------------------------------*/
  while (true) {
    draw(botoes[state], botoes_num[state]);
    /* Check for any pending messages and run message handler if any
		 * message is found in the queue */
    if (mxt_is_message_pending(&device)) {
      mxt_handler(&device, botoes[state], botoes_num[state]);
    }
  }

  return 0;
}
