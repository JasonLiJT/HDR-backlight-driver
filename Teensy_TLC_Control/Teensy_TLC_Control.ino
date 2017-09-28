// TLC5955 control program for Teensy 3.2
// Junteng Li, jl944@cam.ac.uk
// Updated 01/09/2017

#include <SPI.h>
#include <TLC5955.h>

TLC5955 tlc;

#define GSCLK 10  // On Teensy 3.2
#define LAT 9     // On Teensy 3.2

// Spi pins are needed to send out control bit (SPI only supports bytes)
#define SPI_MOSI 11  // 51 on mega, 22 on teensy2.0++
#define SPI_CLK 13   // 52 on mega, 21 on teensy2.0++

#define SPI_DIN 12  //

#define GSCLK_FREQUENCY 2 * 60 * 65535  // Multiple of FPS * brightness PWM resolution

// Unused pins that are connected to other pins to simplify the PCB layout
const int passive_pins[] = {2, 3, 4, 5, 16, 20, 21, 22};

elapsedMicros timer_0;  // automatically incremented, must be global

void serial_control();
void PWM_control(int mDelay = 10, int led1 = 4, int led2 = 8 + LEDS_PER_CHIP);  // Default configurations for testing
int getSerialInt();
void testing_program();
void receiveFrameUpdate();

void setup() {
    // USB is always 12 Mbit/sec for Teensy
    Serial.begin(9600);

    // Set the unused Teensy pins that are connected to other signals to input
    Serial.println("Set the following unused Teensy pins that are connected to other signals to input:");
    for (unsigned int i = 0; i < sizeof(passive_pins) / sizeof(passive_pins[0]); i++) {
        Serial.print(passive_pins[i]);
        Serial.print('\t');
        pinMode(passive_pins[i], INPUT);
    }
    Serial.print('\n');

    // Now set the GSCKGRB to an output and a 50% PWM duty-cycle
    // For simplicity all three grayscale clocks are tied to the same pin
    pinMode(GSCLK, OUTPUT);
    pinMode(LAT, OUTPUT);

    // Adjust PWM timer (Specific to each microcontroller)
    //TCCR2B = TCCR2B & 0b11111000 | 0x01;

    // Set up clock pulse
    // Check https://www.pjrc.com/teensy/td_pulse.html
    // Higher frequency results in low resolution, but it doesn't mater for a clock signal
    analogWriteFrequency(GSCLK, GSCLK_FREQUENCY);
    analogWrite(GSCLK, 127);

    // The library does not ininiate SPI for you, so as to prevent issues with other SPI libraries
    SPI.begin();

    // init(GSLAT pin, XBLNK pin, default grayscale values for all LEDS)
    tlc.init(LAT, SPI_MOSI, SPI_CLK);

    // We must set dot correction values, so set them all to the brightest adjustment
    tlc.setAllDcData(127);

    // Perform Dot Correction (DC) here
    // Dot Correction controls the maximum current of each OUT pin
    // to compensate the difference between individual LEDs
    // Please consult the data sheet
    // tlc.setLedDc(1, 13, 2, 0);

    // Set Max Current Values (see TLC5955 datasheet)
    tlc.setMaxCurrent(4, 4, 4);  // Maximum 2.75A
    // Change to (5, 5, 5) for a maximum current of 3.44A

    // Set Function Control Data Latch values. See the TLC5955 Datasheet for the purpose of this latch.
    // The five bits are:
    // Auto display repeat mode enable bit, set to 1 to auto-repeat if no data for the next frame arrives
    // Display timing reset mode enable bit, set to 1 to synchronize signal and display clock
    // Auto data refresh mode enable bit, refer to data sheet p.36
    // ES-PWM mode enable bit, must be set to 1 unless multiplexing mode is to be used
    // LSD detection voltage selection bit, not currently implemented
    tlc.setFunctionData(false, true, true, true, true);  // WORKS with fast update
    //  tlc.setFunctionData(true, false, false, true, true);   // WORKS generally

    // set all brightness levels to max (127)
    int currentVal = 127;
    tlc.setBrightnessCurrent(currentVal, currentVal, currentVal);

    // Update Control Register
    tlc.updateControl();

    // Update the GS register (ideally LEDs should be dark up to here)
    tlc.setAllLed(0);
    tlc.updateLeds();
}

void loop() {
    receiveFrameUpdate();
}

