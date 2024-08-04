/* A simple sketch to verify correct audio transfer in both receive and transmit directions.
 *  Transmits a 700Hz and 1000Hz tone on the Left and Right channels.
 *  Received audio is played on the Audio Shield's Left and Right DAC outputs
 */
#include <arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <utility/imxrt_hw.h> // needed for SetI2SFreq()
#include <usb_audio.h>

AudioInputUSB            usbIn;
AudioOutputUSB           usbOut;

AudioSynthWaveformSine   sine1;
AudioSynthWaveformSine   sine2;
AudioOutputI2SQuad       i2s_quad;

AudioConnection          patchCord1(usbIn, 0, i2s_quad, 0);
AudioConnection          patchCord2(usbIn, 1, i2s_quad, 1);
AudioConnection          patchCord3(sine1, 0, usbOut, 0);
AudioConnection          patchCord4(sine2, 0, usbOut, 1);

AudioControlSGTL5000     sgtl5000;

int SetI2SFreq(int freq) {  // extracted from T41 source code
  int n1;
  int n2 ;
  int c0;
  int c2;
  int c1;
  double C;

  // PLL between 27*24 = 648MHz und 54*24=1296MHz
  // Fudge to handle 8kHz - El Supremo
  if (freq > 8000) {
    n1 = 4; //SAI prescaler 4 => (n1*n2) = multiple of 4
  } else {
    n1 = 8;
  }
  n2 = 1 + (24000000 * 27) / (freq * 256 * n1);
  C = ((double)freq * 256 * n1 * n2) / 24000000;
  c0 = C;
  c2 = 10000;
  c1 = C * c2 - (c0 * c2);
  set_audioClock(c0, c1, c2, true);
  CCM_CS1CDR = (CCM_CS1CDR & ~(CCM_CS1CDR_SAI1_CLK_PRED_MASK | CCM_CS1CDR_SAI1_CLK_PODF_MASK))
               | CCM_CS1CDR_SAI1_CLK_PRED(n1 - 1) // &0x07
               | CCM_CS1CDR_SAI1_CLK_PODF(n2 - 1); // &0x3f

  CCM_CS2CDR = (CCM_CS2CDR & ~(CCM_CS2CDR_SAI2_CLK_PRED_MASK | CCM_CS2CDR_SAI2_CLK_PODF_MASK))
               | CCM_CS2CDR_SAI2_CLK_PRED(n1 - 1) // &0x07
               | CCM_CS2CDR_SAI2_CLK_PODF(n2 - 1); // &0x3f)
  return freq;
}

#define SAMPLE_RATE 192000

void setup() {
  delay(500);
  Serial.print("usb audio tests at "); Serial.print(SAMPLE_RATE); Serial.println("sps");

  // Audio connections require memory, and the record queue
  // uses this memory to buffer incoming audio.
  AudioMemory(30);  //allocate Int16 audio data blocks

  // Enable the audio shield, no input, and enable output
  sgtl5000.enable();                   //start the audio board
  sgtl5000.volume(0.5);                //volume can be 0.0 to 1.0.  0.5 seems to be the usual default.

  sine1.frequency(700); //*44100/SAMPLE_RATE
  sine1.amplitude(0.5);

  sine2.frequency(1000); //*44100/SAMPLE_RATE
  sine2.amplitude(0.5);

  SetI2SFreq(SAMPLE_RATE); 
}

void loop() {
  printCPUandMemoryUsage(&Serial);
}

void printCPUandMemoryUsage(Stream *s) {
  static unsigned long curTime_millis = 0;
  static unsigned long lastMemUpdate_millis=0;
  
  //print status information to the Serial port
  curTime_millis = millis();
  if ((curTime_millis - lastMemUpdate_millis) < 0) lastMemUpdate_millis=0;  //handle case where millis wraps around!
  
  if ((curTime_millis - lastMemUpdate_millis) > 2000) {  // print a summary of the current & maximum usage
    s->print("Usage/Max: ");
    s->print("oscillator CPU = "); s->print(sine1.processorUsage()); s->print("/"); s->print(sine1.processorUsageMax());s->print(", ");
    s->print("all CPU = " ); s->print(AudioProcessorUsage()); s->print("/");  s->print(AudioProcessorUsageMax());s->print(", ");
    s->print("Int16 Mem = ");s->print(AudioMemoryUsage()); s->print("/"); s->print(AudioMemoryUsageMax());s->print(", ");
    s->println();

    lastMemUpdate_millis = curTime_millis;

    SerialUSB1.println("The second USB serial port is also working");
  } 
};
