// === CONFIGURAÇÃO DE PINOS ===
#define ENA 11
#define IN1 10
#define IN2 9
#define IN3 8
#define IN4 7
#define ENB 6

#define SENSOR_ESQ A2
#define SENSOR_CENTRO A1
#define SENSOR_DIR A0

#define TRIG A4
#define ECHO A3

#define LED_VERDE 4
#define LED_VERMELHO 5

// === VARIÁVEIS E CONSTANTES ===
const int VELOCIDADE_NORMAL = 180;
const int VELOCIDADE_CURVA_ALTA = 220;
const int VELOCIDADE_CURVA_BAIXA = 100;
const int VELOCIDADE_GIRO = 200; // Velocidade para o giro de busca/cruzamento
const int DISTANCIA_MINIMA = 5; // Distância mínima em cm para parar (5cm)
const int LIMITE_SENSOR = 500; // Limite de leitura analógica para detecção de linha

// === FUNÇÕES DE CONTROLE DE MOTOR ===
void parar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void frente(int velE, int velD) {
  // Motor Esquerdo (ENA)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, velE);
  
  // Motor Direito (ENB)
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, velD);
}

void girarDireita() {
  // Roda Esquerda para frente, Roda Direita para trás (Giro no próprio eixo)
  digitalWrite(IN1, HIGH); // Esquerda Frente
  digitalWrite(IN2, LOW);
  analogWrite(ENA, VELOCIDADE_GIRO);
  
  digitalWrite(IN3, LOW); // Direita Trás
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, VELOCIDADE_GIRO);
}

// === FUNÇÃO DE LEITURA DO ULTRASSÔNICO ===
long lerDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  long duracao = pulseIn(ECHO, HIGH, 50000); 
  long distancia = duracao * 0.034 / 2;
  
  if (duracao == 0) {
    return 400; 
  }
  
  return distancia;
}

// === SETUP ===
void setup() {
  Serial.begin(9600);
  
  // Configuração dos pinos da Ponte H
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Configuração dos sensores
  pinMode(SENSOR_ESQ, INPUT);
  pinMode(SENSOR_CENTRO, INPUT);
  pinMode(SENSOR_DIR, INPUT);

  // Configuração do sensor ultrassônico
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // Configuração dos LEDs
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  // Inicialização: Acende LED Verde e aguarda 5 segundos
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_VERMELHO, LOW);
  parar();
  Serial.println("Sistema Iniciado - Aguardando 5 segundos...");
  delay(5000); // 5 segundos
  Serial.println("Início do Percurso.");
}

// === LOOP PRINCIPAL ===
void loop() {
  // === Leitura dos sensores ===
  int leituraE = analogRead(SENSOR_ESQ);
  int leituraC = analogRead(SENSOR_CENTRO);
  int leituraD = analogRead(SENSOR_DIR);
  
  // Lógica: valor < LIMITE_SENSOR significa que está na linha branca
  bool linhaE = leituraE < LIMITE_SENSOR;
  bool linhaC = leituraC < LIMITE_SENSOR;
  bool linhaD = leituraD < LIMITE_SENSOR;

  // === Leitura de distância ===
  long distancia = lerDistancia();
  
  // Debug
  Serial.print("Sensores (E-C-D): ");
  Serial.print(linhaE);
  Serial.print("-");
  Serial.print(linhaC);
  Serial.print("-");
  Serial.print(linhaD);
  Serial.print(" | Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  // === Objeto detectado e Controle de LEDs ===
  if (distancia < DISTANCIA_MINIMA && distancia > 0) {
    parar();
    digitalWrite(LED_VERDE, LOW);
    // Pisca o LED Vermelho
    static bool ledState = false;
    ledState = !ledState;
    digitalWrite(LED_VERMELHO, ledState ? HIGH : LOW);
    
    Serial.println("Ação: OBSTÁCULO - PARADO");
    delay(100);
    return; // Permanece parado e piscando enquanto houver obstáculo
  } else {
    // Caminho livre: Retoma o percurso e acende LED Verde
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, HIGH); 
  }
  
  // 1. Todos no Branco (Cruzamento/Área de Linha) → SEGUE EM FRENTE
  if (linhaE && linhaC && linhaD) {
    frente(VELOCIDADE_NORMAL, VELOCIDADE_NORMAL);
    Serial.println("Ação: 1-1-1 - CRUZAMENTO - FRENTE");
  }
  // 2. Curva Acentuada para Esquerda (1-1-0) → Vira para a DIREITA (CURVA FORTE)
  else if (linhaE && linhaC && !linhaD) {
    frente(VELOCIDADE_CURVA_ALTA, VELOCIDADE_CURVA_BAIXA);
    Serial.println("Ação: 1-1-0 - CURVA FORTE DIREITA");
  }
  // 3. Curva Acentuada para Direita (0-1-1) → Vira para a ESQUERDA (CURVA FORTE)
  else if (!linhaE && linhaC && linhaD) {
    frente(VELOCIDADE_CURVA_BAIXA, VELOCIDADE_CURVA_ALTA);
    Serial.println("Ação: 0-1-1 - CURVA FORTE ESQUERDA");
  }
  // 4. Centro na linha (0-1-0) → Segue Reto
  else if (!linhaE && linhaC && !linhaD) {
    frente(VELOCIDADE_NORMAL, VELOCIDADE_NORMAL);
    Serial.println("Ação: 0-1-0 - FRENTE");
  } 
  // 5. Esquerda detecta (1-0-0) → Vira para a DIREITA
  else if (linhaE && !linhaC && !linhaD) {
    frente(VELOCIDADE_CURVA_ALTA, VELOCIDADE_CURVA_BAIXA);
    Serial.println("Ação: 1-0-0 - VIRAR DIREITA");
  } 
  // 6. Direita detecta (0-0-1) → Vira para a ESQUERDA
  else if (!linhaE && !linhaC && linhaD) {
    frente(VELOCIDADE_CURVA_BAIXA, VELOCIDADE_CURVA_ALTA);
    Serial.println("Ação: 0-0-1 - VIRAR ESQUERDA");
  } 
  // 7. Perdeu a linha (0-0-0) → GIRA para buscar
  else if (!linhaE && !linhaC && !linhaD) {
    girarDireita();
    Serial.println("Ação: 0-0-0 - LINHA PERDIDA - BUSCA");
  }
  // 8. Caso Inesperado (ex: 1-0-1) → Segue Reto
  else {
    frente(VELOCIDADE_NORMAL, VELOCIDADE_NORMAL);
    Serial.println("Ação: PADRÃO - FRENTE");
  }

  delay(10);
}
