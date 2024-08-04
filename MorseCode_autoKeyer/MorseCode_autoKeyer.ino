#include <stdlib.h>
#include <Arduino.h>
#include <stdio.h>


#define DIT_PIN 7
#define DAH_PIN 8

#define buttonPin 3
#define buttonPin2 4

#define SPACE_UNIT 50                   //        15 wpm = ?     ...     20wpm = 50     =====    Need to make a function to calculate words per minute formula returning 1 dit unit size
#define TOLERANCE_MODIFIER 1.3          //        Calibrates for tolerance difference between arduino clock and radio wpm clock
#define DAH_MODIFIER 100

#define debounceTime 50

char* messageIn = "Hey there pal what is your name";
char testMessage[] = ".-- -.... .-. -..- -";

char Morse[26][5] = {".-","-...","-.-.","-..",".","..-.","--.","....","..",
                        ".---","-.-",".-..","--","-.","---",".--.","--.-",".-.",
                        "...","-","..-","...-",".--","-..-","-.--","--.."};

char charBuffer[200];

int buttonState = 0;  // variable for reading the pushbutton status
int buttonState2 = 0;

const float intraCharacter = 1 * (SPACE_UNIT * TOLERANCE_MODIFIER); // gap between dits and dahs of a character
const float interCharacter = 3 * (SPACE_UNIT * TOLERANCE_MODIFIER); // gap between characters of a word
const float wordSpace = 7 * (SPACE_UNIT * TOLERANCE_MODIFIER); // gap between two words

int repeatCode = 3;


// -------------------------------------------- My problem is that I'm creating more characters than the length of the original ascii message when encoding morse message 'b'(size of 1) = '-...'(size of 4)

//unsigned long nextReadMillis = 0;

// Just a test function to ensure the radio wpm speed is in sync with arduino 
void autoSOS(int repeatCode)
{
  for (int i = 0; i < repeatCode; i++)
  {
    digitalWrite(DIT_PIN, HIGH);
    delay(intraCharacter * (5 * TOLERANCE_MODIFIER)) ; // 5 units including space
    digitalWrite(DIT_PIN,LOW);
    delay(interCharacter);

    digitalWrite(DAH_PIN, HIGH);
    delay(intraCharacter * (11 * TOLERANCE_MODIFIER)); // 11 units including space
    digitalWrite(DAH_PIN, LOW);
    delay(interCharacter);

    digitalWrite(DIT_PIN, HIGH);
    delay(intraCharacter * (5 * TOLERANCE_MODIFIER)); // 5 units including space
    digitalWrite(DIT_PIN,LOW);
    delay(wordSpace * 5);
  }
}


// Not necessary at the moment but perhaps soon...
int wordCounter(char *message)
{
  int y = 0, x;

  for (x = 0;message[x] != '\0';x++)
  {
      if (message[x] == ' ' && message[x+1] != ' ')
          y++;    
  }

  return y;
}


// Take user provided message from uart(currently just an initialized global) and convert it to "dits" and "dahs"
char* encodeMessage(char* messageIn)
{
  Serial.println(" \n");
  int str = strlen(messageIn);
  int charBufferCounter = 0;
  int convertingCharCounter = 0;

  Serial.print("\nMessage to convert: \t\t\t");
  Serial.println(messageIn);
  Serial.print("Original message length: \t\t");
  Serial.println(str);
  Serial.println("\n");

  for (int i = 0; i < str; i++)
  {
      if (str == convertingCharCounter)
      {
        Serial.println("DEBUG:   They are the same length, so break");
        break;
      }
      if ((messageIn[i] > 96) && (messageIn[i] < 123))
      {
        int j = messageIn[i] - 97; 
        Serial.print(Morse[j]);
        Serial.print(' ');
        for (int n = 0; n < 5; n++)
        {
          if (Morse[j][n] == ' ' || Morse[j][n] == '\0')
          {
            break;
          }          
          charBuffer[charBufferCounter] = Morse[j][n];
          charBufferCounter++;
        }
        charBuffer[charBufferCounter] = ' ';
        charBufferCounter++;
      }
      else if((messageIn[i] > 64) && (messageIn[i] < 91))
      {
        int k = messageIn[i] - 65;
        Serial.print(Morse[k]);
        Serial.print(' ');
        for (int n = 0; n < 5; n++)
        {
          if (Morse[k][n] == ' ' || Morse[k][n] == '\0')
          {
            break;
          }
          charBuffer[charBufferCounter] = Morse[k][n];
          charBufferCounter++;
        }
        charBuffer[charBufferCounter] = ' ';
        charBufferCounter++;
      }
      else if (messageIn[i] == '\0')
      {
        break;
      }
      else
      {
        Serial.print(messageIn[i]);
        Serial.print(' ');
        // Adds extra 2 spaces to interword spacing to meet morse code standards defined above
        charBuffer[charBufferCounter] = ' ';
        charBufferCounter++;
        charBuffer[charBufferCounter] = ' ';
        charBufferCounter++;
      }
      convertingCharCounter++;
      charBuffer[charBufferCounter] = '\0';
  }

  Serial.println("\n\nConverted Message to be sent: ");
  Serial.println(charBuffer);
  Serial.println("\n");
  Serial.print("Length of charBuffer is: \t");
  Serial.println(strlen(charBuffer));
  Serial.print("charBufferCounter: \t\t");
  Serial.print(charBufferCounter);
  Serial.println("\n");
  
  return charBuffer;
}


