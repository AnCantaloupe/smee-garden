typedef unsigned char u8;

typedef struct {
  u8 R;
  u8 G;
  u8 B;
  u8 X;
} colour;

// All of the sensors will share common config pins
#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define OutL 11
#define OutR 8
#define OutF 9
#define OutB 10

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OutL, INPUT);
  pinMode(OutR, INPUT);
  pinMode(OutF, INPUT);
  pinMode(OutB, INPUT);

  // Setting frequency-scaling to 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
  
  Serial.begin(9600);
}

bool
ButtonDown(colour Colour) {
  u8 Intensity = (u8)(0.2126 * (float)Colour.R + 0.7152 * (float)Colour.G + 0.0722 * (float)Colour.B);
  return Intensity < 70;
}

void loop() {
  colour ColourL;
  colour ColourR;
  colour ColourF;
  colour ColourB;

  // Read RED
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  ColourL.R = pulseIn(OutL, LOW);
  ColourR.R = pulseIn(OutR, LOW);
  ColourF.R = pulseIn(OutF, LOW);
  ColourB.R = pulseIn(OutB, LOW);

  // Read GREEN
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  ColourL.G = pulseIn(OutL, LOW);
  ColourR.G = pulseIn(OutR, LOW);
  ColourF.G = pulseIn(OutF, LOW);
  ColourB.G = pulseIn(OutB, LOW);
  
  // Read BLUE
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  ColourL.B = pulseIn(OutL, LOW);
  ColourR.B = pulseIn(OutR, LOW);
  ColourF.B = pulseIn(OutF, LOW);
  ColourB.B = pulseIn(OutB, LOW);

  // Read TOTAL
  digitalWrite(S2, HIGH);
  digitalWrite(S3, LOW);
  ColourL.X = pulseIn(OutL, LOW);
  ColourR.X = pulseIn(OutR, LOW);
  ColourF.X = pulseIn(OutF, LOW);
  ColourB.X = pulseIn(OutB, LOW);

  // Check sensor activation
  u8 DownL = ButtonDown(ColourL);
  u8 DownR = ButtonDown(ColourR);
  u8 DownF = ButtonDown(ColourF);
  u8 DownB = ButtonDown(ColourB);
  
  Serial.print((u8)(DownL << 3 | DownR << 2 | DownF << 1 | DownB));
  delay(30);
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
