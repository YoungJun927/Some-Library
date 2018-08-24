#include <cox.h>
#include "VC0706.hpp"
#include <HTTPClient.hpp>
#include <base64.hpp>
#include <esp32/ESP32Serial.hpp>

const char *rootCA = \
"-----BEGIN CERTIFICATE-----\n"\
"MIIFdDCCBFygAwIBAgIQJ2buVutJ846r13Ci/ITeIjANBgkqhkiG9w0BAQwFADBv\n"\
"MQswCQYDVQQGEwJTRTEUMBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFk\n"\
"ZFRydXN0IEV4dGVybmFsIFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBF\n"\
"eHRlcm5hbCBDQSBSb290MB4XDTAwMDUzMDEwNDgzOFoXDTIwMDUzMDEwNDgzOFow\n"\
"gYUxCzAJBgNVBAYTAkdCMRswGQYDVQQIExJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\n"\
"BgNVBAcTB1NhbGZvcmQxGjAYBgNVBAoTEUNPTU9ETyBDQSBMaW1pdGVkMSswKQYD\n"\
"VQQDEyJDT01PRE8gUlNBIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIICIjANBgkq\n"\
"hkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAkehUktIKVrGsDSTdxc9EZ3SZKzejfSNw\n"\
"AHG8U9/E+ioSj0t/EFa9n3Byt2F/yUsPF6c947AEYe7/EZfH9IY+Cvo+XPmT5jR6\n"\
"2RRr55yzhaCCenavcZDX7P0N+pxs+t+wgvQUfvm+xKYvT3+Zf7X8Z0NyvQwA1onr\n"\
"ayzT7Y+YHBSrfuXjbvzYqOSSJNpDa2K4Vf3qwbxstovzDo2a5JtsaZn4eEgwRdWt\n"\
"4Q08RWD8MpZRJ7xnw8outmvqRsfHIKCxH2XeSAi6pE6p8oNGN4Tr6MyBSENnTnIq\n"\
"m1y9TBsoilwie7SrmNnu4FGDwwlGTm0+mfqVF9p8M1dBPI1R7Qu2XK8sYxrfV8g/\n"\
"vOldxJuvRZnio1oktLqpVj3Pb6r/SVi+8Kj/9Lit6Tf7urj0Czr56ENCHonYhMsT\n"\
"8dm74YlguIwoVqwUHZwK53Hrzw7dPamWoUi9PPevtQ0iTMARgexWO/bTouJbt7IE\n"\
"IlKVgJNp6I5MZfGRAy1wdALqi2cVKWlSArvX31BqVUa/oKMoYX9w0MOiqiwhqkfO\n"\
"KJwGRXa/ghgntNWutMtQ5mv0TIZxMOmm3xaG4Nj/QN370EKIf6MzOi5cHkERgWPO\n"\
"GHFrK+ymircxXDpqR+DDeVnWIBqv8mqYqnK8V0rSS527EPywTEHl7R09XiidnMy/\n"\
"s1Hap0flhFMCAwEAAaOB9DCB8TAfBgNVHSMEGDAWgBStvZh6NLQm9/rEJlTvA73g\n"\
"JMtUGjAdBgNVHQ4EFgQUu69+Aj36pvE8hI6t7jiY7NkyMtQwDgYDVR0PAQH/BAQD\n"\
"AgGGMA8GA1UdEwEB/wQFMAMBAf8wEQYDVR0gBAowCDAGBgRVHSAAMEQGA1UdHwQ9\n"\
"MDswOaA3oDWGM2h0dHA6Ly9jcmwudXNlcnRydXN0LmNvbS9BZGRUcnVzdEV4dGVy\n"\
"bmFsQ0FSb290LmNybDA1BggrBgEFBQcBAQQpMCcwJQYIKwYBBQUHMAGGGWh0dHA6\n"\
"Ly9vY3NwLnVzZXJ0cnVzdC5jb20wDQYJKoZIhvcNAQEMBQADggEBAGS/g/FfmoXQ\n"\
"zbihKVcN6Fr30ek+8nYEbvFScLsePP9NDXRqzIGCJdPDoCpdTPW6i6FtxFQJdcfj\n"\
"Jw5dhHk3QBN39bSsHNA7qxcS1u80GH4r6XnTq1dFDK8o+tDb5VCViLvfhVdpfZLY\n"\
"Uspzgb8c8+a4bmYRBbMelC1/kZWSWfFMzqORcUx8Rww7Cxn2obFshj5cqsQugsv5\n"\
"B5a6SE2Q8pTIqXOi6wZ7I53eovNNVZ96YUWYGGjHXkBrI/V5eu+MtWuLt29G9Hvx\n"\
"PUsE2JOAWVrgQSQdso8VYFhH2+9uRv0V9dlfmrPb2LjkQLPNlzmuhbsdjrzch5vR\n"\
"pu/xO28QOG8=\n"\
"-----END CERTIFICATE-----\n";

