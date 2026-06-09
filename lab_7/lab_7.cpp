// Лабораторная работа №7
// Твининг, системы частиц, фейерверк (звёзды со следом)
// Объединённая программа: вальс фигур + мотыльки + фейерверк

#pragma comment(lib, "glut32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

using namespace std;

// ==================== РЕЖИМЫ РАБОТЫ ПРОГРАММЫ ====================
#define MODE_WALTZ 1        // Вальс геометрических фигур
#define MODE_MOTHS 2        // Мотыльки (система частиц)
#define MODE_FIREWORKS 3    // Фейерверк (звёзды со следом)
int currentMode = MODE_WALTZ;

// ==================== ПАРАМЕТРЫ ВАЛЬСА ====================
float angle = 0.0f;
float radiusWaltz = 0.7f;
float t = 0.0f;
float tDir = 0.005f;
bool paused = false;

// ==================== ПАРАМЕТРЫ МОТЫЛЬКОВ ====================
#define MAX_PARTICLES 150
struct Particle {
    float x, y;
    float vx, vy;
    float r, g, b;
    float size;
};
Particle particles[MAX_PARTICLES];

// ==================== ПАРАМЕТРЫ ФЕЙЕРВЕРКА (ЗВЁЗДЫ СО СЛЕДОМ) ====================
#define MAX_STARS 100
#define TRAIL_LENGTH 15

struct Star {
    float x, y;
    float vx, vy;
    float r, g, b;
    float size;
    vector<float> trailX;
    vector<float> trailY;
};
Star stars[MAX_STARS];

// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================
float randomRange(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

// ==================== ФУНКЦИИ ВАЛЬСА ====================
void getSquarePoint(float angleDeg, float& x, float& y) {
    float rad = angleDeg * 3.14159f / 180.0f;
    float cx = cos(rad);
    float cy = sin(rad);
    if (fabs(cx) > fabs(cy)) {
        x = (cx > 0 ? 1.0f : -1.0f);
        y = x * (cy / cx);
    }
    else {
        y = (cy > 0 ? 1.0f : -1.0f);
        x = y * (cx / cy);
    }
}

void getCirclePoint(float angleDeg, float& x, float& y) {
    float rad = angleDeg * 3.14159f / 180.0f;
    x = cos(rad);
    y = sin(rad);
}

void getStarPoint(float angleDeg, float& x, float& y) {
    float rad = angleDeg * 3.14159f / 180.0f;
    float cx = cos(rad);
    float cy = sin(rad);
    int spikes = 5;
    float spikeAngle = rad * spikes;
    float r = (3.0f + cos(spikeAngle)) / 4.0f;
    r = pow(r, 0.7f);
    x = cx * r;
    y = cy * r;
}

void drawMorphingShape(float centerX, float centerY, float size, float t_param) {
    int segments = 72;
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= segments; i++) {
        float angleDeg = i * 360.0f / segments;
        float x1, y1, x2, y2;
        float local_t;
        int shape1, shape2;
        if (t_param < 1.0f) {
            shape1 = 0; shape2 = 1;
            local_t = t_param;
        }
        else if (t_param < 2.0f) {
            shape1 = 1; shape2 = 2;
            local_t = t_param - 1.0f;
        }
        else if (t_param < 3.0f) {
            shape1 = 2; shape2 = 1;
            local_t = t_param - 2.0f;
        }
        else {
            shape1 = 1; shape2 = 0;
            local_t = t_param - 3.0f;
        }
        if (shape1 == 0) getSquarePoint(angleDeg, x1, y1);
        else if (shape1 == 1) getCirclePoint(angleDeg, x1, y1);
        else getStarPoint(angleDeg, x1, y1);
        if (shape2 == 0) getSquarePoint(angleDeg, x2, y2);
        else if (shape2 == 1) getCirclePoint(angleDeg, x2, y2);
        else getStarPoint(angleDeg, x2, y2);
        float dx = x1 * (1.0f - local_t) + x2 * local_t;
        float dy = y1 * (1.0f - local_t) + y2 * local_t;
        glVertex2f(centerX + dx * size, centerY + dy * size);
    }
    glEnd();
}

void drawWaltz() {
    float centerX = radiusWaltz * cos(angle);
    float centerY = radiusWaltz * sin(angle);
    if (t < 1.0f) {
        glColor3f(1.0f, 0.8f + t * 0.2f, 0.2f);
    }
    else if (t < 2.0f) {
        float lt = t - 1.0f;
        glColor3f(1.0f - lt * 0.5f, 1.0f, 0.2f + lt * 0.8f);
    }
    else if (t < 3.0f) {
        float lt = t - 2.0f;
        glColor3f(0.5f - lt * 0.3f, 1.0f - lt * 0.5f, 1.0f);
    }
    else {
        float lt = t - 3.0f;
        glColor3f(0.2f + lt * 0.8f, 0.5f - lt * 0.5f, 1.0f - lt * 0.8f);
    }
    drawMorphingShape(centerX, centerY, 0.25f, t);
}

