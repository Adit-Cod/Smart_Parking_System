%% 
%{
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
%}
%% Stage 1 : Initialization of Hardware, Software and Program Requirements
clc;
clear;
Enter_ButtonState   = 0;
Exit_ButtonState    = 0;
max_available_space = 13;
Init_button_status  = 1;
BUTTON_PRESSED      = 0;
current_space       = max_available_space;
NO_SPACE            = 0;
safety_check        = 1;
time_entry_index    = 1;
time_exit_index     = 1;
t_breaker           = 1;
old_date            = 0;
%% Initalize Hardware and Software peripherals 
%{
1. Decide Arduino Hardware Pinout
2. Configure the pin as Output/Input 
3. Initialize LCD, Servo and other peripherals as required
%}
% Create an object to control and work with the arduino; invoke the required libraries.
ard_control = arduino('COM8','Uno','Libraries',{'ExampleLCD/LCDAddon','Servo','Ultrasonic'},'ForceBuildOn',true);
% Create an object for servo control
servo_control = servo(ard_control,'D9');
% Create an object for LCD control 
lcd_control = addon(ard_control,'ExampleLCD/LCDAddon','RegisterSelectPin','D12','EnablePin','D2','DataPins',{'D3','D4','D5','D6'});
% Create an object for ultrasonic sensor
ultrasonic_control = ultrasonic(ard_control,'A1','A0','OutputFormat','double');
% State pin usage
% Two input buttons
configurePin(ard_control,'D8','Pullup');
configurePin(ard_control,'D11','Pullup');
% Configure Pin for Buzzer
configurePin(ard_control,'A2','DigitalOutput');
%Two output LEDs red and green
configurePin(ard_control,'D7','DigitalOutput');
configurePin(ard_control,'D13','DigitalOutput');
initializeLCD(lcd_control);

%% Stage 2 : Init Sequence
%{
The init sequence should set stage for process to start. 
cover project descriptions and all peripherals should be ready.
%}
writePosition(servo_control,0);
printLCD(lcd_control,'Welcome!!!');
pause(5);
clearLCD(lcd_control);
printLCD(lcd_control,'Group1');
printLCD(lcd_control,'By:ADI,JERRY');
while( Init_button_status == 1)
% % Stay here until button is pressed
Init_button_status = readDigitalPin(ard_control,'D11');
end

%% Stage 3 Button is pressed; the actual program can start
%{
This stage is responsible to display the Welcome message, available 
space in the parking lot and ensuring Red light is turned on.
%}
clearLCD(lcd_control);
printLCD(lcd_control,'Welcome!!!');
printLCD(lcd_control,strcat(num2str(max_available_space)));
% By default red ligth is turned on
writeDigitalPin(ard_control,'D7',1);
% Make sure servo is positioned at close
writePosition(servo_control,0);

