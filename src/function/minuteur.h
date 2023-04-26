#ifndef MINUTEUR_FUNCTIONS
#define MINUTEUR_FUNCTIONS

#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#ifdef ESP32
  #include <FS.h>
  #include "SPIFFS.h"
  #define LittleFS SPIFFS // Fonctionne, mais est-ce correct? 
#else
  #include <LittleFS.h>
#endif

//// NTP 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

#include "config/enums.h"

extern System sysvar;


/// @brief ///////init du NTP 
void ntpinit() {
      // Configurer le serveur NTP et le fuseau horaire
  timeClient.begin();
  timeClient.setTimeOffset(3600); // Fuseau horaire (en secondes, ici GMT+1)
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
}


//////// structure pour les programmateurs. 
struct Programme {
  public:char heure_demarrage[6];
  public:char heure_arret[6];
  public:int temperature=50;
  public:bool run; 
  public:int heure;
  public:int minute;
  public:String name;

  /// @brief sauvegarde
  /// @param programme_conf 
  public:void saveProgramme() {
        const char * c_file = name.c_str();
        DynamicJsonDocument doc(192);
      
        // Set the values in the document
        doc["heure_demarrage"] = heure_demarrage;
        doc["heure_arret"] = heure_arret;
        doc["temperature"] = temperature;
        
          // Open file for writing
        File configFile = LittleFS.open(c_file, "w");
        if (!configFile) {
          Serial.println(F("Failed to open config file for writing"));
          return;
        }

        // Serialize JSON to file
        if (serializeJson(doc, configFile) == 0) {
          Serial.println(F("Failed to write to file"));
        }
        
        configFile.close();
  }

  /// @brief chargement
  /// @param programme_conf 
  

  public:bool loadProgramme() {
        const char * c_file = name.c_str();
        File configFile = LittleFS.open(c_file, "r");

        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/v6/assistant to compute the capacity.
        DynamicJsonDocument doc(192);

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, configFile);
        if (error) {
          Serial.println(F("Failed to read minuterie config "));
          return false;
        }
      
        strlcpy(heure_demarrage,                  // <- destination
                doc["heure_demarrage"] | "", // <- source
                sizeof(heure_demarrage));         // <- destination's capacity
        
        strlcpy(heure_arret,                  // <- destination
                doc["heure_arret"] | "", // <- source
                sizeof(heure_arret));         // <- destination's capacity
        temperature = doc["temperature"] | 50 ; /// defaut à 50 °
        configFile.close();
      return true;    
  }


bool start_progr() {
  int heures, minutes;
  sscanf(heure_demarrage, "%d:%d", &heures, &minutes);
      
  if(timeClient.isTimeSet()) {
    if (heures == timeClient.getHours() && minutes == timeClient.getMinutes()) {
        run=true; 
        timeClient.update();
        return true; 
    }
  }
return false; 
}

bool stop_progr() {
  int heures, minutes;
  /// sécurité temp
  if ( sysvar.celsius >= temperature ) { 
    run=false; 
     // protection flicking
    sscanf(heure_demarrage, "%d:%d", &heures, &minutes);  
    if (heures == timeClient.getHours() && minutes == timeClient.getMinutes()) {
      delay(15000);
    }
  return true; 
  }
  
  sscanf(heure_arret, "%d:%d", &heures, &minutes);
  if(timeClient.isTimeSet()) {
    if (heures == timeClient.getHours() && minutes == timeClient.getMinutes()) {
        run=false; 
        timeClient.update();
        return true; 
    }
  }
  return false; 
}


};











#endif