
/*********************************************************************************************\

 De Nodo biedt ruimte voor ondersteuning van meerdere devices en protocollen. De gebruiker kan aangeven welke 
 devices mee gecompileerd moeten worden. Dit vindt plaats door in het tabblad 'Nodo' de
 definitie op te nemen die er voor zorgen dat de juiste code wordt gecompileerd. 

 De volgende devices kunnen worden gekozen:

 #define DEVICE_01  // Kaku              : Klik-Aan-Klik-Uit / HomeEasy protocol ontvangst
 #define DEVICE_02  // SendKaku          : Klik-Aan-Klik-Uit / HomeEasy protocol verzenden 
 #define DEVICE_03  // NewKAKU           : Klik-Aan-Klik-Uit ontvangst van signalen met automatische codering. Tevens bekend als Intertechno.
 #define DEVICE_04  // SendNewKAKU       : Klik-Aan-Klik-Uit ontvangst van signalen met automatische codering. Tevens bekend als Intertechno. 
 #define DEVICE_05  // TempSensor        : Temperatuursensor Dallas DS18B20. (Let op; -B- variant, dus niet DS1820)
 #define DEVICE_06  // DHT11Read         : Uitlezen temperatuur & vochtigheidsgraad sensor DHT-11
 #define DEVICE_07  // Reserved!         : UserEvents van de oude Nodo. t.b.v. compatibiliteit reeds geprogrammeerde universele afstandsbedieningen.
 #define DEVICE_08  // AlectoV1          : Dit protocol zorgt voor ontvangst van Alecto weerstation buitensensoren met protocol V1
 #define DEVICE_09  // AlectoV2          : Dit protocol zorgt voor ontvangst van Alecto weerstation buitensensoren met protocol V2
 #define DEVICE_10  // AlectoV3          : Dit protocol zorgt voor ontvangst van Alecto weerstation buitensensoren met protocol V3
 #define DEVICE_11  // Reserved!         : OpenTherm (SWACDE-11-V10)
 #define DEVICE_12  // OregonV2          : Dit protocol zorgt voor ontvangst van Oregon buitensensoren met protocol versie V2
 #define DEVICE_13  // FA20RF            : Dit protocol zorgt voor ontvangst van Flamingo FA20RF rookmelder signalen
 #define DEVICE_14  // FA20RFSend        : Dit protocol zorgt voor aansturen van Flamingo FA20RF rookmelder
 #define DEVICE_15  // HomeEasy          : Dit protocol zorgt voor ontvangst HomeEasy EU zenders die werken volgens de automatische codering (Ontvangers met leer-knop)
 #define DEVICE_16  // HomeEasySend      : Dit protocol stuurt HomeEasy EU ontvangers aan die werken volgens de automatische codering (Ontvangers met leer-knop)
 #define DEVICE_17  // Reserved!         : Laat continue analoge metingen zien van alle Wired-In poorten. 
 #define DEVICE_18  // RawSignalAnalyze  : Geeft bij een binnenkomend signaal informatie over de pulsen weer.
 #define DEVICE_19  // Reserved!         : Innovations ID-12 RFID Tag reader (SWACDE-19-V10) 
 #define DEVICE_20  // Reserved!         : BMP085 Barometric pressure sensor (SWACDE-20-V10)
 #define DEVICE_21 
 #define DEVICE_22 
 #define DEVICE_23 
 #define DEVICE_24 
 #define DEVICE_99  // UserDevice        : Device voor eigen toepassing door gebruiker te bouwen.
 
 \***********************************************************************************************/





//#######################################################################################################
//#################################### Device-01: Kaku   ################################################
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Klik-Aan-Klik-Uit zenders (Handzender, PIR, etc.)
 * die werken volgens de handmatige codering (draaiwiel met adres- en huiscodering). Dit protocol
 * kan eveneens worden ontvangen door Klik-Aan-Klik-Uit apparaten die werken met automatische
 * code programmering. Dimmen wordt niet ondersteund. Coding/Encoding principe is in de markt bekend
 * onder de namen: Princeton PT2262 / MOSDESIGN M3EB / Domia Lite / Klik-Aan-Klik-Uit / Intertechno
 * 
 * Auteur             : Nodo-team (P.K.Tonkes) www.nodo-domotca.nl
 * Support            : www.nodo-domotica.nl
 * Datum              : Jan.2013
 * Versie             : 1.0
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 507
 * Compiled size      : 796 bytes voor een Mega
 * Vereiste library   : - geen -
 * Externe funkties   : strcat(),  strcpy(),  cmd2str(),  strcasecmp(),  GetArgv()
 *
 ***********************************************************************************************
 * Het signaal bestaat drie soorten reeksen van vier pulsen, te weten: 
 * 0 = T,3T,T,3T, 1 = T,3T,3T,T, short 0 = T,3T,T,T Hierbij is iedere pulse (T) 350us PWDM
 *
 * KAKU ondersteund:
 *        on/off, waarbij de pulsreeks er als volgt uit ziet: 000x en x staat voor Off / On
 *    all on/off, waarbij de pulsreeks er als volgt uit ziet: 001x en x staat voor All Off / All On 
 *
 * Interne gebruik van de parameters in het Nodo event:
 * 
 * Cmd  : Hier zit het commando SendKAKU of het event KAKU in. Deze gebruiken we verder niet.
 * Par1 : Groep commando (true of false)
 * Par2 : Hier zit het KAKU commando (On/Off) in. True of false
 * Par3 : Adres en Home code. Acht bits AAAAHHHH
 *
 \*********************************************************************************************/

#ifdef DEVICE_01
#define KAKU_CodeLength    12  // aantal data bits
#define KAKU_T            350  // us

boolean Device_01(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  
  switch(function)
    {
    #ifdef DEVICE_CORE_01
    case DEVICE_RAWSIGNAL_IN:
      {
      int i,j;
      unsigned long bitstream=0;
      
      // conventionele KAKU bestaat altijd uit 12 data bits plus stop. Ongelijk, dan geen KAKU!
      if (RawSignal.Number!=(KAKU_CodeLength*4)+2)return false;
      RawSignal.Repeats=5;
    
      for (i=0; i<KAKU_CodeLength; i++)
        {
        j=(KAKU_T*2)/RawSignal.Pulses[0]; // Deelfactor zit is eerste element van de array.   
        
        if      (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]<j && RawSignal.Pulses[4*i+4]>j) {bitstream=(bitstream >> 1);} // 0
        else if (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]>j && RawSignal.Pulses[4*i+4]<j) {bitstream=(bitstream >> 1 | (1 << (KAKU_CodeLength-1))); }// 1
        else if (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]<j && RawSignal.Pulses[4*i+4]<j) {bitstream=(bitstream >> 1); event->Par1=2;} // Short 0, Groep commando op 2e bit.
        else {return false;} // foutief signaal
        }
     
      // twee vaste bits van KAKU gebruiken als checksum
      if ((bitstream&0x600)==0x600)
        {    
        // Alles is in orde, bouw event op   
        RawSignal.Repeats    = true; // het is een herhalend signaal. Bij ontvangst herhalingen onderdrukken.
        event->Par2          = bitstream & 0xFF;
        event->Par1         |= (bitstream >> 11) & 0x01;
        event->Command       = CMD_DEVICE_FIRST+1; // KAKU is device 1;
        event->SourceUnit    = 0; // Komt niet van een Nodo unit af.
        success=true;
        }
      break;
      }
      
    case DEVICE_COMMAND:
      {
      // Niet gebruikt voor dit device.
      break;
      }
    #endif //DEVICE_CORE_01
      
    #ifdef NODO_MEGA
    case DEVICE_MMI_IN:
      {
      // Reserveer een kleine string en kap voor de zekerheid de inputstring af om te voorkomen dat er
      // buiten gereserveerde geheugen gewerkt wordt.
      char *TempStr=(char*)malloc(26);
      string[25]=0;

      // Hier aangekomen bevat string het volledige commando. Test als eerste of het opgegeven commando overeen komt met "Kaku"
      // Dit is het eerste argument in het commando.
      if(GetArgv(string,TempStr,1))
        {
        if(strcasecmp(TempStr,"Kaku")==0)
          {
          event->Command=CMD_DEVICE_FIRST+1; // KAKU is device 1
            
          byte grp=0,c;
          byte x=0;       // teller die wijst naar het het te behandelen teken
          byte Home=0;    // KAKU home A..P
          byte Address=0; // KAKU Address 1..16
          byte y=false;   // Notatiewijze
        
          // eerste parameter bevat adres volgens codering A0..P16
          if(GetArgv(string,TempStr,2)) 
            {
            while((c=tolower(TempStr[x++]))!=0)
              {
              if(c>='0' && c<='9'){Address=Address*10;Address=Address+c-'0';}
              if(c>='a' && c<='p'){Home=c-'a';} // KAKU home A is intern 0
              }
          
            if(Address==0)
              {// groep commando is opgegeven: 0=alle adressen
              event->Par1=2; // 2e bit setten voor groep.
              event->Par2=Home;
              }
            else
              event->Par2= Home | ((Address-1)<<4);        
    
            // Het door de gebruiker ingegeven tweede parameter bevat het on/off commando
            if(GetArgv(string,TempStr,3))
              {
              event->Par1 |= str2cmd(TempStr)==VALUE_ON; 
              success=true;
              }
            }
          }
        }
      free(TempStr);
      break;
      }

    case DEVICE_MMI_OUT:
      {
      if(event->Command==CMD_DEVICE_FIRST+1) // KAKU is device 1
        {
        strcpy(string,"Kaku");            // Commando / event 
        strcat(string," ");                
      
        char t[3];                                               // Mini string
        t[0]='A' + (event->Par2 & 0x0f);                         // Home A..P
        t[1]= 0;                                                 // en de mini-string afsluiten.
        strcat(string,t);
      
        if(event->Par1&2) // als 2e bit in commando staat, dan groep.
          strcat(string,int2str(0));                       // Als Groep, dan adres 0       
        else
          strcat(string,int2str(((event->Par2 & 0xf0)>>4)+1)); // Anders adres toevoegen             
      
        if(event->Par1&1) // Het 1e bit is get aan/uit commando
          strcat(string,",On");  
        else
          strcat(string,",Off");  
        }
      break;
      }
    #endif //NODO_MEGA
    }      
  return success;
  }
#endif //DEVICE_01



//#######################################################################################################
//#################################### Device-02: SendKaku   ############################################
//#######################################################################################################


/*********************************************************************************************\
 * Dit device zorgt voor aansturing van Klik-Aan-Klik-Uit ontvangers. 
 * die werken volgens de handmatige codering (draaiwiel met adres- en huiscodering). Dit protocol
 * kan eveneens worden ontvangen door Klik-Aan-Klik-Uit apparaten die werken met automatische
 * code programmering. Dimmen wordt niet ondersteund. Coding/Encoding principe is in de markt bekend
 * onder de namen: Princeton PT2262 / MOSDESIGN M3EB / Domia Lite / Klik-Aan-Klik-Uit / Intertechno
 * 
 * Auteur             : Nodo-team (P.K.Tonkes) www.nodo-domotca.nl
 * Support            : www.nodo-domotica.nl
 * Datum              : Jan.2013
 * Versie             : 1.0
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 507
 * Compiled size      : 1100 bytes voor een Mega, 540 bytes voor een Small
 * Vereiste library   : - geen -
 * Externe funkties   : strcat(),  strcpy(),  cmd2str(),  strcasecmp(),  GetArgv()
 *
 ***********************************************************************************************
 * Het signaal bestaat drie soorten reeksen van vier pulsen, te weten: 
 * 0 = T,3T,T,3T, 1 = T,3T,3T,T, short 0 = T,3T,T,T Hierbij is iedere pulse (T) 350us PWDM
 *
 * KAKU ondersteund:
 *        on/off, waarbij de pulsreeks er als volgt uit ziet: 000x en x staat voor Off / On
 *    all on/off, waarbij de pulsreeks er als volgt uit ziet: 001x en x staat voor All Off / All On 
 *
 * Interne gebruik van de parameters in het Nodo event:
 * 
 * Cmd  : Hier zit het commando SendKAKU of het event KAKU in. Deze gebruiken we verder niet.
 * Par1 : Groep commando (true of false)
 * Par2 : Hier zit het KAKU commando (On/Off) in. True of false
 * Par3 : Adres en Home code. Acht bits AAAAHHHH
 *
 \*********************************************************************************************/

#ifdef DEVICE_02
#define KAKU_CodeLength    12  // aantal data bits
#define KAKU_T            350  // us

