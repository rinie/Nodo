#define SIGNAL_TIMEOUT_RF         5000 // na deze tijd in uSec. wordt één RF signaal als beëindigd beschouwd.
#define SIGNAL_TIMEOUT_IR        10000 // na deze tijd in uSec. wordt één IR signaal als beëindigd beschouwd.
#define SIGNAL_REPEAT_TIME        1000 // Tijd waarbinnen hetzelfde event niet nogmaals via RF of IR mag binnenkomen. Onderdrukt ongewenste herhalingen van signaal

boolean I2C_EventReceived=false;       // Als deze vlag staat, is er een I2C event binnengekomen.
struct NodoEventStruct I2C_Event;

boolean ScanEvent(struct NodoEventStruct *Event)
  {
  byte Fetched=0;

  // we willen graag een voorziening die er voor zorgt dat events die snel achtereenvolgens worden verzonden
  // niet leidt tot meerdere events. onderstaand een timer en een opslag voor het laatst ontvangen event.
  static unsigned long PreviousTime=0L;
  static unsigned long PreviousHash=0L;
  
  // I2C: *************** kijk of er data is binnengekomen op de I2Cbus **********************
  if(I2C_EventReceived)
    {
    Fetched=VALUE_SOURCE_I2C;
    }
    
  // IR: *************** kijk of er data staat op IR en genereer een event als er een code ontvangen is **********************
  else if((*portInputRegister(IRport)&IRbit)==0)// Kijk of er iets op de poort binnenkomt. (Pin=LAAG als signaal in de ether).
    {
    if(FetchSignal(PIN_IR_RX_DATA,LOW,SIGNAL_TIMEOUT_IR))// Als het een duidelijk IR signaal was
      Fetched=VALUE_SOURCE_IR;
    }

  // RF: *************** kijk of er data start op RF en genereer een event als er een code ontvangen is **********************
  else if((*portInputRegister(RFport)&RFbit)==RFbit)// Kijk if er iets op de RF poort binnenkomt. (Pin=HOOG als signaal in de ether).
    {
    if(FetchSignal(PIN_RF_RX_DATA,HIGH,SIGNAL_TIMEOUT_RF))// Als het een duidelijk RF signaal was
      Fetched=VALUE_SOURCE_RF;
    }

  if(Fetched)
    {
    if(AnalyzeRawSignal(Event))// als AnalyzeRawSignal een event heeft opgeleverd dan is het struct Event gevuld.
      {
      if(Fetched==VALUE_SOURCE_IR)
        {
        // er is een IR signaal binnen gekomen. Zet de hardware vlag voor IR en disable tellen van pulsen.
        bitWrite(HW_Config,HW_IR_RX,1);
        bitWrite(HW_Config,HW_PULSE,0);
        detachInterrupt(PULSE_IRQ); // IRQ behorende bij PIN_IR_RX_DATA
        }
      if(Fetched==VALUE_SOURCE_RF)
        bitWrite(HW_Config,HW_RF_RX,1);

      
      // Reset de timers nadat er een event is binnegekomen. Dit om later voorafgaand aan het zenden, indien nodig, een
      // korte pauze te nemen zodat de andere Nodo weer gereed staat voor ontvangst.
      DelayTransmission(Fetched,true);

      // Sommige signaal typen kunnen door de zender herhaald worden verzonden.
      // Indien signaal kort geleden ook ontvangen, dan herhaling gebruiken als checksum. Daarna verdere herhalingen onderdrukken.
      if(RawSignal.Repeats && (Fetched==VALUE_SOURCE_RF || Fetched==VALUE_SOURCE_IR))
        {
        unsigned long Hash=(unsigned long)(Event->Command<<24) || (unsigned long)(Event->Par1<<16) || (unsigned long)(Event->Par2&0xffff);
        
        if(Hash!=PreviousHash)
          {
          PreviousHash=Hash;
          return false;
          }

        if(PreviousTime>(millis()-SIGNAL_REPEAT_TIME))
          {
          return false;
          }
        }
      PreviousTime=millis();

//      // Nodo's kunnen een Home adres ingesteld hebben. Als dit het geval is, dan moet het Home adres overeenkomen
//      if(Event->DestinationUnit>>5 !=Settings.Home)
//        return false;
//???

      Event->Port=Fetched;
      Event->Direction=VALUE_DIRECTION_INPUT;

      // Een event kan een verzoek bevatten om bevestiging. Doe dit dan pas na verwerking.
      if(Event->Flags & TRANSMISSION_CONFIRM)
        RequestForConfirm=true;

      #ifdef NODO_MEGA
      // registreer welke Nodo's op welke poort zitten en actualiseer tabel.
      // Wordt gebruikt voor SendTo en I2C communicatie op de Mega.
      // Hiermee kan later automatisch de juiste poort worden geselecteerd met de SendTo en kan in
      // geval van I2C communicatie uitsluitend naar de I2C verbonden Nodo's worden gecommuniceerd.
      NodoOnline(Event->SourceUnit,Event->Port);
      #endif

      // Als er een specifieke Nodo is geselecteerd, dan moeten andere Nodo's worden gelocked.
      // Hierdoor is het mogelijk dat een master en een slave Nodo tijdelijk exclusief gebruik kunnen maken van de bandbreedte
      // zodat de communicatie niet wordt verstoord.  
      Transmission_SelectedUnit = Event->DestinationUnit;
     
      // Als het Nodo event voor deze unit bestemd is, dan klaar. Zo niet, dan terugkeren met een false
      // zodat er geen verdere verwerking plaatsvindt.
      if(Event->DestinationUnit==0 || Event->DestinationUnit==Settings.Unit)
        {
        return true;
        }
      }
    }
  return false;
  }


