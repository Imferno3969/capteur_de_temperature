#include <LoRaWan.h>
#include "SCD30.h"

#if defined(ARDUINO_ARCH_AVR)
 #pragma message("Defined architecture for ARDUINO_ARCH_AVR.")
 #define SERIAL Serial
#elif defined(ARDUINO_ARCH_SAM)
 #pragma message("Defined architecture for ARDUINO_ARCH_SAM.")
 #define SERIAL SerialUSB
#elif defined(ARDUINO_ARCH_SAMD)
 #pragma message("Defined architecture for ARDUINO_ARCH_SAMD.") 
 #define SERIAL SerialUSB
#elif defined(ARDUINO_ARCH_STM32F4)
 #pragma message("Defined architecture for ARDUINO_ARCH_STM32F4.")
 #define SERIAL SerialUSB
#else
 #pragma message("Not found any architecture.")
 #define SERIAL Serial
#endif

char buffer[256];

// TTN parameters
const char *appEui = "8CF9572000000000";
const char *appKey = "DB9E0937174BCA9A21AAC71083C352BB";

void setup(void)
{
    SerialUSB.begin(115200);
    while(!SerialUSB);
    
    lora.init();
    
    memset(buffer, 0, 256);
    lora.getVersion(buffer, 256, 1);
    SerialUSB.print(buffer); 
    
    memset(buffer, 0, 256);
    lora.getId(buffer, 256, 1);
    SerialUSB.print(buffer);
    
    lora.setId(NULL,"70B3D57ED0061F71","8CF9572000000000");
    lora.setKey(NULL,NULL, "DB9E0937174BCA9A21AAC71083C352BB");
    
    lora.setDeciveMode(LWOTAA);
    lora.setDataRate(DR0, EU868);
    
    lora.setChannel(0, 868.1);
    lora.setChannel(1, 868.3);
    lora.setChannel(2, 868.5);
    
    lora.setReceiceWindowFirst(0, 868.1);
    lora.setReceiceWindowSecond(869.5, DR3);
    
    lora.setDutyCycle(false);
    lora.setJoinDutyCycle(false);
    
    lora.setPower(14);
    
    while(!lora.setOTAAJoin(JOIN));

    SerialUSB.println("SCD30 Raw Data");
    scd30.initialize();
}

void loop(void)
{   
    bool result = false;
    
    float scdresult[3] = {0};
      
    if(scd30.isAvailable())
    {
        scd30.getCarbonDioxideConcentration(scdresult);
        
        // Format the data for TTN payload
        String payload = String(scdresult[0]) + "," + String(scdresult[1]) + "," + String(scdresult[2]);

        // Convert payload to char array
        char payloadCharArray[payload.length() + 1];
        payload.toCharArray(payloadCharArray, payload.length() + 1);

        // Send the data to TTN
        result = lora.transferPacket(payloadCharArray, strlen(payloadCharArray));
        
        if(result)
        {
            short length;
            short rssi;
            
            memset(buffer, 0, 256);
            length = lora.receivePacket(buffer, 256, &rssi);
            
            if(length)
            {
                SerialUSB.print("Length is: ");
                SerialUSB.println(length);
                SerialUSB.print("RSSI is: ");
                SerialUSB.println(rssi);
                SerialUSB.print("Data is: ");
                for(unsigned char i = 0; i < length; i ++)
                {
                    SerialUSB.print("0x");
                    SerialUSB.print(buffer[i], HEX);
                    SerialUSB.print(" ");
                }
                SerialUSB.println();
            }
        }
      
        SERIAL.print("Carbon Dioxide Concentration is: ");
        SERIAL.print(scdresult[0]);
        SERIAL.println(" ppm");
        SERIAL.println(" ");
        SERIAL.print("Temperature = ");
        SERIAL.print(scdresult[1]);
        SERIAL.println(" ℃");
        SERIAL.println(" ");
        SERIAL.print("Humidity = ");
        SERIAL.print(scdresult[2]);
        SERIAL.println(" %");
        SERIAL.println(" ");
        SERIAL.println(" ");
        SERIAL.println(" ");
    }
}