boolean Device_02(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  
  switch(function)
    {
    #ifdef DEVICE_CORE_02
    case DEVICE_RAWSIGNAL_IN:
      {
      // Niet in gebruik. Een device is altijd In of Uit.
      break;
      }
      
    case DEVICE_COMMAND:
      {
      if(event->Command==CMD_DEVICE_FIRST+2) // SendKAKU is device 2
        {
        unsigned long Bitstream;
        byte PTMF=50;
        
        RawSignal.Pulses[0]=PTMF;
        RawSignal.Repeats=7;                   // KAKU heeft minimaal vijf herhalingen nodig om te schakelen.
        RawSignal.Number=KAKU_CodeLength*4+2;
        event->Port=VALUE_ALL;                 // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.
       
        Bitstream = event->Par2 | (0x600 | ((event->Par1&1/*Commando*/) << 11)); // Stel een bitstream samen
        
        // loop de 12-bits langs en vertaal naar pulse/space signalen.  
        for (byte i=0; i<KAKU_CodeLength; i++)
          {
          RawSignal.Pulses[4*i+1]=KAKU_T/PTMF;
          RawSignal.Pulses[4*i+2]=(KAKU_T*3)/PTMF;
      
          if (((event->Par1>>1)&1) /* Groep */ && i>=4 && i<8) 
            {
            RawSignal.Pulses[4*i+3]=KAKU_T/PTMF;
            RawSignal.Pulses[4*i+4]=KAKU_T/PTMF;
            } // short 0
          else
            {
            if((Bitstream>>i)&1)// 1
              {
              RawSignal.Pulses[4*i+3]=(KAKU_T*3)/PTMF;
              RawSignal.Pulses[4*i+4]=KAKU_T/PTMF;
              }
            else //0
              {
              RawSignal.Pulses[4*i+3]=KAKU_T/PTMF;
              RawSignal.Pulses[4*i+4]=(KAKU_T*3)/PTMF;          
              }          
            }
          }
        RawSignal.Pulses[(KAKU_CodeLength*4)+1] = KAKU_T/PTMF;
        RawSignal.Pulses[(KAKU_CodeLength*4)+2] = (KAKU_T*30)/PTMF;// space van de stopbit nodig voor pauze tussen herhalingen.@@@
        SendEvent(event,true,true,true);
        success=true;
        }
      break;
      }
    #endif // DEVICE_CORE_02
      
    #ifdef NODO_MEGA
    case DEVICE_MMI_IN:
      {
      // Reserveer een kleine string en kap voor de zekerheid de inputstring af om te voorkomen dat er
      // buiten gereserveerde geheugen gewerkt wordt.
      char *TempStr=(char*)malloc(26);
      string[25]=0;

      // Hier aangekomen bevat string het volledige commando. Test als eerste of het opgegeven commando overeen komt met "SendKaku"
      // Dit is het eerste argument in het commando.
      if(GetArgv(string,TempStr,1))
        {
        if(strcasecmp(TempStr,"SendKaku")==0)
          {
          event->Command=CMD_DEVICE_FIRST+2; //Send KAKU is device 2
            
          byte grp=0,c;
          byte x=0;       // teller die wijst naar het het te behandelen teken
          byte Home=0;    // KAKU home A..P
          byte Address=0; // KAKU Address 1..16
          byte y=false;   // Notatiewijze
        
          // eerste parameter bevat adres volgens codering A0..P16
          if(GetArgv(string,TempStr,2)) 
            {
            while((c=tolower(TempStr[x++]))!=0)
              {
              if(c>='0' && c<='9'){Address=Address*10;Address=Address+c-'0';}
              if(c>='a' && c<='p'){Home=c-'a';} // KAKU home A is intern 0
              }
          
            if(Address==0)
              {// groep commando is opgegeven: 0=alle adressen
              event->Par1=2; // 2e bit setten voor groep.
              event->Par2=Home;
              }
            else
              event->Par2= Home | ((Address-1)<<4);        
    
            // Het door de gebruiker ingegeven tweede parameter bevat het on/off commando
            if(GetArgv(string,TempStr,3))
              {
              event->Par1 |= str2cmd(TempStr)==VALUE_ON; 
              success=true;
              }
            }
          }
        }
      free(TempStr);
      break;
      }

    case DEVICE_MMI_OUT:
      {
      if(event->Command==CMD_DEVICE_FIRST+2) // KAKU is device 2
        {
        strcpy(string,"SendKaku");            // Commando / event 
        strcat(string," ");                
      
        char t[3];                                               // Mini string
        t[0]='A' + (event->Par2 & 0x0f);                         // Home A..P
        t[1]= 0;                                                 // en de mini-string afsluiten.
        strcat(string,t);
      
        if(event->Par1&2) // als 2e bit in commando staat, dan groep.
          strcat(string,int2str(0));                       // Als Groep, dan adres 0       
        else
          strcat(string,int2str(((event->Par2 & 0xf0)>>4)+1)); // Anders adres toevoegen             
      
        if(event->Par1&1) // Het 1e bit is get aan/uit commando
          strcat(string,",On");  
        else
          strcat(string,",Off");  
        }
      break;
      }
    #endif //NODO_MEGA
    }      
  return success;
  }
#endif //DEVICE_02

  
//#######################################################################################################
//#################################### Device-03: NewKAKU ###############################################
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst Klik-Aan-Klik-Uit zenders
 * die werken volgens de automatische codering (Ontvangers met leer-knop) Dit protocol is 
 * eveneens bekend onder de naam HomeEasy. Het protocol ondersteunt eveneens dim funktionaliteit.
 * 
 * Auteur             : Nodo-team (P.K.Tonkes) www.nodo-domotca.nl
 * Support            : www.nodo-domotica.nl
 * Datum              : Jan.2013
 * Versie             : 1.1
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Compiled size      : ??? bytes voor een Mega, ??? voor een Small
 * Syntax             : "NewKAKU <Adres>,<On|Off|dimlevel 1..15>
 ***********************************************************************************************
 * Vereiste library   : - geen -
 * Externe funkties   : strcat(),  strcpy(),  cmd2str(),  tolower(),  GetArgv(),  str2cmd()
 *
 * Pulse (T) is 275us PDM
 * 0 = T,T,T,4T, 1 = T,4T,T,T, dim = T,T,T,T op bit 27
 *
 * NewKAKU ondersteund:
 *   on/off       ---- 000x Off/On
 *   all on/off   ---- 001x AllOff/AllOn
 *   dim absolute xxxx 0110 Dim16        // dim op bit 27 + 4 extra bits voor dim level
 *
 *  NewKAKU bitstream= (First sent) AAAAAAAAAAAAAAAAAAAAAAAAAACCUUUU(LLLL) -> A=KAKU_adres, C=commando, U=KAKU-Unit, L=extra dimlevel bits (optioneel)
 *
 * Interne gebruik van de parameters in het Nodo event:
 * 
 * Cmd  : Hier zit het commando SendNewKAKU of het event NewKAKU in. Deze gebruiken we verder niet.
 * Par1 : Commando VALUE_ON, VALUE_OFF of dim niveau [1..15]
 * Par2 : Adres
 *
 \*********************************************************************************************/

#ifdef DEVICE_03
#define DEVICE_ID 03
#define DEVICE_NAME "NewKAKU"
#define NewKAKU_RawSignalLength      132
#define NewKAKUdim_RawSignalLength   148
#define NewKAKU_1T                   275        // us
#define NewKAKU_mT                   500        // us, midden tussen 1T en 4T 
#define NewKAKU_4T                  1100        // us
#define NewKAKU_8T                  2200        // us, Tijd van de space na de startbit

boolean Device_03(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  
  switch(function)
    {
    #ifdef DEVICE_CORE_03
    case DEVICE_RAWSIGNAL_IN:
      {
      unsigned long bitstream=0L;
      boolean Bit;
      int i;
      int PTMF=RawSignal.Pulses[0]; // in eerste element zit de waarde waarmee vermenigvuldigd moet worden om te komen tot de echte pulstijden in uSec.
      
      int P0,P1,P2,P3;
      event->Par1=0;
      
      // nieuwe KAKU bestaat altijd uit start bit + 32 bits + evt 4 dim bits. Ongelijk, dan geen NewKAKU
      if (RawSignal.Number==NewKAKU_RawSignalLength && (RawSignal.Number!=NewKAKUdim_RawSignalLength))
        {
        // RawSignal.Number bevat aantal pulsen * 2  => negeren
        // RawSignal.Pulses[1] bevat startbit met tijdsduur van 1T => negeren
        // RawSignal.Pulses[2] bevat lange space na startbit met tijdsduur van 8T => negeren
        i=3; // RawSignal.Pulses[3] is de eerste van een T,xT,T,xT combinatie
        
        do 
          {
          P0=RawSignal.Pulses[i]    * PTMF;
          P1=RawSignal.Pulses[i+1]  * PTMF;
          P2=RawSignal.Pulses[i+2]  * PTMF;
          P3=RawSignal.Pulses[i+3]  * PTMF;
          
          if     (P0<NewKAKU_mT && P1<NewKAKU_mT && P2<NewKAKU_mT && P3>NewKAKU_mT)Bit=0; // T,T,T,4T
          else if(P0<NewKAKU_mT && P1>NewKAKU_mT && P2<NewKAKU_mT && P3<NewKAKU_mT)Bit=1; // T,4T,T,T
          else if(P0<NewKAKU_mT && P1<NewKAKU_mT && P2<NewKAKU_mT && P3<NewKAKU_mT)       // T,T,T,T Deze hoort te zitten op i=111 want: 27e NewKAKU bit maal 4 plus 2 posities voor startbit
            {
            if(RawSignal.Number!=NewKAKUdim_RawSignalLength) // als de dim-bits er niet zijn
              return false;
            }
          else
            return false; // andere mogelijkheden zijn niet geldig in NewKAKU signaal.  
            
          if(i<130) // alle bits die tot de 32-bit pulstrein behoren 32bits * 4posities per bit + pulse/space voor startbit
            bitstream=(bitstream<<1) | Bit;
          else // de resterende vier bits die tot het dimlevel behoren 
            event->Par1=(event->Par1<<1) | Bit;
       
          i+=4;// volgende pulsenquartet
          }while(i<RawSignal.Number-2); //-2 omdat de space/pulse van de stopbit geen deel meer van signaal uit maakt.
            
        // Adres deel:
        if(bitstream>0xffff)                         // Is het signaal van een originele KAKU zender afkomstig, of van een Nodo ingegeven door de gebruiker ?
        #if NODO_30_COMPATIBLE
          event->Par2=bitstream &0x0FFFFFCF;         // Op hoogste nibble zat vroeger het signaaltype. 
        #else
          event->Par2=bitstream &0xFFFFFFCF;         // dan hele adres incl. unitnummer overnemen. Alleen de twee commando-bits worden er uit gefilterd
        #endif
        
        else                                         // Het is van een andere Nodo afkomstig. 
          event->Par2=(bitstream>>6)&0xff;           // Neem dan alleen 8bit v/h adresdeel van KAKU signaal over
          
        // Commando en Dim deel
        if(i>140)
          event->Par1++;                             // Dim level. +1 omdat gebruiker dim level begint bij één.
        else
          event->Par1=((bitstream>>4)&0x01)?VALUE_ON:VALUE_OFF; // On/Off bit omzetten naar een Nodo waarde. 
        event->Command       = CMD_DEVICE_FIRST+DEVICE_ID;
        event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af, dus unit op nul zetten
        RawSignal.Repeats    = true;                  // het is een herhalend signaal. Bij ontvangst herhalingen onderdrukken.
        success=true;
        }
      break;
      }
      
    case DEVICE_COMMAND:
      break;
    #endif // DEVICE_CORE_03
      
    #ifdef NODO_MEGA
    case DEVICE_MMI_IN:
      {
      char* str=(char*)malloc(40);
      string[25]=0; // kap voor de zekerheid de string af.
    
      if(GetArgv(string,str,1))
        {
        if(strcasecmp(str,DEVICE_NAME)==0)
          {
          // Hier wordt de tekst van de protocolnaam gekoppeld aan het device ID.
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          if(GetArgv(string,str,2))
            {
            event->Par2=str2int(str);    
            if(GetArgv(string,str,3))
              {
              // Vul Par1 met het KAKU commando. Dit kan zijn: VALUE_ON, VALUE_OFF, 1..16. Andere waarden zijn ongeldig.
              
              // haal uit de tweede parameter een 'On' of een 'Off'.
              if(event->Par1=str2cmd(str))
                success=true;
                
              // als dit niet is gelukt, dan uit de tweede parameter de dimwaarde halen.
              else
                {
                event->Par1=str2int(str);             // zet string om in integer waarde
                if(event->Par1>=1 && event->Par1<=16) // geldig dim bereik 1..16 ?
                   success=true;
                }
              }
            }
          }
        }
      free(str);
      break;
      }

    case DEVICE_MMI_OUT:
      {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
        {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");
      
        // In Par3 twee mogelijkheden: Het bevat een door gebruiker ingegeven adres 0..255 of een volledig NewKAKU adres.
        if(event->Par2>=0x0ff)
          strcat(string,int2strhex(event->Par2)); 
        else
          strcat(string,int2str(event->Par2)); 
        
        strcat(string,",");
        
        if(event->Par1==VALUE_ON)
          strcat(string,"On");  
        else if(event->Par1==VALUE_OFF)
          strcat(string,"Off");
        else
          strcat(string,int2str(event->Par1));
        success=true;
        }
      break;
      }
    #endif //NODO_MEGA
    }      
  return success;
  }
#endif //DEVICE_03


//#######################################################################################################
//#################################### Device-04: SendNewKAKU ###########################################
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor aansturing van Klik-Aan-Klik-Uit ontvangers
 * die werken volgens de automatische codering (Ontvangers met leer-knop) Dit protocol is 
 * eveneens bekend onder de naam HomeEasy. Het protocol ondersteunt eveneens dim funktionaliteit.
 * 
 * Auteur             : Nodo-team (P.K.Tonkes) www.nodo-domotca.nl
 * Support            : www.nodo-domotica.nl
 * Datum              : Jan.2013
 * Versie             : 1.1
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Compiled size      : ??? bytes voor een Mega, ??? voor een Small
 * Syntax             : "SendNewKAKU <Adres>,<On|Off|dimlevel 1..15>
 ***********************************************************************************************
 * Vereiste library   : - geen -
 * Externe funkties   : strcat(),  strcpy(),  cmd2str(),  tolower(),  GetArgv(),  str2cmd()
 *
 * Pulse (T) is 275us PDM
 * 0 = T,T,T,4T, 1 = T,4T,T,T, dim = T,T,T,T op bit 27
 *
 * NewKAKU ondersteund:
 *   on/off       ---- 000x Off/On
 *   all on/off   ---- 001x AllOff/AllOn
 *   dim absolute xxxx 0110 Dim16        // dim op bit 27 + 4 extra bits voor dim level
 *
 *  NewKAKU bitstream= (First sent) AAAAAAAAAAAAAAAAAAAAAAAAAACCUUUU(LLLL) -> A=KAKU_adres, C=commando, U=KAKU-Unit, L=extra dimlevel bits (optioneel)
 *
 * Interne gebruik van de parameters in het Nodo event:
 * 
 * Cmd  : Hier zit het commando SendNewKAKU of het event NewKAKU in. Deze gebruiken we verder niet.
 * Par1 : Commando VALUE_ON, VALUE_OFF of dim niveau [1..15]
 * Par2 : Adres
 *
 \*********************************************************************************************/

