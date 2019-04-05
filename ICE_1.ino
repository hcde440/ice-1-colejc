//Ex1

//We include libraries to allow for different functions in our code
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> //provides the ability to parse and construct JSON objects

//We create a few variables to allow us to quickly change WiFi networks, and access API keys
const char* ssid = "Sif";
const char* pass = "appletree3";
const char* key = "3f376904a0afa25fde48d7ba9b9cbd71";
const char* key2 = "4eedd73e52133aedc410d57c74cb697f";

//MetData is a structure containing names and values for several different variables. We create
//each container here, and will later use an API to fill in the values. MetData is used for weather information.
typedef struct {
  String nam;    
  String wea;   
  String wea2; 
  String tem;
  String win;
  String win2;
  String hum;
  String clo;
} MetData;

//GeoData is used for providing location data such as city, country, IP, lat/longitude, etc.
typedef struct {
  String ip;
  String cc;
  String cn;
  String rc;
  String rn;
  String cy;
  String tz;
  String ln;
  String lt;
} GeoData;

//These lines create individual instances of GeoData and MetData, for our use.
GeoData location;
MetData weather;

void setup() {
  //These lines of code start up the process, and output the location of the file the machine is running
  //as well as the date and time of upload.
  Serial.begin(115200);
  delay(10);
  Serial.print("This board is running: ");
  Serial.println(F(__FILE__));
  Serial.print("Compiled: ");
  Serial.println(F(__DATE__ " " __TIME__));

  //We output a "connecting to ___" message, and attempt to connect to WiFi. While we aren't connected,
  //a while loop outputs a "." every half second so we know the machine is in the process of connecting.
  Serial.print("Connecting to "); Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //Once we connect, we output a confirmation and the device IP address.
  Serial.println(); Serial.println("WiFi connected"); Serial.println();
  Serial.print("Your ESP has been assigned the internal IP address ");
  Serial.println(WiFi.localIP());

  //These two lines collect our IP by calling getIP(), and then use that information to call getGeo().
  String ipAddress = getIP();
  getGeo(ipAddress);

  //These lines output information we collected from getGeo that has been stored in a GeoData structure.
  Serial.println();
  Serial.println("Your external IP address is " + location.ip);
  Serial.print("Your ESP is currently in " + location.cn + " (" + location.cc + "),");
  Serial.println(" in or near " + location.cy + ", " + location.rc + ".");
  Serial.print("You are in the " + location.tz + " timezone ");
  Serial.println("and located at (roughly) ");
  Serial.println(location.lt + " latitude by " + location.ln + " longitude.");
  Serial.println();

  //These lines call getMet(), which will fill in information in our MetData structure we created earlier.
  //Then this data will be printed in the serial monitor for our use.
  getMet();
  Serial.println();
  Serial.println("Your city is " + weather.nam);
  Serial.println("The current weather classification is: " + weather.wea);
  Serial.println("The current weather description is: " + weather.wea2);
  Serial.println("The current temperature is: " + weather.tem);
  Serial.println("The current humidity is: " + weather.hum);
  Serial.println("The current wind speed is: " + weather.win);
  Serial.println("The current wind direction is: " + weather.win2);
  Serial.println("The current cloud coverage is: " + weather.clo + "%");
  
}

//We don't want to endlessly loop anything in this code, so loop() is left empty.
void loop() {
}

//This code grabs our IP address and returns it.
String getIP() {
  //Create variables and grab our IP address listed inside inside a web address, written in JSON.
  HTTPClient theClient;
  String ipAddress;
  theClient.begin("http://api.ipify.org/?format=json");
  int httpCode = theClient.GET();

  //We make sure we don't have an error code by checking if httpCode is greater than zero, and then if
  //it is equal to 200.
  if (httpCode > 0) {
    if (httpCode == 200) {

      //We create jsonBuffer and a string from the web address called payload. We then parse through this
      //string and organize it in JSON format, and fill in "ipAddress" with the given IP using the JSON
      //data structure.
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      JsonObject& root = jsonBuffer.parse(payload);
      ipAddress = root["ip"].as<String>();

    //If we made an error, print something to let us know.
    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
      return "error";
    }
  }
  //Send the IP address back to any line that called getIP().
  return ipAddress;
}

//A method of grabbing weather data and filling the weather instance of MetData.
void getMet() {
  //Similar to getIP()'s first lines, except the URL is pieced together with information such as
  //the city name provided by our GeoData structure and an API key.
  HTTPClient theClient;
  Serial.println("Making HTTP request");
  String apiCall = "http://api.openweathermap.org/data/2.5/weather?q=";
  apiCall += location.cy;
  apiCall += "&units=imperial&appid=";
  apiCall += key;

  //Collect the information from the URL, and check to make sure we haven't encountered any errors.
  theClient.begin(apiCall);
  int httpCode = theClient.GET();
  if (httpCode > 0) {
    if (httpCode == 200) {

      //Print a line that lets us know the correct payload has been received, and organize the information
      //we've grabbed from online in JSON format in the same way we did for getIP().
      Serial.println("Received HTTP payload.");
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      Serial.println("Parsing...");
      JsonObject& root = jsonBuffer.parse(payload);

      // Test if parsing succeeds, and if it didn't, output a notice.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(payload);
        return;
      }

      //We use these several lines of code to name the previously empty variables in the weather instance
      //of MetData. Each line starts with root and digs through the JSON structure to get to the exact
      //value we need.
      weather.nam = root["name"].as<String>();
      weather.wea = root["weather"][0]["main"].as<String>();
      weather.wea2 = root["weather"][0]["description"].as<String>();
      weather.tem = root["main"]["temp"].as<String>();
      weather.win = root["wind"]["speed"].as<String>();
      weather.hum = root["main"]["humidity"].as<String>();
      weather.win2 = root["wind"]["deg"].as<String>();
      weather.clo = root["clouds"]["all"].as<String>();

    //Output a line in case httpCode is the incorrect value.
    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
    }
  }
}

//A method of grabbing location data and filling the location instance of GeoData.
void getGeo(String IP) {
  //String together a URL using getIP() and a key to begin theClient for later use.
  HTTPClient theClient;
  theClient.begin("http://api.ipstack.com/" + getIP() + "?access_key=" + key2);

  //Make sure we don't have any error codes.
  int httpCode = theClient.GET();
  if (httpCode > 0) {
    if (httpCode == 200) {
      //Collect JSON as a string, and then piece it together in a readable JSON format called root.
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      JsonObject& root = jsonBuffer.parse(payload);

      //If we didn't make root successfully, print a line in the serial monitor and end getGeo().
      if (!root.success()) {
        Serial.println("parseObject() failed in getGeo()");
        return;
      }
      
      //Fill in the location instance of GeoData with information from the JSON structure we just made.
      location.ip = root["ip"].as<String>();
      location.cc = root["country_code"].as<String>();
      location.cn = root["country_name"].as<String>();
      location.rc = root["region_code"].as<String>();
      location.rn = root["region_name"].as<String>();
      location.cy = root["city"].as<String>();
      location.tz = root["time_zone"].as<String>();
      location.lt = root["latitude"].as<String>();
      location.ln = root["longitude"].as<String>();

    }
    //If something went wrong with httpCode, print a line to the serial monitor.
    else {
      Serial.println("Something went wrong with connecting to the endpoint in getGeo().");
    }
  }
}