void updateWaltz() {
    if (!paused) {
        angle += 0.02f;
        if (angle > 360.0f) angle -= 360.0f;
        t += tDir;
        if (t >= 4.0f) { t = 4.0f; tDir = -0.005f; }
        else if (t <= 0.0f) { t = 0.0f; tDir = 0.005f; }
    }
}

// ==================== ФУНКЦИИ МОТЫЛЬКОВ ====================
void initMoths() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].x = randomRange(-0.9f, 0.9f);
        particles[i].y = randomRange(-0.9f, 0.9f);
        particles[i].vx = randomRange(-0.008f, 0.008f);
        particles[i].vy = randomRange(-0.008f, 0.008f);
        particles[i].r = randomRange(0.5f, 1.0f);
        particles[i].g = randomRange(0.3f, 0.9f);
        particles[i].b = randomRange(0.2f, 0.8f);
        particles[i].size = randomRange(0.01f, 0.04f);
    }
}

void updateMoths() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].vx += randomRange(-0.002f, 0.002f);
        particles[i].vy += randomRange(-0.002f, 0.002f);
        if (particles[i].vx > 0.02f) particles[i].vx = 0.02f;
        if (particles[i].vx < -0.02f) particles[i].vx = -0.02f;
        if (particles[i].vy > 0.02f) particles[i].vy = 0.02f;
        if (particles[i].vy < -0.02f) particles[i].vy = -0.02f;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        if (particles[i].x > 0.95f) { particles[i].x = 0.95f; particles[i].vx = -particles[i].vx; }
        if (particles[i].x < -0.95f) { particles[i].x = -0.95f; particles[i].vx = -particles[i].vx; }
        if (particles[i].y > 0.95f) { particles[i].y = 0.95f; particles[i].vy = -particles[i].vy; }
        if (particles[i].y < -0.95f) { particles[i].y = -0.95f; particles[i].vy = -particles[i].vy; }
        particles[i].r += randomRange(-0.02f, 0.02f);
        particles[i].g += randomRange(-0.02f, 0.02f);
        particles[i].b += randomRange(-0.02f, 0.02f);
        if (particles[i].r > 1.0f) particles[i].r = 1.0f;
        if (particles[i].r < 0.2f) particles[i].r = 0.2f;
        if (particles[i].g > 1.0f) particles[i].g = 1.0f;
        if (particles[i].g < 0.2f) particles[i].g = 0.2f;
        if (particles[i].b > 1.0f) particles[i].b = 1.0f;
        if (particles[i].b < 0.2f) particles[i].b = 0.2f;
        particles[i].size += randomRange(-0.002f, 0.002f);
        if (particles[i].size > 0.045f) particles[i].size = 0.045f;
        if (particles[i].size < 0.008f) particles[i].size = 0.008f;
    }
}

void drawMoths() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        glColor3f(particles[i].r, particles[i].g, particles[i].b);
        glBegin(GL_TRIANGLE_FAN);
        for (int j = 0; j <= 12; j++) {
            float rad = j * 2 * 3.14159f / 12;
            float dx = cos(rad) * particles[i].size;
            float dy = sin(rad) * particles[i].size;
            glVertex2f(particles[i].x + dx, particles[i].y + dy);
        }
        glEnd();
    }
}

// ==================== ФУНКЦИИ ФЕЙЕРВЕРКА (исправленная версия) ====================
void initFireworks() {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x = randomRange(-0.9f, 0.9f);
        stars[i].y = randomRange(-0.9f, 0.9f);
        stars[i].vx = randomRange(-0.012f, 0.012f);
        stars[i].vy = randomRange(-0.012f, 0.012f);
        stars[i].r = randomRange(0.8f, 1.0f);
        stars[i].g = randomRange(0.5f, 1.0f);
        stars[i].b = randomRange(0.3f, 1.0f);
        stars[i].size = randomRange(0.02f, 0.04f);
        stars[i].trailX.clear();
        stars[i].trailY.clear();
        for (int j = 0; j < TRAIL_LENGTH; j++) {
            stars[i].trailX.push_back(stars[i].x);
            stars[i].trailY.push_back(stars[i].y);
        }
    }
}

