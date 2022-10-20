#include <LiquidCrystal.h>
#include <Wire.h>

#define SLAVE_ADD 4                                     // defining the address of the slave Arduino

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);


String coinPhotoresistorPin = "A0";

bool coinRolling = false;   // this is a boolean ensuring that while the photoresistor does not get any light, credit will only be added once, not continuously.  
                            // it is set to true when when the photoresistor reading drops below 300 and set back to false when it goes abouve this value
                            
int lightReading;

int credit = 0;             // amount of credit / coins the user inputs dictates the number of drinks the user can make 

byte ratios[3] = {0, 0, 0}; // intitialising all the drink ratios in a byte array so they can each be transmittied in one transmission over to the slave Arduino      

int upButton = 5;
int selectButton = 6;
int downButton = 7;

int debounceTime = 300;     // variable used to add a delay between button presses, making the menu easier to navigate
unsigned long t1;           // t1 is used for reading the time when a button is pressed. It is used in conjunction with debounceTime to determine whether debounceTime

int selectNo = 0;           // these 3 variables are used to navigate and know where in the LCD menu we are
int selectedMenu = 0;
int showingFrame = -1;
bool downShift = false;

// this 2D array determines what each menu has written in it
String menu[4][4] = {
  {"Premade Drinks  ", "Custom Drinks  ", "Shot           ", "Back           "}, 
  {"Back            ", "MalibuPineapple", "Malibu Cran    ", "Cran&Pineapple  "},
  {"Back            ", "Select ratios  ", "               ", "               "},
  {"Back            ", "Single         ", "Double         ", "               "}
};

void setup() {
  // initialising the display with the initial menu settings and a select token ">" 
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print(">Premade Drinks");
  lcd.setCursor(0, 1);
  lcd.print(" Custom Drinks");
  t1 = millis();

  pinMode(upButton, INPUT);
  pinMode(selectButton, INPUT);
  pinMode(downButton, INPUT);

  Wire.begin();  
}

void loop() {
  lightReading = analogRead(A0);

  if (lightReading < 300 && not(coinRolling)){
    coinRolling = true;                               // using this bool so one coin only adds one credit
    credit++;
    Serial.println("Card Block"); 
  } else if (lightReading > 300){
    coinRolling = false;                              // once the coin has completely passed through, reset the bool    
  }


  if (credit == 0) {                                  // checking whether the user has enough credit to buy a drink
    lcd.setCursor(0, 0);
    lcd.print("  Please enter  ");
    lcd.setCursor(0, 1);
    lcd.print("  more credit!  ");
  } else {
    
    updateScreen();                                   // update what is displayed on the screen according to the current menu and selection (scroll)
    
    if (digitalRead(selectButton) == HIGH && millis() - t1 > debounceTime) {  // if statement checks if select button is pressed and more than 300ms has passed since last button press
      Serial.println(millis() - t1);
      // Switch statement that deals with the select button takes into account the current menu, the current selection in order to decide what action to take next
      // whether to switch to another menu, back or to make a drink
      switch (selectedMenu) {
        
        case 0: // homen screen
          switch (selectNo) {
            case 0: 
                selectedMenu = 1;
                break;
              case 1:
                selectedMenu = 2;
                break;
              case 2:
                selectedMenu = 3;
                break;
              default:
                selectedMenu = 0;
                break;
          }
          break;
        
        case 1: // premade drinks
          switch (selectNo) {
            case 0:
                resetLcd();           // case 0 is the back button, so it resets the values of the LCD to go to the top of the home screen
                break;
              case 1:
          enjoyMsg();
                makeDrink(1, 0, 2);     // this switch case calls the make drink function with different ratios, which are easily changed, depending on the drinks which are in the machine
                resetLcd();
                break;
              case 2:
                enjoyMsg();
                makeDrink(1, 0, 5);
                resetLcd();
                break;
              case 3:
                enjoyMsg();
                makeDrink(0, 1, 1);
                resetLcd();
                break;
              default:
                resetLcd();
                break;
         }
           break;
        
        case 2: // custom drink
          switch(selectNo) {
            case 0: // back
                resetLcd();
                break;
              case 1:
              makeOwnDrink();           // because the menu of this choice is quite different, we chose to write a separate function to deal with the menu
              break;    
              default:
                selectNo = 0;
                break;
          }
          break;
          
        
        case 3: // shot menu
          switch(selectNo) {
            case 0: // back
              resetLcd();
              break;
            case 1: // single
              enjoyMsg();
              pourShot(0);            // this option does not use the makeDrink function, as it does not require ratios, but requires a smaller amount, so we created another function for it
              resetLcd();
              break;
            case 2: // double
              enjoyMsg();
              pourShot(1);
              resetLcd();
              break;
            default:
              resetLcd();
              break;
          }
          break;
        
        default:
          resetLcd();
          break;
      } 
      t1 = millis();
      showingFrame = -1;
      selectNo = 0;
      downShift = false;
    }
  
      if (digitalRead(upButton) == HIGH && millis() - t1 > debounceTime) {
      t1 = millis();          // resets the last time a button was pressed 
      downShift = false;
      if (selectNo > 0) {
        selectNo--;
        showingFrame = selectNo - 1;
      } else {
        showingFrame = -1;
      }
    }
    
    if (digitalRead(downButton) == HIGH && millis() - t1 > debounceTime) {
      t1 = millis();
      // since the 2D array menu must have declared dimensions, some of its elements are empty, so for each of these menus
      // we coded a limit as to how far down you can go, so you cannot select an empty element in the display
      if (selectedMenu == 0 && selectNo < 2) {
        downShift = true;
        selectNo++;
        showingFrame = selectNo - 1;
      } else if (selectedMenu == 1 && selectNo < 3) {
          downShift = true;
          selectNo++;
          showingFrame = selectNo - 1;
      } else if (selectedMenu == 2 && selectNo < 1) {
          downShift = true;
          selectNo++;
          showingFrame = selectNo - 1;
      } else if (selectedMenu == 3 && selectNo < 2) {
          downShift = true;
          selectNo++;
          showingFrame = selectNo - 1;
      } 
    }
  } 
}
 
