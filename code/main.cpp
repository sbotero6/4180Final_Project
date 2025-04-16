#include "mbed.h"
#include "uLCD_4DGL.h"
#include "rtos.h"
#include "TextLCD.h"

// Pin configuration
#define LED_CORRECT p21
#define LED_WRONG p22
#define ENTER_BUTTON p16
#define BUTTON1_PIN p19
#define BUTTON2_PIN p20
#define POTENTIOMETER_PIN p18
#define SERVO_PIN p24
#define BUZZER_PIN p25
#define TRIG_PIN p29
#define ECHO_PIN p30

#define RS p23
#define EN p26
#define D4 p27
#define D5 p28
#define D6 p17
#define D7 p15

const int CORRECT_SEQUENCE[] = {1, 2, 1, 1, 2};
const int SEQUENCE_LENGTH = 5;
const int TARGET_POT_VALUE = 20;
const float DETECTION_DISTANCE = 10.0;

uLCD_4DGL myLCD(p13, p14, p11);
AnalogIn potentiometer(POTENTIOMETER_PIN);
DigitalOut ledCorrect(LED_CORRECT);
DigitalOut ledWrong(LED_WRONG);
DigitalIn enterButton(ENTER_BUTTON, PullUp);
DigitalIn button1(BUTTON1_PIN, PullUp);
DigitalIn button2(BUTTON2_PIN, PullUp);
PwmOut servo(SERVO_PIN);
DigitalOut buzzer(BUZZER_PIN); 
DigitalOut trig(TRIG_PIN);
DigitalIn echo(ECHO_PIN);
Timer echoTimer;

TextLCD lcd(RS, EN, D4, D5, D6, D7, TextLCD::LCD16x2);

Thread depthSensorThread;
Thread inputThread;
Thread servoThread(osPriorityNormal, 2048);

Mutex lcdMutex;

//Needed Variables
float distance = 0.0;
bool sequenceCorrect = false;
int inputSequence[SEQUENCE_LENGTH] = {0};
int inputIndex = 0;
int wrongAttempts = 0;
bool correctCodeEntered = false;
int peopleCount = 0;
bool servoMoveFlag = false;
bool passVerified = false;
bool inVerificationMode = false;

//Needed methods
void checkPasscode();
void playBuzzer();
void resetToEnterPasscode();
void measureDistance();
void handleInput();
float getDistance();
void updateLCD();
void displayMessage(const char* msg, int color);
void servoMotorOperation();

int main() {
    // initializing the depth sensor, threading, servo
    depthSensorThread.start(measureDistance);
    inputThread.start(handleInput);
    servoThread.start(servoMotorOperation);

    servo.period_ms(20);
    servo.pulsewidth_ms(1);

    resetToEnterPasscode();
    updateLCD();
}

void measureDistance() {
    
    //It will triger when an object is a certian length away
    while (true) {
        distance = getDistance();
        if (distance <= DETECTION_DISTANCE && peopleCount > 0) {
            //Updates and sets flags
            peopleCount--;
            servoMoveFlag = true;
            updateLCD();
        }
        Thread::wait(500);
    }
}

float getDistance() {
    //This is for the depth sensor
    //It will triger when an object is a certian length away
    trig = 0;
    wait_us(2);
    trig = 1;
    wait_us(10);
    trig = 0;

    while (!echo.read());
    echoTimer.reset();
    echoTimer.start();
    while (echo.read());
    echoTimer.stop();

    float duration = echoTimer.read_us();
    return (duration * 0.0343) / 2;
}

void handleInput() {
    while (true) {
        //When we are in the second part of the passcode, this is true if both parts of the sequence is true
        if (inVerificationMode) {
            Thread::wait(100);
            continue;
        }

        bool buttonPressed = false;

    // All of the button if statements are for when the button passcode is entered
    // It updates the inputIndex/inputSequence
        if (button1 == 0) {
            inputSequence[inputIndex++] = 1;
            buttonPressed = true;
            Thread::wait(300);
        }
        if (button2 == 0) {
            inputSequence[inputIndex++] = 2;
            buttonPressed = true;
            Thread::wait(300);
        }

        if (buttonPressed) {
            lcdMutex.lock();
            myLCD.locate(0, 1);
            //Updates the screen with the entered passcode from the buttons
            myLCD.printf("Enter passcode:\n");
            for (int i = 0; i < inputIndex; i++) {
                myLCD.printf(" %d ", inputSequence[i]);
            }
            lcdMutex.unlock();
        }

        if (inputIndex >= SEQUENCE_LENGTH) {
            checkPasscode();
        }
        Thread::wait(100);
    }
}