#ifdef DEVICE_04
#define DEVICE_ID 04
#define DEVICE_NAME "SendNewKAKU"
#define NewKAKU_RawSignalLength      132
#define NewKAKUdim_RawSignalLength   148
#define NewKAKU_1T                   275        // us
#define NewKAKU_mT                   500        // us, midden tussen 1T en 4T 
#define NewKAKU_4T                  1100        // us
#define NewKAKU_8T                  2200        // us, Tijd van de space na de startbit

boolean Device_04(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  
  switch(function)
    {
    #ifdef DEVICE_CORE_04
    case DEVICE_RAWSIGNAL_IN:
      break;
      
    case DEVICE_COMMAND:
      {
      unsigned long bitstream=0L;
      byte Bit, i=1;
      byte x; /// aantal posities voor pulsen/spaces in RawSignal
    
      const byte PTMF=25;
      RawSignal.Pulses[0]=PTMF;
    
      // bouw het KAKU adres op. Er zijn twee mogelijkheden: Een adres door de gebruiker opgegeven binnen het bereik van 0..255 of een lange hex-waarde
      if(event->Par2<=255)
        bitstream=1|(event->Par2<<6);  // Door gebruiker gekozen adres uit de Nodo_code toevoegen aan adres deel van de KAKU code. 
      else
        bitstream=event->Par2 & 0xFFFFFFCF; // adres geheel over nemen behalve de twee bits 5 en 6 die het schakel commando bevatten.
    
      event->Port=VALUE_ALL; // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.
      RawSignal.Repeats=7;   // Aantal herhalingen van het signaal.
    
      if(event->Par1==VALUE_ON || event->Par1==VALUE_OFF)
        {
        bitstream|=(event->Par1==VALUE_ON)<<4; // bit-5 is het on/off commando in KAKU signaal
        x=130;// verzend startbit + 32-bits = 130
        }
      else
        x=146;// verzend startbit + 32-bits = 130 + 4dimbits = 146
     
      // bitstream bevat nu de KAKU-bits die verzonden moeten worden.
    
      for(i=3;i<=x;i++)RawSignal.Pulses[i]=NewKAKU_1T/PTMF;  // De meeste tijden in signaal zijn T. Vul alle pulstijden met deze waarde. Later worden de 4T waarden op hun plek gezet
      
      i=1;
      RawSignal.Pulses[i++]=NewKAKU_1T/PTMF; //pulse van de startbit
      RawSignal.Pulses[i++]=NewKAKU_8T/PTMF; //space na de startbit
      
      byte y=31; // bit uit de bitstream
      while(i<x)
        {
        if((bitstream>>(y--))&1)
          RawSignal.Pulses[i+1]=NewKAKU_4T/PTMF;     // Bit=1; // T,4T,T,T
        else
          RawSignal.Pulses[i+3]=NewKAKU_4T/PTMF;     // Bit=0; // T,T,T,4T
    
        if(x==146)  // als het een dim opdracht betreft
          {
          if(i==111) // Plaats van de Commando-bit uit KAKU 
            RawSignal.Pulses[i+3]=NewKAKU_1T/PTMF;  // moet een T,T,T,T zijn bij een dim commando.
          if(i==127)  // als alle pulsen van de 32-bits weggeschreven zijn
            {
            bitstream=(unsigned long)event->Par1-1; //  nog vier extra dim-bits om te verzenden. -1 omdat dim niveau voor gebruiker begint bij 1
            y=3;
            }
          }
        i+=4;
        }
      RawSignal.Pulses[i++]=NewKAKU_1T/PTMF; //pulse van de stopbit
      RawSignal.Pulses[i]=255; //space van de stopbit tevens pause tussen signalen. 6.5 msec.
      RawSignal.Number=i; // aantal bits*2 die zich in het opgebouwde RawSignal bevinden
      SendEvent(event,true,true,true);
      success=true;
      break;
      }
    #endif // DEVICE_CORE_04
      
    #ifdef NODO_MEGA
    case DEVICE_MMI_IN:
      {
      char* str=(char*)malloc(40);
      string[25]=0; // kap voor de zekerheid de string af.
    
      if(GetArgv(string,str,1))
        {
        if(strcasecmp(str,DEVICE_NAME)==0)
          {
          // Hier wordt de tekst van de protocolnaam gekoppeld aan het device ID.
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          if(GetArgv(string,str,2))
            {
            event->Par2=str2int(str);    
            if(GetArgv(string,str,3))
              {
              // Vul Par1 met het KAKU commando. Dit kan zijn: VALUE_ON, VALUE_OFF, 1..16. Andere waarden zijn ongeldig.
              
              // haal uit de tweede parameter een 'On' of een 'Off'.
              if(event->Par1=str2cmd(str))
                success=true;
                
              // als dit niet is gelukt, dan uit de tweede parameter de dimwaarde halen.
              else
                {
                event->Par1=str2int(str);             // zet string om in integer waarde
                if(event->Par1>=1 && event->Par1<=16) // geldig dim bereik 1..16 ?
                   success=true;
                }
              }
            }
          }
        }
      free(str);
      break;
      }

    case DEVICE_MMI_OUT:
      {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
        {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");
      
        // In Par3 twee mogelijkheden: Het bevat een door gebruiker ingegeven adres 0..255 of een volledig NewKAKU adres.
        if(event->Par2>=0x0ff)
          strcat(string,int2strhex(event->Par2)); 
        else
          strcat(string,int2str(event->Par2)); 
        
        strcat(string,",");
        
        if(event->Par1==VALUE_ON)
          strcat(string,"On");  
        else if(event->Par1==VALUE_OFF)
          strcat(string,"Off");
        else
          strcat(string,int2str(event->Par1));
        success=true;
        }
      break;
      }
    #endif //NODO_MEGA
    }      
  return success;
  }
#endif //DEVICE_04



//#######################################################################################################
//#################################### Device-05: TempSensor Dallas DS18B20  ############################
//#######################################################################################################


/*********************************************************************************************\
 * Deze funktie leest een Dallas temperatuursensor uit. 
 * Na uitgelezen van de waarde wordt in de opgegeven variabele de temperatuur opgeslagen. 
 * Na uitlezing wordt er een event gegenereerd.
 *
 * Auteur             : Paul Tonkes, p.k.tonkes@gmail.com
 * Support            : <website>
 * Datum              : Feb.2013
 * Versie             : 1.1
 * Nodo productnummer : ???
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Syntax             : "TempRead <Par1:Poortnummer>, <Par2:Variabele>"
 *
 ***********************************************************************************************
 * Technische beschrijving:
 *
 * Compiled size      : ??? bytes voor een Mega en ??? voor een Small.
 * Externe funkties   : float2ul(), 
 *
 * De sensor kan volgens de paracitaire mode worden aangesloten. De signaallijn tevens verbinden met een 4K7 naar de Vcc/+5
 * Deze fucntie kan worden gebruikt voor alle Wired poorten van de Nodo.
 * Er wordt gebruik gemaakt van de ROM-skip techniek, dus er worden geen adressering gebruikt.
 * Dit betekent max. één sensor per poort. Dit om (veel) geheugen te besparen.  *
 \*********************************************************************************************/

#ifdef DEVICE_05

// Ieder device heeft een uniek ID. Deze worden onderhouden door het Nodo team. Als je een device hebt geprogrammeerd
// die van waarde kan zijn voor andere gebruikers, meldt deze dan aan bij het Nodo team zodat deze kan worden meegenomen
// in de Nodo-releases. Device 99 is een "knutsel" device voor de gebruiker.
uint8_t DallasPin;
#define DEVICE_ID 05

// Een device heeft naast een uniek ID ook een eigen MMI string die de gebruiker kan invoeren via Telnet, Serial, HTTP 
// of een script. Geef hier de naam op. De afhandeling is niet hoofdletter gevoelig.
#define DEVICE_NAME "TempRead"

boolean Device_05(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  static byte RepeatFirstTime=2;        // Sommige varianten van de Dallas sensor geven de eerste uitlezing een 0 retour. Daarom de eerste keer tweemaal aanroepen.
  
  switch(function)
    {
    #ifdef DEVICE_CORE_05
    case DEVICE_RAWSIGNAL_IN:
      break;
      
    case DEVICE_COMMAND:
      {
      int DSTemp;                           // Temperature in 16-bit Dallas format.
      byte ScratchPad[12];                  // Scratchpad buffer Dallas sensor.   
      byte var=event->Par2;                 // Variabele die gevuld moet worden.
      
      // De Dallas sensor kan worden aangesloten op iedere digitale poort van de Arduino. In ons geval kiezen we er voor
      // om de sensor aan te sluiten op de Wired-Out poorten van de Nodo. Met Par2 is de Wired poort aangegeven.
      // 1 = WiredOut poort 1.  
      DallasPin=PIN_WIRED_OUT_1+event->Par1-1;
    
      ClearEvent(event);                                      // Ga uit van een default schone event. Oude eventgegevens wissen.
    
      do
        {
        noInterrupts();
        boolean present=DS_reset();DS_write(0xCC /* rom skip */); DS_write(0x44 /* start conversion */);
        interrupts();
        
        if(present)
          {
          delay(800);     // uitleestijd die de sensor nodig heeft
      
          noInterrupts();
          DS_reset(); DS_write(0xCC /* rom skip */); DS_write(0xBE /* Read Scratchpad */);
      
          // Maak de lijn floating zodat de sensor de data op de lijn kan zetten.
          digitalWrite(DallasPin,LOW);
          pinMode(DallasPin,INPUT);
      
          for (byte i = 0; i < 9; i++)            // copy 8 bytes
            ScratchPad[i] = DS_read();
          interrupts();
        
          DSTemp = (ScratchPad[1] << 8) + ScratchPad[0];  
      
          event->Command      = CMD_VARIABLE_SET;                 // Commando "VariableSet"
          event->Par1         = var;                              // Variabele die gevuld moet worden.
          event->Par2         = float2ul(float(DSTemp)*0.0625);   // DS18B20 variant. Waarde terugstoppen in de variabele
          success=true;
          }
        if(RepeatFirstTime)
          RepeatFirstTime--;
        }while(RepeatFirstTime);
      break;
      }
    #endif // DEVICE_CORE_05
      
    #ifdef NODO_MEGA
    case DEVICE_MMI_IN:
      {
      char *TempStr=(char*)malloc(26);
      string[25]=0;

      if(GetArgv(string,TempStr,1))
        {
        if(strcasecmp(TempStr,DEVICE_NAME)==0)
          {
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          // Par1 en Par2 hoeven niet te worden geparsed omdat deze default al door de MMI invoer van de Nodo 
          // worden gevuld indien het integer waarden zijn. Toetsen op bereikenmoet nog wel plaats vinden.
          if(event->Par1>0 && event->Par1<WIRED_PORTS && event->Par2>0 && event->Par2<=USER_VARIABLES_MAX)
            success=true;
          }
        }
      free(TempStr);
      break;
      }

    case DEVICE_MMI_OUT:
      {
      // De code die zich hier bevindt zorgt er voor dan een event met het unieke ID in de struct [event] weer wordt
      // omgezet naar een leesbaar event voor de gebruiker. het resultaat moet worden geplaatst in de string [string]
      // let op dat het totale commando niet meer dan 25 posities in beslag neemt.
      // Dit deel van de code wordt alleen uitgevoerd door een Nodo Mega, omdat alleen deze over een MMI beschikt.
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
        {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");
        strcat(string,int2str(event->Par1));
        strcat(string,",");
        strcat(string,int2str(event->Par2));
        // Vervolgens kan met strcat() hier de parameters aan worden toegevoegd      
        }
      break;
      }
    #endif //NODO_MEGA
    }      
  return success;
  }
#endif //DEVICE_05

#ifdef DEVICE_CORE_05
uint8_t DS_read(void)
  {
  uint8_t bitMask;
  uint8_t r = 0;
  uint8_t BitRead;
  
  for (bitMask = 0x01; bitMask; bitMask <<= 1)
    {
    pinMode(DallasPin,OUTPUT);
    digitalWrite(DallasPin,LOW);
    delayMicroseconds(3);

    pinMode(DallasPin,INPUT);// let pin float, pull up will raise
    delayMicroseconds(10);
    BitRead = digitalRead(DallasPin);
    delayMicroseconds(53);

    if (BitRead)
      r |= bitMask;
    }
  return r;
  }

void DS_write(uint8_t ByteToWrite) 
  {
  uint8_t bitMask;

  pinMode(DallasPin,OUTPUT);
  for (bitMask = 0x01; bitMask; bitMask <<= 1)
    {// BitWrite
    digitalWrite(DallasPin,LOW);
    if(((bitMask & ByteToWrite)?1:0) & 1)
      {
      delayMicroseconds(5);// Dallas spec.= 5..15 uSec.
      digitalWrite(DallasPin,HIGH);
      delayMicroseconds(55);// Dallas spec.= 60uSec.
      }
    else
      {
      delayMicroseconds(55);// Dallas spec.= 60uSec.
      digitalWrite(DallasPin,HIGH);
      delayMicroseconds(5);// Dallas spec.= 5..15 uSec.
      }
    }
  }

uint8_t DS_reset()
  {
  uint8_t r;
  uint8_t retries = 125;
  
  pinMode(DallasPin,INPUT);
  do  {  // wait until the wire is high... just in case
      if (--retries == 0) return 0;
      delayMicroseconds(2);
      } while ( !digitalRead(DallasPin));

  pinMode(DallasPin,OUTPUT); digitalWrite(DallasPin,LOW);
  delayMicroseconds(492); // Dallas spec. = Min. 480uSec. Arduino 500uSec.
  pinMode(DallasPin,INPUT);//Float
  delayMicroseconds(40);
  r = !digitalRead(DallasPin);// Dallas sensor houdt voor 60uSec. de bus laag nadat de Arduino de lijn heeft vrijgegeven.
  delayMicroseconds(420);
  return r;
  }
#endif // DEVICE_05_CORE



//#######################################################################################################
//######################## DEVICE-06 Temperatuur en Luchtvochtigheid sensor DHT 11 ######################
//#######################################################################################################

