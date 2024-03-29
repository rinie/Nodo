
/*********************************************************************************************\
 * Eenvoudige check of event een Nodo commando is die voor deze Nodo bestemd is.
 * Test NIET op geldigheid van de parameters
 * geeft de waarde 0, NODO_TYPE_EVENT of NODO_TYPE_COMMAND terug.
 \*********************************************************************************************/
byte NodoType(struct NodoEventStruct *InEvent)
  {
  int x;
  
  if(InEvent->Flags & TRANSMISSION_EVENT)
     return NODO_TYPE_EVENT;

  if(InEvent->Flags & TRANSMISSION_COMMAND)
     return NODO_TYPE_COMMAND;  

  if(InEvent->Command>=FIRST_EVENT && InEvent->Command<=LAST_EVENT)
    return NODO_TYPE_EVENT;

  if(InEvent->Command>=FIRST_COMMAND && InEvent->Command<=LAST_COMMAND)
    return NODO_TYPE_COMMAND;

//  struct NodoEventStruct Event;
//  ClearEvent(&Event);
//  x=InEvent->Command-CMD_DEVICE_FIRST;
//  if(x>=0 && x<DEVICE_MAX)
//    if(Device_ptr[x]!=0)
//      {
//      Device_ptr[x](DEVICE_TYPE,&Event,0);
//      if(Event.Flags | TRANSMISSION_EVENT)
//        return NODO_TYPE_EVENT;
//      if(Event.Flags | TRANSMISSION_COMMAND)
//        return NODO_TYPE_COMMAND;      
//      }???

  return false;
  }  
  
 
/*********************************************************************************************\
 * Deze functie checked of de code die ontvangen is een uitvoerbare opdracht is/
 * Als het een correct commando is wordt deze uitgevoerd en 
 * true teruggegeven. Zo niet dan wordt er een 'false' retour gegeven.
 \*********************************************************************************************/

