// gps.course TRES peu réactif !! mais TinyGPS++ ne bouge plus depuis 11/2013...
// ok UNO 21/08/16

#include "SoftwareSerial.h"
SoftwareSerial mySerial(10, 11); // RX->TXD(neo-6m), TX->RXD(neo-6m)
//#define vitesse
#include <TinyGPS.h>
TinyGPS gps;


#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// pour distance
boolean fix = false;
float dpar = 0.0;
unsigned long milli, milli_1 = 0;

byte affseq = 0;
float alt;
float altmin = 9999.0;
float altmax = -99.9;
float vmax = 0.0;
int mode = 1; // 0:tout 1:vitesse en gros
char s[20];

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.print("Tachino LCD TinyGPS v. ");
  Serial.println(TinyGPS::library_version());

  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(60);

  //display.display(); // show splashscreen
  delay(2000);
  display.clearDisplay();   // clears the screen and buffer

  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0, 0);
  display.println("Tachino 5110");
  display.println("recherche sat");
  display.display();
}

void loop() // run over and over
{

  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (mySerial.available())
    {
      char c = mySerial.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    display.clearDisplay();   // clears the screen and buffer
    if (!fix) {
      //premiere
      fix = true; // once
      milli_1 = millis();
    } else {
      milli = millis();
      // delta T * vitesse (milli secondes => km/h 3600000)
      float fkmph = gps.f_speed_kmph();
      if (fkmph > 0.5) {
        // eliminer les petites erreurs de positionnement
        dpar += (((milli - milli_1) * fkmph) / 1000.0) / 3600.0;
      }

      if (fkmph > vmax) {
        vmax = fkmph;
      }

      milli_1 = milli;
      alt = gps.f_altitude();
      if (alt > altmax) {
        altmax = alt;
      }
      if (alt < altmin) {
        altmin = alt;
      }

      if (mode == 0) {
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.setCursor(0, 0);
        display.print("V=");

        display.print(fkmph);
        display.println(" km/h");

        display.print("D=");
        display.print(dpar);
        display.println(" km");
        display.print("Alt=");
        display.print(alt);
        display.println(" m");
        int year;
        byte month, day, hour, minute, second, hundredths;
        unsigned long fix_age;
        gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &fix_age);

        sprintf(s, "%02d:%02d:%02d UTC", hour, minute, second);
        display.println(s);
        // avant dernière ligne : affichage changeant
        switch (affseq) {
          case 0 :
            sprintf(s, "alt min %d", (int) altmin);
            //display.println(s);
            affseq++;
            break;
          case 1 :
            sprintf(s, "alt max %d", (int) altmax);
            //display.println(s);
            affseq++;
            break;
          case 2 :
            sprintf(s, "v max %d", (int) vmax);
            //display.println(s);
            affseq = 0;
            break;
        }
        display.println(s);
      } else if (mode == 1) {
        display.setTextSize(2);
        display.setTextColor(BLACK);
        display.setCursor(0, 0);
        display.print(int(fkmph));
        display.display();

      }

    }


  }

}
String decode_route(unsigned long c) {
  const char cap0[] PROGMEM = "N";
  const char cap22[] PROGMEM = "-";
  const char cap45[] PROGMEM = "NE";
  const char cap67[] PROGMEM = "-";
  const char cap90[] PROGMEM = "E";
  const char cap112[] PROGMEM = "-";
  const char cap135[] PROGMEM = "SE";
  const char cap157[] PROGMEM = "-";
  const char cap190[] PROGMEM = "S";
  const char cap212[] PROGMEM = "-";
  const char cap235[] PROGMEM = "SO";
  const char cap257[] PROGMEM = "-";
  const char cap270[] PROGMEM = "O";
  const char cap292[] PROGMEM = "-";
  const char cap314[] PROGMEM = "NO";
  const char cap336[] PROGMEM = "-";
  const char* string_table[] = {cap0, cap22, cap45, cap67, cap90, cap112, cap135, cap157, cap190, cap212, cap235, cap257, cap270, cap292, cap314, cap336};
  float cap = (float)c / 2250.;
  int ind = int(cap);
  int ind_inf = (ind + 15) % 16;

  int ind_sup = (ind + 1) % 16;
  char res[10];
  int ptres = 0;
  int pttmp = 0;
  while (string_table[ind_inf][pttmp] != 0) {
    res[ptres++] = string_table[ind_inf][pttmp++];
  }
  pttmp = 0;
  while (string_table[ind][pttmp] != 0) {
    res[ptres++] = string_table[ind][pttmp++];
  }
  pttmp = 0;
  while (string_table[ind_sup][pttmp] != 0) {
    res[ptres++] = string_table[ind_sup][pttmp++];
  }
  res[ptres] = 0;
  return res;

}

