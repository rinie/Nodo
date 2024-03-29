//#######################################################################################################
//##################################### Misc: EEPROM / Eventlist  #######################################
//#######################################################################################################


 // Niet alle gegevens uit een event zijn relevant. Om ruimte in EEPROM te besparen worden uitsluitend
 // de alleen noodzakelijke gegevens in EEPROM opgeslagen. Hiervoor een struct vullen die later als
 // één blok weggeschreven kan worden.

 struct EventlistStruct
   {
   byte EventCommand;
   byte EventPar1;
   unsigned long EventPar2;
   
   byte ActionCommand;
   byte ActionPar1;
   unsigned long ActionPar2;
   }EEPROM_Block;


/**********************************************************************************************\
 * Schrijft een event in de Eventlist. Deze Eventlist bevindt zich in het EEPROM geheugen.
 \*********************************************************************************************/
boolean Eventlist_Write(int Line, struct NodoEventStruct *Event, struct NodoEventStruct *Action)// LET OP: Gebruikers input. Eerste adres=1
  {
  struct EventlistStruct EEPROM_Block;
  struct NodoEventStruct dummy;
  int x,address;
  
  // als opgegeven adres=0, zoek dan de eerste vrije plaats.
  if(Line==0)
    {
    Line++;
    while(Eventlist_Read(Line,&dummy,&dummy) && dummy.Command!=0)Line++;
    }
  Line--;                                                                          // echte adressering begint vanaf nul. voor de user vanaf 1.

  if(Line>EVENTLIST_MAX)
    return false;

  address=Line * sizeof(struct EventlistStruct) + sizeof(struct SettingsStruct);     // Eerste deel van het EEPROM geheugen is voor de settings. Reserveer deze bytes. Deze niet te gebruiken voor de Eventlist!
  byte *B=(byte*)&EEPROM_Block;                                                       // B wijst naar de eerste byte van de struct

  // Nu wegschrijven.
  address=Line * sizeof(struct EventlistStruct) + sizeof(struct SettingsStruct);     // Eerste deel van het EEPROM geheugen is voor de settings. Reserveer deze bytes. Deze niet te gebruiken voor de Eventlist!
  EEPROM_Block.EventCommand=Event->Command;
  EEPROM_Block.EventPar1=Event->Par1;
  EEPROM_Block.EventPar2=Event->Par2;
 
  EEPROM_Block.ActionCommand=Action->Command;
  EEPROM_Block.ActionPar1=Action->Par1;
  EEPROM_Block.ActionPar2=Action->Par2;

  for(x=0;x<sizeof(struct EventlistStruct);x++) // schrijf alle bytes van de struct
    {
    if(address<EEPROM_SIZE)
      EEPROM.write(address++, *(B+x));
    else
      return false;
    }
  return true;
  }

/**********************************************************************************************\
 * 
 * Revision 01, 09-12-2009, P.K.Tonkes@gmail.com
 \*********************************************************************************************/
boolean Eventlist_Read(int address, struct NodoEventStruct *Event, struct NodoEventStruct *Action)// LET OP: eerste adres=1
  {
  struct EventlistStruct EEPROM_Block;

  if(address>EVENTLIST_MAX)
    return false;

  address--;// echte adressering begint vanaf nul. voor de user vanaf 1.
  address=address*sizeof(struct EventlistStruct) + sizeof(struct SettingsStruct);     // Eerste deel van het EEPROM geheugen is voor de settings. Reserveer deze bytes. Deze niet te gebruiken voor de Eventlist!
  byte *B=(byte*)&EEPROM_Block; // B wijst naar de eerste byte van de struct

  for(int x=0;x<sizeof(struct EventlistStruct);x++) // lees alle bytes van de struct
    {
    if(address<EEPROM_SIZE)
      *(B+x)=EEPROM.read(address++);
    else
      return false;
    }

  ClearEvent(Event);
  Event->Command=EEPROM_Block.EventCommand;
  Event->Par1=EEPROM_Block.EventPar1;
  Event->Par2=EEPROM_Block.EventPar2;
  
  ClearEvent(Action);
  Action->Command=EEPROM_Block.ActionCommand;
  Action->Par1=EEPROM_Block.ActionPar1;
  Action->Par2=EEPROM_Block.ActionPar2;
  Action->Port=VALUE_SOURCE_EVENTLIST;
  Action->Direction=VALUE_DIRECTION_INPUT;
  return true;
  }


void RaiseMessage(byte MessageCode)
  {
  if(MessageCode==0)
    return;

  struct NodoEventStruct TempEvent;
  ClearEvent(&TempEvent);
  TempEvent.Command   = CMD_MESSAGE;
  TempEvent.Par1      = MessageCode;
  TempEvent.Direction = VALUE_DIRECTION_INPUT;
  TempEvent.Port      = VALUE_SOURCE_SYSTEM;
  PrintEvent(&TempEvent);

  if(MessageCode==MESSAGE_09)// Stop
    return;

  TempEvent.Port      = VALUE_ALL;
  SendEvent(&TempEvent,false,true,true);
  }


/**********************************************************************************************\
 * Geef een geluidssignaal met toonhoogte van 'frequentie' in Herz en tijdsduur 'duration' in milliseconden.
 * LET OP: toonhoogte is slechts een grove indicatie. Deze routine is bedoeld als signaalfunctie
 * en is niet bruikbaar voor toepassingen waar de toonhoogte zuiver/exact moet zijn. Geen PWM.
 * Definieer de constante:
 * #define PIN_SPEAKER <LuidsprekerAansluiting>
 * Neem in setup() de volgende regel op:
 * pinMode(PIN_SPEAKER, OUTPUT);
 * Routine wordt verlaten na beeindiging van de pieptoon.
 * Revision 01, 13-02-2009, P.K.Tonkes@gmail.com
 \*********************************************************************************************/

void Beep(int frequency, int duration)//Herz,millisec 
{
  long halfperiod=500000L/frequency;
  long loops=(long)duration*frequency/(long)1000;

  for(loops;loops>0;loops--) 
  {
    digitalWrite(PIN_SPEAKER, HIGH);
    delayMicroseconds(halfperiod);
    digitalWrite(PIN_SPEAKER, LOW);
    delayMicroseconds(halfperiod);
  }
}

/**********************************************************************************************\
 * Geeft een belsignaal.
 * Revision 01, 09-03-2009, P.K.Tonkes@gmail.com
 \*********************************************************************************************/
void Alarm(int Variant,int Option)
{
  byte x,y;

  switch (Variant)
  { 
  case 1:// four beeps
    for(y=1;y<=(Option>1?Option:1);y++)
    {
      Beep(3000,30);
      delay(100);
      Beep(3000,30);
      delay(100);
      Beep(3000,30);
      delay(100);
      Beep(3000,30);
      delay(1000);
    }    
    break;

  case 2: // whoop up
    for(y=1;y<=(Option>1?Option:1);y++)
    {
      for(x=1;x<=50;x++)
        Beep(250*x/4,20);
    }          
    break;

  case 3: // whoop down
    for(y=1;y<=(Option>1?Option:1);y++)
    {
      for(x=50;x>0;x--)
        Beep(250*x/4,20);
    }          
    break;

  case 4:// Settings.O.Settings.
    for(y=1;y<=(Option>1?Option:1);y++)
    {
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      delay(200);
      Beep(1200,300);
      delay(100);
      Beep(1200,300);
      delay(100);
      Beep(1200,300);
      delay(200);
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      if(Option>1)delay(500);
    }
    break;

  case 5:// ding-dong
    for(x=1;x<=(Option>1?Option:1);x++)
    {
      if(x>1)delay(2000);
      Beep(1500,500);
      Beep(1200,500);
    }    
    break;

  case 6: // phone ring
    for(x=1;x<(15*(Option>1?Option:1));x++)
    {
      Beep(1000,40);
      Beep(750,40);
    }
    break;

  case 7: // boot
    Beep(1500,100);
    Beep(1000,100);
    break;

  default:// beep
    if(Variant==0)
      Variant=5; // tijdsduur

    if(Option==0)
      Option=20; // toonhoogte

    Beep(100*Option,Variant*10);
    break;
  }
}


/**********************************************************************************************\
 * Stuur de RGB-led.
 *
 * Voor de Nodo geldt:
 *
 * Groen = Nodo in rust en wacht op een event.
 * Rood = Nodo verwerkt event of commando.
 * Blauw = Bijzondere modus Nodo waarin Nodo niet in staat is om events te ontvangen of genereren.
 \*********************************************************************************************/
void Led(byte Color)
{
#ifdef NODO_MEGA
  digitalWrite(PIN_LED_RGB_R,Color==RED);
  digitalWrite(PIN_LED_RGB_B,Color==BLUE);
  digitalWrite(PIN_LED_RGB_G,Color==GREEN);
#else
  digitalWrite(PIN_LED_RGB_R,(Color==RED || Color==BLUE));
#endif
}


/*********************************************************************************************\
 * Wachtloop. Als <EventsInQueue>=true dan worden voorbijkomende events in de queue geplaatst
 *
 * wachtloop die wordt afgebroken als:
 * - <Timeout> seconden zijn voorbij. In dit geval geeft deze funktie een <false> terug.
 * - Het opgegeven event <WaitForEvent> voorbij is gekomen
 * - De ether weer is vrijgegeven voor Nodo communicatie (WaitForFreeTransmission=true)
 * - Er is een event opgevangen waar de TRANSMISSION_SEQUENCE vlag NIET staat.
 \*********************************************************************************************/
