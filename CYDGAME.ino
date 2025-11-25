#include <TFT_eSPI.h>
#include <SPI.h>
#include <Bluepad32.h>

TFT_eSPI tft = TFT_eSPI();
ControllerPtr controller = nullptr;

// -------------------
// Jeu et état des flèches
// -------------------
enum Arrow { UP, DOWN, LEFT, RIGHT };
Arrow currentArrow;
int score = 0;
bool waitingInput = false;

// -------------------
// Flèches : affichage propre
// -------------------
void showArrow(Arrow a) {
  // Efface toute la zone des flèches (suffisamment large pour setTextSize(8))
  tft.fillRect(0, 0, 240, 160, TFT_BLACK);

  tft.setTextSize(8); // après l'effacement

  // Affiche seulement la flèche active
  if(a == UP)    { tft.setTextColor(TFT_GREEN, TFT_BLACK); tft.setCursor(140, 20); tft.print("^"); }
  if(a == DOWN)  { tft.setTextColor(TFT_YELLOW, TFT_BLACK); tft.setCursor(140, 100); tft.print("v"); }
  if(a == LEFT)  { tft.setTextColor(TFT_CYAN, TFT_BLACK); tft.setCursor(40, 60); tft.print("<"); }
  if(a == RIGHT) { tft.setTextColor(TFT_MAGENTA, TFT_BLACK); tft.setCursor(180, 60); tft.print(">"); }
}

// -------------------
// Génère une flèche aléatoire
// -------------------
Arrow nextArrow() {
  int r = random(0, 4);
  switch(r) {
    case 0: return UP;
    case 1: return DOWN;
    case 2: return LEFT;
    case 3: return RIGHT;
  }
  return UP;
}

// -------------------
// Callbacks Bluetooth
// -------------------
void onControllerConnected(ControllerPtr ctl) {
  controller = ctl;
  Serial.println("MANETTE CONNECTEE !");
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(30, 100);
  tft.println("GAME BY Wave");
  delay(3000);
  tft.setCursor(30, 100);
  tft.println("JOY-CON OK!");
  delay(1000);
  tft.fillScreen(TFT_BLACK);
}

void onControllerDisconnected(ControllerPtr ctl) {
  controller = nullptr;
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setCursor(30, 100);
  tft.println("Deconnectee");
}

// -------------------
// Setup
// -------------------
void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH); // backlight ON

  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);

  BP32.setup(&onControllerConnected, &onControllerDisconnected);
  BP32.forgetBluetoothKeys();

  // Lancer la première flèche
  currentArrow = nextArrow();
  showArrow(currentArrow);
  waitingInput = true;
  score = 0;
}

// -------------------
// Loop
// -------------------
void loop() {
  BP32.update();
  if (!controller || !controller->isConnected()) return;

  uint16_t buttons = controller->buttons();
  int axisX = controller->axisX();
  int axisY = controller->axisY();

  bool up    = buttons & 0x0001 || axisY < -300;
  bool down  = buttons & 0x0002 || axisY > 300;
  bool left  = buttons & 0x0004 || axisX < -300;
  bool right = buttons & 0x0008 || axisX > 300;

  if (waitingInput) {
    bool correct = false;

    switch(currentArrow) {
      case UP:    correct = up; break;
      case DOWN:  correct = down; break;
      case LEFT:  correct = left; break;
      case RIGHT: correct = right; break;
    }

    if (up || down || left || right) { // joueur a appuyé
      if (correct) {
        score++;
        Serial.printf("Score: %d\n", score);
        currentArrow = nextArrow();
        showArrow(currentArrow);
      } else {
        // GAME OVER
        tft.fillScreen(TFT_BLACK);
        tft.setTextSize(3);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setCursor(20, 100);
        tft.printf("GAME OVER\nScore: %d", score);
        delay(2000);
        // Recommencer
        score = 0;
        currentArrow = nextArrow();
        tft.fillScreen(TFT_BLACK);
        showArrow(currentArrow);
      }
      // attendre que joueur relâche pour compter une seule entrée
      waitingInput = false;
    }
  } else {
    // attendre que le joueur relâche tout
    if (!up && !down && !left && !right) waitingInput = true;
  }

  delay(20);
}
