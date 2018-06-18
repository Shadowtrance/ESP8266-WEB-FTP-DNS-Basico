#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include "./DNSServer.h"
#include "./ESP8266FtpServer.h"
#include <ArduinoJson.h>

//DNS configuration port
const byte DNSPort = 53;
//HTTP configuration port
const byte HTTPPort = 80;

//Configuration file structure
struct FileConfiguration {
  char *FTPUser = "Admin";
  char *FTPPass = "Upload";
  char *WIFISSID = "ESP8266";
  char *WIFIPass = "12345678";
  IPAddress IP = IPAddress(13,13,13,13);
  IPAddress Subnet = IPAddress(255,255,255,0);
};

//Configuration file
FileConfiguration Configuration;
//File to upload
File fileUpload;

//Servers
ESP8266WebServer WebServer(HTTPPort);
DNSServer DNS;
FtpServer FTP;

//Function to load the configuration from JSON
void ReadConfiguration() {
  //Open the configuration.json file
  File FileConfig = SPIFFS.open("/Configuration.json", "r");
  //Create a dynamic buffer for the JSON
  DynamicJsonBuffer jsonBuffer;

  //Parse the file to JSON
  JsonObject &JSON = jsonBuffer.parseObject(FileConfig);
  //Could not read the JSON
  if (!JSON.success()) {
    Serial.println(F("Read Config File Failed"));
  } else {
    //Copy Values
    Configuration.FTPUser = (char*)JSON["FTPUser"].as<char*>();
    Configuration.FTPPass = (char*)JSON["FTPPass"].as<char*>();
    Configuration.WIFISSID = (char*)JSON["WIFISSID"].as<char*>();
    Configuration.WIFIPass = (char*)JSON["WIFIPass"].as<char*>();
    Configuration.IP.fromString(JSON["IP"].as<String>());
    Configuration.Subnet.fromString(JSON["Subnet"].as<String>());
    //Close the file
    FileConfig.close();
  }
}

//Function to write the configuration
void WriteConfiguration() {
  //Open the configuration.json file
  File FileConfig = SPIFFS.open("/Configuration.json", "w");
  //Create a dynamic buffer for the JSON
  DynamicJsonBuffer jsonBuffer;
  //Create the JSON object
  JsonObject &JSON = jsonBuffer.createObject();
  //Create the JSON root
  JSON["FTPUser"] = Configuration.FTPUser;
  JSON["FTPPass"] = Configuration.FTPPass;
  JSON["WIFISSID"] = Configuration.WIFISSID;
  JSON["WIFIPass"] = Configuration.WIFIPass;
  JSON["IP"] = IpAddress2String(Configuration.IP);
  JSON["Subnet"] = IpAddress2String(Configuration.Subnet);
  //Write the content of the configuration and return an error if something failed
  if (JSON.printTo(FileConfig) == 0) { Serial.println(F("Error when writing the configuration")); }
  //Close the file
  FileConfig.close();
}


//Function to convert an IPAdress to a string
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}


//Function to convert a string into a char*
char* String2Char(String Texto) {
  //If the text has more than 0 characters i return as string
  if (Texto.length()!=0) { return const_cast<char*>(Texto.c_str()); }
  //Else return an empty string
  else return "";
}

//Function to configure WIFI and FTP
void ConfigureWIFIFTP() {
  //Set to AP Mode
  WiFi.mode(WIFI_AP);
  //Configure IP, Gateway and Network Mask
  WiFi.softAPConfig(Configuration.IP, Configuration.IP, Configuration.Subnet);
  //If the password is not empty - The 10 is the Channel, And The False is so that it is not invisible
  if (sizeof(Configuration.WIFIPass)-1 != 0) { WiFi.softAP(Configuration.WIFISSID, Configuration.WIFIPass, 10, false); } 
  //If the password is empty
  else { WiFi.softAP(Configuration.WIFISSID); }
  //Start the FTP server
  FTP.begin(Configuration.FTPUser, Configuration.FTPPass);
}


void setup() {
  Serial.begin(115200);
  //The root filesystem where i upload the html
  SPIFFS.begin();

  //Read configuration
  ReadConfiguration();
    
  //WIFI and FTP configuration
  ConfigureWIFIFTP();

  //DNS configuration
  DNS.setTTL(300);
  DNS.setErrorReplyCode(DNSReplyCode::ServerFailure);
  DNS.start(DNSPort, "*", Configuration.IP);

  //Web to upload the files - update
  WebServer.on("/upload", HTTP_GET, []() {
    //If found the file sent and returns a true
    if (!ManageFile("/upload.html"))
      //If the file is not found, a 404 is sent and a false is returned
      WebServer.send(404, "text/plain", "File not found...");
  });

  //Web to receive files - update (POST)
  WebServer.on("/upload", HTTP_POST,
            //Send status 200 (OK), To tell the client that we are ready to receive
            []() { WebServer.send(200);},
            //Receive the file
            uploadFile
  );
           
  //I always look for the file
  WebServer.onNotFound([]() {
    //If found the file sent and returns true
    if (!ManageFile(WebServer.uri()))
      //If the file is not found, a 404 is sent and false is returned
      WebServer.send(404, "text/plain", "File not found...");
  });

  //Start the WEB server
  WebServer.begin();
}

//I get the MIME type according to the extension
String getMimeType(String filename) {
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  //Create the compressed extension for less network traffic
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//Send the requested file if there is
bool ManageFile(String path) {
  //If they ask me for a folder I take them to the index file of that folder
  if (path.endsWith("/")) path += "index.html";
  //Get the MIME Type
  String mimeType = getMimeType(path);
  //GZ compressed file
  String pathCompressed = path + ".gz";
  //If the file exists
  if (SPIFFS.exists(pathCompressed) || SPIFFS.exists(path)) {
    //If compressed send compressed
    if(SPIFFS.exists(pathCompressed)) path += ".gz";
    //Open the file in read mode ;)
    File file = SPIFFS.open(path, "r");
    //I send it to the client
    size_t sent = WebServer.streamFile(file, mimeType);
    //Close the file
    file.close();
    return true;
  }
  //If the file does not exist return false
  return false;
}

//Upload a new file to SPIFFS
void uploadFile() {
  HTTPUpload& upload = WebServer.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    //Open the file to write in SPIFFS (i believe it if it does not exist)
    fileUpload = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fileUpload)
      //Write the bytes received to the file
      fileUpload.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    //If the file was up correctly
    if (fileUpload) {
      //Close the file
      fileUpload.close();
      //Redirect the client to the success page
      WebServer.sendHeader("Location", "/correct.html");
      WebServer.send(303);
    } else {
      //Else show an error
      WebServer.send(500, "text/plain", "Could not upload the file.");
    }
  }
}

void loop() {
  //Await the request
  WebServer.handleClient();
  //Process the DNS request
  DNS.processNextRequest();
  //Handle FTP requests
  FTP.handleFTP();
}

