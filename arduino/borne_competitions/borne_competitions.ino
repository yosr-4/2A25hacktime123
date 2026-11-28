/* =====================================================================
   HackTime - Scénario Arduino du module « Gestion des compétitions »
   « Borne d'affichage des compétitions »  (LCD 16x2 I2C + 2 boutons)

   - L'écran LCD affiche la compétition envoyée par l'application Qt
     (lue dans la base Oracle) :  ligne 1 = nom, ligne 2 = statut.
   - Bouton SUIVANTE : compétition suivante              -> envoie "N" à Qt
   - Bouton STATUT   : statut suivant, enregistré dans Oracle -> envoie "S" à Qt
       (PLANIFIEE -> EN COURS -> TERMINEE ; Qt fait l'UPDATE puis renvoie l'affichage)

   Message reçu de Qt (une ligne terminée par '\n') :
     C|ligne1|ligne2      afficher une compétition

   Câblage :
     LCD 16x2 I2C : SDA -> A4, SCL -> A5, VCC -> 5V, GND -> GND
     Bouton SUIVANTE : D2 <-> GND   (INPUT_PULLUP : pas de résistance externe)
     Bouton STATUT   : D3 <-> GND
   Bibliothèque : « LiquidCrystal I2C » (Frank de Brabander).
   ===================================================================== */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // si l'écran reste vide : essayer l'adresse 0x3F

const int BTN_SUIVANTE = 2;
const int BTN_STATUT = 3;

String recu;                          // ligne en cours de réception
int etatSuiv = HIGH, etatStatut = HIGH;
unsigned long dernierAppui = 0;

void afficher(const String &l1, const String &l2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(l1.substring(0, 16));
  lcd.setCursor(0, 1);
  lcd.print(l2.substring(0, 16));
}

// traite un message de Qt : "C|ligne1|ligne2"
void traiter(const String &ligne) {
  if (ligne.length() == 0 || ligne.charAt(0) != 'C') return;
  const int p1 = ligne.indexOf('|');
  const int p2 = ligne.indexOf('|', p1 + 1);
  if (p1 < 0) return;
  const String l1 = (p2 > 0) ? ligne.substring(p1 + 1, p2) : ligne.substring(p1 + 1);
  const String l2 = (p2 > 0) ? ligne.substring(p2 + 1) : "";
  afficher(l1, l2);
}

// bouton appuyé (passage de relâché à appuyé) -> envoie la commande à Qt
void lireBouton(int broche, int &ancien, const char *commande) {
  const int etat = digitalRead(broche);
  if (etat == LOW && ancien == HIGH && millis() - dernierAppui > 250) {   // anti-rebond
    Serial.println(commande);
    dernierAppui = millis();
  }
  ancien = etat;
}

void setup() {
  Serial.begin(9600);
  pinMode(BTN_SUIVANTE, INPUT_PULLUP);
  pinMode(BTN_STATUT, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  afficher("HACKTIME", "EN ATTENTE PC...");
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
