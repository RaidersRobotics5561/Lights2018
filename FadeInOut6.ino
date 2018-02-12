#include "FadeInOut.h"
 
LED_Color V_LED_Color = LED_Color_Red;
int       V_Brightness = 0;                 // Value of how bright the LEDs are (0-255)
int       V_PulseSpeed = 0;                   // Counter that contains the number of pulses for the given color (really only useful for multi mode)
LED_Mode  V_LED_ModeFinal = LED_Mode0;      // The current sw loop, debounced, determined mode
LED_Mode  V_LED_ModeFinalPrev = LED_Mode1;  // The previous sw loop, debounced, determined mode
LED_Mode  V_LED_ModeRaw = LED_Mode0;        // The current sw loop, raw mode
float     V_ModeDebounceTime = 0;           // Deboune timer for the switching of modes
bool      V_Forward = true;                 // The direction of the fading action (when enabled).  True is fade up, false is fade down.
CRGB      V_LEDs[C_NUM_LEDS];               // Array that contains the color mixture for each led in the strip
int       V_ChanInputRaw[InputChanSz];      // Raw value read for the digital input channel
int       V_ChanInputFinal[InputChanSz];    // Final, debounced, on/off value of the input channels
float     V_ChanInputDbTime[InputChanSz];   // Deboune timer for each of the channels
int       V_LED_Rand[C_NUM_LEDS];           // Array of random numbers used in the rainbow mode
bool      V_LED_HalfOn[C_NUM_LEDS];         // Array of on/off bits used in the half on mode
float     V_LED_Shift = 0;                  // Timer used to determine when to shift the leds, used in shift mode
bool      V_Initialized = false;            //Gone through initialization


void setup() 
{
  
  int       L_Index = 0;
  LED_Color L_Color;
  int       L_RandNum = 0;
  
  int  L_LightSpacing = 0;
          
          for(L_Index = 0; L_Index < C_NUM_LEDS; L_Index++)
            {
              if (L_LightSpacing < K_LightSpacing[0])
                {
                  V_LED_HalfOn[L_Index] = false;
                  L_LightSpacing++;
                }
              else
                {
                  V_LED_HalfOn[L_Index] = true;
                  L_LightSpacing = 0;
                }
            } 
          for(L_Index = 0; L_Index < C_NUM_LEDS; L_Index++)
            {
              V_LED_Rand[L_Index] = random(0, int(LED_Color_Rainbow-1));
            } 
  delay (1000);
  FastLED.addLeds<C_CHIPSET, C_LED_PIN, C_COLOR_ORDER>(V_LEDs, C_NUM_LEDS).setCorrection (TypicalLEDStrip);
  FastLED.setBrightness(V_Brightness);
  pinMode(C_Chan0, INPUT);
  pinMode(C_Chan1, INPUT);
  pinMode(C_Chan2, INPUT);
  pinMode(C_Chan3, INPUT);
  
  for(L_Index = 0; L_Index < C_NUM_LEDS; L_Index++)
    {
      V_LED_HalfOn[L_Index] = true;
    }
}

