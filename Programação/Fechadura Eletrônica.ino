#include <Wire.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
#else
#endif

int count = 0;
int pikachu = 0;

int D0 = 22; //rele
int Botao = 23; //ao apertar o botao a trava será aberta
int estadoBotao = 0;
int buttonState = 0;
int buttonState1 = 0;
int b2 = 24;
uint8_t id;

uint8_t getFingerprintEnroll();

SoftwareSerial mySerial(51, 52);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
#define mySerial Serial1
char pass [4] = {'4', '6', '5', '9'}; // senha padrão, você pode muito bem atera-lá
const int ledgreen = 8; //era 13 //solicita a digitação da senha
const int duration = 100;
const byte ROWS = 4; //Quatro linhas
const byte COLS = 3; //Três colunas
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};// deinição do layout do teclado
byte rowPins[ROWS] = {3, 4, 5, 6}; // pinagem para as linhas do teclado
byte colPins[COLS] = {9, 10, 11}; // pinagem para as colunas do teclado
// mapeamento do teclado
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup() {

  Serial.begin(9600);
   Serial.setTimeout(30);
  pinMode(b2, INPUT);
  pinMode(D0, OUTPUT);
  pinMode(Botao, INPUT);
  pinMode(ledgreen, OUTPUT);
  Serial.begin(9600);
  Serial.println("Procurando Leitor Biometrico ...");
  finger.begin(57600); // Define a taxa de dados para a porta serial do sensor
  Serial.begin(9600);
  while (!Serial);
  delay(100);
  Serial.println("\n\nDelete Finger");
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Leitor Biometrico encontrado");
  }
  else {
    Serial.println("Leitor Biometrico nao encontrado");
    while (1);
  }
  Serial.println("Esperando Dedo para Verificar");
  key_init();
}
uint8_t readnumber(void) {
 
  uint8_t num = 0;
  boolean validnum = false;
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
    return num;
  }
}
String comando;

void loop() {

  teclado();
}

void teclado() {
  {
    getFingerprintIDez();
    char key = keypad.getKey(); // obtém informação do teclado
    code_entry_init(); // mensagem, som e LED
    int entrada = 0; // variável de apoio; números de entradas feitas via teclado
    while (pikachu < 4 ) { // enquanto não entrou os 4 números necessários para a senha
      getFingerprintIDez();
      botoes();
      interno();
       if (Serial.available() > 0) {
    comando = Serial.readString();
    if (comando == "abrir") {
      digitalWrite(D0, HIGH);
      Serial.println ("Trava aberta pelo celular");
      delay(2000);
      digitalWrite(D0, LOW);
    } else {
      digitalWrite(D0, LOW);
    }
  }
      char key = keypad.getKey(); // obtém informação do teclado
      if (key != NO_KEY) { // se foi teclado algo
        entrada += 1; // aumenta contrador de entradas
        delay(duration);
        if (key == pass[pikachu])pikachu += 1; // verifica na sequencia da senha, se correto aumenta contador
        if ( pikachu == 4 ) {
           Serial.println("Trava aberta por senha");
          unlocked(); // chegou a 4 digitos corretos, libera acesso
        }
        if (entrada == 4) { // foi teclado 0 ou 4 entradas incorretas
          key_init();// inicializa
          break;// interrompe loop
        }
      }

      if (key == '*') { // se foi teclado *
        Serial.println("Digite a senha antiga");
        old_pass_check();// mensagem para entrar a senha antiga
        int entrada = 0;
        while (pikachu < 4 ) {
          char key = keypad.getKey();
          if (key != NO_KEY) {
            entrada += 1;
            delay(duration);
            if (key == pass[pikachu])pikachu += 1;
            if ( pikachu == 4 ) { // foi teclado a senha antiga corretamente
              Serial.println("Digite uma nova senha");
              get_new_pass();// chama função para entrada da nova senha

            }
            if ((key == '*') || (entrada == 4)) { // foi teclado * ou entrou 4 números errados

              return loop();
            }
          }
        }
      }


      if (key == '#') { // se foi teclado #
        Serial.println("Digite senha atual");
        old_pass_check();// mensagem para entrar a senha antiga
        int entrada = 0;
        while (pikachu < 4 ) {
          char key = keypad.getKey();
          if (key != NO_KEY) {
            entrada += 1;
            delay(duration);
            if (key == pass[pikachu])pikachu += 1;
            if ( pikachu == 4 ) { // foi teclado a senha antiga corretamente

              bio();
              
  
            }

            if ((key == '#') || (entrada == 4)) { // foi teclado # ou entrou 4 números errados
             
              return loop();
            }
          }
        }
      }
    }
  }
}


void deleta() {

  Serial.println("Digite a # (de 1 à 162) você quer deletar...");
  uint8_t id = readnumber();
  if (id == 0) {
    return;
  }

  Serial.print("Deletando ID #");
  Serial.println(id);
  deleteFingerprint(id);
}

