# ESP8266-WEB-FTP-DNS-Basic
A Basic Firm To Host Your Customizable PS4 Exploit - Open Source ( No Exploits Included )

Includes DNSServer Sources 
- They Can Downloaded from the following link
https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer/src

Includes The Sources of ESP8266FtpServer  
- They Can Downloaded from the following link
https://github.com/nailbuster/esp8266FTPServer

It Is Necessary To Have Installed In The Arduino IDE The Repository
http://arduino.esp8266.com/stable/package_esp8266com_index.json 
And The card esp8266 by ESP8266 Community Version 2.4.1 (Minimally)

You also need to have the ArduinoJson library installed (version 5.13.2 seems to work) - Click "Sketch" menu > "Include Library" > "Manage Libraries" > Search json and select ArduinoJson and version 5.13.2 from the select version drop down list and click install.

The Default WIFI Configuration is:
WIFISSID = ESP8266
WIFIPass = 12345678

The Default FTP Configuration is:
FTPUser = Admin
FTPPass = Upload

The Default IP Configuration is:
IP = 13.13.13.13
Subnet = 255.255.255.0

The DNS Configuration:
Resolves All Requests To The Configured IP Of ESP8266.
(Eg: www.google.com - 13.13.13.13 , www.playstation.com - 13.13.13.13)

The files of the folder data, can be uploaded by FTP with the previous configuration. 
It is necessary to connect by FTP Without Encryption (Only Use Plain FTP (Insecure) in the FileZilla for example)

Or Flashing Through Arduino IDE
From Tools -> ESP8266 Sketch Data Upload (This will delete all files that are found in the SPIFFS). 
To Activate The Option This Follow This Tutorial: http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html#uploading-files-to-file-system

Once Uploaded The files upload.html And correct.html, The next files can be uploaded directly from: Http://13.13.13.13/upload

This Sketch Can Be Flashed, On A Previous Bin For Example (# ESP8266XploitHost 2.7) To Keep The Exploit Files.
For This In The Arduino IDE The Erase Flash Option, It Should Be In Only Sketch. 

The idea of this project is to allow users to have a WebHost Customizable and that can modify the full operation of it.
