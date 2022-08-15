/******************************************************************************
*****                      SMART PARKING SYSTEM                           *****
******************************************************************************/
/*!
***     \file       Smart_Parking.c
***   
***     \author     Aditya Subramanian
***                 Jerry Akpan
***
***     \brief      This file controls the working of a smart parking system
***     \par        File_description
***                                        
***
*\n*/
/*****************************************************************************/

/*=============================================================================
=======                            INCLUDES                             =======
=============================================================================*/
/*------ standard includes -------*/
#include "stdio.h"
#include "string.h"
/*------ project includes --------*/

/*------ module includes --------*/
#include <LiquidCrystal.h>
#include <Servo.h>
/*=============================================================================
=======               DEFINES & MACROS FOR GENERAL PURPOSE              =======
=============================================================================*/
/* MACROS */
#define LCD_D0 3
#define LCD_D1 4
#define LCD_D2 5
#define LCD_D3 6
#define LED_RED 7
#define LED_GREEN 13
#define BUTTON_ENTER 11
#define BUTTON_EXIT 8
#define SERVO_CONTROL 9
#define LCD_RS 12
#define LCD_EN 2
#define MAX_SPACE 13
#define NO_SPACE 0 
#define BUTTON_PRESSED LOW
/* Local, Global variable declaration */
static int current_space = MAX_SPACE,servo_position,dist_check;
static bool Enter_ButtonState = HIGH,Exit_ButtonState=HIGH, safety_check = HIGH,Init_Button_State = HIGH;
long duration = 0;
/*=============================================================================
=======                       CONSTANTS  &  TYPES                       =======
=============================================================================*/
/* Initialize the LCD pins */
LiquidCrystal lcd(LCD_RS,LCD_EN,LCD_D0,LCD_D1,LCD_D2,LCD_D3); 
Servo Servo_Arm;
/*=============================================================================
=======                       PRIVATE FUNCTION DECLARATIONS             =======
=============================================================================*/
static void parkingEntrySequence(void);
static void parkingExitSequence(void);
static void parkingNoSpaceSequence(void);
static void parkingDefaultSequence(void);
static void parkingHardwareInitSequence(void);
/*=============================================================================
=======                              METHODS                            =======
=============================================================================*/

void setup() 
{
  /* Initialize Hardware Peripherals */
   parkingHardwareInitSequence();
}

void loop() 
{
 
  /* Keep Reading Button states at the start of every loop cycle */
  Enter_ButtonState = digitalRead(BUTTON_ENTER);
  Exit_ButtonState  = digitalRead(BUTTON_EXIT);
  
  /* ENTRY BUTTON PRESSED */
  if (Enter_ButtonState == BUTTON_PRESSED)
  { 
    /* Call Enter Parking Function */
    parkingEntrySequence();  
  }

  /* EXIT BUTTON PRESSED */
  if(Exit_ButtonState == BUTTON_PRESSED)
  {
    /* Call Exit Sequence */
    parkingExitSequence();
  }
  
  if(current_space == NO_SPACE)
  {
    /* Call No Space Sequence */
    parkingNoSpaceSequence();
  }
}

static void parkingHardwareInitSequence(void)
{
  Serial.begin(9600);
  /* Setup the LCD Module */
  lcd.begin(16,2);
  /* Setup PUSH Buttons */
  pinMode(BUTTON_ENTER,INPUT_PULLUP);
  pinMode(BUTTON_EXIT,INPUT_PULLUP);
  /* Setup LEDs */
  pinMode(LED_RED,OUTPUT);
  pinMode(LED_GREEN,OUTPUT);
  /* Setup Servo Motor */
  Servo_Arm.attach(SERVO_CONTROL);
  /* Setup Ultrasonic Sensor Pin Mode */
  pinMode(A1,OUTPUT);
  pinMode(A0,INPUT);
  /* Setup Buzzer */
  pinMode(A2,OUTPUT);
  /* For the very first time, set default condition */
  parkingDefaultSequence();
}