boolean Wait(int Timeout, boolean WaitForFreeTransmission, struct NodoEventStruct *WaitForEvent, boolean EndSequence)
  {
  unsigned long TimeoutTimer=millis() + (unsigned long)(Timeout)*1000;

  #ifdef NODO_MEGA
  unsigned long MessageTimer=millis() + 5000;
  boolean WaitMessage=false;
  #endif
  
  // Initialiseer een Event en Transmissie
  struct NodoEventStruct Event;
  ClearEvent(&Event);

  Led(BLUE);

  while(TimeoutTimer>millis())
    {
    #ifdef NODO_MEGA
    if(!WaitMessage && MessageTimer<millis())
      {
      WaitMessage=true;
      PrintTerminal(ProgmemString(Text_07));
      }
    #endif
      
    if(ScanEvent(&Event))
      {
      // Events voor deze Nodo kunnen NU niet worden verwerkt. Plaats daarom in de queue
      QueueAdd(&Event);
      if(EndSequence && (Event.Flags&TRANSMISSION_NEXT)==0)
        break;
        
      // als het gewacht wordt totdat de communicatie poorten weer beschikbaar zijn, dan wachtloop verlaten.        
      if(WaitForFreeTransmission && (Transmission_SelectedUnit==0 || Transmission_SelectedUnit==Settings.Unit))
        break;
      
      // break af als opgegeven event voorbij komt. Let op, alleen events met als bestemming 0 of dit unitnummer worden gedetecteerd!
      // De check vindt alleen plaats Command en Unit, dus niet op Par1 en Par2.
      // Als SourceUnit==0 dan wordt input van alle units geaccepteerd.
      if(WaitForEvent!=0)
        {
        if(WaitForEvent->Command==Event.Command)
          {
          if(WaitForEvent->SourceUnit==Event.SourceUnit || WaitForEvent->SourceUnit==0)
            {
            *WaitForEvent=Event;
            break;
            }
          }
        }
      }
    }   
    
  // als timeout, dan error terug geven
  if(TimeoutTimer<=millis())
    return false;
  else
    return true;
  }


/*********************************************************************************************\
 * (re)set een timer. Eenheden in seconden. 
 * Timer 1..15. Timer=0 is een wildcard voor alle timers
 * Als de timer op 0 wordt gezet, wordt er geen event gegenereerd.
 \*********************************************************************************************/
void TimerSet(byte Timer, unsigned long Time)
  {
  if(Timer==0)// 0=wildcard voor alle timers
    {
    for(int x=0;x<TIMER_MAX;x++)
      {
      if(Time==0)
        UserTimer[x]=0L;
      else
        UserTimer[x]=millis()+(unsigned long)(Time)*1000L;
      }
    }
  else if(Timer<=TIMER_MAX)
    {
    if(Time==0)
      UserTimer[Timer-1]=0L;
    else
      UserTimer[Timer-1]=millis()+(unsigned long)Time*1000L;
    }
  }

/*********************************************************************************************\
 * Haal voor het opgegeven Command de status op door resultaat in de event struct te plaatsen.
 * Geef een false terug als commando niet bestaat.
 \*********************************************************************************************/
boolean GetStatus(struct NodoEventStruct *Event)
  { 
  int x;

  // bewaar de opvraag gegevens en begin met een schoon event
  byte xPar1=Event->Par1;
  byte xCommand=Event->Command;  
  ClearEvent(Event);

  Event->Flags|=TRANSMISSION_EVENT; // forceer dat deze wordt behandeld als een event
  Event->Command=xCommand;
  
  switch (xCommand)
    {
    case VALUE_EVENTLIST_COUNT:
      x=0;
      struct NodoEventStruct dummy;
      while(Eventlist_Read(++x,&dummy,&dummy))
        {
        if(dummy.Command)
          Event->Par1=x;
        }
      Event->Par2=x-Event->Par1;
        
      break;

  case CMD_WAITFREERF: 
    Event->Par1=Settings.WaitFree;
    break;

  case VALUE_BUILD:
    Event->Par2=NODO_BUILD;      
    break;        

  case VALUE_HWCONFIG: 
    Event->Par2=HW_Config;      
    break;        

  case CMD_DEBUG:
    Event->Par1=Settings.Debug;
    break;

  case CMD_RAWSIGNAL_RECEIVE:
    Event->Par1=Settings.RawSignalReceive;
    break;

  case CMD_CLOCK_EVENT_DAYLIGHT:
    Event->Par1=Time.Daylight;
    break;

  case CMD_OUTPUT:
    Event->Par1=xPar1;
    switch(xPar1)
      {
      case VALUE_SOURCE_IR:
        Event->Par2=Settings.TransmitIR;
        break;
  
      case VALUE_SOURCE_RF:
        Event->Par2=Settings.TransmitRF;
        break;
  
  #ifdef NODO_MEGA
      case VALUE_SOURCE_HTTP:
        Event->Par2=Settings.TransmitIP;
        break;
  #endif 
      }
    break;

  case CMD_VARIABLE_SET:
    Event->Par1=xPar1;
    Event->Par2=float2ul(UserVar[xPar1-1]);
    break;

  case CMD_CLOCK_DATE:
    if(!bitRead(HW_Config,HW_CLOCK))return false;
    Event->Par1=Time.Date;
    Event->Par2=Time.Month;
    break;

  case CMD_CLOCK_TIME:
    if(!bitRead(HW_Config,HW_CLOCK))return false;
    Event->Par1=Time.Hour;
    Event->Par2=Time.Minutes;
    break;

  case CMD_CLOCK_DOW:
    if(!bitRead(HW_Config,HW_CLOCK))return false;
    Event->Par1=Time.Day;
    break;

  case CMD_CLOCK_YEAR:
    if(!bitRead(HW_Config,HW_CLOCK))return false;
    Event->Par1=Time.Year/100;
    Event->Par2=Time.Year-2000;
    break;

  case CMD_TIMER_SET:
    Event->Par1=xPar1;
    if(UserTimer[xPar1-1])
      Event->Par2=(UserTimer[xPar1-1]-millis())/1000;
    else
      Event->Par2=0;
    break;

  case CMD_WIRED_PULLUP:
    Event->Par1=xPar1;
    Event->Par2=Settings.WiredInputPullUp[xPar1-1];
    break;

  case CMD_WIRED_ANALOG:
    Event->Par1=xPar1;
    Event->Par2=analogRead(PIN_WIRED_IN_1+xPar1-1);
    break;

  case CMD_WIRED_THRESHOLD:
    Event->Par1=xPar1;
    Event->Par2=Settings.WiredInputThreshold[xPar1-1];
    break;

  case CMD_WIRED_SMITTTRIGGER:
    Event->Par1=xPar1;
    Event->Par2=Settings.WiredInputSmittTrigger[xPar1-1];
    break;

  case VALUE_FREEMEM:    
    Event->Par2=FreeMem();
    break;

  case CMD_WIRED_IN_EVENT:
    Event->Par1=xPar1;
    Event->Par2=(WiredInputStatus[xPar1-1])?VALUE_ON:VALUE_OFF;
    break;

  case CMD_WIRED_OUT:
    Event->Par1=xPar1;
    Event->Par2=(WiredOutputStatus[xPar1-1])?VALUE_ON:VALUE_OFF;
    break;

  case CMD_LOCK:
    Event->Par1=Settings.Lock?VALUE_ON:VALUE_OFF;;
    break;

#ifdef NODO_MEGA

  case CMD_UNIT_SET:
    x=NodoOnline(xPar1,0);
    if(x!=0)
      {
      Event->Par1=xPar1;
      Event->Par2=x;
      }
    else
      Event->Command=0;// Als resultaat niet geldig is en niet weergegeven mag worden
    
    break;

  case CMD_ALARM_SET:
    Event->Par1=xPar1;
    Event->Par2=Settings.Alarm[xPar1-1];
    break;

  case CMD_PORT_SERVER:
    Event->Par2=Settings.OutputPort;
    break;

  case CMD_PORT_CLIENT:
    Event->Par2=Settings.PortClient;
    break;

  case CMD_LOG:
    Event->Par1=Settings.Log;
    break;

  case CMD_RAWSIGNAL_SAVE:
    Event->Par1=Settings.RawSignalSave;
    break;

    // pro-forma de commando's die geen fout op mogen leveren omdat deze elders in de statusafhandeling worden weergegeven
  case CMD_NODO_IP:
  case CMD_GATEWAY:
  case CMD_SUBNET:
  case CMD_DNS_SERVER:
  case CMD_HTTP_REQUEST:
  case CMD_ID:
  case CMD_TEMP:
    break;
#endif      
  default:
    return false;
  }
  return true;
}


/**********************************************************************************************\
 * Deze functie haalt een tekst op uit PROGMEM en geeft als string terug
 \*********************************************************************************************/
char* ProgmemString(prog_char* text)
{
  byte x=0;
  static char buffer[90];

  do
  {
    buffer[x]=pgm_read_byte_near(text+x);
  }
  while(buffer[x++]!=0);
  return buffer;
}


/*********************************************************************************************\
 * Sla alle settings op in het EEPROM geheugen.
 \*********************************************************************************************/
void Save_Settings(void)  
{
  Led(BLUE);
  char ByteToSave,*pointerToByteToSave=pointerToByteToSave=(char*)&Settings;    //pointer verwijst nu naar startadres van de struct. 

  for(int x=0; x<sizeof(struct SettingsStruct) ;x++)
  {
    EEPROM.write(x,*pointerToByteToSave); 
    pointerToByteToSave++;
  }  
}

/*********************************************************************************************\
 * Laad de settings uit het EEPROM geheugen.
 \*********************************************************************************************/
boolean LoadSettings()
 {
  byte x;

  char ByteToSave,*pointerToByteToRead=(char*)&Settings;    //pointer verwijst nu naar startadres van de struct.

  for(int x=0; x<sizeof(struct SettingsStruct);x++)
    {
    *pointerToByteToRead=EEPROM.read(x);
    pointerToByteToRead++;// volgende byte uit de struct
    }
  }


/*********************************************************************************************\
 * Alle settings van de Nodo weer op default.
 \*********************************************************************************************/
