// === CONFIGURAÇÃO DE PINOS ===
#define enA 11
#define in1 10
#define in2 9
#define in3 8
#define in4 7
#define enB 6

#define sensorEsq A2
#define sensorCentro A1
#define sensorDir A0

#define trig A4
#define echo A3

#define ledVerde 4
#define ledVermelho 5

// === VARIÁVEIS ===
int velocidadeNormal = 180;
int velocidadeCurva = 220;
int distanciaMinima = 15; // distância mínima em cm para parar

// === FUNÇÕES ===
long lerDistancia() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duracao = pulseIn(echo, HIGH, 20000); // timeout de 20ms
  long distancia = duracao / 58.2;
  return distancia;
}

void parar() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void frente(int velE, int velD) {
  analogWrite(enA, velE);
  analogWrite(enB, velD);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void setup() {
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  pinMode(sensorEsq, INPUT);
  pinMode(sensorCentro, INPUT);
  pinMode(sensorDir, INPUT);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);

  parar();
}

void loop() {
  // === Leitura dos sensores ===
  int esquerda = analogRead(sensorEsq);
  int centro = analogRead(sensorCentro);
  int direita = analogRead(sensorDir);

  // Ajuste de faixa conforme cor da linha:
  // linha branca = valor alto, fundo preto = valor baixo
  bool linhaE = esquerda > 500;
  bool linhaC = centro > 500;
  bool linhaD = direita > 500;

  // === Leitura de distância ===
  long distancia = lerDistancia();

  // === Objeto detectado ===
  if (distancia < distanciaMinima && distancia > 0) {
    parar();
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledVermelho, HIGH);
    delay(100);
    return; // volta pro início do loop
  } else {
    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledVerde, HIGH);
  }

  // === Lógica do seguidor de linha ===
  if (linhaC && !linhaE && !linhaD) {
    // centro na linha → segue reto
    frente(velocidadeNormal, velocidadeNormal);
  } 
  else if (linhaE && !linhaC) {
    // esquerda detecta → vira levemente pra esquerda
    frente(velocidadeCurva, velocidadeNormal / 2);
  } 
  else if (linhaD && !linhaC) {
    // direita detecta → vira levemente pra direita
    frente(velocidadeNormal / 2, velocidadeCurva);
  } 
  else if (!linhaC && !linhaE && !linhaD) {
    // perdeu a linha → para
    parar();
  }

  delay(10);
}