static void parkingDefaultSequence(void)
{
  lcd.setCursor(0,0);
  lcd.print("Welcome!!!");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("GROUP 1 ");
  lcd.setCursor(0,1);
  lcd.print("BY:ADI,JERRY");
  while (Init_Button_State == HIGH)
  {
    /* Stay Here Until the button is pressed to initiate Main program*/
     Init_Button_State = digitalRead(BUTTON_ENTER);
     while(digitalRead(BUTTON_ENTER)== BUTTON_PRESSED);
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Welcome!!!");
  lcd.setCursor(0,1);
  lcd.print(MAX_SPACE);
  digitalWrite(LED_RED,HIGH);
  Servo_Arm.write(0);
}

static void parkingNoSpaceSequence(void)
{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Parking Full!");
      lcd.setCursor(0,1);
      lcd.print("Plz Come Later!");
}

static void parkingEntrySequence(void)
{
   /*
     * IF Enter Button is pressed do the following
     * 1. Check if there is parking available
     * 2. If yes, turn servo motor by 90 degrees
     * 3. print Enter for 2 seconds and change it back to slot availability
     * 4. Enable Green LED 
     * 5. With 3 second delay; push all configurations back to default.
     * 6. If no parking is available: Enter Parking FULL ; Please Try later
     * 7. Edit the availability and feed it back to the LCD
    */
     if(current_space == NO_SPACE)
     {
      /* The number of available slots are 0. Send Regret */
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Parking Full!");
      lcd.setCursor(0,1);
      lcd.print("Plz Come Later!");
      delay(2000);
     }
     else
     {
      /* Parking Available ; allow CAR */
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Enter!");
      digitalWrite(LED_RED,LOW);
      digitalWrite(LED_GREEN,HIGH);
      Servo_Arm.write(90);
      /*Delay for car to pass through */
      safety_check = HIGH;
      delay(3000);
      while (safety_check == HIGH)
      {
         digitalWrite(A1, LOW);
         delayMicroseconds(2);
         // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
         digitalWrite(A1, HIGH);
         delayMicroseconds(10);
         digitalWrite(A1, LOW); 
         duration = (pulseIn(A0,HIGH));
         dist_check = (duration*0.034)/2;
         Serial.print("Distance: ");
         Serial.print(dist_check);
         Serial.println(" cm");
         if (dist_check < 10)
         {
          safety_check = HIGH;
          digitalWrite(A2,HIGH);
          delay(500);
          digitalWrite(A2,LOW);
         }
         else
         {
          safety_check = LOW;
          digitalWrite(A2,LOW);
         }
      }
      current_space--;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Welcome!!!");
      lcd.setCursor(0,1);
      lcd.print(current_space);
      /* Time to retract to default */
      digitalWrite(LED_GREEN,LOW);
      digitalWrite(LED_RED,HIGH);
      Servo_Arm.write(0);
     }
     /* To avoid multiple press or debouncing effect */
     while(digitalRead(BUTTON_ENTER)== BUTTON_PRESSED);
}

static void parkingExitSequence(void)
{
  /*   1. Allow the car to exit  if there is Car in spot
       2. Modify slot availability 
       3. Close barrier and retract to default
    */
    if(current_space == MAX_SPACE)
    {
      /* Parking Empty
         Do Nothing */
    }
    else
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("ThankYou!");
      lcd.setCursor(0,1);
      lcd.print("Come Back Soon!");
      Servo_Arm.write(90);
      digitalWrite(LED_RED,LOW);
      digitalWrite(LED_GREEN,HIGH);
      current_space++;
      safety_check = HIGH;
 /* 2 second delay for car to exit */
      delay(2000);
      while (safety_check == HIGH)
      {
         digitalWrite(A1, LOW);
         delayMicroseconds(2);
         // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
         digitalWrite(A1, HIGH);
         delayMicroseconds(10);
         digitalWrite(A1, LOW); 
         duration = (pulseIn(A0,HIGH));
         dist_check = (duration*0.034)/2;
         if (dist_check < 30)
         {
          safety_check = HIGH;
          digitalWrite(A2,HIGH);
          delay(500);
          digitalWrite(A2,LOW);
         }
         else
         {
          safety_check = LOW;
          digitalWrite(A2,LOW);
         }
      }
      Servo_Arm.write(0);
      digitalWrite(LED_GREEN,LOW);
      digitalWrite(LED_RED,HIGH);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Welcome!");
      lcd.setCursor(0,1);
      lcd.print(current_space);
       /* To avoid multiple press or debouncing effect */
      while(digitalRead(BUTTON_EXIT)== BUTTON_PRESSED);
      }  
}
