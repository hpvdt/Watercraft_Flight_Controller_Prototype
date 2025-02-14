const int trigPin = 7; 
const int echoPin = 6; 
const int trigPin2 = 11; 
const int echoPin2 = 10; 


void setup() { // Starts the arduino board, echoPin / trigPin tell the board the pins to read / use for input/output
  Serial.begin(115200); // Use 115200 baud for serial monitor display
  pinMode(trigPin, OUTPUT); // sets the pin defined in trigPin for output signals
  pinMode(echoPin, INPUT); // sets the pin defined in echoPin for input signals
  pinMode(trigPin2, OUTPUT); // sets the pin defined in trigPin for output signals
  pinMode(echoPin2, INPUT); // sets the pin defined in echoPin for input signals
  Serial.println("start"); // check serial monitor for "start" message
}

void loop() { 
  digitalWrite(trigPin, LOW); // resets the state of the output (if for whatever reason it starts high)
  digitalWrite(trigPin2, LOW); // resets the state of the output (if for whatever reason it starts high)
  delayMicroseconds(1000); // ideally want delays of > 1000 microseconds

  digitalWrite(trigPin, HIGH); // starts generating a signal
  digitalWrite(trigPin2, HIGH); // starts generating a signal
  delayMicroseconds(10); // sets a signal pulse duration of 10 microseconds
  digitalWrite(trigPin, LOW); // stops generating a signal
  digitalWrite(trigPin2, LOW); // stops generating a signal

  long duration = pulseIn(echoPin, HIGH); // starts searching for a reflected signal, and measures the duration of a detected signal
  long duration2 = pulseIn(echoPin2, HIGH); // starts searching for a reflected signal, and measures the duration of a detected signal
  long distance = duration * 0.3400 / 2; // 0.34 is to convert the duration into distance using the speed of sound, the 2 is because the measurement considers both leaving and returning distance
  long distance2 = duration2 * 0.3400 / 2;

  Serial.print("Distance 1, Distance 2 (in mm): ("); // the following three lines display the recorded distance
  Serial.print(distance);
  Serial.print(", ");
  Serial.print(distance2);
  Serial.println(")");
}