/**
 * updateScreen function deals with displaying the select ">" symbol to show the user which choice they have selected
 * checks whether the last button press was up or down in order to determine whether ">" goes on top or botto row of LCD
 * prints the current array elements of selected menu
 */
void updateScreen(){
  if (showingFrame >= -1 && !downShift) {
      lcd.setCursor(0, 0);
      lcd.print(">" + menu[selectedMenu][showingFrame + 1]);
      lcd.setCursor(0, 1);
      lcd.print(" " + menu[selectedMenu][showingFrame + 2]);
  }
  else if (downShift && showingFrame >= 0) {
      lcd.setCursor(0, 0);
      lcd.print(" " + menu[selectedMenu][showingFrame]);
      lcd.setCursor(0, 1);
      lcd.print(">" + menu[selectedMenu][showingFrame + 1]);
  }
  else {
    lcd.setCursor(0,0);
    resetLcd();
    Serial.print(String(showingFrame) + " " + String(downShift));
  }
}


/**
 * Function that resets all the values that have anything to do with the LCD menu, in order to return it to its starting state
 */
void resetLcd() {
  selectNo = 0;
  showingFrame = -1;
  selectedMenu = 0;
  downShift = false;
  t1 = millis();
  lcd.clear();
}

// function to print enjoy message when drink is being poured
void enjoyMsg(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("     Enjoy!    ");
  lcd.print("               ");
}


/**
 * Function which creates a menu allowing users to select their own ratios
 * the function iterates through the 3 ratios, allowing users to increae or decrease the ratio of each liquid in their drink
 */
void makeOwnDrink() {
  lcd.clear();
  volatile byte currentSelection = 0;
  
  while (currentSelection < 4) {
    lcd.setCursor(0, 0);
    lcd.print(String("Ratio ") + String(currentSelection) + String(":"));
    lcd.setCursor(0, 1);
    lcd.print(String(ratios[currentSelection - 1]) + String("          ")); // printing the current ratio number of the drink

    if (digitalRead(selectButton) == HIGH && millis() - t1 > 200) {
      currentSelection++;
        t1 = millis();
    }
    
    if (digitalRead(downButton) == HIGH && millis() - t1 > 200) {
      if (ratios[currentSelection - 1] < 256) {       // keeping the value of the ratio lower than one byte, to make transmission of data easier
        ratios[currentSelection - 1]++;
        t1 = millis();
      }        
    }
    if (digitalRead(upButton) == HIGH && millis() - t1 > 200) {
      if (ratios[currentSelection - 1] > 0) {
        ratios[currentSelection - 1]--;
        t1 = millis();
      }
    }
  }
  if (ratios[0] != 0 || ratios[1] != 0 || ratios[2] != 0) {
    enjoyMsg();
    makeDrink(ratios[0], ratios[1], ratios[2]);
  }
  resetLcd();
}



/**
 * Sends data to other arduino, taking each ratio as a parameter.
 * Decrements the credit (since drink is being poured)
 * Displays loading bar, in time with how long the drink will take
 */
void makeDrink(byte ratio1, byte ratio2, byte ratio3) {
    credit--;
    Wire.beginTransmission(SLAVE_ADD);
    Serial.println(String(ratio1) + " " + String(ratio2) + " " + String(ratio3));
    Wire.write((byte)0);        // first byte sent across indicates this is not a shot
    Wire.write((byte)ratio1);
    Wire.write((byte)ratio2);
    Wire.write((byte)ratio3);
    Wire.endTransmission();
    int maxRatio = maximum(ratio1, ratio2, ratio3);
    int totalRatio = ratio1 + ratio2 + ratio3;
    t1 = millis();
    while (millis() - t1 <= 14000 * (double)maxRatio / totalRatio){
      Serial.println(String(millis() - t1) + " " + String(14000 * (double)maxRatio / totalRatio));
      int i = (((double) millis() - t1) / (14000 * (double)maxRatio / totalRatio)) * 16;  // calculating how far along the pouring is as a percent then multiplying it by 16 (cells in 1 row of LCD)  
      lcd.setCursor(i, 1);  // moving the cursor along to progress the bar
      lcd.print("\xff");    // prints a filled block cell for the loading bar
     }
    ratios[0] = 0;  // reseting the ratio array
    ratios[1] = 0;
    ratios[2] = 0;
}


/**
 * Sends across to slave arduino that this is a shot and whether it is a double or a single
 */
void pourShot(byte amount){
  credit--;
  Wire.beginTransmission(SLAVE_ADD);  
  Wire.write((byte)1);
  Wire.write(amount);
  Wire.write((byte)0);
  Wire.write((byte)0);
  Wire.endTransmission();
  t1 = millis();
  while (millis() - t1 <= 4000 * (amount + 1)) {
    int i = (((double) millis() - t1) / (4000 * (amount + 1))) * 16;
      lcd.setCursor(i, 1);
      lcd.print("\xff");
  }
}

/**
 * Returns the maximum value out of three parameters
 */
int maximum(byte r1, byte r2, byte r3) {
  if (r1 > r2) {
    if (r1 >= r3) {
      return r1;
    } else {
      return r3;
    }
  } else {
    if (r2 > r3) {
      return r2;
    } else {
      return r3;
    }
  }
}