#ifdef NODO_MEGA
boolean ScanAlarm(struct NodoEventStruct *Event)
  {
  unsigned long Mask;
  
  for(byte x=0;x<ALARM_MAX;x++)
    {
    if((Settings.Alarm[x]>>20)&1) // Als alarm enabled is, dan ingestelde alarmtijd vergelijke met de echte tijd.
      {
      // stel een vergelijkingswaarde op
      unsigned long Cmp=Time.Minutes%10 | (unsigned long)(Time.Minutes/10)<<4 | (unsigned long)(Time.Hour%10)<<8 | (unsigned long)(Time.Hour/10)<<12 | (unsigned long)Time.Day<<16 | 1UL<<20;

      // In het ingestelde alarm kunnen zich wildcards bevinden. Maskeer de posities met 0xF wildcard nibble        
      for(byte y=0;y<8;y++)// loop de acht nibbles van de 32-bit Par2 langs
        {          
        if(((Settings.Alarm[x]>>(y*4))&0xf) == 0xf) // als in nibble y een wildcard waarde 0xf staat
          {
          Mask=0xffffffff  ^ (0xFUL <<(y*4)); // Mask maken om de nibble positie y te wissen.
          Cmp&=Mask;                          // Maak nibble leeg
          Cmp|=(0xFUL<<(y*4));                    // vul met wildcard waarde 0xf
          }
        }
      
     //Serial.print(F("*** debug: Alarm (bron)="));Serial.print(Settings.Alarm[x],HEX);Serial.print(F(", Cmp="));Serial.println(Cmp,HEX); //??? Debug
     if(Settings.Alarm[x]==Cmp) // Als ingestelde alarmtijd overeen komt met huidige tijd.
       {
       // Serial.print(F("*** Time.Minutes="));Serial.print(Time.Minutes);Serial.print(F(", Prev="));Serial.println(AlarmPrevious[x]); //??? Debug
       if(AlarmPrevious[x]!=Time.Minutes) // Als alarm niet eerder is afgegaan
         {
         AlarmPrevious[x]=Time.Minutes;

         ClearEvent(Event);
         Event->Direction=VALUE_DIRECTION_INPUT;
         Event->Port=VALUE_SOURCE_CLOCK;
         Event->Command=CMD_ALARM;
         Event->Par1=x+1;
         return true;
         }
       }
     else
       AlarmPrevious[x]=0xFF;
     }
   }
 return false; 
 }
#endif

