  // 11/07/24  SE PISAN LOS TEXTOS
  // 11/07/24  AGREGAR FUNCION PARA BLOQEUAR LUEGO DE 2 INTENTOS
  // 11/07/24  AGREGAR OPCION 2 INTENTOS MAX, AHORA SON INFITOS INTENTOS
  // 11/07/24  AGREGAR NUEVO ESTADO selec_ocupar EN CasilleroSeleccionado 
 

#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

//funciones prototipos
String ObtenerContrasena();
void checkDoor(int Nro, int alarmPin);
void CasilleroSeleccionado();
unsigned long startTime;


LiquidCrystal_I2C lcd(0x27, 20, 4);  // Ampliar si cambiamos LCD

//Variables de Uso Multiple
//int contador = 0; // contador para puerta cerrada (borrar si no se usa)
//int selectedLocker = 0; // Variable para almacenar el número de casillero (borrar si no se usa)
int Nro;
String enteredPassword;
char key = '\0';
char pirulin = '\0';


//Parametros Generales
const int cantCasillero = 8;  //cantidad predefinida de casillero
String MasterKEY = "202422";

//Definicion Teclado Matricial

const char* contrasenasTontas[] = {
  "0123", "1234", "2345", "3456", "4567", "5678", "6789", "7890", "8901",          // números consecutivos
  "0011", "1122", "2233", "3344", "4455", "5566", "6677", "7788", "8899", "9900",  // números en pares iguales
  "0000", "1111", "2222", "3333", "4444", "5555", "6666", "7777", "8888", "9999",  // números iguales
};

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 22, 24, 26, 28 };
byte colPins[COLS] = { 30, 32, 34, 36 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//Definicion de IO  Revisar locker
/*  // Lineas comentadas para maqueta,se redefinieron pines para conveniencia de layout
  const int ledRojo[cantCasillero] =        {8,  6, 14, 18, 27, 35, 40, 41}; // Pines para los LEDs rojos
  const int ledVerde[cantCasillero] =       {10, 5, 15, 19, 29, 37, 42, 43}; // Pines para los LEDs verdes
  const int sensorPuerta[cantCasillero] =   {13, 4, 16, 23, 31, 38, 44, 45}; // Pines para los sensores de puerta
*/
//INICIA PINES DE MAQUETA
const int sensorPuerta[cantCasillero] = { 23, 25, 27, 29, 31, 33, 35, 37 };  // Pines para los sensores de puerta
const int ledRojo[cantCasillero] = { 38, 40, 42, 44, 46, 48, 50, 52 };       // Pines para los LEDs rojos
const int ledVerde[cantCasillero] = { 39, 41, 43, 45, 47, 49, 51, 53 };      // Pines para los LEDs verdes
//Termina PINES MAQUETA
const int LiberaPestillo[cantCasillero] = { 2, 3, 4, 5, 6, 7, 8, 9 };  // Pines de libera pestillo
const int alarmPin = 10;

struct Casillero {
  int sensorPuerta;
  int LiberaPestillo;
  String password;
  bool Ocupacion;
  int estado;
  int estadoPrevio;
};
Casillero casilleros[8];

enum Estados {
  MasterPASS,
  Inicializacion,
  seleccionarCasillero,
  revisarPuerta,
  ingresarContrasena,
  verificarContrasena,
  guardarContrasena,  //revisar que se use o cambiar
  cerrarPuerta,
  selec_liberar,
  selec_ocupar,
};
Estados Actual = MasterPASS;
void setup() {
  Serial.begin(9600);
  Serial.println("Twisted Transistors - Sistema de Casilleros");

  //definicion salidas y entradas y led verde como liberado
  for (int i = 0; i < cantCasillero; i++) {
    pinMode(ledRojo[i], OUTPUT);
    pinMode(ledVerde[i], OUTPUT);
    pinMode(LiberaPestillo[i], OUTPUT);
    digitalWrite(ledRojo[i], HIGH);
    pinMode(sensorPuerta[i], INPUT_PULLUP);
    digitalWrite(LiberaPestillo[i], HIGH);
  }
  //Alarma
  pinMode(alarmPin, OUTPUT);
  digitalWrite(alarmPin, LOW);

  //LCD
  lcd.init();           // Inicializa el LCD
  lcd.backlight();      // Enciende la luz de fondo
  lcd.setCursor(0, 0);  // Imprime un mensaje en el LCD
  lcd.print("Twisted");
  lcd.setCursor(0, 1);  // Imprime un mensaje en el LCD
  lcd.print("Transistors");
  delay(3000);
  lcd.clear();

  for (int i = 0; i < cantCasillero; i++) {
    casilleros[i].Ocupacion = true;  // estado inicial ocupado
  }

  for (int i = 0; i < cantCasillero; i++) {
    casilleros[i].estadoPrevio = digitalRead(sensorPuerta[i]);  // hacer arreglo de estados previos para los 8 casilleros
  }
}
void imprimirEstadoCasilleros() {
  Serial.println("|| Estado de los casilleros:   ||\n");
  for (int i = 0; i < cantCasillero; i++) {
    Serial.print("C ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(casilleros[i].Ocupacion ? "Ocupado || " : "Desocupado || ");
    Serial.print(" Contraseña: ");
    Serial.print(casilleros[i].password);
    Serial.print(" || Puerta: ");
    Serial.print(digitalRead(sensorPuerta[i]) == HIGH ? "Abierta|| " : "Cerrada|| ");
    Serial.print(" Luces: ");
    Serial.print(digitalRead(ledRojo[i]) == HIGH ? "Rojo|| " : "");
    Serial.println(digitalRead(ledVerde[i]) == HIGH ? "Verde||" : "");
  }
}
int pepe = 1;  //11/07/2024 PEPE
unsigned long previousMillis = 0;
String MasterPW = "2024AB";

void loop() {
  static unsigned long previousMillisPrint = 0;
  const long intervalPrint = 5000;  // Intervalo de 2 segundos

  // Verifica si han pasado 2 segundos desde la última vez que se imprimió el estado de los casilleros
  if (millis() - previousMillisPrint >= intervalPrint) {
    previousMillisPrint = millis();
    imprimirEstadoCasilleros();
  }

  Nro = key - '0';
  const long interval = 5000;

  switch (Actual) {
    case seleccionarCasillero:
      {
        //Serial.println("Seleccionar Casillero");
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {

          Serial.println("Seleccionar Casilleros||");
          previousMillis = currentMillis;


          /* Correccion de formateo pantalla
            lcd.print("Seleccione");
            lcd.setCursor(0, 1);
            lcd.print("Casillero:");
          */
          lcd.clear();  //REVISAR QUE ONDA
          lcd.setCursor(0, 0);
          lcd.print("Seleccione Box");
          lcd.setCursor(0, 1);
          lcd.print("Box: ");
          lcd.setCursor(0, 2);
          lcd.print(" # - Liberar");
          lcd.setCursor(0, 3);
          lcd.print(" * - Ocupar");
        }
        key = keypad.getKey();
        Nro = key - '0';
        //Serial.println(Nro);
        if (key >= '1' && key <= '8') {  // Verificamos si es un número válido
          Nro = key - '0';
          lcd.setCursor(6, 1);
          lcd.print(Nro);
          Serial.print("Nro de Seleccionar Casillero:_");
          Serial.println(Nro);
          //delay(1000);
          CasilleroSeleccionado();
        }
        break;
      }

    case verificarContrasena:
      {
        Serial.println("Estado Verificar Contrasena");
        lcd.setCursor(0, 2);
        lcd.print("PAPAPAP");  // 11/07/2024 !?!?!??!

        String enteredPassword = ObtenerContrasena();
        delay(1000);  // 11/07/2024 cambiar por syst
        if (enteredPassword == casilleros[Nro - 1].password) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Acceso Concedido");
          digitalWrite(LiberaPestillo[Nro - 1], LOW);  // abre la puerta

          unsigned long startTime = millis();
          while (millis() - startTime < 10000 && digitalRead(sensorPuerta[Nro - 1]) == LOW) {
            // Espera hasta que pasen 10 segundos o hasta que el sensor de la puerta detecte que la puerta está abierta
          }
          //PRUEBA CONDICION
          if (digitalRead(sensorPuerta[Nro - 1]) == LOW) {
            digitalWrite(LiberaPestillo[Nro - 1], HIGH);  // cierra la puerta
            digitalWrite(ledVerde[Nro - 1], LOW);         // enciende la luz verde
            digitalWrite(ledRojo[Nro - 1], HIGH);         // apaga la luz roja
            casilleros[Nro - 1].Ocupacion = true;         // marca el casillero como desocupado
            Actual = seleccionarCasillero;
          } else {
            digitalWrite(LiberaPestillo[Nro - 1], HIGH);  // cierra la puerta
            digitalWrite(ledVerde[Nro - 1], HIGH);        // enciende la luz verde
            digitalWrite(ledRojo[Nro - 1], LOW);          // apaga la luz roja
            casilleros[Nro - 1].Ocupacion = false;        // marca el casillero como desocupado
            Serial.println("Casillero Liberado");
            delay(500);
            Actual = seleccionarCasillero;
          }
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Acceso Denegado");
          Serial.println("Acceso Denegado");
          delay(2000);
          Actual = seleccionarCasillero;
        }
        break;
      }

    case selec_liberar:
      {

        lcd.clear();
        casilleros[Nro - 1].estado = digitalRead(sensorPuerta[Nro - 1]);  // revisa el estado de la puerta al seleccionar casillero (desocupado pero puerta cerrada)
        Serial.print("Estado: ");
        Serial.print(casilleros[Nro - 1].estado);

        if (casilleros[Nro - 1].Ocupacion == false) {
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Casillero desocupado.");
          unsigned long startTime = millis();
          Actual = seleccionarCasillero;
        }

        else if (casilleros[Nro - 1].Ocupacion == true) {
          lcd.clear();
          lcd.setCursor(0, 2);
          lcd.print("Box Ocupado");
          delay(1000);
          lcd.clear();
          Actual = verificarContrasena;

        } else {
          Actual = ingresarContrasena;
        }
        break;
      }


    case revisarPuerta:
      {
        Serial.println("Revisar Puerta");
        Serial.print("Nro:_");
        Serial.println(Nro);
        casilleros[Nro - 1].estado = digitalRead(sensorPuerta[Nro - 1]);  // revisa el estado de la puerta al seleccionar casillero (desocupado pero puerta cerrada)
        Serial.print("Estado: ");
        Serial.print(casilleros[Nro - 1].estado);

        if (casilleros[Nro - 1].estado == LOW && casilleros[Nro - 1].Ocupacion == false) {
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("Abrir puerta");
          digitalWrite(LiberaPestillo[Nro - 1], LOW);  // abre la puerta

          unsigned long startTime = millis();
          while (millis() - startTime < 4000 && digitalRead(sensorPuerta[Nro - 1]) == LOW) {
            // Espera hasta que pasen 4 segundos o hasta que el sensor de la puerta detecte que la puerta está abierta
          }
          lcd.clear();
          digitalWrite(LiberaPestillo[Nro - 1], HIGH);  // cierra la puerta
          Actual = seleccionarCasillero;
        } else if (casilleros[Nro - 1].Ocupacion) {
          Serial.println("Ocupado");
          lcd.clear();
          /* REFORMATEO LCD
            lcd.setCursor(0, 0);     // Imprime un mensaje en el LCD
            lcd.print("Box");  // puerta cerrada por favor abrir
            lcd.setCursor(0, 1);     // Imprime un mensaje en el LCD
            lcd.print("Ocupado");    // puerta cerrada por favor abrir
          */
          lcd.setCursor(0, 2);
          lcd.print("Box Ocupado");
          delay(1000);
          Actual = verificarContrasena;
        } else {
          Actual = ingresarContrasena;
        }
        break;
      }

    case ingresarContrasena:
      {
        Serial.println("Estado Ingresar Contrasena");
        enteredPassword = ObtenerContrasena();
        casilleros[Nro - 1].password = enteredPassword;
        digitalWrite(ledRojo[Nro - 1], HIGH);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Contrasena");
        lcd.setCursor(0, 1);
        lcd.print("guardada");
        Actual = cerrarPuerta;
        break;
      }
    case cerrarPuerta:
      if (digitalRead(sensorPuerta[Nro - 1]) == HIGH) {
        Serial.println("La puerta está abierta. Por favor, cierre la puerta para continuar.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Cierre");
        lcd.setCursor(0, 1);
        lcd.print("Puerta");
        checkDoor(Nro, alarmPin);  // Llama a la función checkDoor
        casilleros[Nro - 1].Ocupacion = true;
        Serial.println("Casillero en uso.");  // Nuevo mensaje
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Casillero");
        lcd.setCursor(0, 1);
        lcd.print("en uso");
        digitalWrite(ledRojo[Nro - 1], HIGH);
        digitalWrite(ledVerde[Nro - 1], LOW);
        Actual = seleccionarCasillero;
        break;
      }
    case Inicializacion:
      if (pepe == 1) { //11/07/2024 PEPE
        Serial.println("Inicializacion de casilleros");
        lcd.setCursor(0, 0);
        lcd.print("Iniciar Sistema");
        lcd.setCursor(0, 1);
        lcd.print("Seleccione BOX");
        while (key != '*') {
          key = keypad.getKey();
          Nro = int(key - '0');
          if (Nro > 0 && Nro <= 8 && key != '\0') {
            casilleros[Nro - 1].Ocupacion = false;
            Serial.println(casilleros[Nro - 1].Ocupacion);
            lcd.setCursor(0, 2);
            lcd.print("Box ");
            lcd.print(Nro);
            lcd.setCursor(0, 3);
            lcd.print("Desbloqueado");
            digitalWrite(ledRojo[Nro - 1], LOW);
            digitalWrite(ledVerde[Nro - 1], HIGH);
          }
        }
        Actual = seleccionarCasillero;
        lcd.clear(); 
        pepe = 0; // 11/07/2024 PEPE 
        break;
      }

    case MasterPASS:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Inicio Sistema");
      lcd.setCursor(0, 1);
      lcd.print("Ingresar Clave:");
      String enteredPassword = "";
      while (enteredPassword.length() < 10) {
        char digit = keypad.getKey();
        if (digit && digit != 'A' && digit != 'B') {
          enteredPassword += digit;
          lcd.setCursor(enteredPassword.length() - 1, 3);  // Asume que quieres imprimir en la cuarta fila
          lcd.print("*");
          //Serial.print(digit);
        }
        if (digit != '\0')

          Serial.print(digit);

        if (digit == 'A')  //si presiono A ya confirma de una
        {
          Serial.println("Entre a A");
          if (enteredPassword == MasterKEY) {
            lcd.clear();  //Corregido 06/07/2024
            Serial.println("Cambie a INICIALIZACION");
            Actual = Inicializacion;  //202422
            //Carteles de confirmacion en LCD FALTAN
            break;
          } else {
            Serial.println("Entre a else de A");
            enteredPassword = "";
            lcd.setCursor(0, 1);
            lcd.print("Clave Invalida");
            delay(1000);
            break;
          }
        } else if (digit == 'B' && enteredPassword.length() > 0) {
          enteredPassword = enteredPassword.substring(0, enteredPassword.length() - 1);  // Borra el último carácter ingresado
          lcd.setCursor(enteredPassword.length(), 3);
          lcd.print(" ");
          Serial.println("Borré algo: ");
          Serial.println(enteredPassword);
        }
      }
      Serial.println("Sali del while");
      break;
  }
}
String ObtenerContrasena() {

  // 11/07/24  SE PISAN LOS TEXTOS
  // 11/07/24  AGREGAR FUNCION PARA BLOQEUAR LUEGO DE 2 INTENTOS
  // 11/07/24  AGREGAR OPCION 2 INTENTOS MAX, AHORA SON INFITOS INTENTOS
  Serial.println("Obtener Contraseña");
  lcd.setCursor(0, 2);
  lcd.print("                 ");
  lcd.setCursor(0, 3);
  lcd.print("                 ");
  lcd.setCursor(0, 2);
  lcd.print("Contrasena:");
  String enteredPassword = "";
  int intentos = 0;  // Contador de intentos
  unsigned long startTime = millis();  // Tiempo de inicio

  while (enteredPassword.length() < 4) {
    char digit = keypad.getKey();

    // Verificar tiempo límite de 20 segundos
    unsigned long currentTime = millis();
    int elapsedTime = (currentTime - startTime) / 1000;
    if (elapsedTime > 20) {
      lcd.setCursor(0, 3);
      lcd.print("Tiempo Excedido");
      delay(2000);
      return "";
    }

    // Mostrar el tiempo restante en la línea 3 del LCD
    lcd.setCursor(0, 3);
    lcd.print("Tiempo: ");
    lcd.print(20 - elapsedTime);
    lcd.print("s     ");

    if (digit && digit != '*' && digit != '#') {
      enteredPassword += digit;
      lcd.setCursor(enteredPassword.length() - 1, 3);  // Asume que quieres imprimir en la cuarta fila
      lcd.print("*");
      Serial.print(digit);
    } else if (digit == '*' || digit == '#') {
      enteredPassword = "";
      intentos++;

      lcd.setCursor(0, 3);
      lcd.print("Caracter Invalido ");
      lcd.setCursor(0, 2);
      lcd.print("Intentos: ");
      lcd.print(intentos);
      delay(2000);
      lcd.setCursor(0, 2);
      lcd.print("Contrasena:");
    }

    for (const char* tonta : contrasenasTontas) {
      if (enteredPassword == tonta) {
        enteredPassword = "";
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Clave Invalida");
        lcd.setCursor(0, 2);
        lcd.print("Ingrese Nuevamente");
        delay(2000);
        Actual = seleccionarCasillero;
        intentos++;
        lcd.setCursor(0, 3);
        lcd.print("Intentos: ");
        lcd.setCursor(10, 3);
        lcd.print(intentos);
      }
    }
  }

  return enteredPassword;
}

void checkDoor(int Nro, int alarmPin) {
  unsigned long startTime = millis();
  while (digitalRead(sensorPuerta[Nro - 1]) == HIGH) {
    if (millis() - startTime > 10000) {  // Si han pasado más de 10 segundos
      digitalWrite(alarmPin, HIGH);      // Enciende la alarma
    }
  }
  digitalWrite(alarmPin, LOW);  // Apaga la alarma cuando la puerta se cierra
}

void CasilleroSeleccionado()
{
  Serial.print("KEY||");
  pirulin = keypad.getKey(); // 11/07/2024 PIRULÍN
  Serial.println(key);
  Serial.print("||TERMINA KEY");
  Serial.println("Entré en CasilleroSeleccionado"); // 11/07/2024 NO SE USA MÁS WHILE FLAKO NI VARIABLES HORRIBLES
  while (pirulin != '#' && pirulin != '*' )
  {

    //Serial.println("Entré en while de CasilleroSeleccionado");
    pirulin = keypad.getKey();
    if (pirulin == '#' || pirulin == '*' )
    {
      if (pirulin == '#')
      {
        Serial.println("Liberar casillero");
        Actual = selec_liberar;
        break;
      }
      else if (pirulin == '*')
      {
        Serial.println("Ocupar casillero");
        Actual = revisarPuerta;                // 11/07/24 QUEDA HACER EL ESTAD selec_ocupar
        break;
      }
    }
  }
}
