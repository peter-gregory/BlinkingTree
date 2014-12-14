/******************************************************************************
* Project Name		: Christmas Tree Blinking LED
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CY8C4245AXI-483
* Software Used		: PSoC Creator 3.0 Service Pack 2
* Compiler    		: ARMGCC 4.7.3, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CY8CKIT-049-42xx PSoC 4 Prototyping Kit 
*
*******************************************************************************/

/******************************************************************************
*                           THEORY OF OPERATION
* This is a Christmas Tree 13 RGB blinking LED project. 
* The LED control lines are organized in a matrix of 4 source lines and 12 control 
* lines.  A timer interrupt is used to update the matrix and emulate PWM operations 
* on the LED lines.  Each Red, Green, Blue control line has 64 levels of brightness
* Default UART Port Configuration for bootloading the PSoC 4 in CY8CKIT-049-42xx
* Baud Rate : 115200 bps
* Data Bits : 8
* Stop Bits : 1
* Parity    : None
******************************************************************************/
#include <project.h>

uint16 lights[48];
uint16 activeRow;
uint16 blinkTime;
uint16 delay;

uint8 lightTable[13][3] =
{
    {4,6,5}, // Top

    {39,42,40}, // Level 3 - Front-Right
    {37,47,38}, // Level 3 - Front-Left
    {41,46,43}, // Level 3 - Back-Left
    {36,45,44}, // Level 3 - Back-Right

    {25,24,31}, // Level 2 - Front-Right
    {33,27,32}, // Level 2 - Front-Left
    {26,28,30}, // Level 2 - Back-Left
    {35,34,29}, // Level 2 - Back-Right

    {17,16,18}, // Level 1 - Front-Right
    {14,21,23}, // Level 1 - Front-Left
    {22,13,12}, // Level 1 - Back-Left
    {20,19,15}, // Level 1 - Back-Right
};
uint8 stateTable[13];

/*******************************************************************************
* Define Interrupt service routine and allocate an vector to the Interrupt
********************************************************************************/
CY_ISR(InterruptHandler) {
    
    Driver1_GetInterruptSource();
    
    if (delay > 0) delay--;
    
    
    if (blinkTime > 0) {
        blinkTime--;
        if (blinkTime == 0) {
            Driver1_WriteCompare1(0);
            Driver1_WriteCompare2(0);
            Driver2_WriteCompare1(0);
            Driver2_WriteCompare2(0);
            Driver3_WriteCompare1(0);
            Driver3_WriteCompare2(0);
            Driver4_WriteCompare1(0);
            Driver4_WriteCompare2(0);
            Driver5_WriteCompare(0);
            Driver6_WriteCompare(0);
            Driver7_WriteCompare(0);
            Driver8_WriteCompare(0);
        }
    } else {
        blinkTime = 4;

        int value;
        int lightIndex;

        Source_Write(-1);
        
        if (activeRow == 0) {
            value = ~1;
            lightIndex = 0;
        }
        if (activeRow == 1) {
            value = ~2;
            lightIndex = 12;
        }
        if (activeRow == 2) {
            value = ~4;
            lightIndex = 24;
        }
        if (activeRow == 3) {
            value = ~8;
            lightIndex = 36;
        }
        
        Driver1_WriteCompare1(lights[lightIndex]);
        Driver1_WriteCompare2(lights[lightIndex+1]);
        Driver2_WriteCompare1(lights[lightIndex+2]);
        Driver2_WriteCompare2(lights[lightIndex+3]);
        Driver3_WriteCompare1(lights[lightIndex+4]);
        Driver3_WriteCompare2(lights[lightIndex+5]);
        Driver4_WriteCompare1(lights[lightIndex+6]);
        Driver4_WriteCompare2(lights[lightIndex+7]);
        Driver5_WriteCompare(lights[lightIndex+8]);
        Driver6_WriteCompare(lights[lightIndex+9]);
        Driver7_WriteCompare(lights[lightIndex+10]);
        Driver8_WriteCompare(lights[lightIndex+11]);

        Source_Write(value);

        activeRow++;
        if (activeRow > 3) {
            activeRow = 0;
        }
    }
}

void wait(int msDelay) {
    delay = msDelay * 2;
    while (delay != 0);
}

void setLight(uint8 index, uint8 red, uint8 green, uint8 blue) {
    lights[lightTable[index][0]] = red;
    lights[lightTable[index][1]] = green;
    lights[lightTable[index][2]] = blue;
}

void setRed(uint8 index, uint8 red) {
    lights[lightTable[index][0]] = red;
}

void setGreen(uint8 index, uint8 green) {
    lights[lightTable[index][1]] = green;
}

void setBlue(uint8 index, uint8 blue) {
    lights[lightTable[index][2]] = blue;
}

uint8 getRed(uint8 index) {
    return lights[lightTable[index][0]];
}

uint8 getGreen(uint8 index) {
    return lights[lightTable[index][1]];
}

uint8 getBlue(uint8 index) {
    return lights[lightTable[index][2]];
}

int main()
{   
    /* Enable the global interrupt */
    CyGlobalIntEnable;
    
    /* Enable the Interrupt component connected to interrupt */
    isr_lights_StartEx(InterruptHandler);

	/* Start the components */
    Driver1_Start();
    Driver2_Start();
    Driver3_Start();
    Driver4_Start();
    Driver5_Start();
    Driver6_Start();
    Driver7_Start();
    Driver8_Start();
    
    uint8 brightness;
    int repeats;
    uint8 light;
    while (1) {
        for (repeats = 0; repeats < 5; repeats++) {
            for (light = 0; light < 13; light++) {
                setLight(light, 0, 0, 255);
                wait(200);
            }
            for (light = 0; light < 13; light++) {
                setLight(light, 0, 255, 0);
                wait(200);
            }
            for (light = 0; light < 13; light++) {
                setLight(light, 255, 0, 0);
                wait(200);
            }
        }

        for (light = 0; light < 13; light++) stateTable[light] = 0;
        stateTable[0] = 1;
        setLight(0, 0, 0, 0);
        for (repeats = 0; repeats < 10000; repeats++) {
            for (light = 0; light < 13; light++) {
                switch (stateTable[light]) {
                    case 1: // Fade out blue, Fade in red
                        brightness = getBlue(light) - 1;
                        if (brightness < 255) setBlue(light, brightness);
                        brightness = getRed(light) + 1;
                        if (brightness == 0) {
                            stateTable[light]++;
                            if (light < 12 && stateTable[light+1] == 0) {
                                stateTable[light+1] = 1;
                                setLight(light + 1, 0, 0, 0);
                            }
                        } else {
                            setRed(light, brightness);
                        }
                        break;
                    case 2: // Fade out red / fade in green
                        brightness = getRed(light) - 1;
                        if (brightness < 255) setRed(light, brightness);
                        brightness = getGreen(light) + 1;
                        if (brightness == 0) {
                            stateTable[light]++;
                        } else {
                            setGreen(light, brightness);
                        }
                        break;
                    case 3: // Fade out green / fade in blue
                        brightness = getGreen(light) - 1;
                        if (brightness < 255) setGreen(light, brightness);
                        brightness = getBlue(light) + 1;
                        if (brightness == 0) {
                            stateTable[light] = 1;
                        } else {
                            setBlue(light, brightness);
                        }
                        break;
                }
            }
            wait(2);
        }

    }
    return 0;
}
