#include <stdint.h>
#include "gpio_def.h"
#include "spi.h"

void AD7606_Init(void);
void AD7606_StartConversion(void);
int16_t AD7606_ReadChannel(uint8_t channel);
int8_t AD7606_ReadAllChannels(uint8_t* data);

// Initialize AD7606
void AD7606_Init(void)
{
    // Initialize GPIO pins for AD7606
    // SPI0_Init();  // Initialize SPI for AD7606 communication
    
    GPIO_INIT(AD7606_CONVST_PIN, GPIO_OUTPUT);  // Initialize CONVST pin as output
    GPIO_INIT(AD7606_RESET_PIN, GPIO_OUTPUT);   // Initialize RESET pin as output
    GPIO_INIT(AD7606_BUSY_PIN, GPIO_INPUT);     // Initialize BUSY pin as input (optional, if needed)

    // Initialize control pins as outputs
    GPIO_SET(AD7606_CONVST_PIN, GPIO_LEVEL_HIGH);  // Set CONVST high initially
    GPIO_SET(AD7606_RESET_PIN, GPIO_LEVEL_LOW);   // Set RESET low initially
    
    // Perform reset sequence
    GPIO_SET(AD7606_RESET_PIN, GPIO_LEVEL_HIGH);   // Reset high
    // Small delay could be added here if needed
    for (volatile int i = 0; i < 100; i++);  // Simple delay loop (adjust as needed)
    GPIO_SET(AD7606_RESET_PIN, GPIO_LEVEL_LOW);   // Reset low
}

// Start conversion
void AD7606_StartConversion(void)
{
    GPIO_SET(AD7606_CONVST_PIN, GPIO_LEVEL_LOW);  // CONVST low to start conversion
    // Small delay could be added here if needed
    for (volatile int i = 0; i < 10; i++);  // Simple delay loop (adjust as needed)
    GPIO_SET(AD7606_CONVST_PIN, GPIO_LEVEL_HIGH);  // CONVST back to high
}

// Read data from AD7606 (reads one channel)
int16_t AD7606_ReadChannel(uint8_t channel)
{
    int16_t result;
    uint8_t msb, lsb;
    
    if (channel > 7) return 0;  // AD7606 has 8 channels (0-7)
    
    // Read MSB and LSB
    msb = SPI0_TransferByte(AD7606_CS_PIN, 0x05);  // Send dummy byte to receive MSB
    lsb = SPI0_TransferByte(AD7606_CS_PIN, 0xAF);  // Send dummy byte to receive LSB
    
    // Combine MSB and LSB to form 16-bit result
    result = (msb << 8) | lsb;
    
    return result;
}

// Read all channels at once
int8_t AD7606_ReadAllChannels(uint8_t* data)
{
    int timeout = 1000;  // Timeout for BUSY signal (in case it doesn't go low)
    static uint8_t send_buffer[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                             0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Dummy data to read all channels
    
    // Start conversion
    AD7606_StartConversion();
    
    // Wait for BUSY to go high then low
    // Note: In actual implementation, you might need to implement a proper way to read BUSY pin
    for (volatile int j = 0; j < 10; j++);
    while (GPIO_GETVALUE(AD7606_BUSY_PIN) == GPIO_LEVEL_HIGH)  // Wait until BUSY goes low
    {
        if (timeout-- == 0) {
            // Handle timeout error (e.g., log error, return, etc.)
            return -1;  // Indicate error
        }
    }
    
    // Read 2 channels
    SPI0_TransferBuffer(AD7606_CS_PIN, send_buffer, data, 4);

    return 0;  // Indicate success
}