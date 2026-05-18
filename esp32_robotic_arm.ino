/**
 * BRAT ROBOTIZAT ESP32-S3
 * 3x MG996R + Vacuum
 *
 * Include:
 * - comenzi manuale din consola
 * - miscare automata
 * - detectie simulata pentru aluminium can si plastic bottle
 * - actiune definita automat in functie de obiect
 * - vacuum GRAB / DROP
 * - servo detach dupa fiecare miscare
 */

#include <ESP32Servo.h>

// ============================================================
// PINI SERVO
// ============================================================

const int PIN_BAZA = 16;
const int PIN_JOS  = 17;
const int PIN_TIJA = 18;

// ============================================================
// PINI VACUUM
// ============================================================

const int pumpRelay     = 20;
const int solenoidRelay = 21;

// Pentru relee HIGH trigger:
// HIGH = pornit, LOW = oprit
const int RELAY_ON  = HIGH;
const int RELAY_OFF = LOW;

// ============================================================
// SETARI SERVO
// ============================================================

const int SERVO_MIN_US = 400;
const int SERVO_MAX_US = 2600;

const int VITEZA_MS_PER_GRAD = 20;

const int UNGHI_MIN = 0;
const int UNGHI_MAX = 180;

// ============================================================
// POZITII STANDARD
// ============================================================

const int BAZA_HOME = 90;
const int JOS_HOME  = 90;
const int TIJA_HOME = 50;

// Pozitie de unde robotul ridica obiectul
const int BAZA_PICK = 90;
const int JOS_PICK  = 120;
const int TIJA_PICK = 80;

// Zona de sortare pentru doza de aluminiu
const int BAZA_ALUMINIUM = 30;
const int JOS_ALUMINIUM  = 100;
const int TIJA_ALUMINIUM = 60;

// Zona de sortare pentru sticla de plastic
const int BAZA_PLASTIC = 150;
const int JOS_PLASTIC  = 100;
const int TIJA_PLASTIC = 60;

// ============================================================
// TIPURI DE OBIECTE
// ============================================================

enum ObjectType {
  NO_OBJECT,
  ALUMINIUM_CAN,
  PLASTIC_BOTTLE
};

// ============================================================
// ACTIUNI ROBOT
// ============================================================

enum RobotAction {
  ACTION_NONE,
  ACTION_SORT_ALUMINIUM,
  ACTION_SORT_PLASTIC
};

// ============================================================
// OBIECTE SERVO
// ============================================================

Servo servoBaza;
Servo servoJos;
Servo servoTija;

// ============================================================
// VARIABILE SISTEM
// ============================================================

int pozBaza = BAZA_HOME;
int pozJos  = JOS_HOME;
int pozTija = TIJA_HOME;

volatile int servoActiv = 0;

bool vacuumActiv = false;

ObjectType detectedObject = NO_OBJECT;
RobotAction currentAction = ACTION_NONE;

// ============================================================
// CONTROL VACUUM
// ============================================================

void pompaON() {
  digitalWrite(pumpRelay, RELAY_ON);
}

void pompaOFF() {
  digitalWrite(pumpRelay, RELAY_OFF);
}

void solenoidON() {
  digitalWrite(solenoidRelay, RELAY_ON);
}

void solenoidOFF() {
  digitalWrite(solenoidRelay, RELAY_OFF);
}

// ============================================================
// VACUUM GRAB
// ============================================================

void vacuumGrab() {
  Serial.println("[VACUUM] Pornire vacuum pentru prindere obiect.");

  // Solenoid inchis pentru mentinerea vacuumului
  solenoidOFF();

  // Pompa creeaza vacuum
  pompaON();

  delay(1500);

  // Pompa se opreste dupa ce vacuumul s-a format
  pompaOFF();

  vacuumActiv = true;

  Serial.println("[VACUUM] Obiect prins. Pompa oprita, vacuum mentinut.");
}

// ============================================================
// VACUUM DROP
// ============================================================

void vacuumDrop() {
  Serial.println("[VACUUM] Eliberare obiect.");

  // Pompa trebuie oprita inainte de eliberare
  pompaOFF();

  // Solenoidul se deschide scurt pentru a intra aer
  solenoidON();

  delay(800);

  solenoidOFF();

  vacuumActiv = false;

  Serial.println("[VACUUM] Obiect eliberat.");
}

// ============================================================
// STOP TOTAL
// ============================================================

void stopTot() {
  pompaOFF();
  solenoidOFF();

  servoBaza.detach();
  servoJos.detach();
  servoTija.detach();

  pinMode(PIN_BAZA, INPUT);
  pinMode(PIN_JOS, INPUT);
  pinMode(PIN_TIJA, INPUT);

  servoActiv = 0;
  vacuumActiv = false;

  Serial.println("[STOP] Pompa, solenoidul si servo-urile au fost oprite.");
}

