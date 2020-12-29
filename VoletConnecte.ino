#include <ESP8266WiFi.h>
#include <Servo.h>

const char* ssid     = "votreResauWifi";
const char* password = "votreMotDePasseWifi";

Servo monServo; //creéation de l'objet servo
int pos = 0;

const int ledPinR = 5; //led rouge à D1
const int ledPinV = 4; //led verte à D2
int servo = 12; //servomoteur à D6
int PR = 0; //valeur photorésistance initialisée
int autotest; //cette variable servira pour le mode automatique

WiFiServer server(80);

void setup()
{
    Serial.begin(9600); //pour l'affichage dans le moniteur série
    pinMode(ledPinR, OUTPUT);   //configuration de la led rouge
    pinMode(ledPinV, OUTPUT);   //configuration de la led verte
    pinMode(PR,INPUT);  //configuration de la photorésistance
    
    monServo.attach(servo); //pour savoir la broche du servomoteur (ici 12 qui coorespond à D6)
    monServo.write(pos);  //position de départ (équivalent du volet fermé)
    
    delay(10);
    
    //Connexion au réseau WiFi
    Serial.println();
    Serial.println();
    Serial.print("Connexion a");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connecte.");
    Serial.println("Adresse IP : ");
    Serial.println(WiFi.localIP());
    //digitalWrite(ledPinR,HIGH);
    //digitalWrite(ledPinV,HIGH);
    server.begin();

}




void loop(){
  
  //PhotoResistance
  int PR = analogRead(A0); //récupération de la valeur de la photorésistance
  Serial.print("Valeur : ");
  Serial.println(PR);
  //Si le mode est automatique
  if(autotest == 1){
    //Si le valeur de la luminosité est inférieure à 500 alors le volet est fermé, la led rouge nous confirme
    if(PR < 500){
      digitalWrite(ledPinR,HIGH);
      digitalWrite(ledPinV,LOW);
      monServo.write(0);     
      
      //Serial.println("DOWN");
    }
    //Si le valeur de la luminosité est supérieure à 500 alors le volet est ouvert, la led verte nous confirme
    if(PR > 500){
      digitalWrite(ledPinV,HIGH);
      digitalWrite(ledPinR,LOW);
      monServo.write(180);     
      //Serial.println("UP");
    }
  }


 //Wifi
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Allumer la LED : <a href=\"/H\">DOWN</a> !<br>");
            client.print("Eteindre la LED : <a href=\"/L\">UP</a> !<br>");
            client.print("Mode automatique : <a href=\"/AUTO\">AUTO</a> !<br>");
            client.print("Luminosite : ");
            client.print(PR);
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

     // On vérifie la demande du client
        //Si le client veut le mode automatique alors autotest est à 1 (ce qui renvoie à la boucle vue précédemment)
        if (currentLine.endsWith("GET /AUTO")) {
          //Automatique
          ////Serial.println("AUTO");
          delay(1000);
          autotest = 1;
          
        }
        //Si le client veut fermer le volet alors la led rouge s'allume et le volet se ferme
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(ledPinR,HIGH);
          digitalWrite(ledPinV,LOW);
          monServo.write(0);
          delay(30);
          autotest = 0;
        }
        //Si le client veut ouvrir le volet alors la led verte s'allume et le volet s'ouvre
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(ledPinV,HIGH);
          digitalWrite(ledPinR,LOW);
          monServo.write(180);
          delay(30);
          autotest = 0;
        }
      }
    }
    //Fermeture de la connexion
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