/*********************************************************************************************\
 * Deze funktie leest een DHT11 temperatuur en luchtvochtigheidssensor uit.
 * Deze funktie kan worden gebruikt voor alle digitale poorten van de Arduino.
 * De uitgelezen temperatuur waarde wordt in de opgegeven variabele opgeslagen.
 * De uitgelezen luchtvochtigheidsgraad wordt in de opgegeven variabele +1 opgeslagen.
 * 
 * Auteur             : Nodo-team (Martinus van den Broek) www.nodo-domotica.nl
 * Datum              : Mrt.2013
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Syntax             : "DHT11 <Par1:Poortnummer>, <Par2:Basis Variabele>"
 *********************************************************************************************
 * Technische informatie:
 * De DHT11 sensor is een 3 pin sensor met een bidirectionele datapin
 * Het principe is "onewire" maar dit protocol is NIET compatible met het bekende Dallas onewire protocol
 * Dit protocol gebruikt twee variabelen, 1 voor temperatuur en 1 voor luchtvochtigheid
 \*********************************************************************************************/

#ifdef DEVICE_06
#define DEVICE_ID 06
#define DEVICE_NAME "DHT11Read"
uint8_t DHT11_Pin;

boolean Device_06(byte function, struct NodoEventStruct *event, char *string)
{

  boolean success=false;

  switch(function)
  {
#ifdef DEVICE_CORE_06
  case DEVICE_RAWSIGNAL_IN:
    break;

  case DEVICE_COMMAND:
    {
      DHT11_Pin=PIN_WIRED_OUT_1+event->Par1-1;
      byte dht11_dat[5];
      byte dht11_in;
      byte i;
      byte temperature=0;
      byte humidity=0;

      pinMode(DHT11_Pin,OUTPUT);

      // DHT11 start condition, pull-down i/o pin for 18ms
      digitalWrite(DHT11_Pin,LOW);               // Pull low
      delay(18);
      digitalWrite(DHT11_Pin,HIGH);              // Pull high
      delayMicroseconds(40);
      pinMode(DHT11_Pin,INPUT);                  // change pin to input
      delayMicroseconds(40);

      dht11_in = digitalRead(DHT11_Pin);
      if(dht11_in) return false;

      delayMicroseconds(80);
      dht11_in = digitalRead(DHT11_Pin);
      if(!dht11_in) return false;

      delayMicroseconds(40);                     // now ready for data reception
      for (i=0; i<5; i++)
        dht11_dat[i] = read_dht11_dat();
        
      // Checksum calculation is a Rollover Checksum by design!
      byte dht11_check_sum = dht11_dat[0]+dht11_dat[1]+dht11_dat[2]+dht11_dat[3];// check check_sum
      if(dht11_dat[4]!= dht11_check_sum) return false;

      temperature = dht11_dat[2];
      humidity = dht11_dat[0];

      byte VarNr = event->Par2; // De originele Par1 tijdelijk opslaan want hier zit de variabelenummer in waar de gebruiker de uitgelezen waarde in wil hebben
        ClearEvent(event);                                    // Ga uit van een default schone event. Oude eventgegevens wissen.
      event->Command      = CMD_VARIABLE_SET;                 // Commando "VariableSet"
      event->Par1         = VarNr;                            // Par1 is de variabele die we willen vullen.
      event->Par2         = float2ul(float(temperature));
      QueueAdd(event);                                        // Event opslaan in de event queue, hierna komt de volgende meetwaarde
      event->Par1         = VarNr+1;                          // Par1+1 is de variabele die we willen vullen voor luchtvochtigheid
      event->Par2         = float2ul(float(humidity));
      QueueAdd(event);
      success=true;

      break;
    }
#endif // DEVICE_CORE_06

#ifdef NODO_MEGA
  case DEVICE_MMI_IN:
    {
      char *TempStr=(char*)malloc(26);
      string[25]=0;

      if(GetArgv(string,TempStr,1))
      {
        if(strcasecmp(TempStr,DEVICE_NAME)==0)
        {
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          if(event->Par1>0 && event->Par1<WIRED_PORTS && event->Par2>0 && event->Par2<=USER_VARIABLES_MAX-1)
            success=true;
        }
      }
      free(TempStr);
      break;
    }

  case DEVICE_MMI_OUT:
    {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
      {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");
        strcat(string,int2str(event->Par1));
        strcat(string,",");
        strcat(string,int2str(event->Par2));
      }
      break;
    }
#endif //NODO_MEGA
  }      
  return success;
}

#ifdef DEVICE_CORE_06
/*********************************************************************************************\
 * DHT11 sub to get an 8 bit value from the receiving bitstream
 \*********************************************************************************************/
byte read_dht11_dat()
{
  byte i = 0;
  byte result=0;
  noInterrupts();
  for(i=0; i< 8; i++)
  {
    while(!digitalRead(DHT11_Pin));  // wait for 50us
    delayMicroseconds(30);
    if(digitalRead(DHT11_Pin)) 
      result |=(1<<(7-i));
    while(digitalRead(DHT11_Pin));  // wait '1' finish
  }
  interrupts();
  return result;
}
#endif //DEVICE_CORE_06
#endif //DEVICE_06


//#######################################################################################################
//############################### Generic code for all Alecto devices  ##################################
//#######################################################################################################

#if defined(DEVICE_CORE_08) || defined(DEVICE_CORE_09) || defined(DEVICE_CORE_10)
byte ProtocolAlectoValidID[5];
byte ProtocolAlectoVar[5];
unsigned int ProtocolAlectoRainBase=0;

/*********************************************************************************************\
 * Calculates CRC-8 checksum
 * reference http://lucsmall.com/2012/04/29/weather-station-hacking-part-2/
 *           http://lucsmall.com/2012/04/30/weather-station-hacking-part-3/
 *           https://github.com/lucsmall/WH2-Weather-Sensor-Library-for-Arduino/blob/master/WeatherSensorWH2.cpp
 \*********************************************************************************************/
uint8_t ProtocolAlectoCRC8( uint8_t *addr, uint8_t len)
{
  uint8_t crc = 0;
  // Indicated changes are from reference CRC-8 function in OneWire library
  while (len--) {
    uint8_t inbyte = *addr++;
    for (uint8_t i = 8; i; i--) {
      uint8_t mix = (crc ^ inbyte) & 0x80; // changed from & 0x01
      crc <<= 1; // changed from right shift
      if (mix) crc ^= 0x31;// changed from 0x8C;
      inbyte <<= 1; // changed from right shift
    }
  }
  return crc;
}

/*********************************************************************************************\
 * Check for valid sensor ID
 \*********************************************************************************************/
byte ProtocolAlectoCheckID(byte checkID)
{
  for (byte x=0; x<5; x++) if (ProtocolAlectoValidID[x] == checkID) return ProtocolAlectoVar[x];
  return 0;
}
#endif // ALECTO Basic stuff...

//#######################################################################################################
//##################################### Device-08 AlectoV1  #############################################
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Alecto weerstation buitensensoren met protocol V1
 * 
 * Auteur             : Nodo-team (Martinus van den Broek) www.nodo-domotica.nl
 * Support            : www.nodo-domotica.nl
 * Datum              : Mrt.2013
 * Versie             : 1.1
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Syntax             : "AlectoV1 <Par1:Sensor ID>, <Par2:Basis Variabele>"
 *********************************************************************************************
 * Technische informatie:
 * Message Format: (9 nibbles, 36 bits):
 *
 * Format for Temperature Humidity
 *   AAAAAAAA BBBB CCCC CCCC CCCC DDDDDDDD EEEE
 *   RC       Type Temperature___ Humidity Checksum
 *   A = Rolling Code
 *   B = Message type (xyyx = temp/humidity if yy <> '11')
 *   C = Temperature (two's complement)
 *   D = Humidity BCD format
 *   E = Checksum
 *
 * Format for Rain
 *   AAAAAAAA BBBB CCCC DDDD DDDD DDDD DDDD EEEE
 *   RC       Type      Rain                Checksum
 *   A = Rolling Code
 *   B = Message type (xyyx = NON temp/humidity data if yy = '11')
 *   C = fixed to 1100
 *   D = Rain (bitvalue * 0.25 mm)
 *   E = Checksum
 *
 * Format for Windspeed
 *   AAAAAAAA BBBB CCCC CCCC CCCC DDDDDDDD EEEE
 *   RC       Type                Windspd  Checksum
 *   A = Rolling Code
 *   B = Message type (xyyx = NON temp/humidity data if yy = '11')
 *   C = Fixed to 1000 0000 0000
 *   D = Windspeed  (bitvalue * 0.2 m/s, correction for webapp = 3600/1000 * 0.2 * 100 = 72)
 *   E = Checksum
 *
 * Format for Winddirection & Windgust
 *   AAAAAAAA BBBB CCCD DDDD DDDD EEEEEEEE FFFF
 *   RC       Type      Winddir   Windgust Checksum
 *   A = Rolling Code
 *   B = Message type (xyyx = NON temp/humidity data if yy = '11')
 *   C = Fixed to 111
 *   D = Wind direction
 *   E = Windgust (bitvalue * 0.2 m/s, correction for webapp = 3600/1000 * 0.2 * 100 = 72)
 *   F = Checksum
 \*********************************************************************************************/
 
#ifdef DEVICE_08
#define DEVICE_ID 8
#define DEVICE_NAME "AlectoV1"

#define WS3500_PULSECOUNT 74

boolean Device_08(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef DEVICE_CORE_08
  case DEVICE_RAWSIGNAL_IN:
    {
      if (RawSignal.Number != WS3500_PULSECOUNT) return false;

      const byte PTMF=50;
      unsigned long bitstream=0;
      byte nibble0=0;
      byte nibble1=0;
      byte nibble2=0;
      byte nibble3=0;
      byte nibble4=0;
      byte nibble5=0;
      byte nibble6=0;
      byte nibble7=0;
      byte checksum=0;
      int temperature=0;
      byte humidity=0;
      unsigned int rain=0;
      byte windspeed=0;
      byte windgust=0;
      int winddirection=0;
      byte checksumcalc = 0;
      byte rc=0;
      byte basevar=0;

      for(byte x=2; x<=64; x=x+2)
      {
        if(RawSignal.Pulses[x]*PTMF > 0xA00) bitstream = ((bitstream >> 1) |(0x1L << 31)); 
        else bitstream = (bitstream >> 1);
      }

      for(byte x=66; x<=72; x=x+2)
      {
        if(RawSignal.Pulses[x]*PTMF > 0xA00) checksum = ((checksum >> 1) |(0x1L << 3)); 
        else checksum = (checksum >> 1);
      }

      nibble7 = (bitstream >> 28) & 0xf;
      nibble6 = (bitstream >> 24) & 0xf;
      nibble5 = (bitstream >> 20) & 0xf;
      nibble4 = (bitstream >> 16) & 0xf;
      nibble3 = (bitstream >> 12) & 0xf;
      nibble2 = (bitstream >> 8) & 0xf;
      nibble1 = (bitstream >> 4) & 0xf;
      nibble0 = bitstream & 0xf;

      // checksum calculations
      if ((nibble2 & 0x6) != 6) {
        checksumcalc = (0xf - nibble0 - nibble1 - nibble2 - nibble3 - nibble4 - nibble5 - nibble6 - nibble7) & 0xf;
      }
      else
      {
        // Alecto checksums are Rollover Checksums by design!
        if (nibble3 == 3)
          checksumcalc = (0x7 + nibble0 + nibble1 + nibble2 + nibble3 + nibble4 + nibble5 + nibble6 + nibble7) & 0xf;
        else
          checksumcalc = (0xf - nibble0 - nibble1 - nibble2 - nibble3 - nibble4 - nibble5 - nibble6 - nibble7) & 0xf;
      }

      if (checksum != checksumcalc) return false;
      rc = bitstream & 0xff;

      basevar = ProtocolAlectoCheckID(rc);

      event->Command       = CMD_DEVICE_FIRST+DEVICE_ID;
      event->Par1=rc;
      event->Par2=basevar;
      event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af, dus unit op nul zetten
      event->Port          = VALUE_SOURCE_RF;

      if (basevar == 0) return true;

      if ((nibble2 & 0x6) != 6) {

        temperature = (bitstream >> 12) & 0xfff;
        //fix 12 bit signed number conversion
        if ((temperature & 0x800) == 0x800) temperature = temperature - 0x1000;
        UserVar[basevar -1] = (float)temperature/10;

        humidity = (10 * nibble7) + nibble6;
        UserVar[basevar +1 -1] = (float)humidity;
        RawSignal.Number=0;
        return true;
      }
      else
      {
        if (nibble3 == 3)
        {
          rain = ((bitstream >> 16) & 0xffff);
          // check if rain unit has been reset!
          if (rain < ProtocolAlectoRainBase) ProtocolAlectoRainBase=rain;
          if (ProtocolAlectoRainBase > 0)
          {
            UserVar[basevar +2 -1] += ((float)rain - ProtocolAlectoRainBase) * 0.25;
          }
          ProtocolAlectoRainBase = rain;
          return true;
        }

        if (nibble3 == 1)
        {
          windspeed = ((bitstream >> 24) & 0xff);
          UserVar[basevar +3 -1] = (float)windspeed * 0.72;
          RawSignal.Number=0;
          return true;
        }

        if ((nibble3 & 0x7) == 7)
        {
          winddirection = ((bitstream >> 15) & 0x1ff) / 45;
          UserVar[basevar +4 -1] = (float)winddirection;

          windgust = ((bitstream >> 24) & 0xff);
          UserVar[basevar +5 -1] = (float)windgust * 0.72;

          RawSignal.Number=0;
          return true;
        }
      }
      success = true;
      break;
    }
  case DEVICE_COMMAND:
    {
      if ((event->Par2 > 0) && (ProtocolAlectoCheckID(event->Par1) == 0))
        {
          for (byte x=0; x<5; x++)
          {
            if (ProtocolAlectoValidID[x] == 0)
            {
              ProtocolAlectoValidID[x] = event->Par1;
              ProtocolAlectoVar[x] = event->Par2;
              break;
            }
          }
        }
      break;
    }
#endif // DEVICE_CORE_08

#ifdef NODO_MEGA
  case DEVICE_MMI_IN:
    {
      char *TempStr=(char*)malloc(26);
      string[25]=0;

      if(GetArgv(string,TempStr,1))
      {
        if(strcasecmp(TempStr,DEVICE_NAME)==0)
        {
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          if(event->Par1>0 && event->Par1<255 && event->Par2>0 && event->Par2<=USER_VARIABLES_MAX)
            success=true;
        }
      }
      free(TempStr);
      break;
    }

  case DEVICE_MMI_OUT:
    {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
      {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");
        strcat(string,int2str(event->Par1));
        strcat(string,",");
        strcat(string,int2str(event->Par2));
      }
      break;
    }
#endif //NODO_MEGA
  }      
  return success;
}
#endif //DEVICE_08


