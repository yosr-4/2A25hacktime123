/* =====================================================================
   HackTime - Scénario Arduino du module « Gestion des participants »
   « Barrière d'accès / pointage des participants à l'entrée du hackathon »

   - Le POTENTIOMÈTRE (bouton rotatif) choisit le participant : sa position est
     envoyée à l'application Qt, qui sélectionne la ligne correspondante dans
     la liste des participants (lue dans Oracle).
   - Le bouton ENTRER demande le pointage : l'application interroge Oracle
     (le participant est-il « Validé » ? déjà pointé ?), fait l'UPDATE
     (date_pointage), puis renvoie la décision à la carte.
   - Le SERVOMOTEUR est la barrière : il s'ouvre 3 secondes si l'entrée est acceptée,
     reste fermé sinon. Le BUZZER confirme (1 bip long) ou refuse (3 bips graves).
   Aucune LED, aucun écran : la décision vient de la base de données.

   Messages Arduino -> Qt (une ligne terminée par '\n') :
     P|valeur     position du potentiomètre (0 à 1023), envoyée quand elle change
     V            bouton ENTRER appuyé
   Messages Qt -> Arduino :
     O            entrée acceptée   : barrière ouverte 3 s + 1 bip long
     R            entrée refusée    : barrière fermée + 3 bips graves
     D            déjà pointé       : barrière fermée + 2 bips courts

   Câblage :
     Potentiomètre : broche centrale -> A0, une extrémité -> 5V, l'autre -> GND
     Bouton ENTRER : D2 <-> GND  (résistance de tirage interne : INPUT_PULLUP)
     Servomoteur   : signal (orange/jaune) -> D9, rouge -> 5V, marron/noir -> GND
     Buzzer        : D8 (+) , GND (-)
   Bibliothèque : Servo (déjà fournie avec l'IDE Arduino).
   ===================================================================== */
#include <Servo.h>

const int POT = A0;
const int BOUTON = 2;
const int BUZZER = 8;
const int SERVO_PIN = 9;
const int ANGLE_FERME = 0;
const int ANGLE_OUVERT = 90;

Servo barriere;
String recu;
int dernierePosition = -100;
int etatBouton = HIGH;
unsigned long dernierEnvoi = 0, dernierAppui = 0;

void bip(int frequence, int duree, int pause) {
  tone(BUZZER, frequence, duree);
  delay(duree + pause);
}

void traiter(char commande) {
  if (commande == 'O') {                       // accepté : la barrière s'ouvre
    bip(1800, 400, 0);
    barriere.write(ANGLE_OUVERT);
    delay(3000);
    barriere.write(ANGLE_FERME);
  } else if (commande == 'R') {                // refusé : 3 bips graves
    for (int i = 0; i < 3; ++i) bip(300, 150, 100);
  } else if (commande == 'D') {                // déjà pointé : 2 bips courts
    for (int i = 0; i < 2; ++i) bip(900, 80, 80);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(BOUTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  barriere.attach(SERVO_PIN);
  barriere.write(ANGLE_FERME);
}

void loop() {
  // 1) potentiomètre : on envoie la position quand elle change (au plus toutes les 150 ms)
  const int position = analogRead(POT);
  if (abs(position - dernierePosition) > 12 && millis() - dernierEnvoi > 150) {
    Serial.print("P|");
    Serial.println(position);
    dernierePosition = position;
    dernierEnvoi = millis();
  }

  // 2) bouton ENTRER : une commande par appui
  const int etat = digitalRead(BOUTON);
  if (etat == LOW && etatBouton == HIGH && millis() - dernierAppui > 250) {
    Serial.println("V");
    dernierAppui = millis();
  }
  etatBouton = etat;

  // 3) réponse de l'application Qt (décision issue d'Oracle)
  while (Serial.available() > 0) {
    const char c = Serial.read();
    if (c == '\n') {
      if (recu.length() > 0) traiter(recu.charAt(0));
      recu = "";
    } else if (c != '\r') {
      recu += c;
    }
  }
}
