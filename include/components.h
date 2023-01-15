class Component {
    protected:
        int pin;

    public:
        // Constructor
        Component(int Pin){
            pin = Pin;
        }

        int getPin(){
            return pin;
        }
};

class BaseRequireData {
    protected:
        bool* stageModeptr;
        int* stagesptr;
        int* peopleCounterptr;

    public:
    // Constructor
    BaseRequireData(bool* StageModeptr, int* Stagesptr, int* PeopleCounterptr){
        stageModeptr = StageModeptr;
        stagesptr = Stagesptr;
        peopleCounterptr = PeopleCounterptr;
    }
};

class PIR: public Component {
    private:
        long long int awaitTime;
        long long int activatedTime;

    public:
        // Constructor
        PIR(int Pin, int AwaitTime)
            :Component(Pin){
                awaitTime = AwaitTime;
            }

        bool awaiting = false;

        bool isActivated(){
            if(digitalRead(pin) == HIGH){
                activatedTime = millis();
                return true;
            }
            else return false;
        }

        bool isOverAwait(){
            if(millis() - activatedTime > awaitTime){
                awaiting = false;
                return true;
            }
            else return false;
        } 
};

class LED: public Component, public BaseRequireData {
    private:
        bool allowStageModeToggle;

    public:
        bool isOn = false;

        // Constructor (Allow Stage Mode)
        LED(int Pin, bool* StageModeptr, int* Stagesptr, int* PeopleCounterptr)
            :Component(Pin), BaseRequireData(StageModeptr, Stagesptr, PeopleCounterptr){
                allowStageModeToggle = true;
            }

        // Constructor (ON/OFF Only)
        LED(int Pin)
            :Component(Pin), BaseRequireData(NULL, NULL, NULL){
                allowStageModeToggle = false;
            }

        void toggle(){
            if(allowStageModeToggle){
                if(*peopleCounterptr > *stagesptr) return;

                if(*stageModeptr){
                    int value = map(*peopleCounterptr, 0, *stagesptr, 0, 255);
                    analogWrite(pin, value);
                    
                    if(*peopleCounterptr == 0) isOn = false;
                    else isOn = true;
                }
                else {
                    if(*peopleCounterptr == 0){
                        analogWrite(pin, 0);
                        isOn = false;
                    }
                    else{
                        analogWrite(pin, 255);
                        isOn = true;
                    } 
                }
            }
            // if no stage mode toggle
            else {
                // if lamp is on
                if(isOn){
                    // turn off lamp
                    analogWrite(pin, 0);
                    isOn = false;
                }
                // if lamp is off 
                else {
                    // turn on lamp
                    analogWrite(pin, 255);
                    isOn = true;
                }
            }
        }

        void keepHigh(){
            analogWrite(pin, 255);
            isOn = true;
        }
};

class LcdProfile {
    protected:
        LiquidCrystal* lcdDisplay;

    public:
        // Constructor
        LcdProfile(LiquidCrystal *LcdDisplay){
            lcdDisplay = LcdDisplay;
        };

        // Derived class need to override
        virtual void initial() = 0;
};

class MainProfile: LcdProfile, BaseRequireData {
    private:
        long long* lastMillisReadptr;
    public:
        // Constructor
        MainProfile(LiquidCrystal* LcdDisplayptr, bool* StageModeptr, int* Stagesptr, int* PeopleCounterptr, long long* LastMillisReadptr)
            :LcdProfile(LcdDisplayptr), BaseRequireData(StageModeptr, Stagesptr, PeopleCounterptr){
                lastMillisReadptr = LastMillisReadptr;
            }

        // initialize main display in lcd
        void initial(){
            char intBuffer[4]; // e.g intBuffer[4] = "003\0"
            sprintf(intBuffer, "%03d", *peopleCounterptr);

            // prints
            // row 0 "PEOPLE:003 MM:SS"
            // row 1 "STAGE MODE:ON(3)"
            lcdDisplay->clear();
            lcdDisplay->setCursor(0,0);
            lcdDisplay->print("People:"); lcdDisplay->print(intBuffer);
            lcdDisplay->setCursor(0,1);
            lcdDisplay->print("Stage Mode:");
            if(*stageModeptr){
                lcdDisplay->print("ON(");
                lcdDisplay->print(*stagesptr);
                lcdDisplay->print(")");
            }
            else lcdDisplay->print("OFF");
        }

        // update people count in lcd
        void updatePeople(){
            char intBuffer[4]; // e.g intBuffer[4] = "012\0"
            sprintf(intBuffer, "%03d", *peopleCounterptr);

            // PEOPLE:003 MM:SS   -> Row 0 of LCD
            //        ^           -> Set cursor to column 7
            lcdDisplay->setCursor(7,0);
            lcdDisplay->print(intBuffer);
        }

        // update stage mode in lcd
        void updateMode(){
            char stringBuffer[6]; // e.g stringBuffer[6] = "ON(3)\0"

            // STAGE MODE:ON(3)   -> Row 1 of LCD
            //            ^       -> Set cursor to column 11
            lcdDisplay->setCursor(11,1);
                
            if(*stageModeptr) sprintf(stringBuffer, "ON(%d)", *stagesptr);
            else sprintf(stringBuffer, "OFF  ");
            
            lcdDisplay->print(stringBuffer);
        }

        // updates idle time
        void updateIdleTime(){
            lcdDisplay->setCursor(11,0);

            if(*peopleCounterptr == 0){
                lcdDisplay->print("     ");
                return;
            }

            long long int secondPassed = (millis() - *lastMillisReadptr)/1000;
            char timeStrBuffer[5];
            
            int minutes = secondPassed / 60; // round down to closes minute
            int seconds = secondPassed % 60; // left over seconds
            sprintf(timeStrBuffer,"%02d:%02d", minutes, seconds);

            // PEOPLE:003 MM:SS   -> Row 0 of LCD
            //            ^       -> Set cursor to column 11
            lcdDisplay->print(timeStrBuffer);
        }
};

class ConfigureProfile: public LcdProfile {
    private:
        int* stagesptr;
    public:
        // Constructor
        ConfigureProfile(LiquidCrystal* LcdDisplay, int* Stagesptr)
            :LcdProfile(LcdDisplay){
                stagesptr = Stagesptr;
            }
        
        // Initialize Page
        void initial(){
            // prints
            // row -> 0
            lcdDisplay->clear();
            lcdDisplay->print("---Set  Stage---");
            lcdDisplay->setCursor(0,1);
            lcdDisplay->print("Stage: ");
            lcdDisplay->print(*stagesptr);
        }

        // update stage mode
        void updateStage(){
            // prints
            // row 0 "---Set  Stage---"
            // row 1 "Stages: 3       "
            // cursor         ^
            lcdDisplay->setCursor(7,1);
            lcdDisplay->print(*stagesptr);
        }
};