//#######################################################################################################
//##################################### Device-09 AlectoV2  #############################################
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Alecto weerstation buitensensoren
 * 
 * Auteur             : Nodo-team (Martinus van den Broek) www.nodo-domotica.nl
 *                      Support ACH2010 en code optimalisatie door forumlid: Arendst
 * Support            : www.nodo-domotica.nl
 * Datum              : Mrt.2013
 * Versie             : 1.1
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Syntax             : "AlectoV2 <Par1:Sensor ID>, <Par2:Basis Variabele>"
 *********************************************************************************************
 * Technische informatie:
 * DKW2012 Message Format: (11 Bytes, 88 bits):
 * AAAAAAAA AAAABBBB BBBB__CC CCCCCCCC DDDDDDDD EEEEEEEE FFFFFFFF GGGGGGGG GGGGGGGG HHHHHHHH IIIIIIII
 *                         Temperature Humidity Windspd_ Windgust Rain____ ________ Winddir  Checksum
 * A = start/unknown, first 8 bits are always 11111111
 * B = Rolling code
 * C = Temperature (10 bit value with -400 base)
 * D = Humidity
 * E = windspeed (* 0.3 m/s, correction for webapp = 3600/1000 * 0.3 * 100 = 108))
 * F = windgust (* 0.3 m/s, correction for webapp = 3600/1000 * 0.3 * 100 = 108))
 * G = Rain ( * 0.3 mm)
 * H = winddirection (0 = north, 4 = east, 8 = south 12 = west)
 * I = Checksum, calculation is still under investigation
 *
 * WS3000 and ACH2010 systems have no winddirection, message format is 8 bit shorter
 * Message Format: (10 Bytes, 80 bits):
 * AAAAAAAA AAAABBBB BBBB__CC CCCCCCCC DDDDDDDD EEEEEEEE FFFFFFFF GGGGGGGG GGGGGGGG HHHHHHHH
 *                         Temperature Humidity Windspd_ Windgust Rain____ ________ Checksum
 * 
 * DCF Time Message Format: (NOT DECODED!, we already have time sync through webapp)
 * AAAAAAAA BBBBCCCC DDDDDDDD EFFFFFFF GGGGGGGG HHHHHHHH IIIIIIII JJJJJJJJ KKKKKKKK LLLLLLLL MMMMMMMM
 * 	    11                 Hours   Minutes  Seconds  Year     Month    Day      ?        Checksum
 * B = 11 = DCF
 * C = ?
 * D = ?
 * E = ?
 * F = Hours BCD format (7 bits only for this byte, MSB could be '1')
 * G = Minutes BCD format
 * H = Seconds BCD format
 * I = Year BCD format (only two digits!)
 * J = Month BCD format
 * K = Day BCD format
 * L = ?
 * M = Checksum
 \*********************************************************************************************/

#ifdef DEVICE_09
#define DEVICE_ID 9
#define DEVICE_NAME "AlectoV2"

#define DKW2012_PULSECOUNT 176
#define ACH2010_MIN_PULSECOUNT 160 // reduce this value (144?) in case of bad reception
#define ACH2010_MAX_PULSECOUNT 160

boolean Device_09(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef DEVICE_CORE_09
  case DEVICE_RAWSIGNAL_IN:
    {
      if (!(((RawSignal.Number >= ACH2010_MIN_PULSECOUNT) && (RawSignal.Number <= ACH2010_MAX_PULSECOUNT)) || (RawSignal.Number == DKW2012_PULSECOUNT))) return false;

      const byte PTMF=50;
      byte c=0;
      byte rfbit;
      byte data[9]; 
      byte msgtype=0;
      byte rc=0;
      unsigned int rain=0;
      byte checksum=0;
      byte checksumcalc=0;
      byte basevar;
      byte maxidx = 8;

      if(RawSignal.Number > ACH2010_MAX_PULSECOUNT) maxidx = 9;  
      // Get message back to front as the header is almost never received complete for ACH2010
      byte idx = maxidx;
      for(byte x=RawSignal.Number; x>0; x=x-2)
        {
          if(RawSignal.Pulses[x-1]*PTMF < 0x300) rfbit = 0x80; else rfbit = 0;  
          data[idx] = (data[idx] >> 1) | rfbit;
          c++;
          if (c == 8) 
          {
            if (idx == 0) break;
            c = 0;
            idx--;
          }   
        }

      checksum = data[maxidx];
      checksumcalc = ProtocolAlectoCRC8(data, maxidx);
  
      msgtype = (data[0] >> 4) & 0xf;
      rc = (data[0] << 4) | (data[1] >> 4);

      if (checksum != checksumcalc) return false;
  
      basevar = ProtocolAlectoCheckID(rc);

      event->Command       = CMD_DEVICE_FIRST+DEVICE_ID;
      event->Par1=rc;
      event->Par2=basevar;
      event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af, dus unit op nul zetten
      event->Port          = VALUE_SOURCE_RF;

      if (basevar == 0) return true;
      if ((msgtype != 10) && (msgtype != 5)) return true;
  
      UserVar[basevar -1] = (float)(((data[1] & 0x3) * 256 + data[2]) - 400) / 10;
      UserVar[basevar +1 -1] = (float)data[3];

      rain = (data[6] * 256) + data[7];
      // check if rain unit has been reset!
      if (rain < ProtocolAlectoRainBase) ProtocolAlectoRainBase=rain;
      if (ProtocolAlectoRainBase > 0)
      {
        UserVar[basevar +2 -1] += ((float)rain - ProtocolAlectoRainBase) * 0.30;
      }
      ProtocolAlectoRainBase = rain;

      UserVar[basevar +3 -1] = (float)data[4] * 1.08;
      UserVar[basevar +4 -1] = (float)data[5] * 1.08;
      if (RawSignal.Number == DKW2012_PULSECOUNT) UserVar[basevar +5 -1] = (float)(data[8] & 0xf);

      success = true;
      break;
    }
  case DEVICE_COMMAND:
    {
      if ((event->Par2 > 0) && (ProtocolAlectoCheckID(event->Par1) == 0))
        {
          for (byte x=0; x<5; x++)
          {
            if (ProtocolAlectoValidID[x] == 0)
            {
              ProtocolAlectoValidID[x] = event->Par1;
              ProtocolAlectoVar[x] = event->Par2;
              break;
            }
          }
        }
      break;
    }
#endif // DEVICE_CORE_09

#ifdef NODO_MEGA
  case DEVICE_MMI_IN:
    {
      char *TempStr=(char*)malloc(26);
      string[25]=0;

      if(GetArgv(string,TempStr,1))
      {
        if(strcasecmp(TempStr,DEVICE_NAME)==0)
        {
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          if(event->Par1>0 && event->Par1<255 && event->Par2>0 && event->Par2<=USER_VARIABLES_MAX)
            success=true;
        }
      }
      free(TempStr);
      break;
    }

  case DEVICE_MMI_OUT:
    {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
      {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");
        strcat(string,int2str(event->Par1));
        strcat(string,",");
        strcat(string,int2str(event->Par2));
      }
      break;
    }
#endif //NODO_MEGA
  }      
  return success;
}
#endif //DEVICE_09


//#######################################################################################################
//##################################### Device-10 AlectoV3  #############################################
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Alecto weerstation buitensensoren
 * 
 * Auteur             : Nodo-team (Martinus van den Broek) www.nodo-domotica.nl
 *                      Support WS1100 door forumlid: Arendst
 * Support            : www.nodo-domotica.nl
 * Datum              : Mrt.2013
 * Versie             : 1.0
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Syntax             : "AlectoV3 <Par1:Sensor ID>, <Par2:Basis Variabele>"
 *********************************************************************************************
 * Technische informatie:
 * Decodes signals from Alecto Weatherstation outdoor unit, type 3 (94/126 pulses, 47/63 bits, 433 MHz).
 * WS1100 Message Format: (7 bits preamble, 5 Bytes, 40 bits):
 * AAAAAAA AAAABBBB BBBB__CC CCCCCCCC DDDDDDDD EEEEEEEE
 *                        Temperature Humidity Checksum
 * A = start/unknown, first 8 bits are always 11111111
 * B = Rolling code
 * C = Temperature (10 bit value with -400 base)
 * D = Checksum
 * E = Humidity
 *
 * WS1200 Message Format: (7 bits preamble, 7 Bytes, 56 bits):
 * AAAAAAA AAAABBBB BBBB__CC CCCCCCCC DDDDDDDD DDDDDDDD EEEEEEEE FFFFFFFF 
 *                        Temperature Rain LSB Rain MSB ???????? Checksum
 * A = start/unknown, first 8 bits are always 11111111
 * B = Rolling code
 * C = Temperature (10 bit value with -400 base)
 * D = Rain ( * 0.3 mm)
 * E = ?
 * F = Checksum
 \*********************************************************************************************/
 
#ifdef DEVICE_10
#define DEVICE_ID 10
#define DEVICE_NAME "AlectoV3"

#define WS1100_PULSECOUNT 94
#define WS1200_PULSECOUNT 126

boolean Device_10(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef DEVICE_CORE_10
  case DEVICE_RAWSIGNAL_IN:
    {
      if ((RawSignal.Number != WS1100_PULSECOUNT) && (RawSignal.Number != WS1200_PULSECOUNT)) return false;

      const byte PTMF=50;
      unsigned long bitstream1=0;
      unsigned long bitstream2=0;
      byte rc=0;
      int temperature=0;
      byte humidity=0;
      unsigned int rain=0;
      byte checksum=0;
      byte checksumcalc=0;
      byte basevar=0;
      byte data[6];

      // get first 32 relevant bits
      for(byte x=15; x<=77; x=x+2) if(RawSignal.Pulses[x]*PTMF < 0x300) bitstream1 = (bitstream1 << 1) | 0x1; 
      else bitstream1 = (bitstream1 << 1);
      // get second 32 relevant bits
      for(byte x=79; x<=141; x=x+2) if(RawSignal.Pulses[x]*PTMF < 0x300) bitstream2 = (bitstream2 << 1) | 0x1; 
      else bitstream2 = (bitstream2 << 1);

      data[0] = (bitstream1 >> 24) & 0xff;
      data[1] = (bitstream1 >> 16) & 0xff;
      data[2] = (bitstream1 >>  8) & 0xff;
      data[3] = (bitstream1 >>  0) & 0xff;
      data[4] = (bitstream2 >> 24) & 0xff;
      data[5] = (bitstream2 >> 16) & 0xff;

      if (RawSignal.Number == WS1200_PULSECOUNT)
      {
        checksum = (bitstream2 >> 8) & 0xff;
        checksumcalc = ProtocolAlectoCRC8(data, 6);
      }
      else
      {
        checksum = (bitstream2 >> 24) & 0xff;
        checksumcalc = ProtocolAlectoCRC8(data, 4);
      }

      rc = (bitstream1 >> 20) & 0xff;

      if (checksum != checksumcalc) return false;
      basevar = ProtocolAlectoCheckID(rc);

      event->Command       = CMD_DEVICE_FIRST+DEVICE_ID;
      event->Par1=rc;
      event->Par2=basevar;
      event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af, dus unit op nul zetten
      event->Port          = VALUE_SOURCE_RF;

      if (basevar == 0) return true;

      temperature = ((bitstream1 >> 8) & 0x3ff) - 400;
      UserVar[basevar-1] = (float)temperature / 10;

      if (RawSignal.Number == WS1200_PULSECOUNT)
      {
        rain = (((bitstream2 >> 24) & 0xff) * 256) + ((bitstream1 >> 0) & 0xff);
        // check if rain unit has been reset!
        if (rain < ProtocolAlectoRainBase) ProtocolAlectoRainBase=rain;
        if (ProtocolAlectoRainBase > 0)
        {
          UserVar[basevar+1 -1] += ((float)rain - ProtocolAlectoRainBase) * 0.30;
        }
        ProtocolAlectoRainBase = rain;
      }
      else
      {
        humidity = bitstream1 & 0xff;
        UserVar[basevar+1 -1] = (float)humidity / 10;
      }

      RawSignal.Number=0;
      success = true;
      break;
    }
  case DEVICE_COMMAND:
    {
      if ((event->Par2 > 0) && (ProtocolAlectoCheckID(event->Par1) == 0))
        {
          for (byte x=0; x<5; x++)
          {
            if (ProtocolAlectoValidID[x] == 0)
            {
              ProtocolAlectoValidID[x] = event->Par1;
              ProtocolAlectoVar[x] = event->Par2;
              break;
            }
          }
        }
      break;
    }
#endif // DEVICE_CORE_10

#ifdef NODO_MEGA
  case DEVICE_MMI_IN:
    {
      char *TempStr=(char*)malloc(26);
      string[25]=0;

      if(GetArgv(string,TempStr,1))
      {
        if(strcasecmp(TempStr,DEVICE_NAME)==0)
        {
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          if(event->Par1>0 && event->Par1<255 && event->Par2>0 && event->Par2<=USER_VARIABLES_MAX)
            success=true;
        }
      }
      free(TempStr);
      break;
    }

  case DEVICE_MMI_OUT:
    {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
      {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");
        strcat(string,int2str(event->Par1));
        strcat(string,",");
        strcat(string,int2str(event->Par2));
      }
      break;
    }
#endif //NODO_MEGA
  }      
  return success;
}
#endif //DEVICE_10


