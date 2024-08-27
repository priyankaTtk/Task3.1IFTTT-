#include <Wire.h>
#include <BH1750.h>
#include <WiFiNINA.h>
#include <ESP_Mail_Client.h>

// Wi-Fi credentials
const char* ssid = "Vivo";
const char* password = "abcdefgh";

// Email settings
const char* smtp_host = "smtp.gmail.com";
const int smtp_port = 465;
const char* email_sender = "priyanka4800.be23@chitkara.edu.in";
const char* email_sender_password = "nwpbenkmdbuhrqwl";
const char* email_recipient = "priyankathukral2004@gmail.com";

// Light sensor and email session
BH1750 lightMeter;
SMTPSession smtp;
SMTP_Message message;
ESP_Mail_Session session;

// Timing variables
unsigned long sunlightStartTime = 0;
unsigned long totalSunlightTime = 0;
const unsigned long sunlightThreshold = 100; // Light level threshold for sunlight
const unsigned long twoHoursInMillis = 2*60*60*1000; // 2 hours in milliseconds
bool wasInSunlight = false;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  connectWiFi();

  if (!lightMeter.begin()) {
    Serial.println("Error initializing BH1750 sensor.");
    while (1);
  }

  setupEmail();
}

void loop() {
  //Taking the reading from the sensor
  float lux = lightMeter.readLightLevel();
  //printing the reading
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

//Storing the time for the arduino board activation 
  unsigned long currentMillis = millis();
//comparing the the threshold value to the lux detected by sensor
  bool isInSunlight = lux > sunlightThreshold;

  if (isInSunlight) {
    if (!wasInSunlight) { // Sunlight state changed from off to on
      sunlightStartTime = currentMillis;
      sendEmail("Terrarium is now in sunlight.");
    }
    totalSunlightTime += currentMillis - sunlightStartTime;
    sunlightStartTime = currentMillis;
  } else {
    if (wasInSunlight) { // Sunlight state changed from on to off
      sendEmail("Terrarium is no longer in sunlight.");
      sunlightStartTime = 0;
    }
  }

  if (totalSunlightTime >= twoHoursInMillis) {
    sendEmail("Your terrarium has received enough sunlight for today.");
    totalSunlightTime = 0; // Reset the timer after sending the email
  }

  wasInSunlight = isInSunlight;

  delay(60000); // Wait for 1 minute before the next check
}

void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");
}

void setupEmail() {
  session.server.host_name = smtp_host;
  session.server.port = smtp_port;
  session.login.email = email_sender;
  session.login.password = email_sender_password;

  message.sender.name = "Arduino Light Monitor";
  message.sender.email = email_sender;
  message.subject = "Sunlight Exposure Notification";
  message.addRecipient("Terrarium Monitor", email_recipient);
}

bool sendEmail(const String& messageContent) {
  message.text.content = messageContent;
  if (!smtp.connect(&session)) {
    Serial.println("Failed to connect to SMTP server");
    return false;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Error sending Email, " + smtp.errorReason());
    return false;
  }

  Serial.println("Email sent successfully");
  return true;
}
