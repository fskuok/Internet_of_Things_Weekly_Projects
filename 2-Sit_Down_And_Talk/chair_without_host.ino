//pin allocations
int redPin = A0;
int greenPin = D0;
int bluePin = D1;
int frsPin = A1;

//four chair states:
//0 - no person sitting
//1 - person sitting in chair but waiting for a person to sit in another chair
//2 - person in another chair but waiting for a person to sit in this chair
//3 - connection established
int state = 0;

const char *selfID = "F";
char targetID[5]; //create a buffer with more than enough space to hold a character
char requestID[5]; //create a buffer with more than enough space to hold a character
char confirmID[5]; //create a buffer with more than enough space to hold a character

int frsThreshold = 2000;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(frsPin, INPUT);

  Spark.subscribe("SDAT2015/chair-occupied", sitDownEventHandler);
  Spark.subscribe("SDAT2015/connection-request", connectionRequestHandler);
  Spark.subscribe("SDAT2015/connection-confirm", connectionConfirmHandler);
  Spark.subscribe("SDAT2015/chair-empty", quitConnectionHandler);

  setLEDColor(state);
}

void loop() {
  switch (state) {
    case 0: //no person sitting
      //keep checking for a person to sit down
      if (isSat()) {
        state = 1;
      }
      break;

    case 1: //waiting for another person to sit down in another chair
      Spark.publish("SDAT2015/chair-occupied", selfID);
      //if the person leaves
      if (!isSat()) {
        state = 0;
        //publish an event here
        Spark.publish("SDAT2015/chair-empty", selfID);
      }
      break;

    case 2: //waiting for person to sit in this chair
      //a person sits down
      if (isSat()) {
        Spark.publish("SDAT2015/connection-request", requestID);
      }
      break;

    case 3: //connection built
      //one or both people leave
      if (!isSat()) {
        state = 0;
        Spark.publish("SDAT2015/chair-empty", selfID);
      }
  }

  setLEDColor(state);
  delay(100);
}

void sitDownEventHandler(const char *event, const char *id) {
  //if this event is from another chair
  if ((id[0] != *selfID) && ((state == 1) || (state == 0))) {
    //record ID of other chair
    strcpy(targetID, id);
    requestID[0] = *targetID;
    requestID[1] = *selfID;
    state = 2;
  }
}

void connectionRequestHandler(const char *event, const char *id) {
  //if the request is for this chair
  if ((id[0] == *selfID) && (id[1] == requestID[0]) && (state != 3) && (state != 0)) {
    strcpy(confirmID, id);
    Spark.publish("SDAT2015/connection-confirm", confirmID);
    state = 3;

  //if the request is not for this chair
  //which means two other chairs have been connected
  } else if ((id[0] != *selfID)) {
    state = 1;
  }
}

void connectionConfirmHandler(const char *event, const char *id) {
  //if correct confirmation code received
  if (((id[0] == *selfID) || (id[1] == *selfID)) && (state != 3) && (state != 0)) {
    state = 3;
  } else {
    state = 0;
  }
}

void quitConnectionHandler(const char *event, const char *id) {
  if ((state == 3) && ((id[0] == confirmID[0]) || (id[0] == confirmID[1]))) {
    state = 0;
  }
}

bool isSat() {
  return analogRead(frsPin) > frsThreshold ? true : false;
}

//set color output for RGB LED
void setLEDColor(int state) {
  int r, g, b;
  switch (state) {
    case 0:
      r = 0;
      g = 255;
      b = 255;
      break;
    case 2:
      r = 255;
      g = 255;
      b = 0;
      break;
    case 1:
      r = 0;
      g = 255;
      b = 0;
      break;
    case 3:
      r = 255;
      g = 0;
      b = 255;
      break;
  }

  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}