//#######################################################################################################
//##################################### Device-12 OregonV2  #############################################
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Oregon buitensensoren
 * 
 * Auteur             : Nodo-team (Martinus van den Broek) www.nodo-domotica.nl
 *                      Support THGN132N en code optimalisatie door forumlid: Arendst
 * Support            : www.nodo-domotica.nl
 * Datum              : Mrt.2013
 * Versie             : 1.0
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Syntax             : "OregonV2 <Par1:Sensor ID>, <Par2:Basis Variabele>"
 *********************************************************************************************
 * Technische informatie:
 * Only supports Oregon V2 protocol messages, message format consists of 18 or 21 nibbles:
 * THN132
 * AAAA AAAA AAAA AAAA BBBB CCCC CCCC CCCC CCCC DDDD EEEE EEEE FFFF GGGG GGGG GGGG HHHH IIII IIII
 *                          0    1    2    3    4    5    6    7    8    9    10   11   12   13
 *   A = preamble, B = sync bits, C = ID, D = Channel, E = RC, F = Flags,
 *   G = Measured value, 3 digits BCD encoded, H = sign, bit3 set if negative temperature
 *   I = Checksum, sum of nibbles C,D,E,F,G,H
 *
 * THGN132
 * AAAA AAAA AAAA AAAA BBBB CCCC CCCC CCCC CCCC DDDD EEEE EEEE FFFF GGGG GGGG GGGG HHHH IIII IIII JJJJ KKKK KKKK
 *                          0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16
 *   A = preamble, B = sync bits, C = ID, D = Channel, E = RC, F = Flags,
 *   G = Measured value, 3 digits BCD encoded, H = sign, bit3 set if negative temperature
 *   I = Humidity value
 *   J = Unknown
 *   K = Checksum, sum of nibbles C,D,E,F,G,H,I,J
 **********************************************************************************************/
 
#ifdef DEVICE_12
#define DEVICE_ID 12
#define DEVICE_NAME   "OregonV2"

#define THN132N_ID              1230
#define THGN123N_ID              721
#define THGR810_ID             17039
#define THN132N_MIN_PULSECOUNT   196
#define THN132N_MAX_PULSECOUNT   205
#define THGN123N_MIN_PULSECOUNT  228
#define THGN123N_MAX_PULSECOUNT  238

byte ProtocolOregonValidID[5];
byte ProtocolOregonVar[5];

boolean Device_12(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef DEVICE_CORE_12
  case DEVICE_RAWSIGNAL_IN:
    {
      const byte PTMF=50;
      byte nibble[17]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      byte y = 1;
      byte c = 1;
      byte rfbit = 1;
      byte sync = 0; 
      int id = 0;
      byte checksum = 0;
      byte checksumcalc = 0;
      int datavalue = 0;
      byte basevar=0;

      if (!((RawSignal.Number >= THN132N_MIN_PULSECOUNT && RawSignal.Number <= THN132N_MAX_PULSECOUNT) || (RawSignal.Number >= THGN123N_MIN_PULSECOUNT && RawSignal.Number <= THGN123N_MAX_PULSECOUNT))) return false;

      for(byte x=1;x<=RawSignal.Number;x++)
      {
        if(RawSignal.Pulses[x]*PTMF < 600)
        {
          rfbit = (RawSignal.Pulses[x]*PTMF < RawSignal.Pulses[x+1]*PTMF);
          x++;
          y = 2;
        }
        if (y%2 == 1)
        {
          // Find sync pattern as THN132N and THGN132N have different preamble length
          if (c == 1)
          {
            sync = (sync >> 1) | (rfbit << 3);
            sync = sync & 0xf;
            if (sync == 0xA) 
            {
              c = 2;
              if (x < 40) return false;
            }
          }
          else
          {
            if (c < 70) nibble[(c-2)/4] = (nibble[(c-2)/4] >> 1) | rfbit << 3;
            c++;
          }
        }
        y++;
      }
      // if no sync pattern match found, return
      if (c == 1) return false;
      
      // calculate sensor ID
      id = (nibble[3] << 16) |(nibble[2] << 8) | (nibble[1] << 4) | nibble[0];
 
      // calculate and verify checksum
      for(byte x=0; x<12;x++) checksumcalc += nibble[x];
      checksum = (nibble[13] << 4) | nibble[12];
      if ((id == THGN123N_ID) || (id == THGR810_ID))                           // Units with humidity sensor have extra data
        {
          checksum = (nibble[16] << 4) | nibble[15];
          for(byte x=13; x<16;x++) checksumcalc += nibble[x];
        }
      if (checksum != checksumcalc) return false;

      basevar = ProtocolOregonCheckID((nibble[6] << 4) | nibble[5]);

      event->Command       = CMD_DEVICE_FIRST+DEVICE_ID;
      event->Par1=(nibble[6] << 4) | nibble[5];
      event->Par2=basevar;
      event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af, dus unit op nul zetten
      event->Port          = VALUE_SOURCE_RF;

      if (basevar == 0) return true;

      // if valid sensor type, update user variable and process event
      if ((id == THGN123N_ID) || (id == THGR810_ID) || (id == THN132N_ID))
      {
        datavalue = ((1000 * nibble[10]) + (100 * nibble[9]) + (10 * nibble[8]));
        if ((nibble[11] & 0x8) == 8) datavalue = -1 * datavalue;
        UserVar[basevar -1] = (float)datavalue/100;
        if ((id == THGN123N_ID) || (id == THGR810_ID))
        {
          datavalue = ((1000 * nibble[13]) + (100 * nibble[12]));
          UserVar[basevar + 1 -1] = (float)datavalue/100;
        }
      }
      success = true;
      break;
    }
  case DEVICE_COMMAND:
    {
      if ((event->Par2 > 0) && (ProtocolOregonCheckID(event->Par1) == 0))
        {
          for (byte x=0; x<5; x++)
          {
            if (ProtocolOregonValidID[x] == 0)
            {
              ProtocolOregonValidID[x] = event->Par1;
              ProtocolOregonVar[x] = event->Par2;
              break;
            }
          }
        }
      break;
    }
#endif // DEVICE_CORE_12

#ifdef NODO_MEGA
  case DEVICE_MMI_IN:
    {
      char *TempStr=(char*)malloc(26);
      string[25]=0;

      if(GetArgv(string,TempStr,1))
      {
        if(strcasecmp(TempStr,DEVICE_NAME)==0)
        {
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          if(event->Par1>0 && event->Par1<255 && event->Par2>0 && event->Par2<=USER_VARIABLES_MAX)
            success=true;
        }
      }
      free(TempStr);
      break;
    }

  case DEVICE_MMI_OUT:
    {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
      {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");
        strcat(string,int2str(event->Par1));
        strcat(string,",");
        strcat(string,int2str(event->Par2));
      }
      break;
    }
#endif //NODO_MEGA
  }      
  return success;
}

#ifdef DEVICE_CORE_12
/*********************************************************************************************\
 * Check for valid sensor ID
 \*********************************************************************************************/
byte ProtocolOregonCheckID(byte checkID)
{
  for (byte x=0; x<5; x++) if (ProtocolOregonValidID[x] == checkID) return ProtocolOregonVar[x];
  return 0;
}
#endif //DEVICE_CORE_12
#endif //DEVICE_12


//#######################################################################################################
//##################################### Device-13 Flamingo FA20RF Rookmelder ############################
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Flamingo FA20RF rookmelder
 * 
 * Auteur             : Nodo-team (Martinus van den Broek) www.nodo-domotica.nl
 * Support            : www.nodo-domotica.nl
 * Datum              : Mrt.2013
 * Versie             : 1.0
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Syntax             : "SmokeAlert 0, <Par2: rookmelder ID>"
 *********************************************************************************************
 * Technische informatie:
 * De Flamingo FA20RF rookmelder bevat een RF zender en ontvanger. Standaard heeft elke unit een uniek ID
 * De rookmelder heeft een learn knop waardoor hij het ID van een andere unit kan overnemen
 * Daardoor kunnen ze onderling worden gekoppeld.
 * Na het koppelen hebben ze dus allemaal hetzelfde ID!
 * Je gebruikt 1 unit als master, waarvan de je code aanleert aan de andere units (slaves)
 \*********************************************************************************************/
#define FA20RFSTART                 3000
#define FA20RFSPACE                  800
#define FA20RFLOW                   1300
#define FA20RFHIGH                  2600

#ifdef DEVICE_13
#define DEVICE_ID 13
#define DEVICE_NAME "SmokeAlert"

boolean Device_13(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef DEVICE_CORE_13
  case DEVICE_RAWSIGNAL_IN:
    {
      byte PTMF=50;
      if (RawSignal.Number != 52) return false;

      unsigned long bitstream=0L;
      for(byte x=4;x<=50;x=x+2)
      {
        if (RawSignal.Pulses[x]*PTMF > 1800) bitstream = (bitstream << 1) | 0x1; 
        else bitstream = bitstream << 1;
      }

      event->Command       = CMD_DEVICE_FIRST+DEVICE_ID;
      event->Par1=0;
      event->Par2=bitstream;
      event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af, dus unit op nul zetten
      event->Port          = VALUE_SOURCE_RF;

      return true;
      break;
    }
  case DEVICE_COMMAND:
    {
      break;
    }
#endif // DEVICE_CORE_13

#ifdef NODO_MEGA
  case DEVICE_MMI_IN:
    {
      break;
    }

  case DEVICE_MMI_OUT:
    {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
      {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");
        strcat(string,int2str(event->Par1));
        strcat(string,",");
        strcat(string,int2str(event->Par2));
      }
      break;
    }
#endif //NODO_MEGA
  }      
  return success;
}
#endif //DEVICE_13


//#######################################################################################################
//##################################### Device-14 Flamingo FA20RF Rookmelder ############################
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor aansturen van Flamingo FA20RF rookmelder
 * 
 * Auteur             : Nodo-team (Martinus van den Broek) www.nodo-domotica.nl
 * Support            : www.nodo-domotica.nl
 * Datum              : Mrt.2013
 * Versie             : 1.0
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Syntax             : "SmokeAlertSend 0, <Par2: rookmelder ID>"
 *********************************************************************************************
 * Technische informatie:
 * De Flamingo FA20RF rookmelder bevat een RF zender en ontvanger. Standaard heeft elke unit een uniek ID
 * De rookmelder heeft een learn knop waardoor hij het ID van een andere unit kan overnemen
 * Daardoor kunnen ze onderling worden gekoppeld.
 * Na het koppelen hebben ze dus allemaal hetzelfde ID!
 * Je gebruikt 1 unit als master, waarvan de je code aanleert aan de andere units (slaves)
 *
 * Let op: De rookmelder geeft alarm zolang dit bericht wordt verzonden en stopt daarna automatisch
 \*********************************************************************************************/

#ifdef DEVICE_14
#define DEVICE_ID 14
#define DEVICE_NAME "SmokeAlertSend"

boolean Device_14(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef DEVICE_CORE_14
  case DEVICE_RAWSIGNAL_IN:
    {
      break;
    }

  case DEVICE_COMMAND:
    {
      unsigned long bitstream=event->Par2;
      byte PTMF=50;
      RawSignal.Pulses[0]=PTMF;

      RawSignal.Repeats=1;
      RawSignal.Pulses[1]=FA20RFSTART/PTMF;
      RawSignal.Pulses[2]=FA20RFSPACE/PTMF;
      RawSignal.Pulses[3]=FA20RFSPACE/PTMF;
      for(byte x=49;x>=3;x=x-2)
      {
        RawSignal.Pulses[x]=FA20RFSPACE/PTMF;
        if ((bitstream & 1) == 1) RawSignal.Pulses[x+1] = FA20RFHIGH/PTMF; 
        else RawSignal.Pulses[x+1] = FA20RFLOW/PTMF;
        bitstream = bitstream >> 1;
      }

      RawSignal.Pulses[51]=FA20RFSPACE/PTMF;
      RawSignal.Pulses[52]=0;
      RawSignal.Number=52;

      for (byte x =0; x<50; x++) RawSendRF();
      break;
    } 
#endif // DEVICE_CORE_14

#ifdef NODO_MEGA
  case DEVICE_MMI_IN:
    {
      char *TempStr=(char*)malloc(26);
      string[25]=0;

      if(GetArgv(string,TempStr,1))
      {
        if(strcasecmp(TempStr,DEVICE_NAME)==0)
        {
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          success=true;
        }
      }
      free(TempStr);
      break;
    }

  case DEVICE_MMI_OUT:
    {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
      {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");
        strcat(string,int2str(event->Par1));
        strcat(string,",");
        strcat(string,int2str(event->Par2));
      }
      break;
    }
#endif //NODO_MEGA
  }      
  return success;
}
#endif //DEVICE_14


//#######################################################################################################
//#################################### Device-15: HomeEasy EU ###########################################
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst HomeEasy EU zenders
 * die werken volgens de automatische codering (Ontvangers met leer-knop)
 *
 * LET OP: GEEN SUPPORT VOOR DIRECTE DIMWAARDES!!!
 *
 * Auteur             : Nodo-team (Martinus van den Broek) www.nodo-domotca.nl
 * Support            : www.nodo-domotica.nl
 * Datum              : Mrt.2013
 * Versie             : 1.0
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Syntax             : "HomeEasy <Adres>,<On|Off|>
 *********************************************************************************************
 * Technische informatie:
 * Analyses Home Easy Messages and convert these into NewKaku compatible eventcode
 * Only new EU devices with automatic code system are supported
 * Only  On / Off status is decoded, no DIM values
 * Only tested with Home Easy HE300WEU transmitter, doorsensor and PIR sensor
 * Home Easy message structure, by analyzing bitpatterns so far ...
 * AAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBCCCCDDEEFFFFFFG
 *       A = startbits/preamble, B = address, C = ?, D = Command, E = ?, F = Channel, G = Stopbit
 \*********************************************************************************************/
 
#define HomeEasy_LongLow    0x490    // us
#define HomeEasy_ShortHigh  200      // us
#define HomeEasy_ShortLow   150      // us

#ifdef DEVICE_15
#define DEVICE_ID 15
#define DEVICE_NAME "HomeEasy"

