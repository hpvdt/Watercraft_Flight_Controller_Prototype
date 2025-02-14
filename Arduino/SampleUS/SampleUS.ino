const int trigPin = 10; 
const int echoPin = 11; 

void setup() { // Starts the arduino board, echoPin / trigPin tell the board the pins to read / use for input/output
  Serial.begin(115200); // Use 115200 baud for serial monitor display
  pinMode(trigPin, OUTPUT); // sets the pin defined in trigPin for output signals
  pinMode(echoPin, INPUT); // sets the pin defined in echoPin for input signals
  Serial.println("start"); // check serial monitor for "start" message
}

void loop() { 
  digitalWrite(trigPin, LOW); // resets the state of the output (if for whatever reason it starts high)
  delayMicroseconds(8000); // ideally want delays of > 1000 microseconds

  digitalWrite(trigPin, HIGH); // starts generating a signal
  delayMicroseconds(10); // sets a signal pulse duration of 10 microseconds
  digitalWrite(trigPin, LOW); // stops generating a signal

  long duration = pulseIn(echoPin, HIGH); // starts searching for a reflected signal, and measures the duration of a detected signal
  long distance = duration * 0.3400 / 2; // 0.34 is to convert the duration into distance using the speed of sound, the 2 is because the measurement considers both leaving and returning distance

  Serial.print("Distance: "); // the following three lines display the recorded distance
  Serial.print(distance);
  Serial.println(" mm");
}