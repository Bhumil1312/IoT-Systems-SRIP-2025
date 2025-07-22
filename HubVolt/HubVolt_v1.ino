#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <WiFiClient.h>
#include <esp_wpa2.h>
#include <esp_wifi.h>
#include <Wire.h>

#define bootButtonPin 0 //Flash configuration of server
#define POWER_CONTROL_PIN 17 //USB hub power

typedef String string;

// Object declarations
Preferences powerAccessController;
WebServer configServer(80);

// Access Point credentials
const char* ApSSID = "ESP32-Setup";
const char* ApPassword = "configureme";

// Device Name
String device_name = "";

// Web page handler
void handleRoot() {

  String html = R"(<script>function myFunction(){
                            var x = document.getElementById('password');
                            if (x.type === 'password') {x.type = 'text';} 
                            else {x.type = 'password';}}
                  </script>
                  <h2>HubVolt (v1.0)</h2>
                  <p>Welcome to HubVolt.</p>
                  <h3>WiFi & Server Configuration</h3>
                  <form action='/save' method='post'><table>
                  <tr>
                    <td>Device Name (It should not have any space & special characters):</td>
                    <td><input type='text' name='device_name'></td>
                  </tr>
                  <tr>
                    <td>SSID:</td>
                    <td><input type='text' name='ssid'></td>
                  </tr>
                  <tr>
                    <td>Connection-Type:</td> 
                    <td>
                      <select name='connection_type'>
                        <option value='Open'>Open</option>
                        <option value='Personal'>Personal</option>
                        <option value='Enterprise'>Enterprise</option>
                      </select>
                    </td>
                  </tr>
                  <tr>
                    <td>Username (Not needed for Open/Personal):</td>
                    <td><input type='username' name='username'></td>
                  </tr>
                  <tr>
                    <td>Password (Not needed for Open):</td> 
                    <td> <input type='password' name='password' id='password'><br>
                        <input type='checkbox' onclick='myFunction()'>Show Password
                    </td>
                  </tr>
                  <tr>
                    <td>Server IP:</td> 
                    <td> <input type='text' name='server_ip'></td>
                  </tr>
                  <tr>
                    <td>Server TCP Port:</td>
                    <td> <input type='text' name='server_port' ></td>
                  </tr>
                </table>
                <input type='submit' value='Save'>
                </form>)";

  configServer.send(200, "text/html", html);
}

bool isNumber(const string& s)
{
  if(s.length() <= 0) return false;
  for (char c : s) {
      if (!isdigit(c)) return false;
  }
  return true;
} 

void htmlError (const string& s){
  configServer.send(200, "text/html", "<h3>"+s+"</h3>");
  delay(5000);
  ESP.restart();
}

bool isValidIpAddress(const string& str) {
    if (str.isEmpty()) return false;

    int start = 0, end = str.indexOf('.', start);
    int count = 0;
    String str_temp; 

    while (end != -1) {
        str_temp = str.substring(start, end);
        if(!isNumber(str_temp)) return false;
        int num = str_temp.toInt();
        if(num < 0 || num > 255) return false;
        start = end + 1;
        end = str.indexOf('.', start);
        count = count + 1;
    }
    if (count != 3) return false;

    return true;
}

// Save form inputs to Preferences and restart
void handleSave() {
  String ssid;
  String connection_type;
  String username = "";
  String password = "";
  String server_ip;
  String server_port;

  if (!configServer.hasArg("device_name")) htmlError("Device name is not provided.");
  else device_name = configServer.arg("device_name");
  if (device_name.length() <= 0) htmlError("Device name is not provided.");

  if (!configServer.hasArg("ssid")) htmlError("SSID is not provided.");
  else ssid = configServer.arg("ssid");
  if (ssid.length() <= 0) htmlError("SSID is not provided.");

  if (!configServer.hasArg("connection_type")) htmlError("Connection-Type is not provided.");
  else connection_type = configServer.arg("connection_type");
  if (connection_type.length() <= 0) htmlError("Connection-Type is not provided.");

  if (connection_type != "Open" && !configServer.hasArg("password")) htmlError("Password expected and it is not provided.");
  else password = configServer.arg("password");
  if (connection_type != "Open" && (password.length() <= 0)) htmlError("Password expected and it is not provided.");

  if(connection_type == "Enterprise" && !configServer.hasArg("username")) htmlError("Username expected and it is not provided.");
  else username = configServer.arg("username");
  if (connection_type == "Enterprise" && (username.length() <= 0)) htmlError("Username expected and it is not provided.");

  if(!configServer.hasArg("server_ip")) htmlError("Server IP address is not provided.");
  else{
    server_ip = configServer.arg("server_ip");
    if (!isValidIpAddress(server_ip)) htmlError("Invalid Server IP address.");
  }

  if(!configServer.hasArg("server_port")) htmlError("Server port number is not provided.");
  server_port = configServer.arg("server_port");
  if(!isNumber(server_port)) htmlError("Invalid Server port number.");

  powerAccessController.begin("server-config", false);  
  powerAccessController.putString("device_name", device_name);
  powerAccessController.putString("ssid", ssid);
  powerAccessController.putString("connection_type", connection_type);
  powerAccessController.putString("username", username);
  powerAccessController.putString("password", password);
  powerAccessController.putString("server_ip", server_ip);
  powerAccessController.putString("server_port", server_port);
  powerAccessController.end();
  configServer.send(200, "text/html", "<h3>Configuration saved. Device is ready now.</h3>");
  delay(2000);
  ESP.restart();
}