void ResetFactory(void)
  {
  int x,y;
  Led(BLUE);
  Beep(2000,2000);

  // maak de eventlist leeg.
  struct NodoEventStruct dummy;
  ClearEvent(&dummy);
  x=1;
  while(Eventlist_Write(x++,&dummy,&dummy));

  // Herstel alle settings naar defaults
  Settings.Version                    = SETTINGS_VERSION;
  Settings.NewNodo                    = true;
  Settings.Lock                       = 0;
  Settings.TransmitIR                 = VALUE_ON;
  Settings.TransmitRF                 = VALUE_ON;
  Settings.Unit                       = UNIT_NODO;
  Settings.Home                       = HOME_NODO;
  Settings.WaitFree                   = VALUE_ON;
  Settings.RawSignalReceive           = VALUE_ON;

#ifdef NODO_MEGA
  Settings.WaitFree                   = VALUE_OFF;
  Settings.TransmitIP                 = VALUE_OFF;
  Settings.Debug                      = VALUE_OFF;
  Settings.HTTPRequest[0]             = 0; // string van het HTTP adres leeg maken
  Settings.Client_IP[0]               = 0;
  Settings.Client_IP[1]               = 0;
  Settings.Client_IP[2]               = 0;
  Settings.Client_IP[3]               = 0;
  Settings.Nodo_IP[0]                 = 0;
  Settings.Nodo_IP[1]                 = 0;
  Settings.Nodo_IP[2]                 = 0;
  Settings.Nodo_IP[3]                 = 0;
  Settings.Gateway[0]                 = 0;
  Settings.Gateway[1]                 = 0;
  Settings.Gateway[2]                 = 0;
  Settings.Gateway[3]                 = 0;
  Settings.Subnet[0]                  = 255;
  Settings.Subnet[1]                  = 255;
  Settings.Subnet[2]                  = 255;
  Settings.Subnet[3]                  = 0;
  Settings.DnsServer[0]               = 0;
  Settings.DnsServer[1]               = 0;
  Settings.DnsServer[2]               = 0;
  Settings.DnsServer[3]               = 0;
  Settings.OutputPort                 = 6636;
  Settings.PortClient                 = 80;
  Settings.ID[0]                      = 0; // string leegmaken
  Settings.Temp[0]                    = 0; // string leegmaken
  Settings.EchoSerial                 = VALUE_ON;
  Settings.EchoTelnet                 = VALUE_OFF;  
  Settings.Log                        = VALUE_OFF;  
  Settings.RawSignalSave              = VALUE_OFF;  
  Settings.Password[0]                = 0;

  // Maak de alarmen leeg
  for(x=0;x<ALARM_MAX;x++)
    Settings.Alarm[x]=0L;

#endif

  // zet analoge waarden op default
  for(x=0;x<WIRED_PORTS;x++)
    {
    Settings.WiredInputThreshold[x]=512; 
    Settings.WiredInputSmittTrigger[x]=10;
    Settings.WiredInputPullUp[x]=VALUE_ON;
    }

  Save_Settings();

  RebootNodo=true;
  }

/**********************************************************************************************\
 * Geeft de status weer of genereert een event.
 * Par1 = Command
 \**********************************************************************************************/
void Status(struct NodoEventStruct *Request)
  {
  byte CMD_Start,CMD_End;
  byte Par1_Start,Par1_End;
  byte x;
  boolean s;
  boolean Display=Request->Port==VALUE_SOURCE_SERIAL || Request->Port==VALUE_SOURCE_TELNET || Request->Port==VALUE_SOURCE_HTTP|| Request->Port==VALUE_SOURCE_FILE;
  
  struct NodoEventStruct Result;
  ClearEvent(&Result);
  Result.Command=Request->Par1;
  
  #ifdef NODO_MEGA          
  char *TempString=(char*)malloc(INPUT_BUFFER_SIZE+1);
  #endif

  if(Request->Par2==VALUE_ALL)
    Request->Par2==0;

  if(Request->Par1==CMD_BOOT_EVENT || Request->Par1==0)
    {
    PrintWelcome();
    return;
    }

  if(Display && Request->Par1==VALUE_ALL)
    {
    Request->Par2=0;
    PrintWelcome();
    CMD_Start=FIRST_COMMAND;
    CMD_End=COMMAND_MAX;
    }
  else
    {
    Result.Command=Request->Par1;
    if(!GetStatus(&Result))// kijk of voor de opgegeven parameter de status opvraagbaar is. Zo niet dan klaar.
      return;
    CMD_Start=Request->Par1;
    CMD_End=Request->Par1;
    }

#ifdef NODO_MEGA          
  boolean dhcp=(Settings.Nodo_IP[0] + Settings.Nodo_IP[1] + Settings.Nodo_IP[2] + Settings.Nodo_IP[3])==0;
#endif

  for(x=CMD_Start; x<=CMD_End; x++)
    {
    s=false;
    if(Display)
      {
      s=true;
      switch (x)
        {
        #ifdef NODO_MEGA          
        case CMD_CLIENT_IP:
          sprintf(TempString,"%s %u.%u.%u.%u",cmd2str(CMD_CLIENT_IP),Settings.Client_IP[0],Settings.Client_IP[1],Settings.Client_IP[2],Settings.Client_IP[3]);
          PrintTerminal(TempString);
          break;

        case CMD_NODO_IP:
          sprintf(TempString,"%s %u.%u.%u.%u",cmd2str(CMD_NODO_IP), Ethernet.localIP()[0],Ethernet.localIP()[1],Ethernet.localIP()[2],Ethernet.localIP()[3]);
          if(dhcp)
            strcat(TempString,"(DHCP)");
          PrintTerminal(TempString);
          break;
  
        case CMD_GATEWAY:
          // Gateway
          if(!dhcp)
            {
            sprintf(TempString,"%s %u.%u.%u.%u",cmd2str(CMD_GATEWAY),Settings.Gateway[0],Settings.Gateway[1],Settings.Gateway[2],Settings.Gateway[3]);
            PrintTerminal(TempString);
            }
          break;
  
        case CMD_SUBNET:
          // Subnetmask
          if(!dhcp)
            {
            sprintf(TempString,"%s %u.%u.%u.%u",cmd2str(CMD_SUBNET),Settings.Subnet[0],Settings.Subnet[1],Settings.Subnet[2],Settings.Subnet[3]);
            PrintTerminal(TempString);
            }
          break;
  
        case CMD_DNS_SERVER:
          if(!dhcp)
            {
            // DnsServer
            sprintf(TempString,"%s %u.%u.%u.%u",cmd2str(CMD_DNS_SERVER),Settings.DnsServer[0],Settings.DnsServer[1],Settings.DnsServer[2],Settings.DnsServer[3]);
            PrintTerminal(TempString);
            }
          break;
  
        case CMD_PORT_SERVER:
          sprintf(TempString,"%s %d",cmd2str(CMD_PORT_SERVER), Settings.OutputPort);
          PrintTerminal(TempString);
          break;
  
        case CMD_PORT_CLIENT:
          sprintf(TempString,"%s %d",cmd2str(CMD_PORT_CLIENT), Settings.PortClient);
          PrintTerminal(TempString);
          break;
  
        case CMD_HTTP_REQUEST:
          sprintf(TempString,"%s %s",cmd2str(CMD_HTTP_REQUEST),Settings.HTTPRequest);
          PrintTerminal(TempString);
          break;
  
        case CMD_ID:
          sprintf(TempString,"%s %s",cmd2str(CMD_ID), Settings.ID);
          PrintTerminal(TempString);
          break;
  
        case CMD_TEMP:
          sprintf(TempString,"%s %s",cmd2str(CMD_TEMP), Settings.Temp);
          PrintTerminal(TempString);
          break;
  
  #endif
  
        default:
          s=false; 
          break;
        }
      }

    Result.Command=x;
    Result.Par1=Request->Par1;    
    
    if(!s && GetStatus(&Result)) // Als het een geldige uitvraag is.
      {
      if(Request->Par2==0) // Als in het commando 'Status Par1, Par2' Par2 niet is gevuld met een waarde
        {
        switch(x)
          {
          case CMD_OUTPUT:
            Par1_Start=VALUE_SOURCE_IR;
            #ifdef NODO_MEGA
            Par1_End=VALUE_SOURCE_HTTP;
            #else
            Par1_End=VALUE_SOURCE_RF;
            #endif
            break;
  
          case CMD_WIRED_ANALOG:
          case CMD_WIRED_OUT:
          case CMD_WIRED_PULLUP:
          case CMD_WIRED_SMITTTRIGGER:
          case CMD_WIRED_THRESHOLD:
          case CMD_WIRED_IN_EVENT:
            Par1_Start=1;
            Par1_End=WIRED_PORTS;
            break;      
  
          case CMD_VARIABLE_SET:
            Par1_Start=1;
            Par1_End=USER_VARIABLES_MAX;
            break;
    
          case CMD_TIMER_SET:
            Par1_Start=1;
            Par1_End=TIMER_MAX;
            break;
  
          #ifdef NODO_MEGA
          case CMD_ALARM_SET:
            Par1_Start=1;
            Par1_End=ALARM_MAX;
            break;

          case CMD_UNIT_SET:
            Par1_Start=1;
            Par1_End=UNIT_MAX;
            break;
          #endif

          default:
            Par1_Start=0;
            Par1_End=0;
          }
        }
      else
        {
        Par1_Start=Request->Par2;
        Par1_End=Request->Par2;
        }

      for(byte y=Par1_Start;y<=Par1_End;y++)
        {
        Result.Command=x;
        Result.Par1=y;
        GetStatus(&Result); 
        
        if(Result.Command!=0)
          {
          if(!Display)
            {
            if(Request->Port==VALUE_SOURCE_EVENTLIST)            
              Result.Port=VALUE_ALL;
            else
              Result.Port=Request->Port;
            SendEvent(&Result,false,true,true); // verzend als event
            }
  
          #ifdef NODO_MEGA
          else
            {
            Event2str(&Result,TempString);
            PrintTerminal(TempString); // geef weer op terminal
            }
          #endif
          }
        }
      }
    }

  #ifdef NODO_MEGA
  if(Display && Request->Par1==VALUE_ALL)
    PrintTerminal(ProgmemString(Text_22));

  free(TempString);
  #endif
  }