boolean ExecuteCommand(NodoEventStruct *EventToExecute)
  {
  unsigned long a;
  int x,y;
  byte error=0;
  
  struct NodoEventStruct TempEvent=*EventToExecute;
  
  #ifdef NODO_MEGA
  char *TempString=(char*)malloc(50);
  #endif
  
  switch(EventToExecute->Command)
    {   
    case CMD_VARIABLE_INC:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=USER_VARIABLES_MAX) // in de MMI al afvevangen, maar deze beschermt tegen vastlopers i.g.v. een foutief ontvangen event
        {
        UserVar[EventToExecute->Par1-1]+=ul2float(EventToExecute->Par2);
        TempEvent.Command      = CMD_VARIABLE_EVENT;
        TempEvent.Par2         = float2ul(UserVar[EventToExecute->Par1-1]);
        TempEvent.Direction    = VALUE_DIRECTION_INPUT;
        TempEvent.Port         = VALUE_SOURCE_VARIABLE;
        ProcessEvent2(&TempEvent);
        }
      break;        

    case CMD_VARIABLE_DEC:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=USER_VARIABLES_MAX) // in de MMI al afvevangen, maar deze beschermt tegen vastlopers i.g.v. een foutief ontvangen event
        {
        UserVar[EventToExecute->Par1-1]-=ul2float(EventToExecute->Par2);
        TempEvent.Command      = CMD_VARIABLE_EVENT;
        TempEvent.Par2         = float2ul(UserVar[EventToExecute->Par1-1]);
        TempEvent.Direction    = VALUE_DIRECTION_INPUT;
        TempEvent.Port         = VALUE_SOURCE_VARIABLE;
        ProcessEvent2(&TempEvent);
        }
      break;        

    case CMD_VARIABLE_SET:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=USER_VARIABLES_MAX) // in de MMI al afvevangen, maar deze beschermt tegen vastlopers i.g.v. een foutief ontvangen event
        {
        UserVar[EventToExecute->Par1-1]=ul2float(EventToExecute->Par2);
        TempEvent.Command=CMD_VARIABLE_EVENT;
        TempEvent.Port=VALUE_SOURCE_VARIABLE;
        TempEvent.Direction=VALUE_DIRECTION_INPUT;
        ProcessEvent2(&TempEvent);      // verwerk binnengekomen event.
        }
      break;         

    case CMD_VARIABLE_SET_WIRED_ANALOG:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=USER_VARIABLES_MAX) // in de MMI al afvevangen, maar deze beschermt tegen vastlopers i.g.v. een foutief ontvangen event
        {
        UserVar[EventToExecute->Par1-1]=analogRead(PIN_WIRED_IN_1+EventToExecute->Par2-1);
        TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
        TempEvent.Command=CMD_VARIABLE_EVENT;
        TempEvent.Port=VALUE_SOURCE_VARIABLE;
        TempEvent.Direction=VALUE_DIRECTION_INPUT;
        ProcessEvent2(&TempEvent);      // verwerk binnengekomen event.
        }
      break;         
  
    case CMD_VARIABLE_VARIABLE:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=USER_VARIABLES_MAX) // in de MMI al afvevangen, maar deze beschermt tegen vastlopers i.g.v. een foutief ontvangen event
        {
        UserVar[EventToExecute->Par1-1]=UserVar[EventToExecute->Par2-1];
        TempEvent.Command=CMD_VARIABLE_EVENT;
        TempEvent.Port=VALUE_SOURCE_VARIABLE;
        TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
        TempEvent.Direction=VALUE_DIRECTION_INPUT;
        ProcessEvent2(&TempEvent);      // verwerk binnengekomen event.
        }
      break;        

    case CMD_VARIABLE_LAST_EVENT://???@@
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=USER_VARIABLES_MAX) // in de MMI al afvevangen, maar deze beschermt tegen vastlopers i.g.v. een foutief ontvangen event
        {
        // Als het vorige event betrekking had op variabele/poort die de gebruiker heeft opgegeven in Par2
        // dan de waardeuit het vorige event overnemen.
        if(EventToExecute->Par2==LastReceived.Par1)
          {
          x=EventToExecute->Par1-1;
          
          switch(LastReceived.Command)
            {
            case CMD_VARIABLE_EVENT:
            case CMD_VARIABLE_SET:
              UserVar[x]=ul2float(LastReceived.Par2);
              break;
            case CMD_WIRED_ANALOG:
            case CMD_TIMER_SET:
              UserVar[x]=LastReceived.Par2;
            default:
              UserVar[x]=0;
            }          
            
          TempEvent.Command=CMD_VARIABLE_EVENT;
          TempEvent.Port=VALUE_SOURCE_VARIABLE;
          TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
          TempEvent.Direction=VALUE_DIRECTION_INPUT;
          ProcessEvent2(&TempEvent);      // verwerk binnengekomen event.
          }
        }
      break;        

    case CMD_VARIABLE_PULSE_COUNT:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=USER_VARIABLES_MAX) // in de MMI al afvevangen, maar deze beschermt tegen vastlopers i.g.v. een foutief ontvangen event
        {
        UserVar[EventToExecute->Par1-1]=PulseCount;
        TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
        TempEvent.Command=CMD_VARIABLE_EVENT;
        TempEvent.Port=VALUE_SOURCE_VARIABLE;
        TempEvent.Direction=VALUE_DIRECTION_INPUT;
        PulseCount=0;
        ProcessEvent2(&TempEvent);      // verwerk binnengekomen event.
        }
      break;         

    case CMD_VARIABLE_PULSE_TIME:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=USER_VARIABLES_MAX) // in de MMI al afvevangen, maar deze beschermt tegen vastlopers i.g.v. een foutief ontvangen event
        {
        UserVar[EventToExecute->Par1-1]=PulseTime;
        TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
        TempEvent.Command=CMD_VARIABLE_EVENT;
        TempEvent.Port=VALUE_SOURCE_VARIABLE;
        TempEvent.Direction=VALUE_DIRECTION_INPUT;
        ProcessEvent2(&TempEvent);      // verwerk binnengekomen event.
        }
      break;         

    case CMD_STOP:
      error=MESSAGE_09;
      break;

    case CMD_BREAK_ON_VAR_EQU:
      {
      if((int)UserVar[EventToExecute->Par1-1]==(int)ul2float(EventToExecute->Par2))
        error=MESSAGE_15;
      break;
      }
      
    case CMD_BREAK_ON_VAR_NEQU:
      if((int)UserVar[EventToExecute->Par1-1]!=(int)ul2float(EventToExecute->Par2))
        error=MESSAGE_15;
      break;

    case CMD_BREAK_ON_VAR_MORE:
      if(UserVar[EventToExecute->Par1-1] > ul2float(EventToExecute->Par2))
        error=MESSAGE_15;
      break;

    case CMD_BREAK_ON_VAR_LESS:
      if(UserVar[EventToExecute->Par1-1] < ul2float(EventToExecute->Par2))
        error=MESSAGE_15;
      break;

    case CMD_BREAK_ON_VAR_LESS_VAR:
      if(UserVar[EventToExecute->Par1-1] < UserVar[EventToExecute->Par2-1])
        error=MESSAGE_15;
      break;

    case CMD_BREAK_ON_VAR_MORE_VAR:
      if(UserVar[EventToExecute->Par1-1] > UserVar[EventToExecute->Par2-1])
        error=MESSAGE_15;
      break;


    case CMD_BREAK_ON_DAYLIGHT:
      if(EventToExecute->Par1==VALUE_ON && (Time.Daylight==2 || Time.Daylight==3))
        error=MESSAGE_15;

      if(EventToExecute->Par1==VALUE_OFF && (Time.Daylight==0 || Time.Daylight==1 || Time.Daylight==4))
        error=MESSAGE_15;
      break;

    case CMD_BREAK_ON_TIME_LATER:
      if((EventToExecute->Par1*60+EventToExecute->Par2)<(Time.Hour*60+Time.Minutes))
        error=MESSAGE_15;
      break;

    case CMD_BREAK_ON_TIME_EARLIER:
      if((EventToExecute->Par1*60+EventToExecute->Par2)>(Time.Hour*60+Time.Minutes))
        error=MESSAGE_15;
      break;

    case CMD_SEND_USEREVENT:
      ClearEvent(&TempEvent);    
      TempEvent.Port                  = VALUE_ALL;
      TempEvent.Command               = CMD_USEREVENT;
      TempEvent.Par1                  = EventToExecute->Par1;
      TempEvent.Par2                  = EventToExecute->Par2;
      SendEvent(&TempEvent, false, true,true);
      break;

    case CMD_LOCK:
      if(EventToExecute->Par1==VALUE_ON)
        {// Als verzoek om inschakelen dan Lock waarde vullen
        if(Settings.Lock==0)// mits niet al gelocked.
          Settings.Lock=EventToExecute->Par2; 
        else
          error=MESSAGE_10;
        }
      else
        {// Verzoek om uitschakelen
        if(Settings.Lock==EventToExecute->Par2 || Settings.Lock==0)// als lock code overeen komt of nog niet gevuld
          Settings.Lock=0;
        else
          error=MESSAGE_10;
        }              
      Save_Settings();
      break;


    #ifdef NODO_MEGA
    case CMD_ALARM_SET:
      if(EventToExecute->Par1>=1 && EventToExecute->Par1<=ALARM_MAX)              // niet buiten bereik array!
        {
        for(x=0;x<8;x++)                                                          // loop de acht nibbles van de 32-bit Par2 langs
          {          
          y=(EventToExecute->Par2>>(x*4))&0xF;                                    // selecter nibble
          if(y!=0xE)                                                              // als de waarde geset moet worden
            {
            a=0xffffffff  ^ (0xfUL <<(x*4));                                      // Mask maken om de nibble positie y te wissen.
            Settings.Alarm[EventToExecute->Par1-1]  &=  a;                        // Maak nibble leeg
            Settings.Alarm[EventToExecute->Par1-1]  |= ((unsigned long)y)<<(x*4); // vul met door user opgegeven token
            }
          }
        }
      break;

    case CMD_CLOCK_SYNC:
      if(bitRead(HW_Config,HW_CLOCK)) // bitRead(HW_Config,HW_CLOCK)=true want dan is er een RTC aanwezig.
        {   
        // Haal eerst de juiste tijd op van de WebApp
        if(bitRead(HW_Config,HW_WEBAPP && EventToExecute->Par1==VALUE_SOURCE_HTTP))
          {
          SendHTTPEvent(EventToExecute);
          }
        else
          {
          // Geef de juiste tijd nu door aan alle andere Nodo's
          for(y=1; y<=UNIT_MAX; y++)
            {
            if(y!=Settings.Unit)
              {        
              x=NodoOnline(y,0);
              if(x)
                {           
                ClearEvent(&TempEvent);    
                TempEvent.Port                  = x;
                TempEvent.DestinationUnit       = y;    
                TempEvent.Flags                 = TRANSMISSION_QUEUE | TRANSMISSION_NEXT | TRANSMISSION_LOCK; 
    
                TempEvent.Command               = CMD_CLOCK_YEAR;
                TempEvent.Par1                  = Time.Year/100;
                TempEvent.Par2                  = Time.Year%100;
                SendEvent(&TempEvent, false, true, true);
                
                TempEvent.Command               = CMD_CLOCK_DATE;
                TempEvent.Par1                  = Time.Date;
                TempEvent.Par2                  = Time.Month;
                SendEvent(&TempEvent, false, true, true);
                
                TempEvent.Command               = CMD_CLOCK_TIME;
                TempEvent.Par1                  = Time.Hour;
                TempEvent.Par2                  = Time.Minutes;
                SendEvent(&TempEvent, false, true, true);
                
                TempEvent.Flags                 = 0; 
                TempEvent.Command               = CMD_CLOCK_DOW;
                TempEvent.Par1                  = Time.Day;
                TempEvent.Par2                  = 0;
                SendEvent(&TempEvent, false, true, true);
                }
              }
            }
          }
        }
      break;
    #endif
      
    case CMD_CLOCK_YEAR:
      x=EventToExecute->Par1*100+EventToExecute->Par2;
      Time.Year=x;
      ClockSet();
      ClockRead();
      break;
    
    case CMD_CLOCK_TIME:
      Time.Hour=EventToExecute->Par1;
      Time.Minutes=EventToExecute->Par2;
      Time.Seconds=0;
      ClockSet();
      break;

    case CMD_CLOCK_DATE: // data1=datum, data2=maand
      Time.Date  = EventToExecute->Par1;
      Time.Month = EventToExecute->Par2;
      ClockSet();
      ClockRead();
      break;

    case CMD_CLOCK_DOW:
      Time.Day=EventToExecute->Par1;
      ClockSet();
      break;
      
    case CMD_TIMER_SET:
       TimerSet(EventToExecute->Par1,EventToExecute->Par2);
      break;

    case CMD_TIMER_RANDOM:
      UserTimer[EventToExecute->Par1-1]=millis()+random(EventToExecute->Par2)*1000;
      break;

    case CMD_WAIT_EVENT:// WaitEvent <unit>, <command>
      ClearEvent(&TempEvent);
      TempEvent.SourceUnit=EventToExecute->Par1;
      TempEvent.Command=EventToExecute->Par2;
      Wait(30,false,&TempEvent,false);
      break;

    case CMD_DELAY:
      Wait(EventToExecute->Par1, false, 0, false);
      break;        

    case CMD_SEND_EVENT:
      ClearEvent(&TempEvent);
      TempEvent=LastReceived;
      TempEvent.Port=EventToExecute->Par1==0?VALUE_ALL:EventToExecute->Par1;
      SendEvent(&TempEvent, TempEvent.Command==CMD_RAWSIGNAL,true, true);
      break;        

    case CMD_SOUND: 
      Alarm(EventToExecute->Par1,EventToExecute->Par2);
      break;     
  
    case CMD_WIRED_PULLUP:
      Settings.WiredInputPullUp[EventToExecute->Par1-1]=EventToExecute->Par2; // EventToExecute->Par1 is de poort[1..]
      if(Settings.WiredInputPullUp[x]==VALUE_ON)
        pinMode(A0+PIN_WIRED_IN_1+EventToExecute->Par1-1,INPUT_PULLUP);
      else
        pinMode(A0+PIN_WIRED_IN_1+EventToExecute->Par1-1,INPUT);
      break;
                 
    case CMD_WIRED_OUT:
      if(EventToExecute->Par1==0)
        {
        x=1;
        y=WIRED_PORTS;
        }
      else
        {
        x=EventToExecute->Par1;
        y=EventToExecute->Par1;
        }
      for(x;x<=y;x++)
        {
        digitalWrite(PIN_WIRED_OUT_1+x-1,(EventToExecute->Par2==VALUE_ON));
        WiredOutputStatus[x-1]=(EventToExecute->Par2==VALUE_ON);
        TempEvent.Par1=x;
        TempEvent.Port=VALUE_SOURCE_WIRED;
        TempEvent.Direction=VALUE_DIRECTION_OUTPUT;
        PrintEvent(&TempEvent);
        }
      break;
                         
    case CMD_SETTINGS_SAVE:
      UndoNewNodo();// Status NewNodo verwijderen indien van toepassing
      Save_Settings();
      break;

    case CMD_WAITFREERF: 
      Settings.WaitFree=EventToExecute->Par1;
      break;

    case CMD_OUTPUT:
      switch(EventToExecute->Par1)
        {
        case VALUE_SOURCE_IR:
          Settings.TransmitIR=EventToExecute->Par2;
          break;       
        case VALUE_SOURCE_RF:
          Settings.TransmitRF=EventToExecute->Par2;
          break;       

        #ifdef NODO_MEGA
        case VALUE_SOURCE_HTTP:
          Settings.TransmitIP=EventToExecute->Par2;        
          break;       
        #endif
        }
      
      break;
      
    case CMD_WIRED_SMITTTRIGGER:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=WIRED_PORTS)
        Settings.WiredInputSmittTrigger[EventToExecute->Par1-1]=EventToExecute->Par2;
      break;                  

    case CMD_WIRED_THRESHOLD:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=WIRED_PORTS)
        Settings.WiredInputThreshold[EventToExecute->Par1-1]=EventToExecute->Par2;
      break;                  

    case CMD_STATUS:
      Status(EventToExecute);
      break;
      
    case CMD_UNIT_SET:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=UNIT_MAX)
        {
        Settings.Unit=EventToExecute->Par1;  
        Save_Settings();
        RebootNodo=true;
        break;
        }
      
    case CMD_HOME_SET:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=HOME_MAX)
        Settings.Home=EventToExecute->Par1;  
      break;
      
    case CMD_REBOOT:
      RebootNodo=true;
      break;        
      
    case CMD_RESET:
      ResetFactory();
      break;

    case CMD_SENDTO:
      Transmission_SendToUnit=EventToExecute->Par1;
       Transmission_SendToAll=EventToExecute->Par2;
      // als de SendTo wordt opgeheven vanuit de master, geef dit dan aan alle Nodo's te kennen
      // anders blijven deze in de wachtstand staan.
      if(Transmission_SendToUnit==0)
        {
        EventToExecute->SourceUnit=Settings.Unit;
        EventToExecute->Port=VALUE_ALL;
        SendEvent(EventToExecute, false,true, true);
        }
      break;    

    case CMD_EVENTLIST_ERASE:
      Led(BLUE);
      ClearEvent(&TempEvent);
      
      if(EventToExecute->Par1==0)
        {
        x=1;
        UndoNewNodo();// Status NewNodo verwijderen indien van toepassing
        while(Eventlist_Write(x++,&TempEvent,&TempEvent));
        }
      else
        {
        Eventlist_Write(EventToExecute->Par1,&TempEvent,&TempEvent);
        }
      break;        
        