//Start Access Point mode
void startAccessPointMode() {
  WiFi.mode(WIFI_AP);
  delay(1000);
  WiFi.softAP(ApSSID, ApPassword);

  Serial.println("Access Point mode started.");
  Serial.println(WiFi.softAPIP());

  configServer.on("/", handleRoot);
  configServer.on("/save", handleSave);
  configServer.begin();
}

String mac2String(byte ar[]) {
  String s;
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%02X", ar[i]);
    s += buf;
    if (i < 5) s += ':';
  }
  return s;
}

bool conveyIP_Port(const String& ip,const String& port){
  //Create a TCP client object
  WiFiClient client;
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret != ESP_OK) return false;

  //Attempt to connect to the server with given IP and port
  if(!client.connect(ip.c_str(), port.toInt())){
    Serial.println("Connection to host failed");
    return false;
  }

  Serial.println("Connection to host successful");
  String myIP = WiFi.localIP().toString();
  String payload = device_name + " " + myIP + " " + mac2String(baseMac);
  //HTTP POST request (plain text) because TCP client send raw TCP data
  String request = String("POST /hubvolt/register HTTP/1.1\r\n") +
                  "Host: " + ip + ":" + port + "\r\n" +
                  "Content-Type: text/plain\r\n" +
                  "Content-Length: " + payload.length() + "\r\n" +
                  "Connection: close\r\n\r\n" +
                  payload;
  // Send the complete HTTP request to server
  client.print(request);
  client.stop();
  return true;
}

//Connect to WiFi using saved credentials
void connectToSavedWiFi() {
  powerAccessController.begin("server-config", true);
  device_name = powerAccessController.getString("device_name", "");
  String ssid = powerAccessController.getString("ssid", "");
  String connection_type = powerAccessController.getString("connection_type", "");
  String username = powerAccessController.getString("username", "");
  String password = powerAccessController.getString("password", "");
  String server_ip = powerAccessController.getString("server_ip", "");
  String server_port = powerAccessController.getString("server_port", "3000");

  powerAccessController.end();

  if (ssid.isEmpty()) {
    Serial.println("No saved WiFi credentials found. Starting AP mode...");
    startAccessPointMode();
    return;
  }
  
  WiFi.mode(WIFI_STA);
  if(connection_type == "Enterprise"){
    //WPA2-Enterprise configuration
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username.c_str(), strlen(username.c_str()));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username.c_str(), strlen(username.c_str()));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password.c_str(), strlen(password.c_str()));
    esp_wifi_sta_wpa2_ent_enable();
  }

  WiFi.begin(ssid.c_str(),(connection_type == "Personal")?password.c_str():"");
  Serial.println("Connecting to "+ssid);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    if(conveyIP_Port(server_ip, server_port) == false)  ESP.restart();
  }
  else {
    Serial.println("\nFailed to connect to WiFi. Restarting...");
    ESP.restart();
  }
}

String control(const String& arg){
  if(arg == "ON") {
    //Turn ON HubVolt port (power of USB hub)
    digitalWrite(POWER_CONTROL_PIN, HIGH);
    return device_name + " ON";
  }
  //Turn OFF HubVolt port (power of USB hub)
  digitalWrite(POWER_CONTROL_PIN, LOW);
  return device_name + " OFF";
}

void handleServercommands() {
  if (configServer.method() == HTTP_POST) {
    String requestBody = configServer.arg("plain");
    Serial.println("Received POST body:");
    Serial.println(requestBody);

    if (requestBody.length() == 0) {
      configServer.send(400, "text/plain", "Error: Empty body");
      return;
    }

    int firstSpace = requestBody.indexOf(' ');
    if (firstSpace == -1) {
      configServer.send(400, "text/plain", "Error: Invalid command syntax");
      return;
    }

    String command = requestBody.substring(0, firstSpace);
    String argument = requestBody.substring(firstSpace + 1);
    
    argument.trim();

    if (command == "CONTROL") configServer.send(200, "text/plain", control(argument));
    else if (command == "ECHO") configServer.send(200, "text/plain", device_name + " " + argument);
    else configServer.send(400, "text/plain", "Error: Unknown command");
    return;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  //Configures POWER_CONTROL_PIN as Output pin
  pinMode(POWER_CONTROL_PIN, OUTPUT);

  pinMode(bootButtonPin, INPUT_PULLUP);

  int buttonState = digitalRead(bootButtonPin);
  if (buttonState == LOW) {
    Serial.println("Boot button is pressed");
    powerAccessController.begin("server-config", false);
    powerAccessController.clear();
    powerAccessController.end();
  }

  //Initially set POWER_CONTROL_PIN to LOW 
  digitalWrite(POWER_CONTROL_PIN, LOW);

  connectToSavedWiFi();

  configServer.enableCORS();
  configServer.on("/",handleServercommands);
  configServer.begin();
}

void loop() {
  configServer.handleClient();
}