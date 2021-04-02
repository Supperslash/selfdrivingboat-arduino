//          h-bridge by madderdash & http_lessand_ro
//             04-01-2021
//      in serial monitor press keys 1,2,3,4,5, to conroll the motors.

class HBridgeMaderController {
  private:
    int pinArray[4] = {     // The pins used as output
      13,                 // motor 1 pin A
      12,                 // motor 1 pin B
      14,                  // motor 2 pin A
      27                   // motor 2 pin B
    };
    int direction[5][4] = {
      { 1, 0, 1, 0 },     // Forward =0
      { 0, 1, 0, 1 },     // Backwords =1
      { 0, 0, 0, 0 },     // Standby =2
      { 1, 0, 0, 0 },     // turn left =3
      { 0, 0, 1, 0 }      // Turn right = 4
    };
    int pinCount = 4;       // Pins uses in array
    
    
  public:
    HBridgeMaderController() {
      for (int count = 0; count <= pinCount; count++)
      {
          pinMode(this->pinArray[count], OUTPUT);
      }
    }

    void drive(int x) { // Driveing the pins off of the input of x.
        for (int i = 0; i <= 4; i++)
        {
            if (direction[x][i] == 1)
            {
                digitalWrite(pinArray[i], HIGH);
            }
            else
            {
                digitalWrite(pinArray[i], LOW);
            }
        }
    }
};
  
   
HBridgeMaderController motor_controller;

void setup(){ Serial.begin(115200); }

int captureSerial = 0;
int state = 5;          // Starts motors in standby mode.

void loop()
{

    if (Serial.available() > 0)        // gets input from serial
    {

        captureSerial = Serial.read(); // read the incoming byte:

        Serial.print(" I received:");  // Debug prints what is received on the serial terminal.

        Serial.println(captureSerial); // Debug prints what is received on the serial terminal.
        // This simply directs the pinout as to what needs to be driven, and reversed.

        switch(captureSerial){
          case 49: state = 0; break;
          case 50: state = 1; break;
          case 51: state = 3; break;
          case 52: state = 4; break;
          case 53: state = 2; break;
        }

        motor_controller.drive(state);
    }
}