void loop() 
{
  int  L_Index = 0;
  int  L_LastReg1 = 0;
  bool L_LastReg2 = false;
  int  L_LightSpacing = 0;

  // Read the chanels here:
  V_ChanInputRaw[InputChan0] = digitalRead(C_Chan0);
  V_ChanInputRaw[InputChan1] = digitalRead(C_Chan1);
  V_ChanInputRaw[InputChan2] = digitalRead(C_Chan2);
  V_ChanInputRaw[InputChan3] = digitalRead(C_Chan3);

  // Debounce the channels
  for (L_Index = 0; L_Index < InputChanSz; L_Index++)
    {
      V_ChanInputFinal[L_Index] = DebounceInput(V_ChanInputRaw[L_Index], &V_ChanInputDbTime[L_Index]);
    }

  // Let's determine what the raw mode is from the input lines
  for (L_Index = 0; L_Index < LED_ModeSz; L_Index++)
    {
    if (V_ChanInputFinal[InputChan0] == C_InputConfig[L_Index][InputChan0] &&
        V_ChanInputFinal[InputChan1] == C_InputConfig[L_Index][InputChan1] &&
        V_ChanInputFinal[InputChan2] == C_InputConfig[L_Index][InputChan2] &&   
        V_ChanInputFinal[InputChan2] == C_InputConfig[L_Index][InputChan3])
      {
        V_LED_ModeRaw = LED_Mode(L_Index);
        break;
      }
    }

  V_LED_ModeFinalPrev = V_LED_ModeFinal;
  // Let's debounce the mode:
  if (V_LED_ModeRaw != V_LED_ModeFinal)
    {
      if ( DebounceInput(true, &V_ModeDebounceTime) == true )
        {
          V_LED_ModeFinal = V_LED_ModeRaw;
        }
    }
  else
    {
      V_ModeDebounceTime = 0;
    }

  // There was a new mode, let's generate some random numbers just in case the user wants the rainbow effect
  if ((V_LED_ModeFinalPrev != V_LED_ModeFinal) || (V_Initialized == false))
    {
      for(L_Index = 0; L_Index < C_NUM_LEDS; L_Index++)
        {
          V_LED_HalfOn[L_Index] = true;
        }
        
      if (K_LightSettings[V_LED_ModeFinal][LED_ConfigHalfOn] >= 1)
        {
          L_LightSpacing = 0;
          
          for(L_Index = 0; L_Index < C_NUM_LEDS; L_Index++)
            {
              if (L_LightSpacing < K_LightSpacing[V_LED_ModeFinal])
                {
                  V_LED_HalfOn[L_Index] = false;
                  L_LightSpacing++;
                }
              else
                {
                  V_LED_HalfOn[L_Index] = true;
                  L_LightSpacing = 0;
                }
            } 
        }

      if (K_LightSettings[V_LED_ModeFinal][LED_ConfigColor] == LED_Color_Rainbow)
        {
          for(L_Index = 0; L_Index < C_NUM_LEDS; L_Index++)
            {
              V_LED_Rand[L_Index] = random(0, int(LED_Color_Rainbow-1));
            } 
        }
    }
  // Let's check to see if we are doing the "trace effect"...
  if (K_LightSettings[V_LED_ModeFinal][LED_ConfigTrace] >= 1)
    {
      V_LED_Shift += K_DelayBetweenUpdate;
        if (V_LED_Shift >= K_ShiftTime[V_LED_ModeFinal])
          {
            L_LastReg1 = V_LED_Rand[0];
            L_LastReg2 = V_LED_HalfOn[0];
            for (L_Index = 0; L_Index < (C_NUM_LEDS - 1); L_Index++)
              {
                V_LED_Rand[L_Index] = V_LED_Rand[L_Index+1];
                V_LED_HalfOn[L_Index] = V_LED_HalfOn[L_Index+1];
              }
            V_LED_Rand[C_NUM_LEDS-1] = L_LastReg1;
            V_LED_HalfOn[C_NUM_LEDS-1] = L_LastReg2;

            V_LED_Shift = 0;
          }
    }
  else
    {
      V_LED_Shift = 0;
    }
    
  // Let's determine what the actual desired color is... 
  if (K_LightSettings[V_LED_ModeFinal][LED_ConfigColor] < LED_Color_Rainbow)
    {
      V_LED_Color = K_LightSettings[V_LED_ModeFinal][LED_ConfigColor];
    }

  if (K_LightSettings[V_LED_ModeFinal][LED_ConfigFade] >= 1)
    {
      // Let's modify the brightness
      V_Brightness = SetLED_Brightness(V_Brightness, &V_Forward, K_LightSettings[V_LED_ModeFinal][LED_ConfigFade]);

      if (K_LightSettings[V_LED_ModeFinal][LED_ConfigColor] == LED_Color_Multi)
        {
          if (V_PulseSpeed >= K_PulseCounts)
            {
              //V_LED_Color = V_LED_Color + LED_Color(1);
              V_LED_Color = V_LED_Color + 1;
                            
              if (V_LED_Color >= LED_Color_Rainbow)
                {
                // This resets the color back to the starting point
                V_LED_Color = LED_Color_Red;
                }
              V_PulseSpeed = 0;
            }
        }
    }
  else
    {
      V_Brightness = K_LED_MaxBrightness;
    }

  if (K_LightSettings[V_LED_ModeFinal][LED_ConfigColor] == LED_Color_Rainbow)
    {
      SetLED_Color(LED_Color_Rainbow);
    }
  else
    {
      SetLED_Color(V_LED_Color);
    }

    V_Initialized = true;

    FastLED.show();
    FastLED.delay(int(K_DelayBetweenUpdate));
    FastLED.setBrightness(V_Brightness);
}

