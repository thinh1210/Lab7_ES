/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "fsmc.h"
#include "stdio.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "software_timer.h"
#include "led_7seg.h"
#include "button.h"
#include "lcd.h"
#include "picture.h"
#include "ds3231.h"
#include "sensor.h"
#include "buzzer.h"
#include "touch.h"
#include "snake.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#define INIT 0
#define DRAW 1
#define CLEAR 2
#define SELECT_SPEED 3 // <-- TRẠNG THÁI MỚI
#define START 4        // <-- ĐỔI SỐ (TRƯỚC LÀ 3)
#define INITIAL_SNAKE_LENGTH 3
// GAME_OVER không phải là một trạng thái, nó là một cờ (flag), vẫn giữ nguyên

int draw_Status = INIT;
int button[4] = {0}; // Up : 0 ; Down: 1; Left: 2; Right: 3

// --- THÊM CÁC DEFINE TỐC ĐỘ ---
// Tốc độ là số tick (50ms) chờ trước khi rắn di chuyển
// Số càng nhỏ, game càng nhanh
#define SPEED_EASY 10  // 1000ms mỗi bước
#define SPEED_MEDIUM 5 // 750ms mỗi bước
#define SPEED_HARD 3   // 500ms mỗi bước
#define SPEED_EXPERT 1 // 250ms mỗi bước

// --- THÊM BIẾN TỐC ĐỘ ĐÃ CHỌN ---
uint8_t selected_Speed = SPEED_MEDIUM; // Tốc độ mặc định

