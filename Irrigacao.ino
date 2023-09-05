// Blibliotecas necessárias
//#include <Wire.h>           //Biblioteca para ??
#include <DHT.h>              //Biblioteca para o sensor de temperatura/umidade
#include <RTClib.h>           //Biblioteca para o marcador de data/hora
#include <TimeLib.h>          //Biblioteca para o sistema de controle de data/hora
#include <TimeAlarms.h>       //Biblioteca para o sistema de alarme
#include <SPI.h>              //Biblioteca para a gravação de dados
#include <SD.h>               //Biblioteca para a gravação de dados
#include <LiquidCrystal_I2C.h>//Biblioteca para expor os dados no LCD


LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
//i2c 16x4
File myFile;               // Tipo file para salvar os dados coletados
const int chipSelect = 53; // Porta de comunicação com o SD Card
// Define o objeto RTC para futura definição de horário
RTC_DS3231 rtc; 
// Definir portas de sensores de umidade.
// provavelmente podem ser apagados
#define DHTPIN 49           // Define  o pino do sensor de umidade/temperatura
#define DHTTYPE DHT22       // Define o modelo do sensor (DHT22)
#define sensorcaixadagua A2 // Define o sensor da caixa d'agua
DHT dht(DHTPIN, DHTTYPE);   // Passa os parâmetros para criar o objeto dht
// Configurações para ativar os 2 relês.
//Porta ligada ao pino IN1 do modulo relê
int porta_rele1 = 12;
//Porta ligada ao pino IN2 do modulo relê
int porta_rele2 = 13;


// Configurações do sensor de umidade solo.
const int Sensor_Umidadesolo = A0; //PINO UTILIZADO PELO SENSOR
bool estado = false; // define se a coleta de dados está ligada ou desligada
int caixadagua = 0; // define o estado da caixa dagua (cheia ou vazia)
String estadocaixa = "";
String estadosolo = "";
void setup()
{
  lcd.init();               // Inicia o LCD
  lcd.backlight();          // Inicia a backlight do LCD
  dht.begin();              // Inicia o dht para a coleta de dados
  Serial.begin(9600);       // Inicia a Serial
  desligar1();               // Desliga o relê por segurança
  //Define pinos para o relê como saida
  pinMode(porta_rele1, OUTPUT); // Relê 1
  pinMode(porta_rele2, OUTPUT); // Relê 2
  pinMode(sensorcaixadagua, INPUT);

  // Confere se o RTC está funcionando
  if (! rtc.begin()) { // se não encontrar o rtc:
    Serial.println("DS3231 não encontrado");
    while (1);
  }
  Serial.println("DS3231 OK!");
  if (rtc.lostPower()) {
    Serial.print("RTC RESETADO");
  }

  int dataehora[6];
  getdate(dataehora);
  setTime(dataehora[0], dataehora[1], dataehora[2], dataehora[3], dataehora[4], dataehora[5]);
  //Ver função "Get date" para explicações sobre este trecho

  SetAlarms(); // Liga os alarmes definido na função "SetAlarms"

  // Confere se o cartão SD está funcionando
  Serial.print("Inicializando Cartão SD...");
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
}

