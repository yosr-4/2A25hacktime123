/* =====================================================================
   HackTime - Scénario Arduino du module « Gestion des compétitions »
   « Borne de suivi des compétitions »

   - L'écran LCD affiche la compétition envoyée par l'application Qt
     (lue dans la base Oracle) : nom + statut + jours restants.
   - Bouton SUIVANTE : demande la compétition suivante      -> envoie "N"
   - Bouton STATUT   : fait avancer le statut dans Oracle    -> envoie "S"
       (Planifiée -> En cours -> Terminée), l'application fait l'UPDATE
       puis renvoie l'affichage à jour.
   - Buzzer : bip de confirmation, et 3 bips quand une compétition
     commence aujourd'hui (alerte envoyée par l'application).

   Messages reçus de Qt (une ligne terminée par '\n') :
     C|ligne1|ligne2   afficher une compétition
     A|ligne1|ligne2   alerte : afficher + 3 bips
     B                 bip court (confirmation)

   Câblage :
     LCD 16x2 I2C : SDA -> A4, SCL -> A5, VCC -> 5V, GND -> GND
     Bouton SUIVANTE : D2 <-> GND   (résistance de tirage interne)
     Bouton STATUT   : D3 <-> GND
     Buzzer          : D8 (+) , GND (-)
   Bibliothèque : « LiquidCrystal I2C » (Frank de Brabander), via le gestionnaire de bibliothèques.
   ===================================================================== */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // si l'écran reste vide, essayer l'adresse 0x3F

const int BTN_SUIVANTE = 2;
const int BTN_STATUT = 3;
const int BUZZER = 8;

String recu;                 // ligne en cours de réception
int etatSuiv = HIGH, etatStatut = HIGH;
unsigned long dernierAppui = 0;

void bip(int duree) {
  tone(BUZZER, 1800, duree);
  delay(duree + 40);
}

void afficher(const String &l1, const String &l2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(l1.substring(0, 16));
  lcd.setCursor(0, 1);
  lcd.print(l2.substring(0, 16));
}

// traite une ligne reçue de l'application Qt
void traiter(const String &ligne) {
  if (ligne.length() == 0) return;
  const char type = ligne.charAt(0);
  if (type == 'B') { bip(80); return; }

  // découpe "X|ligne1|ligne2"
  const int p1 = ligne.indexOf('|');
  const int p2 = ligne.indexOf('|', p1 + 1);
  if (p1 < 0) return;
  const String l1 = (p2 > 0) ? ligne.substring(p1 + 1, p2) : ligne.substring(p1 + 1);
  const String l2 = (p2 > 0) ? ligne.substring(p2 + 1) : "";

  afficher(l1, l2);
  if (type == 'A') {
    for (int i = 0; i < 3; ++i) { bip(150); delay(120); }
  }
}

// envoie une commande quand un bouton passe de relâché à appuyé
void lireBouton(int broche, int &ancien, const char *commande) {
  const int etat = digitalRead(broche);
  if (etat == LOW && ancien == HIGH && millis() - dernierAppui > 250) {
    Serial.println(commande);
    dernierAppui = millis();
  }
  ancien = etat;
}

void setup() {
  Serial.begin(9600);
  pinMode(BTN_SUIVANTE, INPUT_PULLUP);
  pinMode(BTN_STATUT, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  lcd.init();
  lcd.backlight();
  afficher("HackTime", "En attente PC...");
}

void loop() {
  lireBouton(BTN_SUIVANTE, etatSuiv, "N");
  lireBouton(BTN_STATUT, etatStatut, "S");

  while (Serial.available()) {
    const char c = Serial.read();
    if (c == '\n') {
      recu.trim();
      traiter(recu);
      recu = "";
    } else if (recu.length() < 60) {
      recu += c;
    }
  }
}