#ifdef NODO_MEGA
/*********************************************************************************************\
 * Deze routine parsed string en geeft het opgegeven argument nummer Argc terug in Argv
 * argumenten worden van elkaar gescheiden door een komma of een spatie.
 * Let op dat de ruimte in de doelstring voldoende is EN dat de bron string netjes is afgesloten 
 * met een 0-byte.
 \*********************************************************************************************/
boolean GetArgv(char *string, char *argv, int argc)
{
  int string_pos=0,argv_pos=0,argc_pos=0; 
  char c,d;

  while(string_pos<strlen(string))
  {
    c=string[string_pos];
    d=string[string_pos+1];

    // dit is niet meer de handigste methode. Op termijn vereenvoudigen.???
    if       (c==' ' && d==' '){}
    else if  (c==' ' && d==','){}
    else if  (c==',' && d==' '){}
    else if  (c==' ' && d>=33 && d<=126){}
    else if  (c==',' && d>=33 && d<=126){}
    else 
      {
      argv[argv_pos++]=c;
      argv[argv_pos]=0;          

      if(d==' ' || d==',' || d==0)
        {
        // Bezig met toevoegen van tekens aan een argument, maar er kwam een scheidingsteken.
        argv[argv_pos]=0;
        argc_pos++;

        if(argc_pos==argc)
          {
          return true;
          }
          
        argv[0]=0;
        argv_pos=0;
        string_pos++;
      }
    }
    string_pos++;
  }
  return false;
}


/*********************************************************************************************\
 * Deze routine parsed string en zoekt naar keyword. Geeft de startpositie terug waar het keyword
 * gevonden is. -1 indien niet gevonden. Niet casesensitive.
 \*********************************************************************************************/
int StringFind(char *string, char *keyword)
{
  int x,y;
  int keyword_len=strlen(keyword);
  int string_len=strlen(string);

  if(keyword_len>string_len) // doe geen moeite als het te vinden eyword langer is dan de string.
    return -1;

  for(x=0; x<=(string_len-keyword_len); x++)
  {
    y=0;
    while(y<keyword_len && (tolower(string[x+y])==tolower(keyword[y])))
      y++;

    if(y==keyword_len)
      return x;
  }
  return -1;
}


/**********************************************************************************************\
 * Geeft een string terug met een cookie op basis van een random HEX-waarde van 32-bit.
 \*********************************************************************************************/
void RandomCookie(char* Ck)
  {
  byte  x,y;

  for(x=0;x<8;x++)
    {
    y=random(0x0, 0xf);
    Ck[x]=y<10?(y+'0'):((y-10)+'A');
    }
  Ck[8]=0; // afsluiten string
  }




//#######################################################################################################
//######################################### Misc: MD5      ##############################################
//#######################################################################################################


uint32_t md5_T[] PROGMEM = {
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf,
  0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af,
  0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e,
  0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
  0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6,
  0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
  0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122,
  0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
  0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039,
  0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244, 0x432aff97,
  0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d,
  0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
  0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };


#define MD5_HASH_BITS  128
#define MD5_HASH_BYTES (MD5_HASH_BITS/8)
#define MD5_BLOCK_BITS 512
#define MD5_BLOCK_BYTES (MD5_BLOCK_BITS/8)
#define MD5_HASH_BITS  128

struct md5_ctx_t 
{
  uint32_t a[4];
  int32_t counter;
};

// typedef uint8_t md5_hash_t[MD5_HASH_BYTES];

void md5_init(struct md5_ctx_t *s){
  s->counter = 0;
  s->a[0] = 0x67452301;
  s->a[1] = 0xefcdab89;
  s->a[2] = 0x98badcfe;
  s->a[3] = 0x10325476;
}

static
uint32_t md5_F(uint32_t x, uint32_t y, uint32_t z){
  return ((x&y)|((~x)&z));
}

static
uint32_t md5_G(uint32_t x, uint32_t y, uint32_t z){
  return ((x&z)|((~z)&y));
}

static
uint32_t md5_H(uint32_t x, uint32_t y, uint32_t z){
  return (x^y^z);
}

static
uint32_t md5_I(uint32_t x, uint32_t y, uint32_t z){
  return (y ^ (x | (~z)));
}

typedef uint32_t md5_func_t(uint32_t, uint32_t, uint32_t);

#define ROTL32(x,n) (((x)<<(n)) | ((x)>>(32-(n))))  

static void md5_core(uint32_t* a, void* block, uint8_t as, uint8_t s, uint8_t i, uint8_t fi){
  uint32_t t;
  md5_func_t* funcs[]={
    md5_F, md5_G, md5_H, md5_I      };
  as &= 0x3;
  /* a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */
  t = a[as] + funcs[fi](a[(as+1)&3], a[(as+2)&3], a[(as+3)&3])
    + *((uint32_t*)block) + pgm_read_dword(md5_T+i) ;
  a[as]=a[(as+1)&3] + ROTL32(t, s);
}

void md5_nextBlock(struct md5_ctx_t *state, const void* block){
  uint32_t	a[4];
  uint8_t		m,n,i=0;
  /* this requires other mixed sboxes */
  a[0]=state->a[0];
  a[1]=state->a[1];
  a[2]=state->a[2];
  a[3]=state->a[3];

  /* round 1 */
  uint8_t s1t[]={
    7,12,17,22      }; // 1,-1   1,4   2,-1   3,-2
  for(m=0;m<4;++m){
    for(n=0;n<4;++n){
      md5_core(a, &(((uint32_t*)block)[m*4+n]), 4-n, s1t[n],i++,0);
    }
  }
  /* round 2 */
  uint8_t s2t[]={
    5,9,14,20      }; // 1,-3   1,1   2,-2   2,4
  for(m=0;m<4;++m){
    for(n=0;n<4;++n){
      md5_core(a, &(((uint32_t*)block)[(1+m*4+n*5)&0xf]), 4-n, s2t[n],i++,1);
    }
  }
  /* round 3 */
  uint8_t s3t[]={
    4,11,16,23      }; // 0,4   1,3   2,0   3,-1
  for(m=0;m<4;++m){
    for(n=0;n<4;++n){
      md5_core(a, &(((uint32_t*)block)[(5-m*4+n*3)&0xf]), 4-n, s3t[n],i++,2);
    }
  }
  /* round 4 */
  uint8_t s4t[]={
    6,10,15,21      }; // 1,-2   1,2   2,-1   3,-3
  for(m=0;m<4;++m){
    for(n=0;n<4;++n){
      md5_core(a, &(((uint32_t*)block)[(0-m*4+n*7)&0xf]), 4-n, s4t[n],i++,3);
    }
  }
  state->a[0] += a[0];
  state->a[1] += a[1];
  state->a[2] += a[2];
  state->a[3] += a[3];
  state->counter++;
}

void md5_lastBlock(struct md5_ctx_t *state, const void* block, uint16_t length_b){
  uint16_t l;
  uint8_t b[64];
  while (length_b >= 512){
    md5_nextBlock(state, block);
    length_b -= 512;
    block = ((uint8_t*)block) + 512/8;
  }
  memset(b, 0, 64);
  memcpy(b, block, length_b/8);
  /* insert padding one */
  l=length_b/8;
  if(length_b%8){
    uint8_t t;
    t = ((uint8_t*)block)[l];
    t |= (0x80>>(length_b%8));
    b[l]=t;
  }
  else{
    b[l]=0x80;
  }
  /* insert length value */
  if(l+sizeof(uint64_t) >= 512/8){
    md5_nextBlock(state, b);
    state->counter--;
    memset(b, 0, 64-8);
  }
  *((uint64_t*)&b[64-sizeof(uint64_t)]) = (state->counter * 512) + length_b;
  md5_nextBlock(state, b);
}

void md5_ctx2hash(struct md5_hash_t* dest, const struct md5_ctx_t* state){
  memcpy(dest, state->a, MD5_HASH_BYTES);
}

void md5(char* dest)
{
  const void* src=dest;
  uint32_t length_b = strlen((char*)src) * 8;
  struct md5_ctx_t ctx;
  char *Str=(char*)malloc(20);

  md5_init(&ctx);
  while(length_b>=MD5_BLOCK_BITS)
  {
    md5_nextBlock(&ctx, src);
    src = (uint8_t*)src + MD5_BLOCK_BYTES;
    length_b -= MD5_BLOCK_BITS;
  }
  md5_lastBlock(&ctx, src, length_b);
  md5_ctx2hash((md5_hash_t*)&MD5HashCode, &ctx);

  strcpy(Str,PROGMEM2str(Text_05));              
  int y=0;
  for(int x=0; x<16; x++)
  {
    dest[y++]=Str[MD5HashCode[x]>>4  ];
    dest[y++]=Str[MD5HashCode[x]&0x0f];
  }
  dest[y]=0;

  free(Str);
}

#endif

uint8_t *heapptr, *stackptr;

unsigned long FreeMem(void)
  {
  stackptr = (uint8_t *)malloc(4);        // use stackptr temporarily
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);                         // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);            // save value of stack pointer
  return (stackptr-heapptr);
  }
    
void Trace(char *Func, unsigned long Value)
  {
  stackptr = (uint8_t *)malloc(4);        // use stackptr temporarily
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);                         // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);            // save value of stack pointer
  static unsigned long time=millis();

  if(Func!=0 && Func[0]!=0)
    {
    Serial.print(F("=> Trace: "));
    Serial.print(Func);
    Serial.print(F(", Value="));
    Serial.print(Value);
    Serial.print(F(", dTime="));
    Serial.print(millis()-time);
    Serial.print(F(", FreeMem="));
    Serial.println(stackptr-heapptr);
    delay(100);
    }
  time=millis();
  }