void sendUserMessage(char* message)
{
  // take already encoded message ( "Hey There" -> ".... . .._.   _ .... . ._. ." ) then calculate length of dits and dahs per WPM 
  int totalCharLength = strlen(message);
  Serial.print("The actual length of the message is: \t\t");
  Serial.println(totalCharLength);
  Serial.println("\n\n");

  //int wordCounter = wordCounter(message);
  int wordCharCounter = 0;

  for (int i = 0; i < totalCharLength; i++)
  {
    wordCharCounter++;
    
    switch(message[i])
    {
      case '.':
        digitalWrite(DIT_PIN, HIGH);
        Serial.println("DIT");
        delay(intraCharacter);
        digitalWrite(DIT_PIN, LOW);
        break;
      

      case '-':
        digitalWrite(DAH_PIN, HIGH);
        Serial.println("DAH");
        delay(intraCharacter);
        digitalWrite(DAH_PIN, LOW);
        delay(DAH_MODIFIER);
        break;

      case ' ':
      
        digitalWrite(DIT_PIN, LOW);
        digitalWrite(DAH_PIN, LOW);
        Serial.println("1x Space Unit");
        delay(interCharacter);
        //wordCounter++;
        break;
      case '\0':
        Serial.println("That's all folks");
        break;
      default:
        digitalWrite(DIT_PIN, LOW);
        digitalWrite(DAH_PIN, LOW);
        Serial.println("Something is seriously wrong");
        delay(5000);
    }

    Serial.println(wordCharCounter);
    delay(75);
    
  }
}


/* =======================================================================

----------------------- SETUP --------------------------------------------

======================================================================= */

void setup()
{

  pinMode(buttonPin, INPUT);
  pinMode(buttonPin2, INPUT);

  pinMode(DIT_PIN, OUTPUT);
  pinMode(DAH_PIN, OUTPUT);

  pinMode(DIT_PIN, LOW);
  pinMode(DAH_PIN, LOW);

  Serial.begin(9600);
 
}

/* =======================================================================

------------------------ LOOP --------------------------------------------

======================================================================= */

void loop()
{

  // autoSOS(2);

  encodeMessage(messageIn);

  Serial.println("Here is the encoded Message to now pass to sendUserMessage function: ");
  Serial.print(charBuffer);
  Serial.println("\n");

  sendUserMessage(charBuffer);
  
  delay(5000);


}


// char* encodeMessage(char* messageIn)
// {
//   Serial.println(" \n");
//   int str = strlen(messageIn);
//   char charBuffer[200];
//   char *sentMessage = malloc((str * 5) * sizeof(char));;

//   for (int i = 0; i < str; i++)
//   {
//     for (int n = 0; n < 5; n++)
//     {
//       if (messageIn[i] == '\0')
//       {
//         Serial.println("\n");
//         break;
//       }
//       else
//       {
//         if ((messageIn[i] > 96) && (messageIn[i] < 123))
//         {
//             int j = messageIn[i] - 97; 
//             Serial.print(Morse[j]);
//             sentMessage[i] = Morse[j];
//         }
//         else if((messageIn[i] > 64) && (messageIn[i] < 91))
//         {
//             int k = messageIn[i] - 65;
//             Serial.print(Morse[k]);
//             sentMessage[i] = Morse[k];
//         }
//         else
//         {
//             Serial.print(messageIn[i]);
//             sentMessage[i] = Morse[i];
//         }
//       }
//       if (messageIn[i] == ' ')
//       {
//         Serial.print(" ");
//         // if (messageIn[i+1] == ' ')
//         // {
//         //   messageIn[i+1] = '\0';
//         // }
//       }
//     }
    
//   } 
//   //messageIn[str + 1] = '\0';
//   Serial.println("\n\nConverted Message to be sent: ");
//   Serial.print(sentMessage);
//   Serial.println("\n");
  
//   return sentMessage;