#define IOTOWN_TOKEN "b9df6fd441fda4e8f76e1c446fd141ebae8672ed24aca15c4522184fb706411a"; // replace with your token
#define IOTOWN_NID   "WF1" // replace with your node ID

Timer timerTakePicture;
Timer timerKeyInput;
Timer temp;

ESP32Serial Serial2(ESP32Serial::PORT2, 17, 16);
VC0706 camera(Serial2);

String jsonData;
uint16_t seq = 0;
int i=0;
char keyBuf[128];

static void eventOnPictureTaken(const char *buf, uint32_t size)
{
  printf("ReadImage Done : %ld (BYTE)\n", camera.imageSize );
  uint32_t index = 0;

  String encoded = base64::encode((uint8_t *) buf, size);

  while(encoded.indexOf('\n',index) != -1){
    index = encoded.indexOf('\n',index);
    encoded.remove(index,1);
    System.feedWatchdog();
  }

  jsonData = "{\"type\":\"2\",\"token\":\"";
  jsonData += IOTOWN_TOKEN;
  jsonData += "\",\"nid\":\"";
  jsonData += IOTOWN_NID;
  jsonData += "\",\"data\":{\"cam1\":\"";
  for(uint32_t i=0;i<encoded.length();i++){
    System.feedWatchdog();
    jsonData += encoded[i];
  }
  jsonData += "\"}}";

  if (jsonData.length() == 0) {
    printf("* Report data is not ready.\n");
    return;
  }

  if (WiFi.isConnected()) {
    HTTPClient http;
    http.begin("https://town.coxlab.kr/api/data", rootCA);
    http.addHeader("Content-Type", "application/json");
    int responseCode = http.POST(jsonData);

    if (responseCode == HTTP_CODE_OK) {
      http.writeToStream(&Serial);
      Serial.println();
      Serial.flush();
    } else {
      printf("- POST failed (error:%s)\n", http.errorToString(responseCode).c_str());
      http.end();
    }
  } else {
    printf("* WiFi is not connected.\n");
  }

  timerKeyInput.startOneShot(10000);
}

void eventRatioKeyInputDone(void *)
{
  printf("\nTake a picture\n" );
  Serial.stopListening();
  camera.takePicture(eventOnPictureTaken, camera.ratio);
}

static void eventRatioKeyInput(SerialPort &)
{
  uint8_t numOctets = strlen(keyBuf);
  numOctets /= 2;
  char strOctet[3];
  for (uint8_t j = 0; j < numOctets; j++) {
    strOctet[0] = keyBuf[2 * j];
    strOctet[1] = keyBuf[2 * j + 1];
    strOctet[2] = '\0';

    camera.ratio = strtoul(strOctet, NULL, 16);
  }
  timerKeyInput.startOneShot(5000);
}

static void eventKeyInput(SerialPort &)
{
  timerKeyInput.stop();

  while (Serial.available() > 0) {
    Serial.read();
  }

  Serial.onReceive(eventRatioKeyInput);
  Serial.inputKeyboard(keyBuf, sizeof(keyBuf) - 1);
  Serial.println(
    "* Enter a new compression ratio as a hexadecimal string "
    "[00~FF]"
  );
}

static void eventWiFi(WiFiClass::event_id_t event, void *info)
{
  printf("* WiFi event(%d): ", event);

  switch (event) {
    case (WiFi.EVENT_STA_CONNECTED):
    printf("connected.\n");
    break;

    case (WiFi.EVENT_STA_DISCONNECTED):
    printf("disconnected.\n");
    timerTakePicture.stop();
    WiFi.reconnect();
    break;

    case (WiFi.EVENT_STA_GOT_IP):
    printf("got IP:\n");
    printf("- IP:%s\n", WiFi.localIP().toString().c_str());
    printf("- Gateway:%s\n", WiFi.gatewayIP().toString().c_str());
    printf("- Subnet Mask:%s\n", WiFi.subnetMask().toString().c_str());

    Serial.println("* Press ansy key to enter a compression ratio in 5 seconds..." );
    Serial.onReceive(eventKeyInput);
    timerKeyInput.onFired(eventRatioKeyInputDone,NULL);
    timerKeyInput.startOneShot(5000);
    Serial.listen();
    break;

    case (WiFi.EVENT_STA_LOST_IP):
    printf("lost IP\n");
    break;

    default:
    printf("unhandled\n");
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  camera.begin();
  Serial.println("\n\t***** VC0706 camera test *****" );

  WiFi.mode(WiFi.MODE_STA);
  WiFi.disconnect();

  delay(100);

  WiFi.onEvent(eventWiFi);
  WiFi.begin("coxlab2", "johnsnow");
}
