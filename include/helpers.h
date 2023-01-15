/// @brief delay timer to calibrate sensor
/// @param time in seconds
/// @param led instance of led to use as calibrating indicator
/// @param lcd instance of lcd to print into
/// REFERENCE: https://stackoverflow.com/questions/14539867/how-to-display-a-progress-indicator-in-pure-c-c-cout-printf

void calibrateSensor(int time, LED& led, LiquidCrystal& lcd){
    char stringBuffer[20];
    int position, lcdPosition;
    int barWidth = 70;
    int lcdBarWidth = 14;

    // turn on indicator led
    led.toggle();
    
    Serial.println("Calibrating Sensor");

    // print to lcd
    lcd.print("Calibrating ...");
    lcd.setCursor(0,1);
    lcd.print("[");
    lcd.setCursor(15,1);
    lcd.print("]");
    lcd.setCursor(1,1);

    int lcdCurrentPosition = 1;

    for(int i = 1; i < time+1; i++){
        // calculate width of current loading bar                            
        position = ((float)i/time) * barWidth;
        lcdPosition = ((float)i/time) * lcdBarWidth;
        
        // prints e.g "[=====     ]" to LCD
        if(lcdPosition == lcdCurrentPosition){
            lcd.print("=");
            lcdCurrentPosition++;
        }

        // prints e.g "[=====>     "
        Serial.print("[");                                                        
        for(int u = 0; u < barWidth; u++){                      
            if(u < position) Serial.print("=");
            else if(u == position) Serial.print(">");
            else Serial.print(" ");
        }

        // closes progress bar e.g "[=======>   ]" 
        Serial.print("]");

        // complete progress bar  e.g "[=======>   ] (7/60) seconds"
        sprintf(stringBuffer, " (%d/%d) seconds", i, time);
        Serial.print(stringBuffer);
        Serial.println();

        // delay by exactly 1 second
        delay(1000);
    }
    
    // turn off indicator led
    led.toggle();
    
    // sensor is ready
    Serial.println("Sensor is ready.");
    lcd.clear();
    lcd.print(" Sensor ready. ");
    delay(3000);
    lcd.clear();
}

/// @brief check time between lamp activation and last activity
/// @param lastMillisRead timestamp of last millis read() in milliseconds
/// @param MaxIdleTime maximum time before returning true in milliseconds
/// @return true if time is over designated idle time
bool isOverIdleTime(long long lastMillisRead, long long MaxIdleTime){
    if(millis() - lastMillisRead > MaxIdleTime) return true;
    else return false;
}