void testing_program() {
    PWM_control();

    // Use serial input to control the behaviour of the drivers
    // -    Enter a number in [0, 65535] enter brightness control mode,
    //      -   repeat entering numbers to change brightness
    //      -   enter '\\' to exit.
    // -    Enter '\\' directly to enter PWM_control mode,
    //      -   enter a number to set mDelay of PWM_control(),
    //      -   enter a single quote ' to set all 3 arguments mDelay, led1 & led2
    //      -   enter '\\' again to exit.
    if (Serial.available()) {
        if (Serial.peek() == '\\') {
            Serial.read();
            while (true) {
                if (Serial.peek() == '\'') {
                    // Let \' mark input of all 3 arguments
                    Serial.read();
                    int mDelay = getSerialInt(), led1 = getSerialInt(), led2 = getSerialInt();
                    PWM_control(mDelay, led1, led2);
                } else {
                    PWM_control(getSerialInt());
                }
                while (!Serial.available())
                    ;
                if (Serial.peek() == '\\') {
                    Serial.read();
                    break;
                }
            }
        } else {
            while (true) {
                serial_control();
                while (!Serial.available())
                    ;
                if (Serial.peek() == '\\') {
                    Serial.read();
                    break;
                }
            }
        }
    }
}

void serial_control() {
    //Serial control of global brightness in the serial monitor.
    // Enter a number in [0, 65535]
    static uint16_t bright = 0;
    bright = getSerialInt();
    Serial.println(bright);
    tlc.setAllLed(bright);
    tlc.updateLeds();
}

void PWM_control(int mDelay, int led1, int led2) {
    // Default is int mDelay = 10, int led1 = 4, int led2 = 8 + LEDS_PER_CHIP
    tlc.setAllLed(0);

    int dimMax = 65535;
    int dimDelta = 512;
    // PWM Dimming
    for (int dimVal = 0; dimVal <= dimMax; dimVal += dimDelta) {
        delay(mDelay);
        // tlc.setAllLed((uint16_t)dimVal);
        tlc.setLed(led1, dimVal, 0, dimVal);
        tlc.updateLeds();
        // Serial.println(dimVal);
    }

    tlc.setAllLed(0);
    tlc.updateLeds();

    for (int dimVal = dimMax; dimVal >= 0; dimVal -= dimDelta) {
        delay(mDelay);
        // tlc.setAllLed((uint16_t)dimVal);
        tlc.setLed(led2, 0, dimVal, dimVal);
        tlc.updateLeds();
        // Serial.println(dimVal);
    }
    Serial.print(timer_0);
    Serial.print("\tμs has passed for\t");
    Serial.print((dimMax + 1) / dimDelta);
    Serial.print("\tupdates\n");
    timer_0 = 0;
}

int getSerialInt() {
    while (!Serial.available())
        ;  // Wait for serial input

    int i = 0;
    while (Serial.available()) {
        if (Serial.peek() < '0' || Serial.peek() > '9') {
            Serial.read();  //pop that invalid byte from stream
            break;
        }
        i *= 10;
        i += Serial.read() - '0';
    }
    return i;
}

void receiveFrameUpdate() {
    while (1) {
        // Detect the start of update
        int a, b;
        while (!Serial.available())
            ;
        a = Serial.read();  // Read the first byte
        while (!Serial.available())
            ;
        b = Serial.peek();  // Peek the second byte
        if (a == 'G' && b == 'O') {
            // The start of the update
            // Pop the second byte from the stream
            Serial.read();
            break;
        }
        if (a == 'R' && b == 'T') {
            // Just connected
            // Need to reboot to boost serial speed for some reason

            // Write the value for restart to the Application Interrupt and Reset Control location
            (*(volatile uint32_t *)0xE000ED0C) = 0x05FA0004;
            // _reboot_Teensyduino_();  // Much slower
        }
    }

    uint16_t bright;
    int high_byte, low_byte;  // May be -1 if not available
    for (int i = 0; i < TLC_COUNT; i++) {
        for (int j = 0; j < LEDS_PER_CHIP; j++) {
            for (int k = 0; k < COLOR_CHANNEL_COUNT; k++) {
                while (!Serial.available())
                    ;
                high_byte = Serial.read();  // Receive the higher byte first
                while (!Serial.available())
                    ;
                low_byte = Serial.read();  // Then the lower byte
                bright = ((uint16_t)high_byte) << 8;
                bright |= (uint16_t)(low_byte & 0x00FF);
                tlc.setLEDpin(i, j, k, bright);
            }
        }
    }

    // Optional, for driver chips' testing.
    // Because the drivers are not well connected at startup, i.e. void setup(),
    // Teensy needs to make sure the control bits are configured after connection
    // It will lower the frame rate significantly
    // tlc.updateControl();

    // tlc.updateLeds();
    // This will upload the data and immediately update LEDs
    // The latency is not guaranteed

    // For synchronization with LCD screen, use the no_latch version
    // The data is uploaded, but the LEDs won't be updated until latch() is called
    tlc.updateLeds_no_latch();
    // Wait for synchronization signal from LCD screen (currently not implemented)
    tlc.latch();
    // Refer to the data sheet for timing diagrams

    // Feedback: done
    Serial.write('D');
    Serial.write('N');
}
