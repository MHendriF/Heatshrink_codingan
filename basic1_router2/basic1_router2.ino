void setup() {
  Serial.begin(9600);
  
}

int a = 0;

void loop() {
  String def = "aaabbbcccdddeeefffggghhhiiijjjkkklllmmmnnnooo";
  String node = def + a;
  a++;
  Serial.println(node);
  delay(3000);
}  