void checkPasscode() {
    sequenceCorrect = true;
    //Checks if the sequence is the correct length and correct sequence
    for (int i = 0; i < SEQUENCE_LENGTH; i++) {
        if (inputSequence[i] != CORRECT_SEQUENCE[i]) {
            sequenceCorrect = false;
            break;
        }
    }

    if (sequenceCorrect) {
        lcdMutex.lock();
        myLCD.cls();
        myLCD.color(GREEN);
        myLCD.locate(1, 3);
        myLCD.printf("CORRECT");
        lcdMutex.unlock();

        inVerificationMode = true;

        while (true) {
            float potValue = potentiometer.read() * 50;
            myLCD.locate(1, 5);
            myLCD.printf("Pot: %d", (int)potValue);

            if (enterButton == 0) {
                //Checks if the potentiometer value is the same as the set one
                if (potValue >= TARGET_POT_VALUE - 2 && potValue <= TARGET_POT_VALUE + 2) {
                    //Setting flags
                    ledCorrect = 1;
                    servoMoveFlag = true;
                    passVerified = true;
                    peopleCount++;
                    updateLCD();
                    resetToEnterPasscode();
                    break;
                } else {
                    //Potentiometer section of the passcode was incorrect
                    displayMessage("WRONG", RED);
                    ledWrong = 1;
                    Thread::wait(50);
                    ledWrong = 0;
                    Thread::wait(50);
    
                    wrongAttempts++;
                    if (wrongAttempts == 3) {
                        playBuzzer();
                        wrongAttempts = 0;
                    }
                    //Resetting screen
                    resetToEnterPasscode();
                    break;
                }
            }
            Thread::wait(200);
        }
        //Reseting flags
        inVerificationMode = false;
        inputIndex = 0;
    } else {
        //Displays error messages on the screen
        displayMessage("WRONG", RED);
        
        ledWrong = 1;
        Thread::wait(50);
        ledWrong = 0;
        Thread::wait(50);
        
        wrongAttempts++;
        if (wrongAttempts == 3) {
            playBuzzer();
            wrongAttempts = 0;
        }
        //Resetting screen
        resetToEnterPasscode();
        inputIndex = 0;
    }
}

// Moves the servo to a certian distance
void servoMotorOperation() {
    while (true) {
        if (servoMoveFlag && passVerified) {
            displayMessage("UNLOCKED", GREEN);
            ledCorrect = 1;
            servo.pulsewidth_ms(2);
            Thread::wait(10000);
            servo.pulsewidth_ms(1);
            //Reseting flags
            ledCorrect = 0;
            passVerified = false;
            servoMoveFlag = false;
            resetToEnterPasscode();
        } else if (servoMoveFlag) {
            displayMessage("Door is Opening.", GREEN);
            servo.pulsewidth_ms(2);
            Thread::wait(10000);
            servo.pulsewidth_ms(1);
            //Reseting flags
            ledCorrect = 0;
            servoMoveFlag = false;
            resetToEnterPasscode();
        }
        Thread::wait(100);
    }
}

// This method allows the buzzer to be played when there are
// three consecuative wrong attempts
void playBuzzer() {
    displayMessage("Intruder", RED);
    ledWrong = 1;

    // BEEP LOOP for 5 seconds total
    for (int i = 0; i < 10; i++) {
        buzzer = 1;
        Thread::wait(250);
        buzzer = 0;
        Thread::wait(250);
    }

    //Reseting flags
    ledWrong = 0;
    passVerified = false;
    servoMoveFlag = false;
    sequenceCorrect = false;
    inputIndex = 0;
    inVerificationMode = false;
    resetToEnterPasscode();
}

//It is the "Enter Passcode" home screen
void resetToEnterPasscode() {
    lcdMutex.lock();
    myLCD.cls();
    myLCD.color(WHITE);
    myLCD.locate(0, 1);
    myLCD.printf("Enter passcode:\n");
    lcdMutex.unlock();
    inputIndex = 0;
}

// Updates the smaller lcd screen people count
void updateLCD() {
    lcdMutex.lock();
    lcd.cls();
    lcd.locate(0, 0);
    lcd.printf("People: %d", peopleCount);
    lcdMutex.unlock();
}

// This ensures messages are printed on the uLCD
void displayMessage(const char* msg, int color) {
    lcdMutex.lock();
    myLCD.cls();
    myLCD.color(color);
    myLCD.locate(1, 3);
    myLCD.printf("%s", msg);
    lcdMutex.unlock();
}