// ============================================================
// MISCARE SERVO CU DETACH
// ============================================================

void miscaServoDetach(
  Servo &servo,
  int pin,
  int &pozCurenta,
  int unghiTinta,
  int idServo
) {
  unghiTinta = constrain(unghiTinta, UNGHI_MIN, UNGHI_MAX);

  if (pozCurenta == unghiTinta) {
    Serial.println("[SERVO] Servo-ul este deja la pozitia dorita.");
    return;
  }

  if (servoActiv != 0) {
    Serial.println("[SERVO] Alt servo este in miscare. Comanda anulata.");
    return;
  }

  servoActiv = idServo;

  servo.setPeriodHertz(50);
  servo.attach(pin, SERVO_MIN_US, SERVO_MAX_US);

  delay(30);

  int pas = (unghiTinta > pozCurenta) ? 1 : -1;

  while (pozCurenta != unghiTinta) {
    pozCurenta += pas;
    servo.write(pozCurenta);
    delay(VITEZA_MS_PER_GRAD);
  }

  delay(100);

  // Detach elimina semnalul PWM dupa miscare
  servo.detach();

  // Pin INPUT pentru reducerea jitterului
  pinMode(pin, INPUT);

  servoActiv = 0;

  Serial.print("[SERVO] Pozitie finala: ");
  Serial.println(pozCurenta);
}

// ============================================================
// HOME
// ============================================================

void mergiHome() {
  Serial.println("[AUTO] Revenire la pozitia HOME.");

  miscaServoDetach(servoTija, PIN_TIJA, pozTija, TIJA_HOME, 3);
  miscaServoDetach(servoJos,  PIN_JOS,  pozJos,  JOS_HOME,  2);
  miscaServoDetach(servoBaza, PIN_BAZA, pozBaza, BAZA_HOME, 1);

  Serial.println("[AUTO] Robotul este in pozitia HOME.");
}

// ============================================================
// AUTO PICK
// ============================================================

void autoPick() {
  Serial.println("[AUTO] Miscare automata catre pozitia de ridicare.");

  miscaServoDetach(servoBaza, PIN_BAZA, pozBaza, BAZA_PICK, 1);
  miscaServoDetach(servoJos,  PIN_JOS,  pozJos,  JOS_PICK,  2);
  miscaServoDetach(servoTija, PIN_TIJA, pozTija, TIJA_PICK, 3);

  delay(300);

  vacuumGrab();

  delay(300);

  // Ridicare dupa prinderea obiectului
  miscaServoDetach(servoJos, PIN_JOS, pozJos, JOS_HOME, 2);

  Serial.println("[AUTO] Obiect ridicat.");
}

// ============================================================
// AUTO DROP LA O ZONA
// ============================================================

void autoDropLaZona(
  int bazaZona,
  int josZona,
  int tijaZona
) {
  Serial.println("[AUTO] Mutare catre zona de sortare.");

  miscaServoDetach(servoBaza, PIN_BAZA, pozBaza, bazaZona, 1);
  miscaServoDetach(servoJos,  PIN_JOS,  pozJos,  josZona,  2);
  miscaServoDetach(servoTija, PIN_TIJA, pozTija, tijaZona, 3);

  delay(300);

  vacuumDrop();

  delay(300);

  mergiHome();
}

// ============================================================
// DEFINIRE ACTIUNE DUPA OBIECT
// ============================================================

void defineActionForObject(ObjectType object) {
  if (object == ALUMINIUM_CAN) {
    currentAction = ACTION_SORT_ALUMINIUM;

    Serial.println("[AI] Detected object: aluminium can");
    Serial.println("[AI] If aluminium can -> action: GRAB and SORT TO ALUMINIUM ZONE");
  }

  else if (object == PLASTIC_BOTTLE) {
    currentAction = ACTION_SORT_PLASTIC;

    Serial.println("[AI] Detected object: plastic bottle");
    Serial.println("[AI] If plastic bottle -> action: GRAB and SORT TO PLASTIC ZONE");
  }

  else {
    currentAction = ACTION_NONE;

    Serial.println("[AI] No valid object detected.");
  }
}

// ============================================================
// EXECUTARE ACTIUNE DEFINITA
// ============================================================