void PulseCounterISR()
  {
  static unsigned long PulseTimePrevious=0L;

  // in deze interrupt service routine staat millis() stil. Dit is echter geen bezwaar voor de meting.
  PulseTime=millis()-PulseTimePrevious;
  if(PulseTime>=PULSE_DEBOUNCE_TIME)
    PulseCount++;
  else
    PulseTime=0;

  PulseTimePrevious=millis();
  }     


#ifdef NODO_MEGA
//################### Calculate #################################

#define CALCULATE_OK                            0
#define CALCULATE_ERROR_STACK_OVERFLOW          1
#define CALCULATE_ERROR_BAD_OPERATOR            2
#define CALCULATE_ERROR_PARENTHESES_MISMATCHED  3
#define CALCULATE_ERROR_UNKNOWN_TOKEN           4
#define STACK_SIZE 50
#define TOKEN_MAX 20

float stack[STACK_SIZE];
float *sp = stack-1;
float *sp_max = &stack[STACK_SIZE-1];

#define is_operator(c)  (c == '+' || c == '-' || c == '*' || c == '/' )

int push(float value)
{
  if(sp != sp_max) // Full
  {
    *(++sp) = value;
    return 0;
  }
  else 
    return CALCULATE_ERROR_STACK_OVERFLOW;
}

float pop()
{
  if(sp != (stack-1)) // empty
    return *(sp--);
}

float apply_operator(char op, float first, float second)
{
  switch(op)
  {
  case '+': 
    return first + second;
  case '-': 
    return first - second;
  case '*': 
    return first * second;
  case '/': 
    return first / second;
    return 0;
  }  
}

char *next_token(char *linep)
{
  while(isspace(*(linep++)));
  while(*linep && !isspace(*(linep++)));
  return linep;
}

int RPNCalculate(char* token)
{
  if(token[0]==0)
    return 0; // geen moeite doen voor een lege string

  if(is_operator(token[0]))
  {
    float second = pop();
    float first = pop();

    if(push(apply_operator(token[0], first, second)))
      return CALCULATE_ERROR_STACK_OVERFLOW;
  }
  else // Als er nog een is, dan deze ophalen
  if(push(atof(token))) // is het een waarde, dan op de stack plaatsen
      return CALCULATE_ERROR_STACK_OVERFLOW;

  return 0;
}

// operators
// precedence   operators         associativity
// 3            !                 right to left
// 2            * / %             left to right
// 1            + - ^             left to right
int op_preced(const char c)
{
  switch(c)    
  {
  case '*':  
  case '/': 
    return 2;
  case '+': 
  case '-': 
    return 1;
  }
  return 0;
}

bool op_left_assoc(const char c)
{
  switch(c)
  { 
  case '*': 
  case '/': 
  case '+': 
  case '-': 
    return true;     // left to right
    //case '!': return false;    // right to left
  }
  return false;
}

unsigned int op_arg_count(const char c)
{
  switch(c)  
  {
  case '*': 
  case '/': 
  case '+': 
  case '-': 
    return 2;
    //case '!': return 1;
  }
  return 0;
}


int Calculate(const char *input, float* result)
{
  const char *strpos = input, *strend = input + strlen(input);
  char token[25];
  char c, *TokenPos = token;
  char stack[32];       // operator stack
  unsigned int sl = 0;  // stack length
  char     sc;          // used for record stack element
  int error=0;

  *sp=0;

  while(strpos < strend)   
  {
    // read one token from the input stream
    c = *strpos;
    if(c != ' ')
    {
      // If the token is a number (identifier), then add it to the token queue.
      if((c >= '0' && c <= '9') || c=='.')
      {
        *TokenPos = c; 
        ++TokenPos;
      }

      // If the token is an operator, op1, then:
      else if(is_operator(c))
      {
        *(TokenPos)=0;
        error=RPNCalculate(token);
        TokenPos=token;
        if(error)return error;
        while(sl > 0)
        {
          sc = stack[sl - 1];
          // While there is an operator token, op2, at the top of the stack
          // op1 is left-associative and its precedence is less than or equal to that of op2,
          // or op1 has precedence less than that of op2,
          // The differing operator priority decides pop / push
          // If 2 operators have equal priority then associativity decides.
          if(is_operator(sc) && ((op_left_assoc(c) && (op_preced(c) <= op_preced(sc))) || (op_preced(c) < op_preced(sc))))
          {
            // Pop op2 off the stack, onto the token queue;
            *TokenPos = sc; 
            ++TokenPos;
            *(TokenPos)=0;
            error=RPNCalculate(token);
            TokenPos=token; 
            if(error)return error;
            sl--;
          }
          else
            break;
        }
        // push op1 onto the stack.
        stack[sl] = c;
        ++sl;
      }
      // If the token is a left parenthesis, then push it onto the stack.
      else if(c == '(')
      {
        stack[sl] = c;
        ++sl;
      }
      // If the token is a right parenthesis:
      else if(c == ')')
      {
        bool pe = false;
        // Until the token at the top of the stack is a left parenthesis,
        // pop operators off the stack onto the token queue
        while(sl > 0)
        {
          *(TokenPos)=0;
          error=RPNCalculate(token);
          TokenPos=token; 
          if(error)return error;
          sc = stack[sl - 1];
          if(sc == '(')
          {
            pe = true;
            break;
          }
          else  
          {
            *TokenPos = sc; 
            ++TokenPos;
            sl--;
          }
        }
        // If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
        if(!pe)  
          return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

        // Pop the left parenthesis from the stack, but not onto the token queue.
        sl--;

        // If the token at the top of the stack is a function token, pop it onto the token queue.
        if(sl > 0)
          sc = stack[sl - 1];

      }
      else
        return CALCULATE_ERROR_UNKNOWN_TOKEN;
    }
    ++strpos;
  }
  // When there are no more tokens to read:
  // While there are still operator tokens in the stack:
  while(sl > 0)
  {
    sc = stack[sl - 1];
    if(sc == '(' || sc == ')')
      return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

    *(TokenPos)=0;
    error=RPNCalculate(token);
    TokenPos=token; 
    if(error)return error;
    *TokenPos = sc; 
    ++TokenPos;
    --sl;
  }

  *(TokenPos)=0;
  error=RPNCalculate(token);
  TokenPos=token; 
  if(error)
  {
    *result=0;
    return error;
  }  
  *result=*sp;
  return CALCULATE_OK;
}
//################### Einde Calculate #################################

boolean Substitute(char* Input)
{
  boolean Grab=false;
  byte Res;
  byte x;

  char *Output=(char*)malloc(INPUT_BUFFER_SIZE);
  char *TmpStr=(char*)malloc(INPUT_BUFFER_SIZE);
  char *TmpStr2=(char*)malloc(25);
  char* InputPos  = Input;
  char* OutputPos = Output;
  char* TmpStrPos = TmpStr;
  boolean error=false;

  Res=0;
  while(*(InputPos)!=0)// zolang einde van de string Input nog niet bereikt
  {
    if(*InputPos=='%') 
    {
      if(!Grab)
      {
        Grab=true;
        TmpStrPos=TmpStr;
      }
      else
        {
        Grab=false;
        *TmpStrPos=0;// Sluit string af

        // Haal de status van de variabele
        byte Cmd=0;
        GetArgv(TmpStr,TmpStr2,1);
        Cmd=str2cmd(TmpStr2); // commando deel
        TmpStr2[0]=0;

        if(Cmd!=0)
          {
          // Er zijn twee type mogelijk: A)Direct te vullen omdat ze niet met status opvraagbaar zijn, B)Op te vragen met status
          byte Par1=0;
          byte Par2=0;
          Res=2;

          // A)Direct te vullen omdat ze niet met status opvraagbaar zijn
          switch(Cmd)
          {
          case CMD_ID:
            strcpy(TmpStr2,Settings.ID);
            break;    

          case CMD_TEMP:
            strcpy(TmpStr2,Settings.Temp);
            break;    

          case VALUE_THIS_UNIT:
            strcpy(TmpStr2,int2str(Settings.Unit));
            break;    

          case VALUE_UNIT:
            strcpy(TmpStr2,int2str(LastReceived.SourceUnit));
            break;    

          case VALUE_RECEIVED_EVENT:
            strcpy(TmpStr2,cmd2str(LastReceived.Command));
            break;    

          case VALUE_RECEIVED_PAR1:
            strcpy(TmpStr2,cmd2str(LastReceived.Par1));
            break;    

          case VALUE_RECEIVED_PAR2:
            strcpy(TmpStr2,cmd2str(LastReceived.Par2));
            break;    

          default:
            {
            // B) Op te vragen met status  
            if(GetArgv(TmpStr,TmpStr2,2))
              {
              Par1=str2cmd(TmpStr2);
              if(!Par1)
                Par1=str2int(TmpStr2);
              }

            struct NodoEventStruct Temp;
            ClearEvent(&Temp);
            Temp.Par1=Par1;
            Temp.Par2=Par2;
            Temp.Command=Cmd;
            if(GetStatus(&Temp))
              {
              Event2str(&Temp,TmpStr);
              if(!GetArgv(TmpStr,TmpStr2,3)) // Als de waarde niet in de 3e parameter zat...
                GetArgv(TmpStr,TmpStr2,2);   // dan moet hij in de tweede zitten.
              }
            }
          }
          // plak de opgehaalde waarde aan de output string
          for(x=0;x<strlen(TmpStr2);x++)
            *OutputPos++=TmpStr2[x];
        }
      }     
    }
    else if(Grab)
      *TmpStrPos++=*InputPos;// Voeg teken toe aan variabele      
    else
      *OutputPos++=*InputPos;// Voeg teken toe aan outputstring

    InputPos++;  
  }
  *OutputPos=0;// Sluit string af.

  if(TmpStr[0]!=0)
  {
    strcpy(Input,Output);  
    if(Settings.Debug==VALUE_ON)
    {
      Serial.print(F("Substituted: "));
      Serial.println(Input);
    }
    strcpy(Input,Output);  
  }

  if(Grab) // Als % niet correct afgesloten...
    error=true;
  else
  {
    // Nu zijn de formules aan de beurt.
    InputPos  = Input;
    OutputPos = Output;
    TmpStrPos = TmpStr;
    Res=0;

    while(*(InputPos)!=0)// zolang einde van de string Input nog niet bereikt
    {
      if(*InputPos=='#') 
      {
        if(!Grab)
        {
          Grab=true;
          TmpStrPos=TmpStr;
        }
        else
        {
          Grab=false;
          *TmpStrPos=0;// Sluit string af
          float result;
          if(Calculate(TmpStr,&result)==CALCULATE_OK)
          {
            floatToString(TmpStr,result,2,0);
            x=StringFind(TmpStr,".00");          // de overbodige nullen weghalen
            if(x>0)
              TmpStr[x]=0;

            // plak de opgehaalde waarde aan de output string
            for(x=0;x<strlen(TmpStr);x++)
              *OutputPos++=TmpStr[x];

            Res=true;
          }
          else
            Res=false;

        }     
      }
      else if(Grab)
        *TmpStrPos++=*InputPos;// Voeg teken toe aan variabele      
      else
        *OutputPos++=*InputPos;// Voeg teken toe aan outputstring
      InputPos++;  
    }
    *OutputPos=0;// Sluit string af.

    if(Grab) // Als % niet correct afgesloten...
      error=true;

    if(Res && !error)
    {
      strcpy(Input,Output);  
      if(Settings.Debug==VALUE_ON)
      {
        Serial.print(F("Calculated: "));
        Serial.println(Input);
      }
      strcpy(Input,Output);  
    }
  }
  free(TmpStr2);
  free(TmpStr);
  free(Output);

  return error;
}
#endif