char score_str[20]; // Chuỗi để hiển thị điểm

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void test_LedDebug();
void draw_Game_Border();
void touchProcess();
uint8_t isButtonClear();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_SPI1_Init();
  MX_FSMC_Init();
  MX_I2C1_Init();
  MX_TIM13_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  system_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  touch_Adjust();

  lcd_Clear(BLACK);

  while (1)
  {
    // scan touch screen

    draw_Game_Border();
    touch_Scan();
    // check if touch screen is touched
    if (touch_IsTouched() && draw_Status == DRAW)
    {
      // draw a point at the touch position
      lcd_DrawPoint(touch_GetX(), touch_GetY(), RED);
    }
    // 50ms task
    if (flag_timer2 == 1)
    {
      flag_timer2 = 0;
      touchProcess();
      test_LedDebug();
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

#define GAME_BLOCK_SIZE 10
#define GAME_AREA_WIDTH 240
#define GAME_AREA_HEIGHT 200 // Chừa 120px bên dưới cho nút bấm

void draw_Game_Border()
{
  int x, y;
  // Vẽ viền trên và dưới
  for (x = 0; x < GAME_AREA_WIDTH; x += GAME_BLOCK_SIZE)
  {
    // Viền trên (Y=0)
    lcd_Fill(x, 0, x + GAME_BLOCK_SIZE - 1, GAME_BLOCK_SIZE - 1, BLUE);
    // Viền dưới (Y = 190)
    lcd_Fill(x, GAME_AREA_HEIGHT - GAME_BLOCK_SIZE, x + GAME_BLOCK_SIZE - 1, GAME_AREA_HEIGHT - 1, BLUE);
  }

  // Vẽ viền trái và phải (tránh vẽ đè lên các góc)
  for (y = GAME_BLOCK_SIZE; y < (GAME_AREA_HEIGHT - GAME_BLOCK_SIZE); y += GAME_BLOCK_SIZE)
  {
    // Viền trái (X=0)
    lcd_Fill(0, y, GAME_BLOCK_SIZE - 1, y + GAME_BLOCK_SIZE - 1, BLUE);
    // Viền phải (X = 230)
    lcd_Fill(GAME_AREA_WIDTH - GAME_BLOCK_SIZE, y, GAME_AREA_WIDTH - 1, y + GAME_BLOCK_SIZE - 1, BLUE);
  }
}
void system_init()
{
  timer_init();
  button_init();
  lcd_init();
  touch_init();
  setTimer2(50);
}

uint8_t count_led_debug = 0;

void test_LedDebug()
{
  count_led_debug = (count_led_debug + 1) % 20;
  if (count_led_debug == 0)
  {
    HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
  }
}
// ... (Ngay sau hàm isButtonRight()) ...

uint8_t isButtonSpeedEasy()
{
  if (!touch_IsTouched())
    return 0;
  // Tọa độ X: 20-220, Y: 50-100
  return touch_GetX() > 20 && touch_GetX() < 220 && touch_GetY() > 50 && touch_GetY() < 100;
}
uint8_t isButtonSpeedMedium()
{
  if (!touch_IsTouched())
    return 0;
  // Tọa độ X: 20-220, Y: 120-170
  return touch_GetX() > 20 && touch_GetX() < 220 && touch_GetY() > 120 && touch_GetY() < 170;
}
uint8_t isButtonSpeedHard()
{
  if (!touch_IsTouched())
    return 0;
  // Tọa độ X: 20-220, Y: 190-240
  return touch_GetX() > 20 && touch_GetX() < 220 && touch_GetY() > 190 && touch_GetY() < 240;
}
uint8_t isButtonSpeedExpert()
{
  if (!touch_IsTouched())
    return 0;
  // Tọa độ X: 20-220, Y: 260-310
  return touch_GetX() > 20 && touch_GetX() < 220 && touch_GetY() > 260 && touch_GetY() < 310;
}
uint8_t isButtonClear()
{
  if (!touch_IsTouched())
    return 0;
  return touch_GetX() > 60 && touch_GetX() < 180 && touch_GetY() > 80 && touch_GetY() < 130;
}
uint8_t isButtonStart()
{
  if (!touch_IsTouched())
    return 0;
  return touch_GetX() > 60 && touch_GetX() < 180 && touch_GetY() > 10 && touch_GetY() < 60;
}
uint8_t isButtonUp()
{
  if (!touch_IsTouched())
    return 0;

  return touch_GetX() > 105 && touch_GetX() < 135 && touch_GetY() > 210 && touch_GetY() < 240; // Nút mới: 105-135, 210-240 (30x30)
}
uint8_t isButtonDown()
{
  if (!touch_IsTouched())
    return 0;
  return touch_GetX() > 105 && touch_GetX() < 135 && touch_GetY() > 280 && touch_GetY() < 310; // Nút mới: 105-135, 280-310 (30x30)
}
uint8_t isButtonLeft()
{
  if (!touch_IsTouched())
    return 0;
  return touch_GetX() > 20 && touch_GetX() < 50 && touch_GetY() > 245 && touch_GetY() < 275; // Nút mới: 20-50, 245-275 (30x30) - dời qua trái
}
uint8_t isButtonRight()
{
  if (!touch_IsTouched())
    return 0;
  return touch_GetX() > 190 && touch_GetX() < 220 && touch_GetY() > 245 && touch_GetY() < 275; // Nút mới: 190-220, 245-275 (30x30) - dời qua phải
}
void touchProcess()
{
  switch (draw_Status)
  {
  case INIT:
    lcd_Fill(60, 10, 180, 60, GBLUE);
    lcd_ShowStr(90, 20, "START", RED, BLACK, 24, 1);

    lcd_Fill(60, 80, 180, 130, GBLUE);
    lcd_ShowStr(90, 90, "RESET", RED, BLACK, 24, 1);

    draw_Status = DRAW;
    break;

  case DRAW:
    if (isButtonClear())
    {
      draw_Status = CLEAR;
      lcd_Fill(0, 0, 240, 320, BLACK);
      lcd_Fill(60, 10, 180, 60, GREEN);
      lcd_ShowStr(90, 20, "RESET", RED, BLACK, 24, 1);
    }

    if (isButtonStart())
    {
      draw_Status = SELECT_SPEED;
      lcd_Fill(0, 0, 240, 320, BLACK);

      lcd_ShowStr(40, 20, "SELECT SPEED", WHITE, BLACK, 24, 0);
      lcd_Fill(20, 50, 220, 100, GREEN);
      lcd_ShowStr(90, 65, "EASY", BLACK, GREEN, 24, 1);
      lcd_Fill(20, 120, 220, 170, YELLOW);
      lcd_ShowStr(75, 135, "MEDIUM", BLACK, YELLOW, 24, 1);
      lcd_Fill(20, 190, 220, 240, BLUE);
      lcd_ShowStr(90, 205, "HARD", BLACK, BLUE, 24, 1);
      lcd_Fill(20, 260, 220, 310, RED);
      lcd_ShowStr(75, 275, "EXPERT", BLACK, RED, 24, 1);
    }
    break;

  case SELECT_SPEED:
    uint8_t speed_was_selected = 0;

    if (isButtonSpeedEasy())
    {
      selected_Speed = SPEED_EASY;
      speed_was_selected = 1;
    }
    else if (isButtonSpeedMedium())
    {
      selected_Speed = SPEED_MEDIUM;
      speed_was_selected = 1;
    }
    else if (isButtonSpeedHard())
    {
      selected_Speed = SPEED_HARD;
      speed_was_selected = 1;
    }
    else if (isButtonSpeedExpert())
    {
      selected_Speed = SPEED_EXPERT;
      speed_was_selected = 1;
    }

    if (speed_was_selected)
    {
      draw_Status = START;
      lcd_Fill(0, 0, 240, 320, BLACK);
      draw_Game_Border();
      game_Init();

      // --- THÊM HIỂN THỊ ĐIỂM BAN ĐẦU ---
      score = 0;
      sprintf(score_str, "SCORE: %d", score);
      // Hiển thị ở giữa các nút, Y=255, X=70, chữ trắng, nền đen, font 16, không đè
      lcd_ShowStr(70, 255, score_str, WHITE, BLACK, 16, 0);
    }
    break;

  case START:
    // Vẽ các nút điều khiển
    lcd_Fill(105, 210, 135, 240, RED); // UP
    lcd_Fill(105, 280, 135, 310, RED); // DOWN
    lcd_Fill(20, 245, 50, 275, RED);   // LEFT
    lcd_Fill(190, 245, 220, 275, RED); // RIGHT

    if (is_game_over)
    {
      lcd_ShowStr(60, 100, "GAME OVER", RED, BLACK, 24, 1);
      if (isButtonClear() || isButtonStart())
      {
        draw_Status = CLEAR;
      }
      break;
    }

    // Xử lý điều khiển
    if (isButtonUp() && direction != 1)
    {
      direction = 0;
    }
    else if (isButtonDown() && direction != 0)
    {
      direction = 1;
    }
    else if (isButtonLeft() && direction != 3)
    {
      direction = 2;
    }
    else if (isButtonRight() && direction != 2)
    {
      direction = 3;
    }

    game_tick_counter++;
    if (game_tick_counter >= selected_Speed)
    {
      game_tick_counter = 0;

      // --- CẬP NHẬT LOGIC ĐIỂM SỐ ---
      // 1. Gọi update_Game() và lấy độ dài mới
      update_Game();
      // 2. Tính điểm
      score = snake_length - INITIAL_SNAKE_LENGTH;
      // 3. Chuyển điểm thành chuỗi
      sprintf(score_str, "SCORE: %d", score);
      // 4. Hiển thị chuỗi (vị trí X=70, Y=255)
      lcd_ShowStr(70, 255, score_str, WHITE, BLACK, 16, 0);
    }
    break;

  case CLEAR:
    if (!touch_IsTouched())
      draw_Status = INIT;
    break;

  default:
    break;
  }
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