void executeDefinedAction() {
  if (currentAction == ACTION_SORT_ALUMINIUM) {
    Serial.println("[ACTION] Aluminium can action: GRAB");

    autoPick();

    Serial.println("[ACTION] Aluminium can action: MOVE TO ALUMINIUM ZONE");

    autoDropLaZona(
      BAZA_ALUMINIUM,
      JOS_ALUMINIUM,
      TIJA_ALUMINIUM
    );

    currentAction = ACTION_NONE;
    detectedObject = NO_OBJECT;

    Serial.println("[ACTION] Aluminium can action completed.");

    return;
  }

  if (currentAction == ACTION_SORT_PLASTIC) {
    Serial.println("[ACTION] Plastic bottle action: GRAB");

    autoPick();

    Serial.println("[ACTION] Plastic bottle action: MOVE TO PLASTIC ZONE");

    autoDropLaZona(
      BAZA_PLASTIC,
      JOS_PLASTIC,
      TIJA_PLASTIC
    );

    currentAction = ACTION_NONE;
    detectedObject = NO_OBJECT;

    Serial.println("[ACTION] Plastic bottle action completed.");

    return;
  }

  Serial.println("[ACTION] No action defined. Use CAN or PLASTIC first.");
}

// ============================================================
// TEST VACUUM
// ============================================================

void testVacuum() {
  Serial.println("[TEST] Test pompa vacuum.");

  pompaON();
  delay(1000);
  pompaOFF();

  delay(500);

  Serial.println("[TEST] Test solenoid.");

  solenoidON();
  delay(800);
  solenoidOFF();

  Serial.println("[TEST] Test vacuum terminat.");
}

// ============================================================
// STATUS SISTEM
// ============================================================

void statusSistem() {
  Serial.println();
  Serial.println("=============== STATUS SISTEM ===============");

  Serial.print("Pozitie baza: ");
  Serial.println(pozBaza);

  Serial.print("Pozitie brat jos: ");
  Serial.println(pozJos);

  Serial.print("Pozitie tija: ");
  Serial.println(pozTija);

  Serial.print("Servo activ: ");
  Serial.println(servoActiv);

  Serial.print("Vacuum activ: ");
  Serial.println(vacuumActiv ? "DA" : "NU");

  Serial.print("Obiect detectat: ");

  if (detectedObject == ALUMINIUM_CAN) {
    Serial.println("ALUMINIUM CAN");
  }
  else if (detectedObject == PLASTIC_BOTTLE) {
    Serial.println("PLASTIC BOTTLE");
  }
  else {
    Serial.println("NO OBJECT");
  }

  Serial.print("Actiune curenta: ");

  if (currentAction == ACTION_SORT_ALUMINIUM) {
    Serial.println("SORT ALUMINIUM");
  }
  else if (currentAction == ACTION_SORT_PLASTIC) {
    Serial.println("SORT PLASTIC");
  }
  else {
    Serial.println("NONE");
  }

  Serial.println("==============================================");
  Serial.println();
}

// ============================================================
// HELP COMENZI
// ============================================================

void helpComenzi() {
  Serial.println();
  Serial.println("=========== COMENZI MANUALE DIN CONSOLA ===========");

  Serial.println("B0 - B180       -> misca baza manual");
  Serial.println("J0 - J180       -> misca bratul de jos manual");
  Serial.println("U0 - U180       -> misca tija/bratul de sus manual");

  Serial.println();
  Serial.println("Exemple:");
  Serial.println("B90             -> baza la 90 grade");
  Serial.println("J120            -> bratul jos la 120 grade");
  Serial.println("U50             -> tija la 50 grade");

  Serial.println();
  Serial.println("=========== COMENZI VACUUM ===========");

  Serial.println("GRAB            -> prinde obiectul cu vacuum");
  Serial.println("DROP            -> elibereaza obiectul");
  Serial.println("TEST_VAC        -> testeaza pompa si solenoidul");

  Serial.println();
  Serial.println("=========== COMENZI AUTOMATE ===========");

  Serial.println("HOME            -> revine la pozitia initiala");
  Serial.println("AUTO_PICK       -> merge la obiect si il prinde");
  Serial.println("CAN             -> simuleaza detectarea unei doze de aluminiu");
  Serial.println("PLASTIC         -> simuleaza detectarea unei sticle de plastic");
  Serial.println("RUN_ACTION      -> executa actiunea definita pentru obiect");

  Serial.println();
  Serial.println("Exemplu automat:");
  Serial.println("CAN");
  Serial.println("RUN_ACTION");

  Serial.println();
  Serial.println("=========== COMENZI SISTEM ===========");

  Serial.println("S               -> afiseaza statusul sistemului");
  Serial.println("STOP            -> opreste pompa, solenoidul si servo-urile");
  Serial.println("HELP            -> afiseaza lista comenzilor");

  Serial.println("====================================================");
  Serial.println();
}

// ============================================================
// PROCESARE COMANDA
// ============================================================

