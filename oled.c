#include "oled.h"
#include "oled_font.h"

//PB6接SCL, PB7接SDA）
#define OLED_SCL_PIN     GPIO_PIN_6
#define OLED_SDA_PIN     GPIO_PIN_7
#define OLED_SCL_PORT    GPIOB
#define OLED_SDA_PORT    GPIOB

// 宏定义操作函数
#define OLED_SCL_H()     HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_PIN_SET)
#define OLED_SCL_L()     HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_PIN_RESET)
#define OLED_SDA_H()     HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, GPIO_PIN_SET)
#define OLED_SDA_L()     HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, GPIO_PIN_RESET)
#define OLED_SDA_READ()  HAL_GPIO_ReadPin(OLED_SDA_PORT, OLED_SDA_PIN)

extern void delay_us(uint32_t us);

static void I2C_Start(void)
{
    OLED_SDA_H();
    OLED_SCL_H();
    delay_us(5);
    OLED_SDA_L();
    delay_us(5);
    OLED_SCL_L();
}

static void I2C_Stop(void)
{
    OLED_SDA_L();
    OLED_SCL_H();
    delay_us(5);
    OLED_SDA_H();
    delay_us(5);
}

static void I2C_SendByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (byte & 0x80)
            OLED_SDA_H();
        else
            OLED_SDA_L();
        delay_us(2);
        OLED_SCL_H();
        delay_us(2);
        OLED_SCL_L();
        byte <<= 1;
    }
    // 释放SDA，忽略应答
    OLED_SDA_H();
    OLED_SCL_H();
    delay_us(2);
    OLED_SCL_L();
}

static void OLED_WriteCmd(uint8_t cmd)
{
    I2C_Start();
    I2C_SendByte(0x78);  // 设备地址（写）
    I2C_SendByte(0x00);  // 命令
    I2C_SendByte(cmd);
    I2C_Stop();
}

static void OLED_WriteData(uint8_t data)
{
    I2C_Start();
    I2C_SendByte(0x78);
    I2C_SendByte(0x40);  // 数据
    I2C_SendByte(data);
    I2C_Stop();
}

static void OLED_SetCursor(uint8_t y, uint8_t x)
{
    OLED_WriteCmd(0xB0 | y);
    OLED_WriteCmd(0x10 | ((x & 0xF0) >> 4));
    OLED_WriteCmd(0x00 | (x & 0x0F));
}

void OLED_Clear(void)
{
    for (uint8_t y = 0; y < 8; y++)
    {
        OLED_SetCursor(y, 0);
        for (uint8_t x = 0; x < 128; x++)
        {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);
}

void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    while (*String)
    {
        OLED_ShowChar(Line, Column++, *String++);
    }
}

static uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t res = 1;
    while (Y--) res *= X;
    return res;
}

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    for (uint8_t i = 0; i < Length; i++)
    {
        uint8_t digit = (Number / OLED_Pow(10, Length - i - 1)) % 10;
        OLED_ShowChar(Line, Column + i, digit + '0');
    }
}

void OLED_Init(void)
{
    // 1. 使能GPIOB时钟并配置引脚为开漏输出
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = OLED_SCL_PIN | OLED_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    OLED_SCL_H();
    OLED_SDA_H();
    
    // 2. 上电延时
    HAL_Delay(500);
    
    // 3. SSD1306初始化命令序列
    OLED_WriteCmd(0xAE); // 关闭显示
    OLED_WriteCmd(0xD5); OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xA8); OLED_WriteCmd(0x3F);
    OLED_WriteCmd(0xD3); OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0xA1);
    OLED_WriteCmd(0xC8);
    OLED_WriteCmd(0xDA); OLED_WriteCmd(0x12);
    OLED_WriteCmd(0x81); OLED_WriteCmd(0xCF);
    OLED_WriteCmd(0xD9); OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDB); OLED_WriteCmd(0x30);
    OLED_WriteCmd(0xA4);
    OLED_WriteCmd(0xA6);
    OLED_WriteCmd(0x8D); OLED_WriteCmd(0x14);
    OLED_WriteCmd(0xAF);
    
    OLED_Clear();
}