int SetLED_Brightness(int   L_Brightness,
                      bool *L_Forward,
                      int  L_PulseSpeed)
{
    if (*L_Forward == true && L_Brightness < K_LED_MaxBrightness) //in forward
      {
        L_Brightness+=L_PulseSpeed; //forward speed
      }
    else if (*L_Forward == false && L_Brightness > K_LED_MinBrightness) // in reverse
      {
        L_Brightness-=L_PulseSpeed; //reverse speed
      }
    else if (L_Brightness >= K_LED_MaxBrightness) //switch to reverse
      {
        *L_Forward = false;
      }
    else if (L_Brightness <= K_LED_MinBrightness) // switch to forward
      {
        //*L_PulseSpeed+=3;
        *L_Forward = true;
      }

      //Error proofing in case Brightness is out of range. put it back in range.
      if (L_Brightness <= K_LED_MinBrightness)
      {
        L_Brightness = K_LED_MinBrightness;
      }
      else if (L_Brightness >= K_LED_MaxBrightness)
      {
        L_Brightness = K_LED_MaxBrightness;
      }

  
 return L_Brightness;
}

bool DebounceInput(bool  L_RawInput,
                   float *L_DebounceTime)
{
  bool L_Debounced;
  
  if (L_RawInput == true && *L_DebounceTime < K_Debounce)
    {
      *L_DebounceTime += K_DelayBetweenUpdate;
      L_Debounced = false;
    }
  else if(L_RawInput == true && *L_DebounceTime >= K_Debounce)
    {
      L_Debounced = true;
    }
  else
    {
      *L_DebounceTime = 0;
      L_Debounced = false;
    }
    
  return L_Debounced;
}

void SetLED_Color(LED_Color L_Color)
{
  int       L_Index = 0;
  LED_Color L_MainC = L_Color;
  int       L_RandNum = 0;
  int x = 0;
  
  for( L_Index = 0; L_Index < C_NUM_LEDS; L_Index++)
    {
      if(L_MainC == LED_Color_Rainbow)
        {
          L_RandNum = V_LED_Rand[L_Index];
          L_Color = LED_Color(L_RandNum);
        
          if(L_Color >= LED_Color_Rainbow && L_Index < 0 && L_Index > 120)
            {
              L_Color = LED_Color_Red;
            }
          if(L_Color >= LED_Color_Rainbow && L_Index >= 25 && L_Index <= 45)
            {
              //L_Color = LED_Color_Red;
              L_Color = LED_Color_Black;
             
            }
        }
    
      if(L_MainC == LED_Color_Duo)
        {
          if(L_Color == LED_Color_Blue)
            {
              L_Color = LED_Color_Red;
            }
          else
            {
              L_Color = LED_Color_Blue;
            }
        }


      if(L_MainC == LED_Color_BlinkL)
        {
         
          if(L_Color == LED_Color_Yellow)
            {
              x++;
            }
          if( x == 240)
            {
              L_Color = LED_Color_Black;
             
            }
          else
            {
              L_Color = LED_Color_Yellow;
              x++;
            }
        }

      if(L_MainC == LED_Color_BlinkR)
          {
            if(L_Color == LED_Color_Black)
            {
              x++;
            }
            if(x == 240)
            {
              L_Color = LED_Color_Yellow;
            }
            else
            {
              L_Color = LED_Color_Black;
              x++;
            }
           }
          
      
      if (V_LED_HalfOn[L_Index] == false)
        {
          L_Color = LED_Color_Black;
        }
      else if ((L_MainC != LED_Color_Rainbow) && (L_MainC != LED_Color_Duo) && (L_MainC != LED_Color_BlinkR) && (L_MainC != LED_Color_BlinkL))
        {
          L_Color = L_MainC;
        }

      switch (L_Color)
        {
          case LED_Color_Red:
            V_LEDs[L_Index] = CRGB(0,0,100);
            break;

          case LED_Color_Blue:
            V_LEDs[L_Index] = CRGB(100,0,0);
            break;
          
          case LED_Color_Green:
            V_LEDs[L_Index] = CRGB(0,100,0);
            break;
      
          case LED_Color_White:
            V_LEDs[L_Index] = CRGB(100,100,100);           
            break;
          
          case LED_Color_Purple:
            V_LEDs[L_Index] = CRGB(85,10,40);
            break;
          
          case LED_Color_Yellow:
            V_LEDs[L_Index] = CRGB(0,75,100);
            break;
          
          case LED_Color_Pink:
            V_LEDs[L_Index] = CRGB(100,50,100);
            break;
          
          case LED_Color_Orange:
            V_LEDs[L_Index] = CRGB(0,30,100);
            break;

          case LED_Color_OrangeRed:
            V_LEDs[L_Index] = CRGB(0,10,100);
            break;

          case LED_Color_Aqua:
            V_LEDs[L_Index] = CRGB(10,90,0);
            break;

          

          case LED_Color_Black:
          default:
            V_LEDs[L_Index] = CRGB(0,0,0);
            break;
      }
    }
}