void proceseazaComanda(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  if (cmd.length() == 0) {
    return;
  }

  // Comenzi sistem
  if (cmd == "HELP") {
    helpComenzi();
    return;
  }

  if (cmd == "S") {
    statusSistem();
    return;
  }

  if (cmd == "STOP") {
    stopTot();
    return;
  }

  // Comenzi vacuum
  if (cmd == "GRAB") {
    vacuumGrab();
    return;
  }

  if (cmd == "DROP") {
    vacuumDrop();
    return;
  }

  if (cmd == "TEST_VAC") {
    testVacuum();
    return;
  }

  // Comenzi automate
  if (cmd == "HOME") {
    mergiHome();
    return;
  }

  if (cmd == "AUTO_PICK") {
    autoPick();
    return;
  }

  // Detectie simulata obiecte
  if (cmd == "CAN") {
    detectedObject = ALUMINIUM_CAN;
    defineActionForObject(detectedObject);
    return;
  }

  if (cmd == "PLASTIC") {
    detectedObject = PLASTIC_BOTTLE;
    defineActionForObject(detectedObject);
    return;
  }

  // Executa actiunea definita de obiect
  if (cmd == "RUN_ACTION") {
    executeDefinedAction();
    return;
  }

  // Comenzi manuale servo din consola
  if (cmd.length() < 2) {
    Serial.println("ERR: Comanda invalida. Scrie HELP pentru lista comenzilor.");
    return;
  }

  char litera = cmd.charAt(0);
  int unghi = cmd.substring(1).toInt();

  if (unghi < UNGHI_MIN || unghi > UNGHI_MAX) {
    Serial.println("ERR: Unghi invalid. Foloseste valori intre 0 si 180.");
    return;
  }

  switch (litera) {
    case 'B':
      Serial.println("[MANUAL] Miscare baza din consola.");
      miscaServoDetach(servoBaza, PIN_BAZA, pozBaza, unghi, 1);
      break;

    case 'J':
      Serial.println("[MANUAL] Miscare brat jos din consola.");
      miscaServoDetach(servoJos, PIN_JOS, pozJos, unghi, 2);
      break;

    case 'U':
      Serial.println("[MANUAL] Miscare tija din consola.");
      miscaServoDetach(servoTija, PIN_TIJA, pozTija, unghi, 3);
      break;

    default:
      Serial.println("ERR: Comanda necunoscuta. Scrie HELP pentru lista comenzilor.");
      break;
  }
}

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(9600);

  delay(1000);

  Serial.println();
  Serial.println("==============================================");
  Serial.println(" BRAT ROBOTIZAT ESP32-S3 - SORTARE DESEURI");
  Serial.println(" Aluminium Can / Plastic Bottle");
  Serial.println("==============================================");

  // Initializare relee
  pinMode(pumpRelay, OUTPUT);
  pinMode(solenoidRelay, OUTPUT);

  pompaOFF();
  solenoidOFF();

  // Initializare timere PWM pentru ESP32Servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Pinii servo incep ca INPUT pentru a evita semnalul rezidual
  pinMode(PIN_BAZA, INPUT);
  pinMode(PIN_JOS, INPUT);
  pinMode(PIN_TIJA, INPUT);

  // Initializare servo baza
  servoBaza.setPeriodHertz(50);
  servoBaza.attach(PIN_BAZA, SERVO_MIN_US, SERVO_MAX_US);
  servoBaza.write(BAZA_HOME);
  delay(500);
  servoBaza.detach();
  pinMode(PIN_BAZA, INPUT);

  // Initializare servo brat jos
  servoJos.setPeriodHertz(50);
  servoJos.attach(PIN_JOS, SERVO_MIN_US, SERVO_MAX_US);
  servoJos.write(JOS_HOME);
  delay(500);
  servoJos.detach();
  pinMode(PIN_JOS, INPUT);

  // Initializare servo tija
  servoTija.setPeriodHertz(50);
  servoTija.attach(PIN_TIJA, SERVO_MIN_US, SERVO_MAX_US);
  servoTija.write(TIJA_HOME);
  delay(500);
  servoTija.detach();
  pinMode(PIN_TIJA, INPUT);

  servoActiv = 0;
  vacuumActiv = false;
  detectedObject = NO_OBJECT;
  currentAction = ACTION_NONE;

  Serial.println("[INIT] Servo system ready.");
  Serial.println("[INIT] Vacuum system ready.");
  Serial.println("[INIT] Console control ready.");

  helpComenzi();
}

// ============================================================
// LOOP
// ============================================================

void loop() {
  if (Serial.available() > 0) {
    String comanda = Serial.readStringUntil('\n');

    while (Serial.available() > 0) {
      Serial.read();
    }

    comanda.trim();

    if (comanda.length() > 0) {
      Serial.print("> ");
      Serial.println(comanda);

      proceseazaComanda(comanda);
    }
  }
}