boolean Device_15(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef DEVICE_CORE_15
  case DEVICE_RAWSIGNAL_IN:
    {
      byte PTMF=50;
      unsigned long address = 0;
      unsigned long bitstream = 0;
      int counter = 0;
      byte rfbit =0;
      byte state = 0;
      unsigned long channel = 0;

      // valid messages are 116 pulses          
      if (RawSignal.Number != 116) return false;

      for(byte x=1;x<=RawSignal.Number;x=x+2)
      {
        if ((RawSignal.Pulses[x]*PTMF < 500) & (RawSignal.Pulses[x+1]*PTMF > 500)) 
          rfbit = 1;
        else
          rfbit = 0;

        if ((x>=23) && (x<=86)) address = (address << 1) | rfbit;
        if ((x>=87) && (x<=114)) bitstream = (bitstream << 1) | rfbit;

      }
      state = (bitstream >> 8) & 0x3;
      channel = (bitstream) & 0x3f;

      // Add channel info to base address, first shift channel info 6 positions, so it can't interfere with bit 5
      channel = channel << 6;
      address = address + channel;

      // Set bit 5 based on command information in the Home Easy protocol
      if (state == 1) address = address & 0xFFFFFEF;
      else address = address | 0x00000010;

      event->Par1=((address>>4)&0x01)?VALUE_ON:VALUE_OFF; // On/Off bit omzetten naar een Nodo waarde. 
      event->Par2=address &0x0FFFFFCF;         // Op hoogste nibble zat vroeger het signaaltype. 
      event->Command       = CMD_DEVICE_FIRST+DEVICE_ID;        // Command deel t.b.v. weergave van de string "HomeEasy"
      event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af, dus unit op nul zetten
      RawSignal.Repeats    = true; // het is een herhalend signaal. Bij ontvangst herhalingen onderdrukken.
      
      return true;      
      break;
    }

  case DEVICE_COMMAND:
    break;
#endif // DEVICE_CORE_15

#ifdef NODO_MEGA
  case DEVICE_MMI_IN:
    {
      char* str=(char*)malloc(40);
      string[25]=0; // kap voor de zekerheid de string af.

      if(GetArgv(string,str,1))
      {
        if(strcasecmp(str,DEVICE_NAME)==0)
        {
          // Hier wordt de tekst van de protocolnaam gekoppeld aan het device ID.
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          if(GetArgv(string,str,2))
          {
            event->Par2=str2int(str);    
            if(GetArgv(string,str,3))
            {
              // Vul Par1 met het HomeEasy commando. Dit kan zijn: VALUE_ON, VALUE_OFF, Andere waarden zijn ongeldig.

              // haal uit de tweede parameter een 'On' of een 'Off'.
              if(event->Par1=str2cmd(str))
                success=true;
            }
          }
        }
      }
      free(str);
      break;
    }

  case DEVICE_MMI_OUT:
    {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
      {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");

        // In Par3 twee mogelijkheden: Het bevat een door gebruiker ingegeven adres 0..255 of een volledig HomeEasy adres.
        if(event->Par2>=0x0ff)
          strcat(string,int2strhex(event->Par2)); 
        else
          strcat(string,int2str(event->Par2)); 

        strcat(string,",");

        if(event->Par1==VALUE_ON)
          strcat(string,"On");  
        else if(event->Par1==VALUE_OFF)
          strcat(string,"Off");
        else
          strcat(string,int2str(event->Par1));
        success=true;
      }
      break;
    }
#endif //NODO_MEGA
  }      
  return success;
}
#endif //DEVICE_15


//#######################################################################################################
//#################################### Device-16: SendHome Easy EU ######################################
//#######################################################################################################

/*********************************************************************************************\
 * Dit protocol zorgt voor aansturing van Home Easy EU ontvangers
 * die werken volgens de automatische codering (Ontvangers met leer-knop)
 *
 * LET OP: GEEN SUPPORT VOOR DIRECTE DIMWAARDES!!!
 * 
 * Auteur             : Nodo-team (Martinus van den Broek) www.nodo-domotca.nl
 * Support            : www.nodo-domotica.nl
 * Datum              : Mrt.2013
 * Versie             : 1.0
 * Nodo productnummer : n.v.t. meegeleverd met Nodo code.
 * Compatibiliteit    : Vanaf Nodo build nummer 508
 * Syntax             : "HomeEasySend <Adres>,<On|Off|>
 \*********************************************************************************************/

#ifdef DEVICE_16
#define DEVICE_ID 16
#define DEVICE_NAME "HomeEasySend"

boolean Device_16(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef DEVICE_CORE_16
  case DEVICE_RAWSIGNAL_IN:
    break;

  case DEVICE_COMMAND:
    {
      const byte PTMF=50;
      unsigned long bitstream=0L;
      byte address = 0;
      byte channel = 0;
      byte channelcode = 0;
      byte command = 0;
      byte i=1; // bitcounter in stream
      byte y; // size of partial bitstreams

      address = (event->Par2 >> 4) & 0x7;   // 3 bits address (higher bits from HomeEasy address, bit 7 not used
      channel = event->Par2 & 0xF;    // 4 bits channel (lower bits from HomeEasy address
      command = event->Par1 & 0xF;    // 12 = on, 0 = off

      if (channel == 0) channelcode = 0x8E;
      else if (channel == 1) channelcode = 0x96;
      else if (channel == 2) channelcode = 0x9A;
      else if (channel == 3) channelcode = 0x9C;
      else if (channel == 4) channelcode = 0xA6;
      else if (channel == 5) channelcode = 0xAA;
      else if (channel == 6) channelcode = 0xAC;
      else if (channel == 7) channelcode = 0xB2;
      else if (channel == 8) channelcode = 0xB4;
      else if (channel == 9) channelcode = 0xB8;
      else if (channel == 10) channelcode = 0xC6;
      else if (channel == 11) channelcode = 0xCA;
      else if (channel == 12) channelcode = 0xCC;
      else if (channel == 13) channelcode = 0xD2;
      else if (channel == 14) channelcode = 0xD4;
      else if (channel == 15) channelcode = 0xD8;

      y=11; // bit uit de bitstream, startbits
      bitstream = 0x63C;
      for (i=1;i<=22;i=i+2)
      {
        RawSignal.Pulses[i] = HomeEasy_ShortHigh/PTMF;
        if((bitstream>>(y-1))&1)          // bit 1
            RawSignal.Pulses[i+1] = HomeEasy_LongLow/PTMF;
        else                              // bit 0
        RawSignal.Pulses[i+1] = HomeEasy_ShortLow/PTMF;
        y--;
      }

      y=32; // bit uit de bitstream, address
      bitstream = 0xDAB8F56C + address;

      for (i=23;i<=86;i=i+2)
      {
        RawSignal.Pulses[i] = HomeEasy_ShortHigh/PTMF;
        if((bitstream>>(y-1))&1)          // bit 1
            RawSignal.Pulses[i+1] = HomeEasy_LongLow/PTMF;
        else                              // bit 0
        RawSignal.Pulses[i+1] = HomeEasy_ShortLow/PTMF;
        y--;
      }

      y=15; // bit uit de bitstream, other stuff

      bitstream = 0x5C00;  // bit 10 on, bit 11 off indien OFF
      if (event->Par1==VALUE_OFF) bitstream = 0x5A00;

      bitstream = bitstream + channelcode;

      for (i=87;i<=116;i=i+2)
      {
        RawSignal.Pulses[i] = HomeEasy_ShortHigh/PTMF;
        if((bitstream>>(y-1))&1)          // bit 1
            RawSignal.Pulses[i+1] = HomeEasy_LongLow/PTMF;
        else                              // bit 0
        RawSignal.Pulses[i+1] = HomeEasy_ShortLow/PTMF;
        y--;
      }

      RawSignal.Pulses[116]=0;
      RawSignal.Number=116; // aantal bits*2 die zich in het opgebouwde RawSignal bevinden  unsigned long bitstream=0L;
      event->Port=VALUE_ALL; // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.
      RawSignal.Repeats=5;   // vijf herhalingen.
      SendEvent(event,true,true,true);
      success=true;
      break;
    }
#endif // DEVICE_CORE_16

#ifdef NODO_MEGA
  case DEVICE_MMI_IN:
    {
      char* str=(char*)malloc(40);
      string[25]=0; // kap voor de zekerheid de string af.

      if(GetArgv(string,str,1))
      {
        if(strcasecmp(str,DEVICE_NAME)==0)
        {
          // Hier wordt de tekst van de protocolnaam gekoppeld aan het device ID.
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          if(GetArgv(string,str,2))
          {
            event->Par2=str2int(str);    
            if(GetArgv(string,str,3))
            {
              // Vul Par1 met het HomeEasy commando. Dit kan zijn: VALUE_ON, VALUE_OFF, Andere waarden zijn ongeldig.
              // haal uit de tweede parameter een 'On' of een 'Off'.
              if(event->Par1=str2cmd(str))
                success=true;
            }
          }
        }
      }
      free(str);
      break;
    }

  case DEVICE_MMI_OUT:
    {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
      {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");

        // In Par3 twee mogelijkheden: Het bevat een door gebruiker ingegeven adres 0..255 of een volledig HomeEasy adres.
        if(event->Par2>=0x0ff)
          strcat(string,int2strhex(event->Par2)); 
        else
          strcat(string,int2str(event->Par2)); 

        strcat(string,",");

        if(event->Par1==VALUE_ON)
          strcat(string,"On");  
        else if(event->Par1==VALUE_OFF)
          strcat(string,"Off");
        else
          strcat(string,int2str(event->Par1));
        success=true;
      }
      break;
    }
#endif //NODO_MEGA
  }      
  return success;
}
#endif //DEVICE_16



//#######################################################################################################
//#################################### Device-15: Signal Analyzer   #####################################
//#######################################################################################################


/*********************************************************************************************\
 * Dit device geeft een pulsenreeks weer die op RF of IR is binnengekomen. Het is bedoeld
 * als hulpmiddel om signalen te analyseren in geval je niet beschikt over een scope of
 * een logic analyzer. Signaal wordt alleen weergegeven op Serial/USB. Manchster laat alle
 * Mark & Spaces zien. 
 *
 * Auteur             : Paul Tonkes
 * Support            : www.nodo-domotica.nl
 * Datum              : 10-03-2013
 * Versie             : 0.1
 * Nodo productnummer : <Nodo productnummer. Toegekend door Nodo team>
 * Compatibiliteit    : R513
 * Syntax             : "SignalAnalyze"
 *
 ***********************************************************************************************
 * Technische beschrijving:
 *
 * Compiled size      : 1200 bytes voor een Mega.
 * Externe funkties   : <geef hier aan welke funkties worden gebruikt. 
 *
 * 
 \*********************************************************************************************/
 
#ifdef DEVICE_18

#define DEVICE_ID 18
#define DEVICE_NAME "RawSignalAnalyze"

boolean Device_18(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  
  switch(function)
    {
    #ifdef DEVICE_CORE_18
    case DEVICE_RAWSIGNAL_IN:
      {
      return false;
      }      

    case DEVICE_COMMAND:
      {
      if(RawSignal.Number<8)return false;
      
      int x;
      unsigned int y,z;
      byte PTMF=RawSignal.Pulses[0];
    
      // zoek naar de langste kortst puls en de kortste lange puls
      unsigned int MarkShort=50000;
      unsigned int MarkLong=0;
      for(x=5;x<RawSignal.Number;x+=2)
        {
        y=RawSignal.Pulses[x]*PTMF;
        if(y<MarkShort)
          MarkShort=y;
        if(y>MarkLong)
          MarkLong=y;
        }
      z=true;
      while(z)
        {
        z=false;
        for(x=5;x<RawSignal.Number;x+=2)
          {
          y=RawSignal.Pulses[x]*PTMF;
          if(y>MarkShort && y<(MarkShort+MarkShort/2))
            {
            MarkShort=y;
            z=true;
            }
          if(y<MarkLong && y>(MarkLong-MarkLong/2))
            {
            MarkLong=y;
            z=true;
            }
          }
        }
      unsigned int MarkMid=((MarkLong-MarkShort)/2)+MarkShort;
  
      // zoek naar de langste kortst puls en de kortste lange puls
      unsigned int SpaceShort=50000;
      unsigned int SpaceLong=0;
      for(x=4;x<RawSignal.Number;x+=2)
        {
        y=RawSignal.Pulses[x]*PTMF;
        if(y<SpaceShort)
          SpaceShort=y;
        if(y>SpaceLong)
          SpaceLong=y;
        }
      z=true;
      while(z)
        {
        z=false;
        for(x=4;x<RawSignal.Number;x+=2)
          {
          y=RawSignal.Pulses[x]*PTMF;
          if(y>SpaceShort && y<(SpaceShort+SpaceShort/2))
            {
            SpaceShort=y;
            z=true;
            }
          if(y<SpaceLong && y>(SpaceLong-SpaceLong/2))
            {
            SpaceLong=y;
            z=true;
            }
          }
        }
      int SpaceMid=((SpaceLong-SpaceShort)/2)+SpaceShort;
    
      // Bepaal soort signaal
      y=0;
      if(MarkLong  > (2*MarkShort  ))y=1; // PWM
      if(SpaceLong > (2*SpaceShort ))y+=2;// PDM

      Serial.print(F( "Bits="));

      if(y==0)Serial.println(F("?"));
      if(y==1)
        {
        for(x=1;x<RawSignal.Number;x+=2)
          {
          y=RawSignal.Pulses[x]*PTMF;
          if(y>MarkMid)
            Serial.write('1');
          else
            Serial.write('0');
          }
        Serial.print(F(", Type=PWM"));
        }
      if(y==2)
        {
        for(x=2;x<RawSignal.Number;x+=2)
          {
          y=RawSignal.Pulses[x]*PTMF;
          if(y>SpaceMid)
            Serial.write('1');
          else
            Serial.write('0');
          }
        Serial.print(F(", Type=PDM"));
        }
      if(y==3)
        {
        for(x=1;x<RawSignal.Number;x+=2)
          {
          y=RawSignal.Pulses[x]*PTMF;
          if(y>MarkMid)
            Serial.write('1');
          else
            Serial.write('0');
          
          y=RawSignal.Pulses[x+1]*PTMF;
          if(y>SpaceMid)
            Serial.write('1');
          else
            Serial.write('0');
          }
        Serial.print(F( ", Type=Manchester"));
        }

      Serial.print(F(", Pulses="));
      Serial.print(RawSignal.Number/2);

      Serial.print(F(", Pulses(uSec)="));      
      for(x=1;x<RawSignal.Number;x++)
        {
        Serial.print(RawSignal.Pulses[x]*PTMF); 
        Serial.write(',');       
        }
      Serial.println();
      
//      int dev=250;  
//      for(x=1;x<=RawSignal.Number;x+=2)
//        {
//        for(y=1+int(RawSignal.Pulses[x])*PTMF/dev; y;y--)
//          Serial.write('M');  // Mark  
//        for(y=1+int(RawSignal.Pulses[x+1])*PTMF/dev; y;y--)
//          Serial.write('_');  // Space  
//        }    
//      Serial.println();

      break;
      }      
    #endif // CORE
      
    #ifdef NODO_MEGA
    case DEVICE_MMI_IN:
      {
      char *TempStr=(char*)malloc(26);
      string[25]=0;
      if(GetArgv(string,TempStr,1))
        {
        if(strcasecmp(TempStr,DEVICE_NAME)==0)
          {
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
          success=true;
          }
        }
      free(TempStr);
      break;
      }

    case DEVICE_MMI_OUT:
      {
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
        {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel
        strcat(string," ");
        strcat(string,int2str(event->Par1));
        }
      break;
      }
    #endif //NODO_MEGA
    }      
  return success;
  }