void get_new_pass() {
  new_pass_entry(); // mensagem, som e LED
  int entrada = 0; // inicializa entrada
  while (pikachu < 4) { // enquanto contrador for menor que 4
    char key = keypad.getKey(); // obtem informação do teclado
    if ((key== '#') || (key== '*')) {
      break;
    }
    if (key != NO_KEY) { // se algo foi teclado
      entrada += 1; // aumenta contador de entrada
      delay(duration);
      pass[pikachu] = key; // aramazena o novo dígito
      pikachu += 1; // próximo dígito
      if ((pikachu == 4) || (entrada == 4)) {
        return loop(); // chegou a 4 digitos, interrompe loop
      }
      if ((key == '*') || (entrada == 4)) { // foi telcado * 4 entradas
        key_init();// inicializa sistema
        break; // sai
      }
    }
  }
}

void new_pass_entry() {
  pikachu = 0;
  delay(duration);
}

void old_pass_check() {
  pikachu = 0;
}

void key_init () {
  pikachu = 0;
  digitalWrite(D0, LOW);
  delay(duration);
}

void code_entry_init() {
  pikachu = 0; // contador para zero
  digitalWrite(D0, LOW);
}

void unlocked() {
  for (int x = 0; x < 5; x++) {
    digitalWrite(D0, HIGH);
    delay(325);
    digitalWrite(D0, LOW);
  }
}

uint8_t deleteFingerprint(uint8_t id) {
   
  uint8_t p = -1;
  p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Deletado!");
  }
   else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Erro na comunicação");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Não foi possível excluir naquele local");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Erro");
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    return p;
  }
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  Serial.print("Esperando digital para inscrever #");
  Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    char key = keypad.getKey(); // obtem informação do teclado
    if ((key== '#') || (key== '*')) {
      break;
    }
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Imagem Capturada");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Erro ao se Comunicar");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Erro ao Capturar Imagem");
        break;
        default:
        Serial.println("Erro Desconhecido");
        break;
    }
  }

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagem convertida");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Imagem muito Confusa");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro ao se Comunicar");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Impossivel encontrar caracteristicas da digital");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Impossível encontrar características da digital");
      return p;
    default:
      Serial.println("Erro Desconhecido");
      return p;
  }

  Serial.println("Retire o dedo");
  delay(1000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Coloque o mesmo dedo novamente");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Imagem Capturada");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Erro ao se Comunicar");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Erro ao Capturar Imagem");
        break;
      default:
        Serial.println("Erro Desconhecido");
        break;
    }
  }

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagem Convertida");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Imagem muito Confusa");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro ao se comunicar");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Nao foi possível encontrar características da impressao digital");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Nao foi possível encontrar características da impressao digital");
      return p;
    default:
      Serial.println("Erro Desconhecido");
      return p;
  }

  Serial.print("Criando cadastro para #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Digital Combinada");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Erro ao se Comunicar");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Digital nao corresponde");
    return p;
  }
  else {
    Serial.println("Erro Desconhecido");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Armazenado!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Erro ao se Comunicar");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Impossível Armazenar Dados");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Erro ao salvar na memoria");
    return p;
  }
  else {
    Serial.println("Erro Desconhecido");
    return p;
  }
}
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagem Capturada");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("Dedo nao Localizado");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro ao se comunicar");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Erro ao Capturar");
      return p;
    default:
      Serial.println("Erro desconhecido");
      return p;
     
  }

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagem Convertida");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Imagem muito confusa");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro ao se comunicar");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Impossivel localizar Digital");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Impossivel Localizar Digital");
      return p;
    default:
      Serial.println("Erro Desconhecido");
      return p;
  }

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Digital Encontrada");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Erro ao se comunicar");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Digital Desconhecida");
    return p;
  } else {
    Serial.println("Erro Desconhecido");
    return p;
  }

  Serial.print("ID # Encontrado");
  Serial.print(finger.fingerID);
  Serial.print(" com precisao de ");
  Serial.println(finger.confidence);
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  digitalWrite(D0, HIGH);
  Serial.println("Trava aberta por biometria");
  delay(2000);
  digitalWrite(D0, LOW);

  Serial.print("ID # Encontrado");
  Serial.print(finger.fingerID);
  Serial.print(" com precisao de ");
  Serial.println(finger.confidence);
  return finger.fingerID;
}

void botoes() {
  buttonState1 = digitalRead(b2);
  if (buttonState1 == HIGH) {
    Serial.println("Digite a senha atual");
    old_pass_check();// mensagem para entrar a senha antiga
    int entrada = 0;
    while (pikachu < 4 ) {
      char key = keypad.getKey();
      if ((key== '#') || (key== '*')) {
      break;
    }
      if (key != NO_KEY) {
        entrada += 1;
        delay(duration);
        if (key == pass[pikachu])pikachu += 1;
        if ( pikachu == 4 ) { // foi teclado a senha antiga corretamente
          deleta();
        }
         if (entrada == 4) { //entrou 4 números errados
             
              return loop();
            }
      }
      else {
      }
    }
  }
}

void interno() {
  estadoBotao = digitalRead(Botao);
  if (estadoBotao == HIGH) {
    digitalWrite(D0, HIGH);
    Serial.println("Trava aberta por botao interno");
    delay(2000);
  } else {
    digitalWrite(D0, LOW);
  }

}


void bio() {

  Serial.println("Pronto para Cadastrar Dados!");
  id = count += 1;
  Serial.print("Inscrever ID#");

  Serial.println(id);
 
  getFingerprintEnroll();

}
