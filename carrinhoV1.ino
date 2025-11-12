// ====================================================================
// Carrinho Seguidor de Linha - 2 Rodas e 3 Sensores
// ====================================================================
// Configuração da Ponte H L298N
#define ENA 11  // Enable Motor A (Roda Esquerda) - PWM
#define IN1 10  // Motor A - Entrada 1
#define IN2 9   // Motor A - Entrada 2
#define IN3 8   // Motor B - Entrada 1
#define IN4 7   // Motor B - Entrada 2
#define ENB 6   // Enable Motor B (Roda Direita) - PWM

// Módulo Seguidor de Linha (3 sensores infravermelhos)
#define SENSOR_ESQ A2   // Sensor Esquerdo
#define SENSOR_CENTRO A1 // Sensor Centro
#define SENSOR_DIR A0   // Sensor Direito

// Sensor Ultrassônico HC-SR04
#define TRIG A4  // Trigger
#define ECHO A3  // Echo

// Constantes de velocidade
#define VELOCIDADE_NORMAL 180   // Velocidade quando anda reto
#define VELOCIDADE_CURVA_ALTA 200  // Velocidade da roda externa na curva
#define VELOCIDADE_CURVA_BAIXA 100 // Velocidade da roda interna na curva

// Distância mínima para detecção de obstáculo (em cm)
#define DISTANCIA_MINIMA 15

// Variáveis para leitura dos sensores
int sensorEsq, sensorCentro, sensorDir;
long distancia;

void setup() {
  Serial.begin(9600);
  
  // Configuração dos pinos da Ponte H
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  // Configuração dos sensores de linha
  pinMode(SENSOR_ESQ, INPUT);
  pinMode(SENSOR_CENTRO, INPUT);
  pinMode(SENSOR_DIR, INPUT);
  
  // Configuração do sensor ultrassônico
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  // Inicializa motores parados
  pararMotores();
  
  Serial.println("Sistema Iniciado");
  delay(2000); // Aguarda 2 segundos antes de começar
}

void loop() {
  // Lê os sensores de linha (0 = linha branca/sem linha, 1 = linha preta)
  sensorEsq = digitalRead(SENSOR_ESQ);
  sensorCentro = digitalRead(SENSOR_CENTRO);
  sensorDir = digitalRead(SENSOR_DIR);
  
  // Lê a distância do sensor ultrassônico
  distancia = lerDistancia();
  
  // Debug - imprime valores dos sensores
  Serial.print("Sensores (E-C-D): ");
  Serial.print(sensorEsq);
  Serial.print("-");
  Serial.print(sensorCentro);
  Serial.print("-");
  Serial.print(sensorDir);
  Serial.print(" | Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");
  
  // Verifica se há obstáculo à frente
  if (distancia < DISTANCIA_MINIMA && distancia > 0) {
    // Obstáculo detectado - para o carrinho
    pararMotores();
    Serial.println("OBSTÁCULO DETECTADO - PARADO");
    delay(100);
    return;
  }
  
  // ====================================================================
  // Lógica de Seguimento de Linha com 3 Sensores
  // ====================================================================
  // Linha preta = 1, Superfície branca = 0
  
  // Caso 1: Apenas sensor CENTRO detecta linha (0-1-0)
  // Ação: Seguir em frente com ambas as rodas na mesma velocidade
  if (sensorEsq == 0 && sensorCentro == 1 && sensorDir == 0) {
    andarFrente();
    Serial.println("Ação: FRENTE");
  }
  
  // Caso 2: Sensor ESQUERDO detecta linha (1-X-0)
  // Ação: Virar para a ESQUERDA (roda direita mais rápida)
  else if (sensorEsq == 1 && sensorDir == 0) {
    virarEsquerda();
    Serial.println("Ação: VIRAR ESQUERDA");
  }
  
  // Caso 3: Sensor DIREITO detecta linha (0-X-1)
  // Ação: Virar para a DIREITA (roda esquerda mais rápida)
  else if (sensorEsq == 0 && sensorDir == 1) {
    virarDireita();
    Serial.println("Ação: VIRAR DIREITA");
  }
  
  // Caso 4: Todos os sensores na linha (1-1-1)
  // Ação: Pode ser cruzamento ou área ampla - continua em frente
  else if (sensorEsq == 1 && sensorCentro == 1 && sensorDir == 1) {
    andarFrente();
    Serial.println("Ação: CRUZAMENTO - FRENTE");
  }
  
  // Caso 5: Nenhum sensor detecta linha (0-0-0)
  // Ação: Perdeu a linha - para e aguarda
  else if (sensorEsq == 0 && sensorCentro == 0 && sensorDir == 0) {
    pararMotores();
    Serial.println("Ação: LINHA PERDIDA - PARADO");
  }
  
  // Caso padrão: continua em frente
  else {
    andarFrente();
    Serial.println("Ação: PADRÃO - FRENTE");
  }
  
  delay(50); // Pequeno delay para estabilidade
}

// ====================================================================
// Função para ler distância do sensor ultrassônico
// ====================================================================
long lerDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  long duracao = pulseIn(ECHO, HIGH, 30000); // Timeout de 30ms
  long dist = duracao * 0.034 / 2; // Converte para centímetros
  
  return dist;
}

// ====================================================================
// Funções de Controle dos Motores
// ====================================================================

// Andar para frente com velocidade igual nas duas rodas
void andarFrente() {
  // Motor A (Esquerdo) - Frente
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, VELOCIDADE_NORMAL);
  
  // Motor B (Direito) - Frente
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, VELOCIDADE_NORMAL);
}

// Virar para a esquerda (roda direita mais rápida que a esquerda)
void virarEsquerda() {
  // Motor A (Esquerdo) - Velocidade reduzida
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, VELOCIDADE_CURVA_BAIXA);
  
  // Motor B (Direito) - Velocidade aumentada
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, VELOCIDADE_CURVA_ALTA);
}

// Virar para a direita (roda esquerda mais rápida que a direita)
void virarDireita() {
  // Motor A (Esquerdo) - Velocidade aumentada
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, VELOCIDADE_CURVA_ALTA);
  
  // Motor B (Direito) - Velocidade reduzida
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, VELOCIDADE_CURVA_BAIXA);
}

// Parar ambos os motores
void pararMotores() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
}
