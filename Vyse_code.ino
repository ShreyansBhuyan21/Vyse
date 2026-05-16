// C++ code
//

const int ARRAY_SIZE = 10;
const unsigned long EXPIRATION_TIME = 500; // Time in milliseconds

const unsigned long MOTOR_DURATION = 1000;
bool motorRunning = false;
unsigned long motorStartTime = 0;

double currThreshold = .05; //mA

double prev_current = 0;
double current = 0;
double voltage1 = 0;

double dataArray[ARRAY_SIZE]; //document last x currents
int index = 0;
unsigned long entryTimes[ARRAY_SIZE];
unsigned long lastFillTime = 0;

void setup()
{
  Serial.begin(9600);

  int sensorValue1 = analogRead(A0);
  double voltage1 = sensorValue1 * (5.0 / 1023.0);
  double current = 1000.0 * ((5.0 - voltage1) / 1000.0); //resistor value = 1k
  prev_current = current; //set up first loop as prev_current = current

  for (int i = 0; i < ARRAY_SIZE; i++) { //fill with zeroes
    dataArray[i] = 0;
    entryTimes[i] = 0;
  }

  pinMode(3, OUTPUT);
}

void loop()
{
  unsigned long currentTime = millis();

  int sensorValue1 = analogRead(A0);
  double voltage1 = sensorValue1 * (5.0 / 1023.0);
  double current = 1000.0 * ((5.0 - voltage1) / 1000.0); //resistor value = 1k

  //Serial.print("The current is: ");
  //Serial.println(current); //in mA

  if (!(current <= prev_current + currThreshold && current >= prev_current - currThreshold)) {

    Serial.println("Bad lights detected.");
    lastFillTime = currentTime;

    dataArray[index] = 1;
    entryTimes[index] = currentTime;

    Serial.print("Added value to index [");
    Serial.print(index);
    Serial.println("]");

    index++;
    if (index >= ARRAY_SIZE) {
      index = 0;
    }

    int count = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {

      // Only check spots that aren't already empty (0)
      if (dataArray[i] > 0.0) {
        count++;
      }
    }

    if (count >= 10) { // if 5 or more entries in array (within past 200ms), go panic
      Serial.println("REALLY BAD lights detected. Running notifcation.");
      //insert motor functions here
      digitalWrite(3, HIGH);

      motorStartTime = currentTime; // Record the "Start Time"
      motorRunning = true;          // Set the flag
    }
  }

  for (int i = 0; i < ARRAY_SIZE; i++) {
    if (dataArray[i] != 0) {
      if (currentTime - entryTimes[i] >= EXPIRATION_TIME) {
        dataArray[i] = 0; // Clear the value
        entryTimes[i] = 0; // Reset the timer

        Serial.print("Index [");
        Serial.print(i);
        Serial.println("] expired and cleared.");
      }
    }
  }

  if (motorRunning) {
    if (currentTime - motorStartTime >= MOTOR_DURATION) {
      digitalWrite(3, LOW);  // Turn motor/LED off
      motorRunning = false;  // Reset the flag
      Serial.println("Motor duration complete.");
    }
  }

  delay(10);
  prev_current = current;
}