#ifdef NODO_MEGA

    case CMD_PORT_SERVER:
      Settings.OutputPort=EventToExecute->Par2;
      break;

    case CMD_PORT_CLIENT:
      Settings.PortClient=EventToExecute->Par2;
      break;

    case CMD_ECHO:
      if(EventToExecute->Port==VALUE_SOURCE_TELNET) 
        Settings.EchoTelnet=EventToExecute->Par1;
      if(EventToExecute->Port==VALUE_SOURCE_SERIAL) 
        Settings.EchoSerial=EventToExecute->Par1;        
      break;

    case CMD_DEBUG: 
      Settings.Debug=EventToExecute->Par1;
      break;

    case CMD_RAWSIGNAL_RECEIVE: 
      Settings.RawSignalReceive=EventToExecute->Par1;
      break;

    case CMD_RAWSIGNAL_SAVE: 
      Settings.RawSignalSave=EventToExecute->Par1;
      break;

    case CMD_RAWSIGNAL_ERASE: 
      if(EventToExecute->Par2)
        {
        sprintf(TempString,"%s/%s.raw",ProgmemString(Text_28),int2strhex(EventToExecute->Par2)+2); // +2 om zo de "0x" van de string te strippen.
        FileErase(TempString);
        }
      else
        FileList("/RAW",true);
      break;

    case CMD_RAWSIGNAL_SEND: 
      if(RawSignalLoad(EventToExecute->Par2))
        {
        ClearEvent(&TempEvent);
        TempEvent.Port=VALUE_ALL;
        TempEvent.Command=CMD_RAWSIGNAL;
        TempEvent.Par2=EventToExecute->Par2;
        TempEvent.Par1=EventToExecute->Par1;
        RawSignal.Repeats=5;
        //repeats zetten / en de poorten correct/???@1
        SendEvent(&TempEvent, true ,true, false);
        }
      else
        error=MESSAGE_03;

      break;

    case CMD_LOG: 
      Settings.Log=EventToExecute->Par1;
      break;

//    case CMD_SIMULATE_DAY: // ??? gooien we dit commando weg?
//      SimulateDay(); 
//      break;     
      
    case CMD_FILE_EXECUTE:
      strcpy(TempString,cmd2str(CMD_FILE_EXECUTE));
      strcat(TempString," ");
      strcat(TempString,int2str(EventToExecute->Par1));
      ExecuteLine(TempString,EventToExecute->Port);
      break;      
              
    #endif    
    }

  #ifdef NODO_MEGA
  free(TempString);
  #endif

  return error;
  }