void loop() {
  
  // Printa a hora
  DateTime now = rtc.now(); 
  char date[10] = "hh:mm:ss";
  rtc.now().toString(date);
  Serial.println(date);

  myFile =SD.open("dados.txt", FILE_WRITE); // Abre o SD para futura gravação
  // Se abriu tran quilamente, continua o processo:
  if (myFile) {
    Serial.println("Escrevendo para a TXT:");

    // Obtém o horário atual
    DateTime now = rtc.now();
    char date[20] = "hh:mm:ss,DD/MM/YY"; //Este para impressão no SD
    rtc.now().toString(date);
    char Dat_hora[12] = "hh:mm DD/MM";   //Este para impressão no LCD
    rtc.now().toString(Dat_hora);
    
    // Obtem os dados:
    int Umidade_Solo = analogRead(Sensor_Umidadesolo);
    float Umidade_Ar = dht.readHumidity(); //VALOR DE UMIDADE DO AR MEDIDO
    float Temperatura = dht.readTemperature(); //VALOR DE TEMPERATURA DO AR MEDIDO
          caixadagua = analogRead(sensorcaixadagua);
          Serial.println(caixadagua);
    //Printa dados no LCD
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(1, 0);
    lcd.print("Hora:");
    lcd.setCursor(7, 0);
    lcd.print(Dat_hora);
    lcd.setCursor(1, 1);
    lcd.print("Estado:");
    lcd.setCursor(9, 1);
    if (estado) {
      lcd.print("Ligado");
    }
    else {
      lcd.print("Desligado");
    }
    lcd.setCursor(1, 2);
    lcd.print("UA: ");
    lcd.setCursor(5, 2);
    lcd.print(round(Umidade_Ar));
    lcd.setCursor(9, 2);
    lcd.print("US:");
    lcd.setCursor(13, 2);
    lcd.print(round(Umidade_Solo));
    lcd.setCursor(18, 2);
    if (Umidade_Solo > 450){
      estadosolo = "S";
      lcd.print(estadosolo);
    }
    else{
      estadosolo = "M";
      lcd.print(estadosolo);
    }
    lcd.setCursor(1, 3);
    lcd.print("Temp:");
    lcd.setCursor(7, 3);
    lcd.print(Temperatura);
    lcd.setCursor(12, 3);
    lcd.print(" Caixa:");
    lcd.setCursor(19, 3);
    if (caixadagua > 100){                //Caso a caixa esteja cheia, printa "C" no LCD, caso contrário, printa "V" 
      estadocaixa = "C";
      lcd.print(estadocaixa);
    }
    else {
      estadocaixa = "V";
      lcd.print(estadocaixa);
    }

    //Concatena os dados em uma unica String
    String dados = "," + String(Umidade_Solo) + "," + String(Umidade_Ar) + "," + String(Temperatura) + "," + String(estado);
    String DADOS2 = "," + String(estadosolo) + "," + String(estadocaixa);
    //envia os dados para o SD
    Serial.print("Data: ");
    Serial.print(date);            //Printa a data no Serial mas NÃO pula para a proxima linha
    myFile.print(date);            //Printa a data no SD mas NÃO pula para a proxima linha

    Serial.print("Dados: ");
    Serial.print(dados);         //Printa a data no Serial e pula para a proxima linha
    myFile.print(String(dados)); //Printa a data no SD e pula para a proxima linha
    Serial.println(DADOS2);         //Printa a data no Serial e pula para a proxima linha
    myFile.println(String(DADOS2)); //Printa a data no SD e pula para a proxima linha
    // fecha o arquivo
    myFile.close();
    Serial.println("Feito!!!");
  } else {
    // Se dar erro ao abrir o arquvio:
    Serial.println("erro ao abrir o arquivo");
  }
  Alarm.delay(30000); // Espera até a próxima gravação de dados 
}



void ligar1() {
  if ((caixadagua > 100) and (analogRead(Sensor_Umidadesolo)> 1000 )) { // Se a caixa D'Agua estiver cheia E o solo estiver seco o suficiente
    digitalWrite(porta_rele1, LOW);
    digitalWrite(porta_rele2, LOW);
    Serial.println("liga acionado");
    delay(100);
    estado = true;
  }
}

void desligar1() {
  digitalWrite(porta_rele1, HIGH);
  digitalWrite(porta_rele2, HIGH);
  Serial.println("desliga acionado");
  delay(100);
  estado = false;
}
void ligar2() {
  if ((caixadagua > 100) and (analogRead(Sensor_Umidadesolo)> 450 )) { // Se a caixa D'Agua estiver cheia E o solo estiver seco o suficiente
    digitalWrite(porta_rele1, LOW);
    digitalWrite(porta_rele2, LOW);
    Serial.println("liga acionado");
    delay(100);
    estado = true;
  }
}

void desligar2() {
  digitalWrite(porta_rele1, HIGH);
  digitalWrite(porta_rele2, HIGH);
  Serial.println("desliga acionado");
  delay(100);
  estado = false;
}
void getdate(int dataehora[6]) {
  //variaveis para conversão de hora para int:
  char H[03] = "hh";
  char M[03] = "mm";
  char S[03] = "ss";

  char Y[05] = "YY";
  char Me[03] = "MM";
  char D[03] = "DD";

  DateTime now = rtc.now();

  //Puxa do RTC informações isoladas de cada fragmento da hora, para futuro manejamento dentro da biblioteca Timelib
  //Integrando assim, o RTC com a biblioteca TimeAlarms e Timelib
  rtc.now().toString(H);
  rtc.now().toString(M);
  rtc.now().toString(S);

  rtc.now().toString(Y);
  rtc.now().toString(Me);
  rtc.now().toString(D);

  // Transforma data e hora em inteiro e guarda em uma lista
  dataehora[0] = atol(H);
  dataehora[1] = atol(M);
  dataehora[2] = atol(S);

  dataehora[3] = atol(Me);
  dataehora[4] = atol(D);
  dataehora[5] = atol(Y);

}

void SetAlarms() {
  // HORARIOS QUE O SISTEMA VAI LIGAR/DESLIGAR
  Alarm.alarmRepeat(9, 00, 00, ligar1);
  Alarm.alarmRepeat(9, 15, 00, desligar1);
  Alarm.alarmRepeat(15, 30, 00, ligar2);
  Alarm.alarmRepeat(15, 45, 00, desligar2);


  // SE precisar de mais de 6 alarmes sera necessario aumentar o limite na biblioteca

}