#endif //DEVICE_18


          

//#######################################################################################################
//#################################### Device-99: Leeg   ################################################
//#######################################################################################################


/*********************************************************************************************\
 * Funktionele beschrijving:
 * 
 * <Geef hier een beschrijving van de funktionele werking van het device en hoe deze door de gebruiker
 * moet worden aangeroepen.>
 *
 * Auteur             : <naam en EMail adres van de ontwikkelaar>
 * Support            : <website>
 * Datum              : <datum>
 * Versie             : <verise>
 * Nodo productnummer : <Nodo productnummer. Toegekend door Nodo team>
 * Compatibiliteit    : Vanaf Nodo build nummer <build nummer>
 * Syntax             : "MijnDevice <Par1>, <Par2>"
 *
 ***********************************************************************************************
 * Technische beschrijving:
 *
 * Compiled size      : <grootte> bytes voor een Mega en <grootte> voor een Small.
 * Externe funkties   : <geef hier aan welke funkties worden gebruikt. 
 *
 * <Geef hier een technische toelichting op de werking van het device en eventueel gebruikte protocol>
 * 
 * Tips en aandachtspunten voor programmeren van een protocol:
 * 
 * -  Geheugen is beperkt. Programmeer compact en benut iedere byte RAM en PROGMEM. De Arduino heeft niet de luxe van een PC!
 *    Het is verantwoordelijkheid van de programmeur om te bewaken. Vrij geheugen is opvraagbaar met [Status FreeMem].
 * -  Bouw geen lange wachtlussen in de code. Dit kan leiden tot timings-promlemen waaronder missen van events.
 *    Deze funktie bevindt zich ik een tijdkritische loop. Detecteer zo snel mogelijk of het ontvangen signaal
 *    ook bij dit protocol hoort. 
 * -  De array RawSignal.Pulses[] bevat alle Mark en Space tijden in microseconden. Te beginnen vanaf element [1].
 *    Deze array is reeds gevuld bij aankomst. Element [0] bevat een vermenigvuldigingsfactor voor omrekenen naar
 *    echte microseconden.
 * -  RawSignal.Pulses bevat het aantal posities die een mark/space bevatten. Let op dat de waarde RAW_BUFFER_SIZE
 *    nooit overschreden wordt. Anders gegarandeerd vastlopers! Positie [1] bevat de 'mark' van de startbit. 
 * -  De struct NodoEventStruct bevat alle informatie die nodig is voor verwerking en weergave van het event
 *    dat is ontvangen of moet worden weergegeven. 
 *    of er voldoende geheugen over blijft voor stabiele werking van de Nodo.
 * -  Om uitwisselbaar te blijven met andere Nodo versies en Nodo gebruikers, geen aanpassingen aan de Code maken.
 *    Let op dat je bij gebruik van functies uit de Nodo code je je eigen code gevoelig makt voor onderhoud
 *    bij uitbrengen van nieuwe releases.
 * -  Maak geen gebruik van interrupt driven routines, dit verstoort (mogelijk) de werking van de I2C, Serial en ethernet
 *    communicatie.
 * -  Maak slecht in uitzonderlijke gevallen gebruik van Globals en pas geen waarden van globals uit de Nodo code aan.
 * -  Besteed uitgebreid aandacht aan de documentatie van het protocol. Indien mogelijk verwijzen naar originele
 *    specificatie.
 *
 * Voorbeelden van userdevices:
 * - Digitale temperatuur sensoren (Zoals Dallas DS18B20)
 * - Digitale vochtigheidssensoren (Zoals DTH-11)
 * - Vergroten van aantal digitale wired met een multiplexer. Tot 8-ingangen per Wired poort met bv. een 74151.
 * - Acht verschillende analoge ingangen meten met eén WiredIn met een LTC1380.
 * - WiredOut uitgangen uitbreiden tot 8, 16, 32, 64 verschillende digitale uitgangen met behulp van 74HCT595
 * - I2C devices aansturen via de SLC en SDA lijnen van de arduino.
 * - etcetera.
 *
 * De Wired poorten en de additionele IO poorten op de Mega in uw eigen code gebruiken aan de hand van de naam zoals deze zijn gedefinieerd
 * in de code:
 *
 * PIN_WIRED_OUT_n staat voor WiredOut poort, waarbij n overeen komt met het nummer van de WiredOut poort te beginnen met 1.
 * PIN_WIRED_IN_n staat voor WiredOut poort, waarbij n overeen komt met het nummer van de WiredOut poort te beginnen met 1.
 * PIN_IO_n staat voor additionele IO-poort, waarbij n overeen komt met het nummer van deze poort, te beginnen met 1 (Alleen beschikbaar op de Mega).
 * 
 * Voor de verwijzing naar de Arduino pinnummers: zie schema of declaraties in tabblad "Base"
 * ANDERE POORTEN NIET GEBRUIKEN OMDAT DEZE (IN DE TOEKOMST) EEN SPECIFIEKE FUNKTIE HEBBEN.
 * Let bij het ontwerp van de Hardware en de software op dat geen gebruik wordt gemaakt van de Arduino pinnen die al een voorgedefinieerde
 * Nodo funktie hebben, De WiredIn en WiredOut kunnen wel vrij worden gebruikt met die kanttekening dat Nodo commando's de lijnen eveneens 
 * Besturen wat kan leiden tot beschadiging van hardware. Let hier in het bijzonder op als een Arduino pin wordt gedefinieerd als een output.
 * Op de Nodo Mega zijn nog vier extra communicatielijnen die gebruikt kunnen worden voor User input/output: PIN_IO_1 t/m PIN_IO_4 (Arduino pin 38 t/m 41)
 * Besef dat niet alle pennen van de Arduino gebruikt kunnen worden daar vele Arduino pinnen al een voorgedefinieerde
 * Nodo funktie hebben, De WiredIn en WiredOut kunnen wel vrij worden gebruikt met die kanttekening dat Nodo commando's de lijnen eveneens 
 * Besturen wat kan leiden tot beschadiging van hardware. Let hier in het bijzonder op als een Arduino pin wordt gedefinieerd als een output.
 * Op de Nodo Mega zijn nog vier extra communicatielijnen die gebruikt kunnen worden voor User input/output: PIN_IO_1 t/m PIN_IO_4 (Arduino pin 38 t/m 41)
 \*********************************************************************************************/
 
// Compileer de code van dit divice alleen als de gebruiker in het Nodo tabblad de definitie DEVICE_99 heeft
// opgenomen. Ins dit niet het geval, dan zal de code ook niet worden gecompileerd en geen geheugenruimte in beslag nemen.
#ifdef DEVICE_99

// Ieder device heeft een uniek ID. Deze worden onderhouden door het Nodo team. Als je een device hebt geprogrammeerd
// die van waarde kan zijn voor andere gebruikers, meldt deze dan aan bij het Nodo team zodat deze kan worden meegenomen
// in de Nodo-releases. Device 99 is een "knutsel" device voor de gebruiker.
#define DEVICE_ID 99

// Een device heeft naast een uniek ID ook een eigen MMI string die de gebruiker kan invoeren via Telnet, Serial, HTTP 
// of een script. Geef hier de naam op. De afhandeling is niet hoofdletter gevoelig.
#define DEVICE_NAME "MyUserDevice"

// Deze device code wordt vanuit meerdere plaatsen in de Nodo code aangeroepen, steeds met een doel. Dit doel bevindt zich
// in de variabele [function]. De volgende doelen zijn gedefinieerd:
//
// DEVICE_RAWSIGNAL_IN  => Afhandeling van een via RF/IR ontvangen event
// DEVICE_COMMAND       => Commando voor afhandelen/uitsturen van een event.
// DEVICE_MMI_IN        => Invoer van de gebruiker/script omzetten naar een event. (Alleen voor mega)
// DEVICE_MMI_OUT       => Omzetten van een event naar een voor de gebruiker leesbare tekst (Alleen voor Mega)
// DEVIDE_ONCE_A_SECOND => ongeveer iedere seconde.
// DEVICE_INIT          => Eenmalig, direct na opstarten van de Nodo

  #ifdef DEVICE_CORE_99
  switch(function)
    {    
    case DEVICE_ONCE_A_SECOND:
      {
      // Dit deel van de code wordt (ongeveer!) eenmaal per seconde aangeroepen. Let op dat deze code zich binnen een 
      // tijdkritisch deel van de hoofdloop van de Nodo bevindt! Gebruik dus alleen om snel een waarde te (re)setten
      // of om de status van een poort/variabele te checken. Zolang de verwerking zich hier plaats vindt zal de
      // Nodo géén IR of RF events kunnen ontvangen.
      break;
      }

    case DEVICE_RAWSIGNAL_IN:
      {
      // Code op deze plaats wordt uitgevoerd zodra er een event via RF of IR is binnengekomen
      // De RawSignal buffer is gevuld met pulsen. de struct [event] moet hier worden samengesteld.      
      // Als decoderen is gelukt, dan de variabele [success] vullen met een true. De Nodo zal het 
      // event vervolgens als een regulier event afhandelen.
      break;
      }
      
    case DEVICE_COMMAND:
      {
      // Als er vanuit de gebruiker, script of eventlist dit device een event moet uitsturen, dan is op het
      // moment dat de code hier wordt aangeroepen, de struct [event] gevuld en gereed voor verwerking.
      // Als voor verlaten de struct [event] is gevuld met een ander event, dan wordt deze uitgevoerd als een nieuw
      // event. Dit kan bijvoorbeeld worden benut als een variabele wordt uitgelezen en de waarde verder verwerkt
      // moet worden.
      break;

    case DEVICE_INIT:
      {
      // Code hier wordt eenmalig aangeroepen na een reboot van de Nodo.
      break;
      }
      
    #ifdef NODO_MEGA
    case DEVICE_MMI_IN:
      {
      // Zodra er via een script, HTTP, Telnet of Serial een commando wordt ingevoerd, wordt dit deel van de code langs gelopen.
      // Op deze plet kan de invoer [string] worden gepardsed en omgezet naar een struct [event]. Als parsen van de invoerstring [string]
      // is gelukt en de struct is gevuld, dan de variabele [success] vullen met true zodat de Nod zorg kan dragen voor verdere verwerking van het event.

      char *TempStr=(char*)malloc(26);
      string[25]=0;

      // Met DEVICE_MMI_IN wordt de invoer van de gebruiker (string) omgezet naar een event zodat de Nodo deze intern kan verwerken.
      // Hier aangekomen bevat string het volledige commando. Test als eerste of het opgegeven commando overeen komt met DEVICE_NAME
      // Dit is het eerste argument in het commando. 
      if(GetArgv(string,TempStr,1))
        {
        if(strcasecmp(TempStr,DEVICE_NAME)==0)
          {
          // Hier wordt de tekst van de protocolnaam gekoppeld aan het device ID.
          event->Command=CMD_DEVICE_FIRST+DEVICE_ID;
                    
          // Vervolgens tweede parameter gebruiken
          if(GetArgv(string,TempStr,2)) 
            {
            // plaats hier je code die de tweede parameter die zich in [TempStr] bevindt verder uitparsed
            // De byte Par1 en unsigned long Par2 die zic in de struct [event] bevindt kunnen worden gebruikt.
              
            if(GetArgv(string,TempStr,3))
              {
              // indien gewenst kan een tweede parameter worden gebruikt (=derde argument)
              // Plaats wederom de code paar uitparsen van de parameter hier.
              // heb je het laatste parameter geparsen, dan de variabele [success] vullen 
              // met een true zodat verdere verwerking van het event plaats kan vinden.
              }
            }
          }
        }
      free(TempStr);
      break;
      }

    case DEVICE_MMI_OUT:
      {
      // De code die zich hier bevindt zorgt er voor dan een event met het unieke ID in de struct [event] weer wordt
      // omgezet naar een leesbaar event voor de gebruiker. het resultaat moet worden geplaatst in de string [string]
      // let op dat het totale commando niet meer dan 25 posities in beslag neemt.
      // Dit deel van de code wordt alleen uitgevoerd door een Nodo Mega, omdat alleen deze over een MMI beschikt.
      if(event->Command==CMD_DEVICE_FIRST+DEVICE_ID)
        {
        strcpy(string,DEVICE_NAME);            // Eerste argument=het commando deel

        // Vervolgens kan met strcat() hier de parameters aan worden toegevoegd      
        }
      break;
      }
    #endif //NODO_MEGA
    }      
  #endif // CORE
  return success;
  }
#endif //DEVICE_99