/**********************************************************************************************\
 * Indien het een vers geresette Nodo is, dan ongedaan maken van deze status.
 \*********************************************************************************************/
void UndoNewNodo(void)
  {
  if(Settings.NewNodo)
    {
    Settings.NewNodo=false;
    Save_Settings();
    } 
  }

/**********************************************************************************************\
 * Reset een vers geïnitialiseerde struct. Nodog om dat niet mag worden aangenomen dat alle
 * waarden Na initialisatie leeg zijn.
 \*********************************************************************************************/
void ClearEvent(struct NodoEventStruct *Event)
{    
  Event->Command            = 0;
  Event->Par1               = 0;
  Event->Par2               = 0L;
  Event->Flags              = 0;
  Event->Port               = 0;
  Event->Checksum           = 0;
  Event->Direction          = 0;
  Event->DestinationUnit    = 0;
  Event->SourceUnit         = Settings.Unit;
}


//#######################################################################################################
//##################################### Clock            ################################################
//#######################################################################################################

#define DS1307_SEC       0
#define DS1307_MIN       1
#define DS1307_HR        2
#define DS1307_DOW       3
#define DS1307_DATE      4
#define DS1307_MTH       5
#define DS1307_YR        6
#define DS1307_CTRL      7
#define DS1307_DLS       8 // De DS1307 heeft nog 56 bytes over voor data. De eerste positie 0x08 gebruiken we als vlag voor zomertijd/wintertijd (DLS).
#define DS1307_DLS_M     9 // Maand laatste DLS omschakeling.
#define DS1307_DLS_D    10 // Datum laatste DLS omschakeling.
#define DS1307_RESERVED 11 // Reserve
uint8_t rtc[12];

#define DS1307_BASE_YR   2000
#define DS1307_CTRL_ID   B1101000  //De RTC zit op adres 104. Dit is een vast gegeven van de DS1307 chip.
#define DS1307_CLOCKHALT B10000000
#define DS1307_LO_BCD    B00001111
#define DS1307_HI_BCD    B11110000
#define DS1307_HI_SEC    B01110000
#define DS1307_HI_MIN    B01110000
#define DS1307_HI_HR     B00110000
#define DS1307_LO_DOW    B00000111
#define DS1307_HI_DATE   B00110000
#define DS1307_HI_MTH    B00110000
#define DS1307_HI_YR     B11110000

// update the data on the RTC from the bcd formatted data in the buffer
void DS1307_save(void)
  {
  Wire.beginTransmission(DS1307_CTRL_ID);
  Wire.write((uint8_t)0x00); // reset register pointer
  for(byte i=0; i<11; i++)Wire.write(rtc[i]);
  Wire.endTransmission();
  }

// Aquire data from the RTC chip in BCD format, refresh the buffer

void DS1307_read(void)
  {
  Wire.beginTransmission(DS1307_CTRL_ID);  // reset the register pointer to zero
  Wire.write((uint8_t)0x00);
  if (Wire.endTransmission(false) == 0) // Try to become I2C Master, send data and collect bytes, keep master status for next request...
    {
      Wire.requestFrom(DS1307_CTRL_ID, 11);  // request the 9 bytes of data    (secs, min, hr, dow, date. mth, yr. ctrl, dls)
      if(Wire.available() == 11) for(byte i=0; i<11; i++)rtc[i]=Wire.read();// store data in raw bcd format
    }
  Wire.endTransmission(true); // Release I2C Master status...
  }


/**********************************************************************************************\
 * Zet de RTC op tijd.
 \*********************************************************************************************/
void ClockSet(void) 
{
  rtc[DS1307_SEC]  =DS1307_CLOCKHALT;  // Stop the clock. Set the ClockHalt bit high to stop the rtc. This bit is part of the seconds byte
  DS1307_save();
  rtc[DS1307_MIN]      = ((Time.Minutes/10)<<4)+(Time.Minutes%10);
  rtc[DS1307_HR]       = ((Time.Hour/10)<<4)+(Time.Hour%10); // schrijf de wintertijd weg.
  rtc[DS1307_DOW]      = Time.Day;
  rtc[DS1307_DATE]     = ((Time.Date/10)<<4)+(Time.Date%10);
  rtc[DS1307_MTH]      = ((Time.Month/10)<<4)+(Time.Month%10);
  rtc[DS1307_YR]       = (((Time.Year-DS1307_BASE_YR)/10)<<4)+(Time.Year%10); 
  rtc[DS1307_SEC]      = ((Time.Seconds/10)<<4)+(Time.Seconds%10); // and start the clock again...
  rtc[DS1307_DLS]      = Time.DaylightSaving;
  rtc[DS1307_DLS_M]    = Time.DaylightSavingSetMonth; // Maand waarin de laatste zomertijd/wintertijd omschakeling plaats gevonden heeft
  rtc[DS1307_DLS_D]    = Time.DaylightSavingSetDate;  // Datum waarop de laatste zomertijd/wintertijd omschakeling plaats gevonden heeft
  rtc[DS1307_RESERVED] = 0;
  DS1307_save();
}

/**********************************************************************************************\
 * Leest de realtime clock en plaatst actuele waarden in de struct Time. 
 * Eveneens wordt de Event code terug gegeven
 \*********************************************************************************************/
void ClockRead(void)
  {
  DS1307_read();// lees de RTC chip uit
  if (rtc[4] <= 0) return;

  Time.Seconds                = (10*((rtc[DS1307_SEC] & DS1307_HI_SEC)>>4))+(rtc[DS1307_SEC] & DS1307_LO_BCD);
  Time.Minutes                = (10*((rtc[DS1307_MIN] & DS1307_HI_MIN)>>4))+(rtc[DS1307_MIN] & DS1307_LO_BCD);
  Time.Date                   = (10*((rtc[DS1307_DATE] & DS1307_HI_DATE)>>4))+(rtc[DS1307_DATE] & DS1307_LO_BCD);
  Time.Month                  = (10*((rtc[DS1307_MTH] & DS1307_HI_MTH)>>4))+(rtc[DS1307_MTH] & DS1307_LO_BCD);
  Time.Year                   = (10*((rtc[DS1307_YR] & DS1307_HI_YR)>>4))+(rtc[DS1307_YR] & DS1307_LO_BCD)+DS1307_BASE_YR;
  Time.Hour                   = (10*((rtc[DS1307_HR] & DS1307_HI_HR)>>4))+(rtc[DS1307_HR] & DS1307_LO_BCD);
  Time.Day                    = rtc[DS1307_DOW] & DS1307_LO_DOW;
  Time.DaylightSaving         = rtc[DS1307_DLS];
  Time.DaylightSavingSetMonth = rtc[DS1307_DLS_M];
  Time.DaylightSavingSetDate  = rtc[DS1307_DLS_D];

  // Het kan zijn als de klok niet aangesloten is, dat er 'rommel' gelezen is. Doe eenvoudige check.
  if(Time.Minutes>60 || Time.Hour>23 || Time.Day>8 || Time.Month>12 || Time.Date>31)
    {
    Time.Day=0; // De dag wordt gebruikt als checksum of de klok aanwezig is. Deze nooit op 0 als klok juist aangesloten
    bitWrite(HW_Config,HW_CLOCK,0);
    }
  else
    bitWrite(HW_Config,HW_CLOCK,1);


  // Bereken of het volgens de datum van vandaag zomertijd of wintertijd is. Eventueel de klok verzetten.
  long x=(long)pgm_read_word_near(DLSDate+Time.Year-DLSBase);
  long y=(long)((long)(Time.Date*100L)+(long)(Time.Month*10000L)+(long)Time.Hour);
  boolean DLS = (y>=((x/100L)*100L+30002L) && y<((x%100L)*100L+100003L));  
  x=Time.Month*100                  + Time.Date;  
  y=Time.DaylightSavingSetMonth*100 + Time.DaylightSavingSetDate;
  if(Time.DaylightSaving!=DLS  && x!=y)  // Als DaylightSaving status volgens de RTC niet overeenkomt met de DaylightSaving zoals berekend uit de datum EN de RTC is vandaag nog niet verzet...
    {  
    if(DLS)// als het zomertijd is en wintertijd wordt
      Time.Hour=Time.Hour==0?23:Time.Hour-1;// ...dan de klok een uur terug.
    else // als het wintertijd is en zomertijd wordt
    Time.Hour=Time.Hour<23?Time.Hour+1:0; //... dan klok uur vooruit.

    Time.DaylightSavingSetMonth=Time.Month;
    Time.DaylightSavingSetDate=Time.Date;
    Time.DaylightSaving=DLS;
    ClockSet();// verzet de RTC klok
    }
  }

