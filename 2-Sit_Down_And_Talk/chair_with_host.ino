int redPin = A0;
int greenPin = D0;
int bluePin = D1;
int frsPin = A1;

int state = 0;

int frsThreshold = 2000;

bool alreadySitting = false;
bool isConnected = false;

void setup() {
  pinMode(frsPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  Spark.function("connect", connect);
  Spark.function("disconnect", disconnect);

  setLEDColor(state);
}

void loop() {
  if (isSat() && !alreadySitting) {
    Spark.publish("sdnt/sitdown");
    alreadySitting = true;
  } else if (!isSat() && alreadySitting) {
    Spark.publish("sdnt/leave");
    alreadySitting = false;
  }

  if (isConnected) {
    state = 2;
  } else {
    if (isSat()) {
      state = 1;
    } else {
      state = 0;
    }
  }

  setLEDColor(state);
}

bool isSat() {
  return analogRead(frsPin) > frsThreshold;
}

void setLEDColor(int state) {
  int r, g, b;
  switch (state) {
    case 0:
      r = 0;
      g = 255;
      b = 255;
      break;
    case 1:
      r = 0;
      g = 255;
      b = 0;
      break;
    case 2:
      r = 255;
      g = 0;
      b = 255;
      break;
  }

  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}

int connect(String command) {
  isConnected = true;
  return 1;
}

int disconnect(String command) {
  isConnected = false;
  return 1;
}