void updateFireworks() {
    for (int i = 0; i < MAX_STARS; i++) {
        // сдвиг следа
        for (int j = TRAIL_LENGTH - 1; j > 0; j--) {
            stars[i].trailX[j] = stars[i].trailX[j - 1];
            stars[i].trailY[j] = stars[i].trailY[j - 1];
        }
        stars[i].trailX[0] = stars[i].x;
        stars[i].trailY[0] = stars[i].y;

        stars[i].vx += randomRange(-0.003f, 0.003f);
        stars[i].vy += randomRange(-0.003f, 0.003f);
        if (stars[i].vx > 0.025f) stars[i].vx = 0.025f;
        if (stars[i].vx < -0.025f) stars[i].vx = -0.025f;
        if (stars[i].vy > 0.025f) stars[i].vy = 0.025f;
        if (stars[i].vy < -0.025f) stars[i].vy = -0.025f;

        stars[i].x += stars[i].vx;
        stars[i].y += stars[i].vy;

        if (stars[i].x > 0.95f) { stars[i].x = 0.95f; stars[i].vx = -stars[i].vx; }
        if (stars[i].x < -0.95f) { stars[i].x = -0.95f; stars[i].vx = -stars[i].vx; }
        if (stars[i].y > 0.95f) { stars[i].y = 0.95f; stars[i].vy = -stars[i].vy; }
        if (stars[i].y < -0.95f) { stars[i].y = -0.95f; stars[i].vy = -stars[i].vy; }

        stars[i].r += randomRange(-0.03f, 0.03f);
        stars[i].g += randomRange(-0.03f, 0.03f);
        stars[i].b += randomRange(-0.03f, 0.03f);
        if (stars[i].r > 1.0f) stars[i].r = 1.0f;
        if (stars[i].r < 0.5f) stars[i].r = 0.5f;
        if (stars[i].g > 1.0f) stars[i].g = 1.0f;
        if (stars[i].g < 0.4f) stars[i].g = 0.4f;
        if (stars[i].b > 1.0f) stars[i].b = 1.0f;
        if (stars[i].b < 0.3f) stars[i].b = 0.3f;

        stars[i].size += randomRange(-0.002f, 0.002f);
        if (stars[i].size > 0.045f) stars[i].size = 0.045f;
        if (stars[i].size < 0.015f) stars[i].size = 0.015f;
    }
}

void drawStar(float x, float y, float size, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_LINES);
    // горизонталь
    glVertex2f(x - size, y);
    glVertex2f(x + size, y);
    // вертикаль
    glVertex2f(x, y - size);
    glVertex2f(x, y + size);
    // диагонали
    glVertex2f(x - size * 0.6f, y - size * 0.6f);
    glVertex2f(x + size * 0.6f, y + size * 0.6f);
    glVertex2f(x - size * 0.6f, y + size * 0.6f);
    glVertex2f(x + size * 0.6f, y - size * 0.6f);
    glEnd();
}

void drawFireworks() {
    // следы (тусклые точки)
    glPointSize(1.5f);
    for (int i = 0; i < MAX_STARS; i++) {
        glColor3f(stars[i].r * 0.4f, stars[i].g * 0.4f, stars[i].b * 0.4f);
        glBegin(GL_POINTS);
        for (int j = 1; j < TRAIL_LENGTH; j++) {
            glVertex2f(stars[i].trailX[j], stars[i].trailY[j]);
        }
        glEnd();
    }
    // сами звёзды
    for (int i = 0; i < MAX_STARS; i++) {
        drawStar(stars[i].x, stars[i].y, stars[i].size, stars[i].r, stars[i].g, stars[i].b);
    }
}

// ==================== ОСНОВНЫЕ ФУНКЦИИ ====================
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    if (currentMode == MODE_WALTZ) {
        drawWaltz();
    }
    else if (currentMode == MODE_MOTHS) {
        drawMoths();
    }
    else {
        drawFireworks();
    }

    glColor3f(0.7f, 0.7f, 0.7f);
    glRasterPos2f(-0.95f, 0.95f);
    const char* modeText = "";
    if (currentMode == MODE_WALTZ) modeText = "РЕЖИМ: ВАЛЬС (Квадрат->Круг->Звезда)";
    else if (currentMode == MODE_MOTHS) modeText = "РЕЖИМ: МОТЫЛЬКИ (Хаос)";
    else modeText = "РЕЖИМ: ФЕЙЕРВЕРК (Звёзды со следом)";
    for (const char* c = modeText; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

    glRasterPos2f(-0.95f, 0.88f);
    const char* controls = "1 - Вальс | 2 - Мотыльки | 3 - Фейерверк | Enter - Пауза (вальс) | ESC - Выход";
    for (const char* c = controls; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

    if (currentMode == MODE_WALTZ && paused) {
        glColor3f(1.0f, 1.0f, 0.0f);
        glRasterPos2f(-0.95f, 0.81f);
        const char* p = "ПАУЗА";
        for (const char* c = p; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    glFlush();
    glutPostRedisplay();
}

void update(int value) {
    if (currentMode == MODE_WALTZ) {
        updateWaltz();
    }
    else if (currentMode == MODE_MOTHS) {
        updateMoths();
    }
    else {
        updateFireworks();
    }
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '1': currentMode = MODE_WALTZ; break;
    case '2': currentMode = MODE_MOTHS; break;
    case '3': currentMode = MODE_FIREWORKS; break;
    case 13: if (currentMode == MODE_WALTZ) paused = !paused; break;
    case 27: exit(0); break;
    }
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
    srand((unsigned int)time(NULL));
    initMoths();
    initFireworks();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Лабораторная работа №7: Вальс + Мотыльки + Фейерверк");
    init();
    glutDisplayFunc(display);
    glutTimerFunc(16, update, 0);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}