%% Stage 4 : Main Program Loop : Stay in this loop till power shutdown
%{
Check Different button states and allow/deny entry of vehicles into the
parking lot.
%}
 while(1)
     
     % Continuously check both the button statuses  
     Enter_ButtonState  = readDigitalPin(ard_control,'D11');
     Exit_ButtonState   = readDigitalPin(ard_control,'D8');
     
     if Enter_ButtonState == BUTTON_PRESSED
         % Activate Entry Sequence 
         %{
            /*
             *** IF Enter Button is pressed do the following***
             * 1. Check if there is parking space available
             * 2. If yes, turn servo motor by 90 degrees
             * 3. print Enter for 2 seconds and change it back to slot availability
             * 4. Enable Green LED 
             * 5. With 3 second delay; push all configurations back to default.
             * 6. If no parking is available: Enter Parking FULL ; Please Try later
             * 7. Edit the availability and feed it back to the LCD
            */
         %}
         if current_space == NO_SPACE
            clearLCD(lcd_control);
            printLCD(lcd_control,'Parking Full!');
            printLCD(lcd_control,'Plz Come Later');
            pause(2);
         else
            clearLCD(lcd_control);
            printLCD(lcd_control,'Enter!');
            time_entry(time_entry_index,1) = datetime('now');
            time_entry_index = time_entry_index+1;
            writeDigitalPin(ard_control,'D7',0);
            writeDigitalPin(ard_control,'D13',1);
            writePosition(servo_control,0.5);
            % Allow for car to pass
            safety_check = 1;
            pause(1);
            % To check whether the car has passed ; use the ultrasonic
            % sensor to measure distance.
            % if there is an obstacle within 10 cm of sensor. Assume car
            % hasnt passed and trigger buzzer.
            % stay on this loop until obstacle has cleared the way.
            while safety_check == 1
                  dist_check_1 = readDistance(ultrasonic_control);
                  dist_check = round(dist_check_1*100,2);
                  if dist_check <= 30
                      safety_check = 1;
                      % Trigger Buzzer
                      writeDigitalPin(ard_control,'A2',1);
                      pause(1.5);
                      writeDigitalPin(ard_control,'A2',0);
                  else
                      writeDigitalPin(ard_control,'A2',0);
                      safety_check = 0;
                  end
            end
            current_space = current_space-1;
            writePosition(servo_control,0);
            writeDigitalPin(ard_control,'D13',0);
            writeDigitalPin(ard_control,'D7',1);
            clearLCD(lcd_control);
            printLCD(lcd_control,'Welcome!');
            printLCD(lcd_control,strcat(num2str(current_space)));     
         end
         while (BUTTON_PRESSED == readDigitalPin(ard_control,'D11'))
         end
     end
     
     % Exit State
     if Exit_ButtonState == BUTTON_PRESSED
         %{
          /*   1. Allow the car to exit if there is Car in spot
           *   2. Modify slot availability 
           *   3. Close barrier and retract to default
           */
         %}
          safety_check = 1;
          if current_space == max_available_space
              % Do Nothing; no car is inside. 
          else
            clearLCD(lcd_control);
            printLCD(lcd_control,'Thank You!');
            printLCD(lcd_control,'Come Back Soon!');
            time_exit(time_exit_index,1) = datetime('now');
            time_exit_index = time_exit_index+1;
            writeDigitalPin(ard_control,'D7',0);
            writeDigitalPin(ard_control,'D13',1);
            % Allow Car to pass through
            writePosition(servo_control,0.5);
            % Wait for Car to pass through
            pause(3);
            while safety_check == 1
                  dist_check_1 = readDistance(ultrasonic_control);
                  dist_check = round(dist_check_1*100,2);
                  if dist_check <= 30
                      safety_check = 1;
                      % Trigger Buzzer
                      writeDigitalPin(ard_control,'A2',1);
                      pause(1.5);
                      writeDigitalPin(ard_control,'A2',0);
                  else
                      writeDigitalPin(ard_control,'A2',0);
                      safety_check = 0;
                  end
            end
            current_space = current_space+1;
            writePosition(servo_control,0);
            writeDigitalPin(ard_control,'D13',0);
            writeDigitalPin(ard_control,'D7',1);
            clearLCD(lcd_control);
            printLCD(lcd_control,'Welcome!!!');
            printLCD(lcd_control,strcat(num2str(current_space)));
            while (BUTTON_PRESSED == readDigitalPin(ard_control,'D8'))
            end
          end
     end
     
     if current_space == NO_SPACE
         % No space sequence invoked
         clearLCD(lcd_control);
         printLCD(lcd_control,'Parking Full!');
         printLCD(lcd_control,'Plz Come Later');
     end
     % Send Data to the cloud every one minute; to track the stats of
     % parking utilization. 
     c = clock;
     new_date = c(3);
     if t_breaker == 1
        ref_time = c(5);
        t_breaker = 0;
     end 
     send_time = c(5) - ref_time;
     
     if send_time >= 1 || old_date ~= new_date 
         % entry time buffer is full; send the data to the cloud and reset
         % the index.
         ref_time = c(5);
         ChannelId  = 1807419;
         writekey   = 'JN3VNGACM4THZBCH';
         userApikey = '923ZHT68B6COANY0';
         url = sprintf('https://api.thingspeak.com/channels/1807419/feeds.json?api_key=%s',userApikey);
         if old_date ~= new_date
             %flush the data after the day ends.
             response = webwrite(url,weboptions('RequestMethod','delete'));
             pause(1);
             old_date = new_date;    
         end
         display('Sending Data to cloud');
         data = timetable(time_entry,current_space)
         thingSpeakWrite(ChannelId,current_space,'WriteKey',writekey);   
     end
 end