#ifdef NODO_MEGA
/**********************************************************************************************\
 * 
 * Deze functie vult de globale variabele Time.DayLight met de status van zonsopkomst & -opgang
 \*********************************************************************************************/
void SetDaylight()
{
  // Tabel Sunset & Sunrise: om de 10 dagen de tijden van zonsopkomst en zonsondergang in minuten na middernacht. 
  // Geldig voor in Nederland (gemiddelde voor midden Nederland op 52.00 graden NB en 5.00 graden OL) 
  // Eerste dag is 01 januari, tweede is 10, januari, derde is 20 januari, etc.
  // tussenliggende dagen worden berekend aan de hand van lineaire interpolatie tussen de tabelwaarden. 
  // Afwijking t.o.v. KNMI-tabel is hiermee beperkt tot +/-1 min.

  const int offset=120;  
  int DOY,index,now,up,down;
  int u0,u1,d0,d1;

  DOY=((Time.Month-1)*304)/10+Time.Date;// schrikkeljaar berekening niet nodig, levert slechts naukeurigheidsafwijking van één minuut.
  index=(DOY/10);
  now=Time.Hour*60+Time.Minutes;

  //zomertijd correctie 
  if(Time.DaylightSaving)
  {
    if(now>=60)now-=60;
    else now=now+1440-60;
  }

  u0=pgm_read_word_near(Sunrise+index);
  u1=pgm_read_word_near(Sunrise+index+1);
  d0=pgm_read_word_near(Sunset+index);
  d1=pgm_read_word_near(Sunset+index+1);

  up  =u0+((u1-u0)*(DOY%10))/10;// Zon op in minuten na middernacht
  down=d0+((d1-d0)*(DOY%10))/10;// Zon onder in minuten na middernacht

  Time.Daylight=0;                        // astronomische start van de dag (in de wintertijd om 0:00 uur)
  if(now>=(up-offset))   Time.Daylight=1; // <offset> minuten voor zonsopkomst 
  if(now>=up)            Time.Daylight=2; // zonsopkomst
  if(now>=(down-offset)) Time.Daylight=3; // <offset> minuten voor zonsondergang
  if(now>=down)          Time.Daylight=4; // zonsondergang
}

//void SimulateDay(void) //???
//  {
//  unsigned long SimulatedClockEvent, Event, Action;
//
//  Time.Seconds=0;
//  Time.Minutes=0;
//  Time.Hour=0;
//  DaylightPrevious=4;// vullen met 4, dan wordt in de zomertijd 4 niet tweemaal per etmaal weergegeven
//
//  PrintTerminal(ProgmemString(Text_22));
//  for(int m=0;m<=1439;m++)  // loop alle minuten van één etmaal door
//   {
//    // Simuleer alle minuten van een etmaal
//    if(Time.Minutes==60){
//      Time.Minutes=0;
//      Time.Hour++;
//    }  // roll-over naar volgende uur
//
//    // Kijk of er op het gesimuleerde tijdstip een hit is in de EventList
////    SimulatedClockEvent=command2event( CMD_CLOCK_EVENT_ALL+Time.Day,Time.Hour,Time.Minutes);
////    if(CheckEventlist(SimulatedClockEvent,VALUE_SOURCE_CLOCK)) // kijk of er een hit is in de EventList
////      ProcessEvent2_old(SimulatedClockEvent,VALUE_DIRECTION_INPUT,VALUE_SOURCE_CLOCK);
////??? klok events nog aanpassen
//
//    // Kijk of er op het gesimuleerde tijdstip een zonsondergang of zonsopkomst wisseling heeft voorgedaan
//    SetDaylight(); // Zet in de struct ook de Time.DayLight status behorend bij de tijd
//    if(Time.Daylight!=DaylightPrevious)// er heeft een zonsondergang of zonsopkomst wisseling voorgedaan
//      {
//      struct NodoEventStruct TempEvent;
//      ClearEvent(&TempEvent);
//      TempEvent.Direction = VALUE_DIRECTION_INPUT;
//      TempEvent.Port      = VALUE_SOURCE_CLOCK;
//      TempEvent.Command   = CMD_CLOCK_EVENT_DAYLIGHT;
//      TempEvent.Par1      = Time.Daylight;
//      ProcessEvent2(&TempEvent);  
//      }
//    Time.Minutes++;
//    }
//
//  PrintTerminal(ProgmemString(Text_22));
//  ClockRead();// klok weer op de juiste tijd zetten.
//  SetDaylight();// daglicht status weer terug op de juiste stand zetten
//  DaylightPrevious=Time.Daylight;
//  }
//
#endif

//#######################################################################################################
//##################################### Misc: Conversions     ###########################################
//#######################################################################################################

#ifdef NODO_MEGA
/*********************************************************************************************\
 * kopiëer de string van een commando naar een string[]
 \*********************************************************************************************/
char* cmd2str(int i)
  {
  static char string[80];

  if(i<=COMMAND_MAX)
    strcpy_P(string,(char*)pgm_read_word(&(CommandText_tabel[i])));
  else
    string[0]=0;// als er geen gevonden wordt, dan is de string leeg

  return string;
  }

/*********************************************************************************************\
 * Haal uit een string de commando code. False indien geen geldige commando code.
 \*********************************************************************************************/
int str2cmd(char *command)
{
  for(int x=0;x<=COMMAND_MAX;x++)
    if(strcasecmp(command,cmd2str(x))==0)
      return x;      

  return false;
}


/*********************************************************************************************\
 * String mag HEX, DEC
 * Deze routine converteert uit een string een unsigned long waarde.
 * De string moet beginnen met het eerste teken(dus geen voorloop spaties).
 * bij ongeldige tekens in de string wordt een false terug gegeven.
 \*********************************************************************************************/
unsigned long str2int(char *string)
{
  return(strtol(string,NULL,0));  
}

/**********************************************************************************************\
 * geeft *char pointer terug die naar een PROGMEM string wijst.
 \*********************************************************************************************/
char* PROGMEM2str(prog_char* text)
{
  byte x=0;
  static char buffer[60];

  do
  {
    buffer[x]=pgm_read_byte_near(text+x);
  }
  while(buffer[x++]!=0);
  return buffer;  
}

/**********************************************************************************************\
 * Converteert een 4byte array IP adres naar een string.
 \*********************************************************************************************/
char* ip2str(byte* IP)
{
  static char str[20];
  sprintf(str,"%u.%u.%u.%u",IP[0],IP[1],IP[2],IP[3]);
  return str;
}

/**********************************************************************************************\
 * Converteert een string naar een 4byte array IP adres
 * 
 \*********************************************************************************************/
boolean str2ip(char *string, byte* IP)
{
  byte c;
  byte part=0;
  int value=0;

  for(int x=0;x<=strlen(string);x++)
  {
    c=string[x];
    if(isdigit(c))
    {
      value*=10;
      value+=c-'0';
    }

    else if(c=='.' || c==0) // volgende deel uit IP adres
    {
      if(value<=255)
        IP[part++]=value;
      else 
        return false;
      value=0;
    }
    else if(c==' ') // deze tekens negeren
      ;
    else // ongeldig teken
    return false;
  }
  if(part==4)// correct aantal delen van het IP adres
    return true;
  return false;
}


/**********************************************************************************************\
 * Converteert een unsigned long naar een string met decimale integer.
 \*********************************************************************************************/
char* int2str(unsigned long x)
{
  static char OutputLine[12];
  char* OutputLinePosPtr=&OutputLine[10];
  int y;

  *OutputLinePosPtr=0;

  if(x==0)
    {
    *--OutputLinePosPtr='0';
    }
  else
    {  
    while(x>0)
      {
      *--OutputLinePosPtr='0'+(x%10);
      x/=10;
      }
    }    
  return OutputLinePosPtr;
  }

/**********************************************************************************************\
 * Converteert een unsigned long naar een hexadecimale string.
 \*********************************************************************************************/
char* int2strhex(unsigned long x)
{
  static char OutputLine[12];
  char* OutputLinePosPtr=&OutputLine[10];
  int y;

  *OutputLinePosPtr=0;

  if(x==0)
  {
    *--OutputLinePosPtr='0';
  }
  else
  {  
    while(x>0)
    {
      y=x&0xf;

      if(y<10)
        *--OutputLinePosPtr='0'+y;
      else
        *--OutputLinePosPtr='A'+(y-10);

      x=x>>4;
      ;
    }
    *--OutputLinePosPtr='x';
    *--OutputLinePosPtr='0';
  }
  return OutputLinePosPtr;
}


char* Float2str(float f) // ??? Is deze funktie dubbel?
{
  static char rString[25];
  dtostrf(f, 0, 2, rString); // Kaboem... 2100 bytes programmacode extra !
  return rString;
}


/**********************************************************************************************\
 * vult een string met een regel uit de Eventlist.
 * geeft false terug als de regel leeg is
 * Let op dat er voldoende ruimte is in [Line]
 \*********************************************************************************************/
