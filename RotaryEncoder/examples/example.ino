// For Nano and Uno, you can use digital pins 2 and 3 for interrupts (CLK)

const int LED = 9;

// Hardware setup constants.
const int CLK = 2;    // CLocK.  Used as the source of the interrupt.
const int DT = 3;     // DaTa.  Poll this pin to find out if the rotation was clockwise, or anti-clockwise
const int SW = 4;     // SWitch.  Button.



int turn = 0;

// State recorder for single signal to program.
bool swState = false;

// 
// Marked as volatile so that it can be accessed from an interrupt
volatile int encoderValue = 0;
volatile int millisElapsed = 0;
volatile int debounceSpeedMS = 5;

volatile bool CLKv = 0;
volatile int DTv = -1;

volatile int brightnessDiff = 1;
volatile int brightness = 0;


void setup() {
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("Started...");

  pinMode(SW, OUTPUT);
  

  // Other values for MODE (last argument) include:
  // LOW to trigger the interrupt whenever the pin is low,
  // CHANGE to trigger the interrupt whenever the pin changes value
  // RISING to trigger when the pin goes from low to high,
  // FALLING for when the pin goes from high to low.
  // HIGH to trigger the interrupt whenever the pin is high (Due, Zero and MKR1000 ONLY)
  attachInterrupt(digitalPinToInterrupt(CLK), encoderMoved, CHANGE);

  
}




void loop() {
  // put your main code here, to run repeatedly:

  // Let's see if the switch is pressed:

  if(digitalRead(SW))
  {

    //TODO:
    // The switch seems to be able to bounce on release.
    // Make sure that the debounce, takes this into account by making sure that the button cannot be
    // pressed, n milliseconds after release.
    if(!swState)
    {
      //Serial.println("Pressed");
      encoderValue = 0;
      setBrightness(encoderValue);
      //Serial.println(encoderValue);
    }
    swState = true;
  }
  else
  {
    swState = false;
  }

}

void encoderMoved()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();


  // We have a couple of options to see if we're dealing with a bounce or actual data.
  //
  // The first one (and seems to be the most popular of the software debouncing) is to have a timeout within which we ignore
  // changes of state with n milliseconds of each other.
  // This may have repurcussions regarding slowing down the speed in which a user can input data as if they spin the dial too fast
  // then intentional input may be lost.
  //
  // Second idea would seem to be to activate on all interrupts and then follow the waveform to make sure that
  // something has actually happened.
  // Make sure that CLK and DT actually perform their moves next to each other.
  
  // In all cases, (clockwise and anti-clockwise) the CLK needs to move directly from low to high.
  // DT needs to change state during that transition.

  // keep a record of 

  // Each time we have a LOW input on CLK, treat this as the start of a new attempt at a rotation.
  // Record the DT and make sure it changes when the HIGH on CLK.

  // Get the value for CLK
  CLKv = digitalRead(CLK);
  // If it's LOW, then discard the previous DT state and record for the next interrupt
  if(!CLKv)
  {
    DTv = digitalRead(DT);    
  }
  else
  // If it's HIGH then check that the current DT state has changed from the recorded one.
  {

    // Debounce at this stage.
    if (interrupt_time - last_interrupt_time >= debounceSpeedMS) 
    {
    
      // Check against an initial (pre-rotation) value so that we don't have an initial jump
      if(DTv != -1 && digitalRead(DT) != DTv)
      {
        // we have a changed value of DT, we can now figure out if it was clockwise or Anti-Clockwise.
        // A clockwise signal will go HIGH, an anticlockwise signal will move low
        // We're using the previous reading here so that we don't create another variable so the logic is inverted
        if(!DTv)
        {
          // Anti Clockwise
          encoderValue--;
          Serial.println("anticlockwise");
          
        }
        else
        {
          // Clockwise
          encoderValue++;
          Serial.println("clockwise");
        }

        Serial.println(encoderValue);
        setBrightness(encoderValue * brightnessDiff);

      }
      else
      {
        
        Serial.println("Debounced");
      }
      
    }
    else
    {
      // don't do anything as although the clock has fired the DT has not changed.
    }
  }
  last_interrupt_time = interrupt_time;
}


/*
void changeBrightness(int diff)
{ 
  brightness = checkBrightness(brightness += diff);
  updateBrightness(brightness);
}
*/

void setBrightness(int value)
{
  brightness = checkBrightness(value);
  updateBrightness(brightness);
}

void updateBrightness(int value)
{
  //Serial.print("changing output to: ");
  //Serial.println(value);
  analogWrite(LED, 255-value);
}

int checkBrightness(int brightness)
{
  if(brightness > 255)
  {
    //Serial.println("Reducing brightness to 255");
    brightness = 255;
  }
  if(brightness < 0)
  {
    //Serial.println("Increasing brightness to 0");
    brightness = 0;
  }
  return brightness;
}

