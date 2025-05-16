#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "vl53l1_platform.h"

static int i2c_fd = -1;
static int device_address = 0x52; // Adresse par défaut du VL53L1

VL53L1_Error VL53L1_CommsInitialise(VL53L1_Dev_t *pdev, uint8_t comms_type, uint16_t comms_speed_khz)
{
    if (comms_type != 1) return VL53L1_ERROR_NOT_IMPLEMENTED; // SPI non supporté ici

    if (wiringPiSetup() == -1)
        return VL53L1_ERROR_CONTROL_INTERFACE;

    i2c_fd = wiringPiI2CSetup(device_address);
    if (i2c_fd == -1)
        return VL53L1_ERROR_CONTROL_INTERFACE;

    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_CommsClose(VL53L1_Dev_t *pdev)
{
    i2c_fd = -1;
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WriteMulti(VL53L1_Dev_t *pdev, uint16_t index, uint8_t *pdata, uint32_t count)
{
    wiringPiI2CWriteReg8(i2c_fd, (index >> 8) & 0xFF, index & 0xFF); // Prépare le pointeur
    for (uint32_t i = 0; i < count; ++i)
        if (wiringPiI2CWrite(i2c_fd, pdata[i]) == -1)
            return VL53L1_ERROR_CONTROL_INTERFACE;
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_ReadMulti(VL53L1_Dev_t *pdev, uint16_t index, uint8_t *pdata, uint32_t count)
{
    wiringPiI2CWriteReg8(i2c_fd, (index >> 8) & 0xFF, index & 0xFF);
    for (uint32_t i = 0; i < count; ++i) {
        int val = wiringPiI2CRead(i2c_fd);
        if (val == -1) return VL53L1_ERROR_CONTROL_INTERFACE;
        pdata[i] = (uint8_t)val;
    }
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WrByte(VL53L1_Dev_t *pdev, uint16_t index, uint8_t data)
{
    int res = wiringPiI2CWriteReg8(i2c_fd, index, data);
    return res == -1 ? VL53L1_ERROR_CONTROL_INTERFACE : VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WrWord(VL53L1_Dev_t *pdev, uint16_t index, uint16_t data)
{
    return VL53L1_WriteMulti(pdev, index, (uint8_t[]){(data >> 8) & 0xFF, data & 0xFF}, 2);
}

VL53L1_Error VL53L1_WrDWord(VL53L1_Dev_t *pdev, uint16_t index, uint32_t data)
{
    return VL53L1_WriteMulti(pdev, index, (uint8_t[]){(data >> 24) & 0xFF, (data >> 16) & 0xFF, (data >> 8) & 0xFF, data & 0xFF}, 4);
}

VL53L1_Error VL53L1_RdByte(VL53L1_Dev_t *pdev, uint16_t index, uint8_t *pdata)
{
    *pdata = wiringPiI2CReadReg8(i2c_fd, index);
    return (*pdata == -1) ? VL53L1_ERROR_CONTROL_INTERFACE : VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_RdWord(VL53L1_Dev_t *pdev, uint16_t index, uint16_t *pdata)
{
    uint8_t buffer[2];
    VL53L1_Error status = VL53L1_ReadMulti(pdev, index, buffer, 2);
    if (status == VL53L1_ERROR_NONE)
        *pdata = ((uint16_t)buffer[0] << 8) | buffer[1];
    return status;
}

VL53L1_Error VL53L1_RdDWord(VL53L1_Dev_t *pdev, uint16_t index, uint32_t *pdata)
{
    uint8_t buffer[4];
    VL53L1_Error status = VL53L1_ReadMulti(pdev, index, buffer, 4);
    if (status == VL53L1_ERROR_NONE)
        *pdata = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | buffer[3];
    return status;
}

VL53L1_Error VL53L1_WaitUs(VL53L1_Dev_t *pdev, int32_t wait_us)
{
    usleep(wait_us);
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WaitMs(VL53L1_Dev_t *pdev, int32_t wait_ms)
{
    delay(wait_ms);
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_GetTimerFrequency(int32_t *ptimer_freq_hz)
{
    *ptimer_freq_hz = 1000;
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_GetTimerValue(int32_t *ptimer_count)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    *ptimer_count = (int32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    return VL53L1_ERROR_NONE;
}

// Implémentations GPIO (optionnelles selon câblage)

VL53L1_Error VL53L1_GpioSetMode(uint8_t pin, uint8_t mode)
{
    pinMode(pin, mode); // 0 = input, 1 = output
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_GpioSetValue(uint8_t pin, uint8_t value)
{
    digitalWrite(pin, value);
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_GpioGetValue(uint8_t pin, uint8_t *pvalue)
{
    *pvalue = digitalRead(pin);
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_GpioXshutdown(uint8_t value)
{
    digitalWrite(7, value); // Exemple: GPIO7 pour XSHUT
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_GpioCommsSelect(uint8_t value)
{
    return VL53L1_ERROR_NONE; // Non utilisé pour I2C
}

VL53L1_Error VL53L1_GpioPowerEnable(uint8_t value)
{
    return VL53L1_ERROR_NONE; // Ajoutez selon câblage
}

VL53L1_Error VL53L1_GpioInterruptEnable(void (*function)(void), uint8_t edge_type)
{
    return VL53L1_ERROR_NOT_IMPLEMENTED;
}

VL53L1_Error VL53L1_GpioInterruptDisable(void)
{
    return VL53L1_ERROR_NOT_IMPLEMENTED;
}

VL53L1_Error VL53L1_GetTickCount(VL53L1_DEV Dev, uint32_t *ptime_ms)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    *ptime_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WaitValueMaskEx(VL53L1_Dev_t *pdev, uint32_t timeout_ms, uint16_t index, uint8_t value, uint8_t mask, uint32_t poll_delay_ms)
{
    uint8_t data;
    uint32_t elapsed = 0;
    while (elapsed < timeout_ms) {
        VL53L1_RdByte(pdev, index, &data);
        if ((data & mask) == value)
            return VL53L1_ERROR_NONE;
        VL53L1_WaitMs(pdev, poll_delay_ms);
        elapsed += poll_delay_ms;
    }
    return VL53L1_ERROR_TIME_OUT;
}