boolean EventlistEntry2str(int entry, byte d, char* Line, boolean Script)
  {
  struct NodoEventStruct Event, Action;
  ClearEvent(&Event);
  ClearEvent(&Action);
 
  char *TempString=(char*)malloc(100);
  boolean Ok;

  if(Ok=Eventlist_Read(entry,&Event,&Action)) // lees regel uit de Eventlist. Ga door als gelukt.
    {
    if(Event.Command) // Als de regel gevuld is
      {
      if(!Script)
        {
        strcpy(Line,int2str(entry));
        strcat(Line,": ");
        }
      else
        {
        strcpy(Line,cmd2str(CMD_EVENTLIST_WRITE));
        strcat(Line,"; ");
        }
  
      // geef het event weer
      Event2str(&Event, TempString);
      strcat(Line, TempString);
  
      // geef het action weer
      strcat(Line,"; ");
      Event2str(&Action, TempString);  
      strcat(Line,TempString);
      }
    else
      Line[0]=0;
    }

  free(TempString);
  return Ok;
  }

/*******************************************************************************************
 *  floatToString.h
 *
 *  Usage: floatToString(buffer string, float value, precision, minimum text width)
 *
 *  Example:
 *  char test[20];    // string buffer
 *  float M;          // float variable to be converted
 *                 // precision -> number of decimal places
 *                 // min text width -> character output width, 0 = no right justify
 * 
 *  Serial.print(floatToString(test, M, 3, 7)); // call for conversion function
 *  
 * Thanks to capt.tagon / Orygun
 **********************************************************************************************/

char * floatToString(char * outstr, double val, byte precision, byte widthp){
  char temp[16]; //increase this if you need more digits than 15
  byte i;

  temp[0]='\0';
  outstr[0]='\0';

  if(val < 0.0){
    strcpy(outstr,"-\0");  //print "-" sign
    val *= -1;
  }

  if( precision == 0) {
    strcat(outstr, ltoa(round(val),temp,10));  //prints the int part
  }
  else {
    unsigned long frac, mult = 1;
    byte padding = precision-1;

    while (precision--)
      mult *= 10;

    val += 0.5/(float)mult;      // compute rounding factor

    strcat(outstr, ltoa(floor(val),temp,10));  //prints the integer part without rounding
    strcat(outstr, ".\0"); // print the decimal point

    frac = (val - floor(val)) * mult;

    unsigned long frac1 = frac;

    while(frac1 /= 10) 
      padding--;

    while(padding--) 
      strcat(outstr,"0\0");    // print padding zeros

    strcat(outstr,ltoa(frac,temp,10));  // print fraction part
  }

  // generate width space padding 
  if ((widthp != 0)&&(widthp >= strlen(outstr))){
    byte J=0;
    J = widthp - strlen(outstr);

    for (i=0; i< J; i++) {
      temp[i] = ' ';
    }

    temp[i++] = '\0';
    strcat(temp,outstr);
    strcpy(outstr,temp);
  }

  return outstr;
}

/*******************************************************************************************************\
 * Converteert een string naar een weekdag. Geldige input:
 * Sun, Mon, ..... All, 0,1..7
 * 1..7=Zondag...zaterdag
 * 0xf=Wildcard
 * 0 als geen geldige invoer
 *
 * LET OP: voorloop spaties en vreemde tekens moeten reeds zijn verwijderd
 \*******************************************************************************************************/
int str2weekday(char *Input)
   {
   char Cmp[4];
   
   int x,y;
   
   // check if de gebruiker de waarde 'All', een '0' of een '*' heeft ingevoerd.
   y=str2cmd(Input);
   if(y==VALUE_ALL || Input[0]=='*' || Input[0]=='0')return 0xF;
   
   // check of de gebruiker een getal 1..7 heeft ingevoerd.
   y=str2int(Input);
   if(y>=1 && y<=7)return y;   
   
   // De string moet nu minimaal drie tekens bevatten, anders geen geldige invoer
   if(strlen(Input)<3)return 0;
   
   Input[3]=0; // Sluit de string voor de zekerheid af zodat er niet meer dan drie posities zijn
   
   for(x=0;x<=6;x++)
     {
     char *StrPtr=ProgmemString(Text_04)+(x*3);
     *(StrPtr+3)=0; // Sluit deze string ook af einde afkorting weekdag
     if(strcasecmp(StrPtr,Input)==0)return x+1;
     }
   return false;   
   }       
#endif

/*******************************************************************************************************\
 * Houdt bij welke Nodo's binnen bereik zijn en via welke Poort.
 * Als Port ongelijk aan nul, dan wordt de lijst geactualiseerd.
 \*******************************************************************************************************/
byte NodoOnline(byte Unit, byte Port)
  {
  static byte NodoOnlinePort[UNIT_MAX+1];
  static boolean FirstTime=true;
  
  int x;
  
  // Maak eerste keer de tabel leeg.
  if(FirstTime)
    {
    FirstTime=false;
    for(x=0;x<=UNIT_MAX;x++)
      NodoOnlinePort[x]=0;
    NodoOnlinePort[Settings.Unit]=VALUE_SOURCE_SYSTEM;//Dit is deze unit.
    }
    
  if(Port!=NodoOnlinePort[Unit])
    {
    // Werk tabel bij. Voorkeurspoort voor communicatie.

    if(Port==VALUE_SOURCE_I2C)
      NodoOnlinePort[Unit]=VALUE_SOURCE_I2C;
    else if(Port==VALUE_SOURCE_IR && NodoOnlinePort[Unit]!=VALUE_SOURCE_I2C)
      NodoOnlinePort[Unit]=VALUE_SOURCE_IR;
    else if(Port==VALUE_SOURCE_RF && NodoOnlinePort[Unit]!=VALUE_SOURCE_IR && NodoOnlinePort[Unit]!=VALUE_SOURCE_I2C)
      NodoOnlinePort[Unit]=VALUE_SOURCE_RF;
    }    
  return NodoOnlinePort[Unit];
  }
/*********************************************************************************************\
 * Een float en een unsigned long zijn beide 4bytes groot. We gebruiken ruimte van Par3 om 
 * een float in op te slaan. Onderstaande twee funkties converteren de unsigned long
 * en de float naar het andere format.
 \*********************************************************************************************/
unsigned long float2ul(float f)
  {
  unsigned long ul;
  memcpy(&ul, &f,4);
  return ul;
  }

float ul2float(unsigned long ul)
  {
  float f;
  memcpy(&f, &ul,4);
  return f;
  }
    
void DeviceInit(void)
  {
  byte x;

  for(x=0;x<DEVICE_MAX;x++)
    {
    Device_ptr[x]=0;
    Device_id[x]=0;
    }
    
  x=0;
      
  #ifdef DEVICE_01
  Device_ptr[x]=&Device_01;
  Device_id[x++]=1;
  #endif

  #ifdef DEVICE_02
  Device_ptr[x]=&Device_02;
  Device_id[x++]=2;
  #endif

  #ifdef DEVICE_03
  Device_ptr[x]=&Device_03;
  Device_id[x++]=3;
  #endif

  #ifdef DEVICE_04
  Device_ptr[x]=&Device_04;
  Device_id[x++]=4;
  #endif

  #ifdef DEVICE_05
  Device_ptr[x]=&Device_05;
  Device_id[x++]=5;
  #endif

  #ifdef DEVICE_06
  Device_ptr[x]=&Device_06;
  Device_id[x++]=6;
  #endif

  #ifdef DEVICE_07
  Device_ptr[x]=&Device_07;
  Device_id[x++]=7;
  #endif

  #ifdef DEVICE_08
  Device_ptr[x]=&Device_08;
  Device_id[x++]=8;
  #endif

  #ifdef DEVICE_09
  Device_ptr[x]=&Device_09;
  Device_id[x++]=9;
  #endif

  #ifdef DEVICE_10
  Device_ptr[x]=&Device_10;
  Device_id[x++]=10;
  #endif

  #ifdef DEVICE_11
  Device_ptr[x]=&Device_11;
  Device_id[x++]=11;
  #endif

  #ifdef DEVICE_12
  Device_ptr[x]=&Device_12;
  Device_id[x++]=12;
  #endif

  #ifdef DEVICE_13
  Device_ptr[x]=&Device_13;
  Device_id[x++]=13;
  #endif

  #ifdef DEVICE_14
  Device_ptr[x]=&Device_14;
  Device_id[x++]=14;
  #endif

  #ifdef DEVICE_15
  Device_ptr[x]=&Device_15;
  Device_id[x++]=15;
  #endif

  #ifdef DEVICE_16
  Device_ptr[x]=&Device_16;
  Device_id[x++]=16;
  #endif

  #ifdef DEVICE_17
  Device_ptr[x]=&Device_17;
  Device_id[x++]=17;
  #endif

  #ifdef DEVICE_18
  Device_ptr[x]=&Device_18;
  Device_id[x++]=18;
  #endif

  #ifdef DEVICE_19
  Device_ptr[x]=&Device_19;
  Device_id[x++]=19;
  #endif

  #ifdef DEVICE_20
  Device_ptr[x]=&Device20;
  Device_id[x++]=20;
  #endif

  #ifdef DEVICE_21
  Device_ptr[x]=&Device21;
  Device_id[x++]=21;
  #endif

  #ifdef DEVICE_22
  Device_ptr[x]=&Device22;
  Device_id[x++]=22;
  #endif

  #ifdef DEVICE_23
  Device_ptr[x]=&Device23;
  Device_id[x++]=23;
  #endif

  #ifdef DEVICE_24
  Device_ptr[x]=&Device24;
  Device_id[x++]=24;
  #endif

  #ifdef DEVICE_99
  Device_ptr[x]=&Device_99;
  Device_id[x++]=99;
  #endif    

  for(byte x=0;x<DEVICE_MAX; x++)
    if(Device_ptr[x]!=0)
      Device_ptr[x](DEVICE_INIT,0,0);
  }

