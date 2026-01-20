#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace std;

// Settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;
const float PI = 3.14159265359f;

// --- Global Variables ---
float angleAll = 0.0f;
float moonAngle = 0.0f;
int currentFrame = 1;
int zoomPlanetIndex = -1;
bool isPaused = false;
float transitionFactor = 0.0f;
float currentCamX = 0, currentCamY = 0, currentZoom = 1.0f;
float starScroll = 0;
float meteorX = -1.0f, meteorY = 0.8f;
bool showMeteor = false;
float speedMultiplier = 1.0f;
bool showHelp = false;
float sunPulse = 0.0f;

// Per-planet rotation angles
float planetRotation[8] = {0};
float planetRotationSpeeds[] = {0.5f, 0.3f, 1.0f, 0.9f, 2.5f, 2.2f, 1.5f, 1.6f};

// Ring rotation angles
float ringAngle[8] = {0};

// Cloud layer angle for Earth
float cloudAngle = 0.0f;

// Eclipse mode
bool eclipseMode = false;

// Per-planet pause
bool planetPaused[8] = {false, false, false, false, false, false, false, false};

// Corona animation
float coronaAngle = 0.0f;

// Heatwave phase
float heatwavePhase = 0.0f;

// Aircraft and comet positions
float cometX = -1.5f;
float aircraftX = -1.2f;

// Enhanced Planet Data
string planetNames[] = {"MERCURY", "VENUS", "EARTH", "MARS", "JUPITER", "SATURN", "URANUS", "NEPTUNE"};
string planetFacts[] = {
    "Smallest planet, closest to Sun",
    "Hottest planet, rotates backwards",
    "Our home, has liquid water",
    "The Red Planet, has largest volcano",
    "Largest planet, Great Red Spot",
    "Famous for its rings",
    "Rotates on its side",
    "Strongest winds in solar system"
};
float distances[] = {0.22f, 0.32f, 0.42f, 0.54f, 0.70f, 0.82f, 0.92f, 1.02f};
float speeds[] = {2.0f, 1.6f, 1.2f, 0.9f, 0.5f, 0.4f, 0.3f, 0.2f};
float planetSizes[] = {0.012f, 0.022f, 0.028f, 0.018f, 0.055f, 0.048f, 0.035f, 0.033f};
float pColors[8][3] = {
    {0.75f, 0.75f, 0.75f}, {0.95f, 0.75f, 0.45f}, {0.2f, 0.5f, 1.0f}, {0.95f, 0.35f, 0.15f},
    {0.85f, 0.65f, 0.45f}, {0.95f, 0.85f, 0.55f}, {0.65f, 0.92f, 0.92f}, {0.35f, 0.45f, 0.92f}
};
int moonCounts[] = {0, 0, 1, 2, 4, 3, 2, 1};

// Environment Objects
struct Star { float x, y, brightness, twinkleSpeed; };
struct Asteroid { float angle, distance, size, speed; };
struct SpaceDust { float x, y, vx, vy, size, alpha; };
struct ShootingStar { float x, y, vx, vy, life, maxLife; bool active; };
struct Constellation { float x, y; };

vector<Star> stars;
vector<Asteroid> asteroids;
vector<SpaceDust> spaceDust;
vector<ShootingStar> shootingStars;

// Frame 4 shooting stars
float shootingStarTimer = 0;

// Pluto data
float plutoDistance = 1.15f;
float plutoSpeed = 0.1f;
float plutoSize = 0.008f;

// Ceres data (dwarf planet in asteroid belt)
float ceresAngle = 0.0f;
float ceresDistance = 0.62f;
float ceresSize = 0.006f;

// --- Helper Functions ---

void drawCircle(float x, float y, float r, int seg, bool line = false) {
    if (line) {
        glBegin(GL_LINE_LOOP);
    } else {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
    }
    for (int i = 0; i <= seg; i++) {
        float theta = 2.0f * PI * i / seg;
        glVertex2f(x + r * cos(theta), y + r * sin(theta));
    }
    glEnd();
}

void drawEllipse(float x, float y, float rx, float ry, int seg, bool line = false) {
    if (line) {
        glBegin(GL_LINE_LOOP);
    } else {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
    }
    for (int i = 0; i <= seg; i++) {
        float theta = 2.0f * PI * i / seg;
        glVertex2f(x + rx * cos(theta), y + ry * sin(theta));
    }
    glEnd();
}

void drawGlow(float x, float y, float r, float red, float green, float blue, float intensity = 0.3f) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    int layers = 15;
    for (int i = 0; i < layers; i++) {
        float ratio = (float)i / layers;
        float currentR = r * (1.0f + ratio * 1.5f);
        float alpha = (1.0f - ratio) * intensity;
        glColor4f(red, green, blue, alpha);
        drawCircle(x, y, currentR, 40);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Enhanced sun rays with rotation
void drawSunRays(float x, float y, float r, float rotationOffset) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    int numRays = 16;
    for (int i = 0; i < numRays; i++) {
        float angle = (2.0f * PI * i / numRays) + rotationOffset;
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.9f, 0.5f, 0.6f);
        glVertex2f(x + r * cos(angle - 0.05f), y + r * sin(angle - 0.05f));
        glVertex2f(x + r * cos(angle + 0.05f), y + r * sin(angle + 0.05f));
        glColor4f(1.0f, 0.7f, 0.0f, 0.0f);
        glVertex2f(x + r * 2.5f * cos(angle), y + r * 2.5f * sin(angle));
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Corona ring animation
void drawCorona(float x, float y, float r, float angle) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (int ring = 0; ring < 3; ring++) {
        float ringR = r * (1.2f + ring * 0.15f);
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= 60; i++) {
            float theta = 2.0f * PI * i / 60 + angle + ring * 0.5f;
            float wave = sin(theta * 5 + ring) * 0.02f;
            float alpha = 0.15f - ring * 0.04f;
            glColor4f(1.0f, 0.9f, 0.7f, alpha);
            glVertex2f(x + (ringR + wave) * cos(theta), y + (ringR + wave) * sin(theta));
            glVertex2f(x + (ringR + wave + 0.03f) * cos(theta), y + (ringR + wave + 0.03f) * sin(theta));
        }
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Heatwave effect around sun
void drawHeatwave(float x, float y, float baseR, float phase) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (int i = 0; i < 5; i++) {
        float offset = i * 0.8f;
        glBegin(GL_LINE_LOOP);
        for (int j = 0; j <= 60; j++) {
            float theta = 2.0f * PI * j / 60;
            float wave = sin(theta * 3 + phase + offset) * 0.015f;
            float r = baseR * (1.05f + i * 0.05f) + wave;
            float alpha = (0.15f - i * 0.02f) * (1.0f + 0.3f * sin(phase * 2 + i));
            glColor4f(1.0f, 0.8f, 0.4f, alpha);
            glVertex2f(x + r * cos(theta), y + r * sin(theta));
        }
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Animated sunspots
void drawSunspots(float x, float y, float r, float rotation) {
    glEnable(GL_BLEND);

    float spotData[][3] = {
        {0.3f, 0.4f, 0.12f},
        {-0.5f, -0.2f, 0.08f},
        {0.1f, -0.6f, 0.1f},
        {-0.3f, 0.5f, 0.06f}
    };

    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(rotation, 0, 0, 1);

    for (int i = 0; i < 4; i++) {
        float sx = r * spotData[i][0];
        float sy = r * spotData[i][1];
        float sr = r * spotData[i][2];
        
        glColor4f(0.3f, 0.2f, 0.0f, 0.7f);
        drawCircle(sx, sy, sr, 20);
        glColor4f(0.2f, 0.1f, 0.0f, 0.5f);
        drawCircle(sx, sy, sr * 0.6f, 15);
    }

    glPopMatrix();
}

// Solar flare animation
void drawSolarFlares(float x, float y, float r, float time) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    int numFlares = 3;
    for (int i = 0; i < numFlares; i++) {
        float angle = (2.0f * PI * i / numFlares) + time * 0.05f;
        float flareIntensity = 0.5f + 0.5f * sin(time * 0.3f + i * 2);
        float flareLength = r * (0.3f + 0.2f * flareIntensity);
        
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.9f, 0.7f, 0.4f * flareIntensity);
        glVertex2f(x + r * cos(angle - 0.1f), y + r * sin(angle - 0.1f));
        glVertex2f(x + r * cos(angle + 0.1f), y + r * sin(angle + 0.1f));
        glColor4f(1.0f, 0.5f, 0.0f, 0.0f);
        glVertex2f(x + (r + flareLength) * cos(angle), y + (r + flareLength) * sin(angle));
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void drawRing(float x, float y, float innerR, float outerR, int seg, float r, float g, float b, float a) {
    glEnable(GL_BLEND);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= seg; i++) {
        float theta = 2.0f * PI * i / seg;
        glColor4f(r, g, b, a);
        glVertex2f(x + innerR * cos(theta), y + innerR * sin(theta));
        glVertex2f(x + outerR * cos(theta), y + outerR * sin(theta));
    }
    glEnd();
}

// Animated ring with rotation
void drawAnimatedRing(float x, float y, float innerR, float outerR, int seg,
                      float r, float g, float b, float a, float rotation) {
    glEnable(GL_BLEND);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(rotation, 0, 0, 1);

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= seg; i++) {
        float theta = 2.0f * PI * i / seg;
        glColor4f(r, g, b, a * (0.8f + 0.2f * sin(theta * 5)));
        glVertex2f(innerR * cos(theta), innerR * sin(theta));
        glVertex2f(outerR * cos(theta), outerR * sin(theta));
    }
    glEnd();

    glPopMatrix();
}

void drawStars() {
    glEnable(GL_BLEND);
    for (auto& s : stars) {
        float twinkle = 0.5f + 0.5f * sin(angleAll * s.twinkleSpeed + s.x * 10);
        float brightness = s.brightness * twinkle;
        glColor4f(brightness, brightness, brightness * 1.1f, 1.0f);
        glPointSize(1.0f + brightness * 2.0f);
        glBegin(GL_POINTS);
        glVertex2f(s.x, s.y);
        glEnd();
    }
}

// Space dust effect
void drawSpaceDust() {
    glEnable(GL_BLEND);
    glPointSize(1.0f);

    for (auto& d : spaceDust) {
        glColor4f(0.7f, 0.7f, 0.8f, d.alpha);
        glBegin(GL_POINTS);
        glVertex2f(d.x, d.y);
        glEnd();
    }
}

void drawOrbit(float radius, int segments) {
    glEnable(GL_BLEND);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++) {
        float theta = 2.0f * PI * i / segments;
        glColor4f(0.3f, 0.3f, 0.4f, 0.3f);
        glVertex2f(radius * cos(theta), radius * sin(theta));
    }
    glEnd();
}

void drawAsteroidBelt() {
    glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
    for (auto& a : asteroids) {
        float x = a.distance * cos(a.angle + angleAll * a.speed);
        float y = a.distance * sin(a.angle + angleAll * a.speed);
        drawCircle(x, y, a.size, 8);
    }

    // Draw Ceres (dwarf planet)
    float cx = ceresDistance * cos(ceresAngle);
    float cy = ceresDistance * sin(ceresAngle);
    glColor4f(0.7f, 0.7f, 0.6f, 1.0f);
    drawCircle(cx, cy, ceresSize, 12);
    drawGlow(cx, cy, ceresSize, 0.6f, 0.6f, 0.5f, 0.15f);
}

// Day/Night mask for planets
void drawDayNightMask(float cx, float cy, float r, float angleDeg) {
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(angleDeg, 0, 0, 1);
    glEnable(GL_BLEND);
    glColor4f(0, 0, 0, 0.5f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 90; i <= 270; i++) {
        float rad = i * PI / 180.0f;
        glVertex2f(r * cos(rad), r * sin(rad));
    }
    glEnd();
    glPopMatrix();
}

// Cloud layer for Earth
void drawCloudLayer(float cx, float cy, float r, float angleDeg) {
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(angleDeg, 0, 0, 1);
    glEnable(GL_BLEND);
    glColor4f(1, 1, 1, 0.2f);

    drawEllipse(r * 0.3f, r * 0.4f, r * 0.4f, r * 0.15f, 20);
    drawEllipse(-r * 0.4f, -r * 0.2f, r * 0.35f, r * 0.12f, 18);
    drawEllipse(r * 0.1f, -r * 0.5f, r * 0.3f, r * 0.1f, 15);
    drawEllipse(-r * 0.2f, r * 0.3f, r * 0.25f, r * 0.1f, 15);

    glPopMatrix();
}

// Atmosphere glow for Earth
void drawAtmosphere(float cx, float cy, float r) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (int i = 0; i < 8; i++) {
        float ratio = (float)i / 8;
        float atmR = r * (1.0f + ratio * 0.3f);
        float alpha = (1.0f - ratio) * 0.15f;
        glColor4f(0.3f, 0.6f, 1.0f, alpha);
        drawCircle(cx, cy, atmR, 40);
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Jupiter's Great Red Spot
void drawJupiterSpot(float cx, float cy, float r, float rotation) {
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(rotation, 0, 0, 1);

    glEnable(GL_BLEND);
    glColor4f(0.8f, 0.3f, 0.2f, 0.7f);
    drawEllipse(r * 0.4f, -r * 0.2f, r * 0.25f, r * 0.15f, 20);

    glColor4f(0.9f, 0.4f, 0.3f, 0.5f);
    drawEllipse(r * 0.4f, -r * 0.2f, r * 0.15f, r * 0.08f, 15);

    glColor4f(0.7f, 0.5f, 0.3f, 0.3f);
    for (float by = -0.6f; by <= 0.6f; by += 0.3f) {
        glBegin(GL_LINES);
        glVertex2f(-r, by * r);
        glVertex2f(r, by * r + sin(angleAll * 0.1f + by) * 0.05f);
        glEnd();
    }

    glPopMatrix();
}

// Uranus tilt visualization
void drawUranusTilt(float cx, float cy, float r, float rotation) {
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(98, 0, 0, 1);

    glColor3f(0.65f, 0.92f, 0.92f);
    drawCircle(0, 0, r, 35);

    glColor4f(1, 1, 1, 0.3f);
    glBegin(GL_LINES);
    glVertex2f(0, -r * 1.3f);
    glVertex2f(0, r * 1.3f);
    glEnd();

    glPopMatrix();
}

// Shadow effect for planets
void drawPlanetShadow(float cx, float cy, float r, float sunAngle) {
    glEnable(GL_BLEND);
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(sunAngle, 0, 0, 1);

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0, 0, 0, 0.0f);
    glVertex2f(0, 0);
    glColor4f(0, 0, 0, 0.4f);
    for (int i = 60; i <= 300; i++) {
        float rad = i * PI / 180.0f;
        glVertex2f(r * 1.2f * cos(rad), r * 1.2f * sin(rad));
    }
    glEnd();

    glPopMatrix();
}

// Eclipse shadow on moon
void drawEclipseShadow(float moonX, float moonY, float moonR, float earthX, float earthY) {
    if (!eclipseMode) return;

    float dx = moonX - earthX;
    float dy = moonY - earthY;
    float dist = sqrt(dx * dx + dy * dy);

    if (dist < 0.15f) {
        glEnable(GL_BLEND);
        glColor4f(0, 0, 0, 0.6f);
        drawCircle(moonX, moonY, moonR * 0.8f, 25);
    }
}

void drawPlanetWithMoons(int index, float px, float py, bool isZoomed = false) {
    float sunAngle = atan2(py, px) * 180.0f / PI + 180;

    drawGlow(px, py, planetSizes[index], pColors[index][0], pColors[index][1], pColors[index][2], 0.25f);

    if (index == 2) {
        // Earth special rendering
        drawAtmosphere(px, py, planetSizes[index]);
        
        glColor3f(pColors[index][0], pColors[index][1], pColors[index][2]);
        drawCircle(px, py, planetSizes[index], 35);
        
        // Simplified continents
        glPushMatrix();
        glTranslatef(px, py, 0);
        glRotatef(planetRotation[index], 0, 0, 1);
        glColor4f(0.2f, 0.6f, 0.2f, 0.8f);
        drawEllipse(planetSizes[index] * 0.3f, planetSizes[index] * 0.2f, 
                   planetSizes[index] * 0.3f, planetSizes[index] * 0.2f, 12);
        glPopMatrix();
        
        drawCloudLayer(px, py, planetSizes[index], cloudAngle);
        drawDayNightMask(px, py, planetSizes[index], sunAngle);
    } else {
        glColor3f(pColors[index][0], pColors[index][1], pColors[index][2]);
        drawCircle(px, py, planetSizes[index], 35);
        
        if (index == 3) {
            // Mars polar caps
            glPushMatrix();
            glTranslatef(px, py, 0);
            glRotatef(planetRotation[index], 0, 0, 1);
            glColor4f(1, 1, 1, 0.7f);
            drawCircle(0, planetSizes[index] * 0.8f, planetSizes[index] * 0.15f, 12);
            drawCircle(0, -planetSizes[index] * 0.8f, planetSizes[index] * 0.12f, 12);
            glPopMatrix();
        }
        
        if (index == 4) drawJupiterSpot(px, py, planetSizes[index], planetRotation[index]);
        if (index == 6) drawUranusTilt(px, py, planetSizes[index], planetRotation[index]);
        
        drawPlanetShadow(px, py, planetSizes[index], sunAngle);
    }

    // Saturn's animated rings
    if (index == 5) {
        drawAnimatedRing(px, py, planetSizes[index] * 1.3f, planetSizes[index] * 2.0f, 80,
                        0.9f, 0.8f, 0.6f, 0.7f, ringAngle[index]);
        drawAnimatedRing(px, py, planetSizes[index] * 2.05f, planetSizes[index] * 2.3f, 80,
                        0.8f, 0.7f, 0.5f, 0.5f, ringAngle[index] * 0.8f);
    }

    // Uranus's tilted ring
    if (index == 6) {
        glPushMatrix();
        glTranslatef(px, py, 0);
        glRotatef(98, 0, 0, 1);
        drawRing(0, 0, planetSizes[index] * 1.2f, planetSizes[index] * 1.5f, 60,
                0.7f, 0.9f, 0.9f, 0.4f);
        glPopMatrix();
    }

    // Draw moons
    for (int m = 0; m < moonCounts[index]; m++) {
        float moonOrbit = planetSizes[index] * (2.0f + m * 0.8f);
        float moonAngleCalc = angleAll * (2.0f - m * 0.3f) + m * 60;
        float moonRad = moonAngleCalc * PI / 180.0f;
        float mx = px + moonOrbit * cos(moonRad);
        float my = py + moonOrbit * sin(moonRad);
        float moonSize = planetSizes[index] * (0.15f + m * 0.05f);
        
        if (isZoomed) {
            glColor4f(0.4f, 0.4f, 0.5f, 0.2f);
            drawCircle(px, py, moonOrbit, 50, true);
        }
        
        glColor4f(0.7f, 0.7f, 0.75f, 1.0f);
        drawCircle(mx, my, moonSize, 15);
        
        glColor4f(0.5f, 0.5f, 0.55f, 0.6f);
        drawCircle(mx + moonSize * 0.2f, my + moonSize * 0.1f, moonSize * 0.2f, 8);
    }
}

void drawComet(float x, float y) {
    glEnable(GL_BLEND);

    // Ion tail (blue)
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.5f, 0.7f, 1.0f, 0.8f);
    glVertex2f(x, y);
    glColor4f(0.2f, 0.4f, 1.0f, 0.0f);
    glVertex2f(x - 0.3f, y + 0.02f);
    glVertex2f(x - 0.35f, y);
    glVertex2f(x - 0.3f, y - 0.02f);
    glEnd();

    // Dust tail (yellow/white)
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 0.9f, 0.9f);
    glVertex2f(x, y);
    glColor4f(1.0f, 0.8f, 0.5f, 0.0f);
    glVertex2f(x - 0.2f, y + 0.04f);
    glVertex2f(x - 0.25f, y + 0.02f);
    glVertex2f(x - 0.2f, y - 0.01f);
    glEnd();

    drawGlow(x, y, 0.02f, 0.9f, 0.95f, 1.0f, 0.5f);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(x, y, 0.012f, 15);
}

void drawPluto(float angle) {
    float rad = angle * PI / 180.0f;
    float px = plutoDistance * cos(rad);
    float py = plutoDistance * sin(rad);

    glEnable(GL_BLEND);
    glColor4f(0.4f, 0.3f, 0.3f, 0.3f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 60; i++) {
        float theta = 2.0f * PI * i / 60;
        glVertex2f(plutoDistance * cos(theta), plutoDistance * sin(theta));
    }
    glEnd();

    drawGlow(px, py, plutoSize, 0.8f, 0.7f, 0.6f, 0.2f);
    glColor3f(0.85f, 0.75f, 0.65f);
    drawCircle(px, py, plutoSize, 12);

    float charonAngle = angle * 3;
    float charonRad = charonAngle * PI / 180.0f;
    float cx = px + plutoSize * 2.5f * cos(charonRad);
    float cy = py + plutoSize * 2.5f * sin(charonRad);
    glColor3f(0.6f, 0.6f, 0.65f);
    drawCircle(cx, cy, plutoSize * 0.5f, 8);
}

void drawShootingStar() {
    if (!showMeteor) return;

    glEnable(GL_BLEND);
    glBegin(GL_TRIANGLES);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glVertex2f(meteorX, meteorY);
    glColor4f(0.5f, 0.7f, 1.0f, 0.0f);
    glVertex2f(meteorX - 0.15f, meteorY + 0.05f);
    glVertex2f(meteorX - 0.12f, meteorY + 0.08f);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    drawCircle(meteorX, meteorY, 0.005f, 8);
}

void drawText(const char* text, float x, float y) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *text);
        text++;
    }
}

void drawHUD() {
    glEnable(GL_BLEND);

    // Top bar
    glColor4f(0.0f, 0.1f, 0.2f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, 0.92f);
    glVertex2f(1.0f, 0.92f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    // Frame indicators (1-4)
    for (int i = 1; i <= 4; i++) {
        float fx = -0.9f + (i - 1) * 0.12f;
        if (i == currentFrame) {
            glColor4f(0.0f, 0.8f, 1.0f, 0.9f);
            glBegin(GL_QUADS);
            glVertex2f(fx - 0.04f, 0.94f);
            glVertex2f(fx + 0.04f, 0.94f);
            glVertex2f(fx + 0.04f, 0.98f);
            glVertex2f(fx - 0.04f, 0.98f);
            glEnd();
        } else {
            glColor4f(0.3f, 0.4f, 0.5f, 0.5f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(fx - 0.04f, 0.94f);
            glVertex2f(fx + 0.04f, 0.94f);
            glVertex2f(fx + 0.04f, 0.98f);
            glVertex2f(fx - 0.04f, 0.98f);
            glEnd();
        }
    }

    // Speed indicator
    glColor4f(0.0f, 0.8f, 0.5f, 0.8f);
    float speedBarWidth = 0.12f * speedMultiplier / 2.0f;
    glBegin(GL_QUADS);
    glVertex2f(0.6f, 0.94f);
    glVertex2f(0.6f + speedBarWidth, 0.94f);
    glVertex2f(0.6f + speedBarWidth, 0.98f);
    glVertex2f(0.6f, 0.98f);
    glEnd();

    glColor4f(0.2f, 0.3f, 0.4f, 0.6f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(0.58f, 0.93f);
    glVertex2f(0.8f, 0.93f);
    glVertex2f(0.8f, 0.99f);
    glVertex2f(0.58f, 0.99f);
    glEnd();

    // Eclipse mode indicator
    if (eclipseMode) {
        glColor4f(1.0f, 0.8f, 0.0f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(0.82f, 0.94f);
        glVertex2f(0.90f, 0.94f);
        glVertex2f(0.90f, 0.98f);
        glVertex2f(0.82f, 0.98f);
        glEnd();
    }

    // Pause indicator
    if (isPaused) {
        glColor4f(1.0f, 1.0f, 0.0f, 0.8f);
        // Left bar
        glBegin(GL_QUADS);
        glVertex2f(-0.03f, -0.08f);
        glVertex2f(-0.01f, -0.08f);
        glVertex2f(-0.01f, 0.08f);
        glVertex2f(-0.03f, 0.08f);
        glEnd();
        // Right bar
        glBegin(GL_QUADS);
        glVertex2f(0.01f, -0.08f);
        glVertex2f(0.03f, -0.08f);
        glVertex2f(0.03f, 0.08f);
        glVertex2f(0.01f, 0.08f);
        glEnd();
    }
}

void drawHelpOverlay() {
    if (!showHelp) return;

    glEnable(GL_BLEND);
    glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(-0.75f, -0.85f);
    glVertex2f(0.75f, -0.85f);
    glVertex2f(0.75f, 0.85f);
    glVertex2f(-0.75f, 0.85f);
    glEnd();

    glColor4f(0.0f, 0.8f, 1.0f, 0.8f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.75f, -0.85f);
    glVertex2f(0.75f, -0.85f);
    glVertex2f(0.75f, 0.85f);
    glVertex2f(-0.75f, 0.85f);
    glEnd();

    // Title bar
    glColor4f(0.0f, 0.4f, 0.6f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(-0.73f, 0.73f);
    glVertex2f(0.73f, 0.73f);
    glVertex2f(0.73f, 0.83f);
    glVertex2f(-0.73f, 0.83f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText("SOLAR SYSTEM EXPLORER - CONTROLS", -0.35f, 0.76f);
    
    // Draw controls
    drawText("1-4: Switch Frames", -0.65f, 0.6f);
    drawText("P: Pause/Resume", -0.65f, 0.48f);
    drawText("H: Toggle Help", -0.65f, 0.36f);
    drawText("+/-: Speed Control", -0.65f, 0.24f);
    drawText("E: Eclipse Mode", -0.65f, 0.12f);
    drawText("0-7: Pause Planets", -0.65f, 0.0f);
    drawText("Z: Zoom Planet", -0.65f, -0.12f);
    drawText("ESC: Exit", -0.65f, -0.24f);
}

// --- FRAME 1: FULL SOLAR SYSTEM ---
void drawFrame1() {
    drawStars();
    drawSpaceDust();
    drawShootingStar();

    // Comet
    drawComet(cometX, 0.75f + 0.05f * sin(cometX * 3));

    // Sun with detailed effects
    float sunRadius = 0.12f + 0.008f * sin(sunPulse);

    drawHeatwave(0, 0, sunRadius, heatwavePhase);
    drawCorona(0, 0, sunRadius, coronaAngle);
    drawSolarFlares(0, 0, sunRadius, angleAll);
    drawSunRays(0, 0, sunRadius * 1.3f, angleAll * 0.01f);

    drawGlow(0, 0, sunRadius, 1.0f, 0.7f, 0.0f, 0.5f);
    drawGlow(0, 0, sunRadius * 0.8f, 1.0f, 0.9f, 0.3f, 0.4f);

    glColor3f(1.0f, 0.95f, 0.4f);
    drawCircle(0, 0, sunRadius, 60);

    // Light emission pulse
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    float emissionPulse = 0.1f + 0.05f * sin(sunPulse * 2);
    glColor4f(1.0f, 0.9f, 0.5f, emissionPulse);
    drawCircle(0, 0, sunRadius * 1.8f, 50);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawSunspots(0, 0, sunRadius, planetRotation[0]);

    // Asteroid Belt
    drawAsteroidBelt();

    // All 8 Planets
    for (int i = 0; i < 8; i++) {
        if (planetPaused[i]) continue;
        
        // Orbit path
        glColor4f(0.25f, 0.3f, 0.35f, 0.25f);
        drawOrbit(distances[i], 100);
        
        // Planet position
        float angle = angleAll * speeds[i];
        float rad = angle * PI / 180.0f;
        float px = distances[i] * cos(rad);
        float py = distances[i] * sin(rad);
        
        drawPlanetWithMoons(i, px, py, false);
        
        // Planet name label (small)
        if (currentFrame == 1) {
            glColor3f(0.8f, 0.9f, 1.0f);
            drawText(planetNames[i].c_str(), px - 0.05f, py + planetSizes[i] + 0.03f);
        }
    }

    // Pluto
    drawPluto(angleAll * plutoSpeed);

    drawHUD();
}

// --- FRAME 2: PLANET DETAILS & FEATURES ---
void drawFrame2() {
    if (zoomPlanetIndex == -1) {
        drawStars();
        
        // Prompt text
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText("SELECT A PLANET (0-7) TO ZOOM IN", -0.4f, 0.85f);
        drawText("Press Z to return to normal view", -0.35f, 0.75f);
        
        // Display all planets in grid
        float startX = -0.8f;
        float startY = 0.5f;
        float spacing = 0.4f;
        
        for (int i = 0; i < 8; i++) {
            float px = startX + (i % 4) * spacing;
            float py = startY - (i / 4) * spacing;
            
            drawPlanetWithMoons(i, px, py, false);
            
            glColor3f(0.9f, 0.9f, 1.0f);
            drawText(planetNames[i].c_str(), px - 0.08f, py + 0.15f);
            
            char keyText[10];
            sprintf(keyText, "[%d]", i);
            drawText(keyText, px - 0.03f, py - 0.15f);
        }
    } else {
        drawStars();
        
        float angle = angleAll * speeds[zoomPlanetIndex];
        float rad = angle * PI / 180.0f;
        float px = 0.0f;
        float py = 0.0f;
        
        float zoomSize = planetSizes[zoomPlanetIndex] * 5.0f;
        
        drawPlanetWithMoons(zoomPlanetIndex, px, py, true);
        
        // Planet info
        glColor3f(1.0f, 1.0f, 0.5f);
        drawText(planetNames[zoomPlanetIndex].c_str(), -0.8f, 0.85f);
        glColor3f(0.8f, 0.9f, 1.0f);
        drawText(planetFacts[zoomPlanetIndex].c_str(), -0.8f, 0.75f);
        
        char info[100];
        sprintf(info, "Distance from Sun: %.2f AU", distances[zoomPlanetIndex]);
        drawText(info, -0.8f, 0.65f);
        sprintf(info, "Moons: %d", moonCounts[zoomPlanetIndex]);
        drawText(info, -0.8f, 0.55f);
        
        glColor3f(0.5f, 0.8f, 0.5f);
        drawText("Press Z to exit zoom mode", -0.3f, -0.85f);
    }

    drawHUD();
}

// --- FRAME 3: EARTH & MOON SYSTEM ---
void drawFrame3() {
    // Enhanced starfield
    glEnable(GL_BLEND);
    for (auto& s : stars) {
        float twinkle = 0.5f + 0.5f * sin(angleAll * s.twinkleSpeed * 2.0f + s.x * 10);
        float brightness = s.brightness * twinkle;
        float r = brightness * (0.9f + 0.1f * sin(s.x * 100));
        float g = brightness * (0.85f + 0.15f * sin(s.y * 80));
        float b = brightness * (1.0f + 0.1f * cos(s.x * 50));
        glColor4f(r, g, b, 1.0f);
        glPointSize(1.0f + brightness * 1.5f);
        glBegin(GL_POINTS);
        glVertex2f(s.x, s.y);
        glEnd();
    }

    // Distant nebula
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int i = 0; i < 5; i++) {
        float nx = -0.3f + i * 0.15f;
        float ny = 0.7f + sin(i * 1.0f) * 0.1f;
        float nr = 0.08f + i * 0.02f;
        glColor4f(0.3f + i * 0.1f, 0.1f, 0.4f + i * 0.05f, 0.05f);
        drawCircle(nx, ny, nr, 20);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Background sun
    float sunBgX = -0.75f, sunBgY = 0.65f;
    drawGlow(sunBgX, sunBgY, 0.15f, 1.0f, 0.8f, 0.3f, 0.5f);
    glColor3f(1.0f, 0.95f, 0.5f);
    drawCircle(sunBgX, sunBgY, 0.06f, 35);

    // Lens flare
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int i = 1; i <= 5; i++) {
        float flareX = sunBgX + (0 - sunBgX) * i * 0.2f;
        float flareY = sunBgY + (0 - sunBgY) * i * 0.2f;
        float flareSize = 0.02f + (i % 3) * 0.01f;
        float alpha = 0.15f / i;
        glColor4f(1.0f, 0.7f + i * 0.05f, 0.3f, alpha);
        drawCircle(flareX, flareY, flareSize, 15);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Sun rays
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int i = 0; i < 12; i++) {
        float angle = i * PI / 6 + angleAll * 0.008f;
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.9f, 0.5f, 0.35f);
        glVertex2f(sunBgX + 0.06f * cos(angle - 0.04f), sunBgY + 0.06f * sin(angle - 0.04f));
        glVertex2f(sunBgX + 0.06f * cos(angle + 0.04f), sunBgY + 0.06f * sin(angle + 0.04f));
        glColor4f(1.0f, 0.7f, 0.2f, 0.0f);
        glVertex2f(sunBgX + 0.22f * cos(angle), sunBgY + 0.22f * sin(angle));
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Large Earth
    float earthX = 0.0f, earthY = -0.1f;
    float earthRadius = 0.28f;

    // Atmosphere glow
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int i = 0; i < 12; i++) {
        float ratio = (float)i / 12;
        float ar = earthRadius * (1.08f + ratio * 0.25f);
        float alpha = (1.0f - ratio) * 0.12f;
        glColor4f(0.2f, 0.5f, 1.0f, alpha);
        drawCircle(earthX, earthY, ar, 50);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Earth body
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.1f, 0.3f, 0.7f);
    glVertex2f(earthX, earthY);
    for (int i = 0; i <= 60; i++) {
        float theta = 2.0f * PI * i / 60;
        glColor3f(0.15f + 0.07f * sin(theta), 0.35f + 0.06f * cos(theta), 0.85f);
        glVertex2f(earthX + earthRadius * cos(theta), earthY + earthRadius * sin(theta));
    }
    glEnd();

    // Oceans
    glEnable(GL_BLEND);
    glColor4f(0.05f, 0.2f, 0.5f, 0.45f);
    glPushMatrix();
    glTranslatef(earthX, earthY, 0);
    glRotatef(planetRotation[2] * 0.22f, 0, 0, 1);
    drawEllipse(-earthRadius * 0.5f, 0, earthRadius * 0.3f, earthRadius * 0.4f, 20);
    drawEllipse(earthRadius * 0.3f, earthRadius * 0.2f, earthRadius * 0.25f, earthRadius * 0.3f, 18);
    glPopMatrix();

    // Static continents (no rotation for stability)
    glPushMatrix();
    glTranslatef(earthX, earthY, 0);
    
    // North America
    glColor4f(0.2f, 0.55f, 0.25f, 0.96f);
    glBegin(GL_POLYGON);
    glVertex2f(-earthRadius * 0.55f, earthRadius * 0.45f);
    glVertex2f(-earthRadius * 0.4f, earthRadius * 0.55f);
    glVertex2f(-earthRadius * 0.25f, earthRadius * 0.5f);
    glVertex2f(-earthRadius * 0.2f, earthRadius * 0.35f);
    glVertex2f(-earthRadius * 0.35f, earthRadius * 0.25f);
    glVertex2f(-earthRadius * 0.5f, earthRadius * 0.15f);
    glVertex2f(-earthRadius * 0.6f, earthRadius * 0.25f);
    glEnd();
    
    // South America
    glColor4f(0.18f, 0.5f, 0.22f, 0.96f);
    glBegin(GL_POLYGON);
    glVertex2f(-earthRadius * 0.38f, -earthRadius * 0.05f);
    glVertex2f(-earthRadius * 0.28f, -earthRadius * 0.1f);
    glVertex2f(-earthRadius * 0.22f, -earthRadius * 0.25f);
    glVertex2f(-earthRadius * 0.28f, -earthRadius * 0.5f);
    glVertex2f(-earthRadius * 0.38f, -earthRadius * 0.55f);
    glVertex2f(-earthRadius * 0.45f, -earthRadius * 0.35f);
    glVertex2f(-earthRadius * 0.42f, -earthRadius * 0.15f);
    glEnd();
    
    // Africa
    glColor4f(0.25f, 0.55f, 0.2f, 0.96f);
    glBegin(GL_POLYGON);
    glVertex2f(earthRadius * 0.0f, earthRadius * 0.25f);
    glVertex2f(earthRadius * 0.15f, earthRadius * 0.2f);
    glVertex2f(earthRadius * 0.2f, earthRadius * 0.0f);
    glVertex2f(earthRadius * 0.15f, -earthRadius * 0.25f);
    glVertex2f(earthRadius * 0.05f, -earthRadius * 0.4f);
    glVertex2f(-earthRadius * 0.05f, -earthRadius * 0.3f);
    glVertex2f(-earthRadius * 0.08f, -earthRadius * 0.1f);
    glVertex2f(-earthRadius * 0.05f, earthRadius * 0.15f);
    glEnd();
    
    // Europe
    glColor4f(0.22f, 0.52f, 0.25f, 0.96f);
    glBegin(GL_POLYGON);
    glVertex2f(-earthRadius * 0.05f, earthRadius * 0.5f);
    glVertex2f(earthRadius * 0.1f, earthRadius * 0.55f);
    glVertex2f(earthRadius * 0.2f, earthRadius * 0.45f);
    glVertex2f(earthRadius * 0.15f, earthRadius * 0.35f);
    glVertex2f(earthRadius * 0.0f, earthRadius * 0.3f);
    glVertex2f(-earthRadius * 0.08f, earthRadius * 0.4f);
    glEnd();
    
    // Asia
    glColor4f(0.2f, 0.5f, 0.23f, 0.96f);
    glBegin(GL_POLYGON);
    glVertex2f(earthRadius * 0.2f, earthRadius * 0.55f);
    glVertex2f(earthRadius * 0.5f, earthRadius * 0.5f);
    glVertex2f(earthRadius * 0.65f, earthRadius * 0.35f);
    glVertex2f(earthRadius * 0.6f, earthRadius * 0.15f);
    glVertex2f(earthRadius * 0.45f, earthRadius * 0.05f);
    glVertex2f(earthRadius * 0.3f, earthRadius * 0.1f);
    glVertex2f(earthRadius * 0.2f, earthRadius * 0.25f);
    glVertex2f(earthRadius * 0.15f, earthRadius * 0.4f);
    glEnd();
    
    // Australia
    glColor4f(0.35f, 0.5f, 0.2f, 0.96f);
    glBegin(GL_POLYGON);
    glVertex2f(earthRadius * 0.5f, -earthRadius * 0.25f);
    glVertex2f(earthRadius * 0.65f, -earthRadius * 0.3f);
    glVertex2f(earthRadius * 0.68f, -earthRadius * 0.45f);
    glVertex2f(earthRadius * 0.55f, -earthRadius * 0.5f);
    glVertex2f(earthRadius * 0.45f, -earthRadius * 0.4f);
    glEnd();
    
    // Greenland
    glColor4f(0.85f, 0.9f, 0.95f, 0.95f);
    glBegin(GL_POLYGON);
    glVertex2f(-earthRadius * 0.15f, earthRadius * 0.65f);
    glVertex2f(-earthRadius * 0.05f, earthRadius * 0.7f);
    glVertex2f(earthRadius * 0.0f, earthRadius * 0.6f);
    glVertex2f(-earthRadius * 0.08f, earthRadius * 0.52f);
    glVertex2f(-earthRadius * 0.18f, earthRadius * 0.55f);
    glEnd();

    glPopMatrix();
    
    // Stable ice caps
    glColor4f(0.96f, 0.98f, 1.0f, 0.95f);
    drawCircle(earthX, earthY + earthRadius * 0.72f, earthRadius * 0.18f, 40);
    glColor4f(0.05f, 0.18f, 0.45f, 0.9f);
    drawCircle(earthX + earthRadius * 0.06f, earthY + earthRadius * 0.72f - earthRadius * 0.02f, earthRadius * 0.16f, 40);

    glColor4f(0.92f, 0.95f, 1.0f, 0.95f);
    drawCircle(earthX, earthY - earthRadius * 0.7f, earthRadius * 0.22f, 40);
    glColor4f(0.05f, 0.18f, 0.45f, 0.9f);
    drawCircle(earthX - earthRadius * 0.05f, earthY - earthRadius * 0.68f + earthRadius * 0.02f, earthRadius * 0.18f, 40);

    // Clouds
    glColor4f(1.0f, 1.0f, 1.0f, 0.18f);
    glPushMatrix();
    glTranslatef(earthX, earthY, 0);
    glRotatef(cloudAngle * 1.3f, 0, 0, 1);
    drawEllipse(earthRadius * 0.22f, earthRadius * 0.42f, earthRadius * 0.38f, earthRadius * 0.12f, 22);
    drawEllipse(-earthRadius * 0.35f, earthRadius * 0.12f, earthRadius * 0.42f, earthRadius * 0.14f, 22);
    glPopMatrix();

    // Aurora
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glPushMatrix();
    glTranslatef(earthX, earthY, 0);
    for (int i = 0; i < 6; i++) {
        float auroraAngle = i * PI / 6 - PI / 2;
        float auroraX = earthRadius * 0.78f * cos(auroraAngle);
        float auroraY = earthRadius * 0.68f + earthRadius * 0.10f * sin(auroraAngle);
        float intensity = 0.28f;
        glColor4f(0.2f, 1.0f, 0.5f, intensity * 0.36f);
        drawEllipse(auroraX, auroraY, 0.03f, 0.015f, 10);
    }
    glPopMatrix();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Day/night terminator
    float sunAngle = atan2(sunBgY - earthY, sunBgX - earthX) * 180.0f / PI;
    drawDayNightMask(earthX, earthY, earthRadius * 0.99f, sunAngle);

    // City lights
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glPushMatrix();
    glTranslatef(earthX, earthY, 0);
    glRotatef(sunAngle + 180, 0, 0, 1);
    for (int i = 0; i < 15; i++) {
        float cityAngle = (rand() % 180 - 90) * PI / 180.0f;
        float cityDist = earthRadius * (0.42f + (rand() % 40) / 100.0f);
        float cityX = cityDist * cos(cityAngle + i * 0.3f);
        float cityY = cityDist * sin(cityAngle + i * 0.3f);
        float flicker = 0.9f + 0.1f * sin(angleAll * 0.15f + i * 2);
        glColor4f(1.0f, 0.9f, 0.6f, 0.08f * flicker);
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        glVertex2f(cityX, cityY);
        glEnd();
    }
    glPopMatrix();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ISS
    float issOrbitRadius = earthRadius * 1.15f;
    float issAngle = angleAll * 0.05f;
    float issX = earthX + issOrbitRadius * cos(issAngle * PI / 180.0f);
    float issY = earthY + issOrbitRadius * sin(issAngle * PI / 180.0f);

    glColor4f(0.5f, 0.8f, 1.0f, 0.2f);
    drawCircle(earthX, earthY, issOrbitRadius, 60, true);

    glColor3f(0.9f, 0.9f, 0.95f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(issX - 0.025f, issY);
    glVertex2f(issX + 0.025f, issY);
    glEnd();

    // ISS solar panels
    glColor4f(0.3f, 0.4f, 0.8f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(issX - 0.018f, issY - 0.008f);
    glVertex2f(issX - 0.008f, issY - 0.008f);
    glVertex2f(issX - 0.008f, issY + 0.008f);
    glVertex2f(issX - 0.018f, issY + 0.008f);
    glVertex2f(issX + 0.008f, issY - 0.008f);
    glVertex2f(issX + 0.018f, issY - 0.008f);
    glVertex2f(issX + 0.018f, issY + 0.008f);
    glVertex2f(issX + 0.008f, issY + 0.008f);
    glEnd();

    // Satellites
    for (int s = 0; s < 3; s++) {
        float satOrbit = earthRadius * (1.25f + s * 0.08f);
        float satAngle = angleAll * (0.03f + s * 0.01f) + s * PI * 2 / 3;
        float satX = earthX + satOrbit * cos(satAngle);
        float satY = earthY + satOrbit * sin(satAngle);
        glColor4f(0.8f, 0.8f, 0.9f, 0.9f);
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        glVertex2f(satX, satY);
        glEnd();
    }

    // Moon
    float moonOrbitRadius = 0.5f;
    float moonRad = moonAngle * PI / 180.0f;
    float moonX = earthX + moonOrbitRadius * cos(moonRad);
    float moonY = earthY + moonOrbitRadius * sin(moonRad);
    float moonRadius = 0.08f;

    glColor4f(0.4f, 0.45f, 0.55f, 0.25f);
    drawCircle(earthX, earthY, moonOrbitRadius, 90, true);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int i = 0; i < 6; i++) {
        float ratio = (float)i / 6;
        glColor4f(0.85f, 0.88f, 0.95f, (1.0f - ratio) * 0.1f);
        drawCircle(moonX, moonY, moonRadius * (1.1f + ratio * 0.3f), 30);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor3f(0.88f, 0.88f, 0.92f);
    drawCircle(moonX, moonY, moonRadius, 45);

    // Moon maria
    glColor4f(0.5f, 0.52f, 0.55f, 0.5f);
    drawEllipse(moonX - moonRadius * 0.25f, moonY + moonRadius * 0.15f, moonRadius * 0.22f, moonRadius * 0.18f, 12);
    drawEllipse(moonX + moonRadius * 0.2f, moonY + moonRadius * 0.25f, moonRadius * 0.15f, moonRadius * 0.12f, 10);

    // Moon shadow
    float moonSunAngle = atan2(sunBgY - moonY, sunBgX - moonX) * 180.0f / PI;
    drawDayNightMask(moonX, moonY, moonRadius * 0.96f, moonSunAngle);

    // Tidal lines
    glEnable(GL_BLEND);
    glColor4f(0.3f, 0.5f, 0.8f, 0.15f);
    glLineWidth(1.0f);
    for (int i = 0; i < 5; i++) {
        float offset = (i - 2) * 0.015f;
        glBegin(GL_LINE_STRIP);
        for (float t = 0; t <= 1.0f; t += 0.05f) {
            float x = earthX * (1 - t) + moonX * t;
            float y = earthY * (1 - t) + moonY * t + sin(t * PI * 3 + angleAll * 0.1f) * 0.02f + offset;
            glColor4f(0.3f, 0.5f, 0.8f, 0.1f * sin(t * PI));
            glVertex2f(x, y);
        }
        glEnd();
    }

    // Meteor shower
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int m = 0; m < 5; m++) {
        float meteorPhase = fmod(angleAll * 0.02f + m * 1.5f, 2.0f);
        if (meteorPhase < 1.0f) {
            float mx = 0.5f - m * 0.25f + meteorPhase * 0.3f;
            float my = 0.9f - meteorPhase * 0.4f;
            float meteorAlpha = sin(meteorPhase * PI);
            
            glColor4f(1.0f, 1.0f, 0.9f, meteorAlpha * 0.8f);
            glPointSize(2.5f);
            glBegin(GL_POINTS);
            glVertex2f(mx, my);
            glEnd();
            
            glBegin(GL_LINES);
            glColor4f(1.0f, 0.8f, 0.5f, meteorAlpha * 0.6f);
            glVertex2f(mx, my);
            glColor4f(1.0f, 0.5f, 0.2f, 0.0f);
            glVertex2f(mx - 0.05f, my + 0.03f);
            glEnd();
        }
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawHUD();
}

// --- FRAME 4: NIGHT SKY FROM EARTH ---
void drawFrame4() {
    // Dark gradient sky
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.0f, 0.05f);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glColor3f(0.02f, 0.02f, 0.08f);
    glVertex2f(1.0f, -0.3f);
    glVertex2f(-1.0f, -0.3f);
    glEnd();

    // Milky Way
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int i = 0; i < 200; i++) {
        float t = (float)i / 200;
        float x = -1.0f + t * 2.5f;
        float baseY = 0.3f + 0.4f * sin(t * PI * 0.8f);
        float spread = 0.15f + 0.1f * sin(t * PI * 2);
        for (int j = 0; j < 8; j++) {
            float offsetY = (rand() % 100 - 50) / 100.0f * spread;
            float offsetX = (rand() % 20 - 10) / 100.0f;
            float brightness = 0.1f + 0.15f * (rand() % 100) / 100.0f;
            glColor4f(0.6f + (rand() % 20) / 100.0f, 
                     0.55f + (rand() % 20) / 100.0f, 
                     0.7f + (rand() % 20) / 100.0f, 
                     brightness);
            glPointSize(1.0f + (rand() % 10) / 10.0f);
            glBegin(GL_POINTS);
            glVertex2f(x + offsetX, baseY + offsetY);
            glEnd();
        }
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Stars with colors
    for (auto& s : stars) {
        float twinkle = 0.6f + 0.4f * sin(angleAll * s.twinkleSpeed * 1.5f + s.x * 15);
        float brightness = s.brightness * twinkle;
        float colorPhase = fmod(s.x * 50 + s.y * 30, 4.0f);
        float r, g, b;
        if (colorPhase < 1.0f) { r = 0.7f; g = 0.8f; b = 1.0f; }
        else if (colorPhase < 2.0f) { r = 1.0f; g = 1.0f; b = 1.0f; }
        else if (colorPhase < 3.0f) { r = 1.0f; g = 0.95f; b = 0.7f; }
        else { r = 1.0f; g = 0.8f; b = 0.6f; }
        glColor4f(r * brightness, g * brightness, b * brightness, 1.0f);
        glPointSize(1.5f + brightness * 2.0f);
        glBegin(GL_POINTS);
        glVertex2f(s.x, s.y);
        glEnd();
        if (s.brightness > 0.8f) {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glColor4f(r, g, b, 0.15f * twinkle);
            drawCircle(s.x, s.y, 0.015f, 10);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    // Moon in sky
    float moonSkyX = 0.4f;
    float moonSkyY = 0.55f;
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int i = 0; i < 8; i++) {
        float ratio = (float)i / 8;
        glColor4f(0.9f, 0.92f, 1.0f, (1.0f - ratio) * 0.08f);
        drawCircle(moonSkyX, moonSkyY, 0.06f + ratio * 0.05f, 25);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glColor3f(0.95f, 0.95f, 0.98f);
    drawCircle(moonSkyX, moonSkyY, 0.05f, 35);
    
    glColor3f(0.01f, 0.01f, 0.04f);
    drawCircle(moonSkyX + 0.025f, moonSkyY, 0.045f, 35);

    // Northern lights
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int layer = 0; layer < 3; layer++) {
        for (int i = 0; i < 20; i++) {
            float x = -0.9f + i * 0.09f + layer * 0.02f;
            float baseY = 0.0f + layer * 0.08f;
            float height = 0.3f + 0.2f * sin(angleAll * 0.03f + i * 0.5f + layer);
            float wave = sin(angleAll * 0.05f + i * 0.3f) * 0.05f;
            
            glBegin(GL_QUAD_STRIP);
            for (int h = 0; h <= 10; h++) {
                float t = (float)h / 10;
                float y = baseY + t * height;
                float alpha = sin(t * PI) * (0.15f + 0.1f * sin(angleAll * 0.04f + i));
                float xOff = wave * sin(t * PI * 2);
                
                if (t < 0.5f) {
                    glColor4f(0.2f, 0.9f + t * 0.1f, 0.4f + t * 0.3f, alpha);
                } else {
                    glColor4f(0.3f + (t - 0.5f) * 0.4f, 0.8f - (t - 0.5f) * 0.3f, 0.6f + (t - 0.5f) * 0.3f, alpha);
                }
                
                glVertex2f(x + xOff - 0.02f, y);
                glVertex2f(x + xOff + 0.02f, y);
            }
            glEnd();
        }
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Ground
    glBegin(GL_QUADS);
    glColor3f(0.02f, 0.02f, 0.03f);
    glVertex2f(-1.0f, -0.3f);
    glVertex2f(1.0f, -0.3f);
    glColor3f(0.01f, 0.01f, 0.02f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();

    // Hills (lighter for silhouette contrast)
    glColor3f(0.03f, 0.03f, 0.04f);
    glBegin(GL_POLYGON);
    glVertex2f(-1.0f, -0.3f);
    glVertex2f(-0.8f, -0.22f);
    glVertex2f(-0.6f, -0.28f);
    glVertex2f(-0.4f, -0.18f);
    glVertex2f(-0.2f, -0.25f);
    glVertex2f(0.0f, -0.15f);
    glVertex2f(0.2f, -0.23f);
    glVertex2f(0.4f, -0.2f);
    glVertex2f(0.6f, -0.27f);
    glVertex2f(0.8f, -0.19f);
    glVertex2f(1.0f, -0.25f);
    glVertex2f(1.0f, -0.5f);
    glVertex2f(-1.0f, -0.5f);
    glEnd();

    // Trees
    glColor3f(0.01f, 0.01f, 0.015f);
    for (int i = 0; i < 25; i++) {
        float tx = -0.95f + i * 0.08f;
        float hillY = -0.25f + 0.08f * sin(tx * 2.5f + 0.5f);
        float treeHeight = 0.04f + (i % 3) * 0.02f;
        
        glBegin(GL_TRIANGLES);
        glVertex2f(tx, hillY);
        glVertex2f(tx - 0.015f, hillY - treeHeight);
        glVertex2f(tx + 0.015f, hillY - treeHeight);
        glEnd();
    }

    // Large human silhouette with rim and glow
    float humanX = -0.55f;
    float groundY = -0.2f;
    float scale = 3.6f;

    // Rim outline (dark gray, slightly bigger)
    glColor3f(0.06f, 0.06f, 0.07f);
    float rimScale = scale * 1.06f;
    
    // Head rim
    drawCircle(humanX + 0.02f * rimScale, groundY + 0.1f * rimScale, 0.025f * rimScale, 32);
    
    // Body rim (simplified outline)
    glBegin(GL_POLYGON);
    glVertex2f(humanX - 0.025f * rimScale, groundY + 0.06f * rimScale);
    glVertex2f(humanX + 0.03f * rimScale, groundY + 0.065f * rimScale);
    glVertex2f(humanX + 0.1f * rimScale, groundY - 0.03f * rimScale);
    glVertex2f(humanX - 0.09f * rimScale, groundY - 0.02f * rimScale);
    glEnd();

    // Moonlight back-glow
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(0.12f, 0.16f, 0.25f, 0.18f);
    drawCircle(humanX + 0.02f * scale, groundY + 0.1f * scale, 0.055f * scale, 20);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Main silhouette (pure black)
    glColor3f(0.0f, 0.0f, 0.0f);
    
    float headX = humanX + 0.02f * scale;
    float headY = groundY + 0.1f * scale;
    drawCircle(headX, headY, 0.025f * scale, 30);

    // Hair/cap
    glBegin(GL_POLYGON);
    glVertex2f(headX - 0.028f * scale, headY + 0.008f * scale);
    glVertex2f(headX + 0.018f * scale, headY + 0.022f * scale);
    glVertex2f(headX + 0.025f * scale, headY + 0.01f * scale);
    glVertex2f(headX - 0.022f * scale, headY - 0.002f * scale);
    glEnd();

    // Neck
    glBegin(GL_POLYGON);
    glVertex2f(humanX + 0.0f * scale, groundY + 0.075f * scale);
    glVertex2f(humanX + 0.02f * scale, groundY + 0.08f * scale);
    glVertex2f(humanX + 0.018f * scale, groundY + 0.06f * scale);
    glVertex2f(humanX - 0.002f * scale, groundY + 0.055f * scale);
    glEnd();

    // Torso
    glBegin(GL_POLYGON);
    glVertex2f(humanX - 0.025f * scale, groundY + 0.06f * scale);
    glVertex2f(humanX + 0.03f * scale, groundY + 0.065f * scale);
    glVertex2f(humanX + 0.045f * scale, groundY + 0.01f * scale);
    glVertex2f(humanX + 0.015f * scale, groundY - 0.015f * scale);
    glVertex2f(humanX - 0.02f * scale, groundY + 0.0f * scale);
    glEnd();

    // Left arm
    glBegin(GL_POLYGON);
    glVertex2f(humanX - 0.022f * scale, groundY + 0.052f * scale);
    glVertex2f(humanX - 0.015f * scale, groundY + 0.045f * scale);
    glVertex2f(humanX - 0.055f * scale, groundY + 0.005f * scale);
    glVertex2f(humanX - 0.065f * scale, groundY + 0.012f * scale);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(humanX - 0.055f * scale, groundY + 0.01f * scale);
    glVertex2f(humanX - 0.065f * scale, groundY + 0.003f * scale);
    glVertex2f(humanX - 0.09f * scale, groundY - 0.02f * scale);
    glVertex2f(humanX - 0.082f * scale, groundY - 0.012f * scale);
    glEnd();
    drawCircle(humanX - 0.09f * scale, groundY - 0.018f * scale, 0.015f * scale, 12);

    // Right arm
    glBegin(GL_POLYGON);
    glVertex2f(humanX + 0.025f * scale, groundY + 0.055f * scale);
    glVertex2f(humanX + 0.035f * scale, groundY + 0.048f * scale);
    glVertex2f(humanX + 0.06f * scale, groundY + 0.03f * scale);
    glVertex2f(humanX + 0.052f * scale, groundY + 0.038f * scale);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(humanX + 0.055f * scale, groundY + 0.035f * scale);
    glVertex2f(humanX + 0.065f * scale, groundY + 0.028f * scale);
    glVertex2f(humanX + 0.08f * scale, groundY + 0.012f * scale);
    glVertex2f(humanX + 0.072f * scale, groundY + 0.02f * scale);
    glEnd();
    drawCircle(humanX + 0.078f * scale, groundY + 0.015f * scale, 0.012f * scale, 10);

    // Left leg
    glBegin(GL_POLYGON);
    glVertex2f(humanX - 0.005f * scale, groundY + 0.0f * scale);
    glVertex2f(humanX + 0.02f * scale, groundY - 0.008f * scale);
    glVertex2f(humanX + 0.045f * scale, groundY + 0.025f * scale);
    glVertex2f(humanX + 0.03f * scale, groundY + 0.035f * scale);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(humanX + 0.035f * scale, groundY + 0.03f * scale);
    glVertex2f(humanX + 0.05f * scale, groundY + 0.022f * scale);
    glVertex2f(humanX + 0.065f * scale, groundY - 0.025f * scale);
    glVertex2f(humanX + 0.05f * scale, groundY - 0.02f * scale);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(humanX + 0.05f * scale, groundY - 0.022f * scale);
    glVertex2f(humanX + 0.068f * scale, groundY - 0.025f * scale);
    glVertex2f(humanX + 0.09f * scale, groundY - 0.04f * scale);
    glVertex2f(humanX + 0.052f * scale, groundY - 0.038f * scale);
    glEnd();

    // Right leg
    glBegin(GL_POLYGON);
    glVertex2f(humanX + 0.025f * scale, groundY - 0.01f * scale);
    glVertex2f(humanX + 0.045f * scale, groundY - 0.018f * scale);
    glVertex2f(humanX + 0.1f * scale, groundY - 0.03f * scale);
    glVertex2f(humanX + 0.095f * scale, groundY - 0.02f * scale);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(humanX + 0.095f * scale, groundY - 0.025f * scale);
    glVertex2f(humanX + 0.105f * scale, groundY - 0.035f * scale);
    glVertex2f(humanX + 0.13f * scale, groundY - 0.048f * scale);
    glVertex2f(humanX + 0.12f * scale, groundY - 0.04f * scale);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(humanX + 0.12f * scale, groundY - 0.042f * scale);
    glVertex2f(humanX + 0.135f * scale, groundY - 0.048f * scale);
    glVertex2f(humanX + 0.155f * scale, groundY - 0.065f * scale);
    glVertex2f(humanX + 0.12f * scale, groundY - 0.058f * scale);
    glEnd();

    // Small campfire
    float fireX = 0.55f;
    float fireY = -0.35f;
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int g = 0; g < 3; g++) {
        float glowSize = 0.03f + g * 0.02f;
        float glowAlpha = 0.15f - g * 0.04f;
        glColor4f(1.0f, 0.5f, 0.1f, glowAlpha);
        drawCircle(fireX, fireY + 0.01f, glowSize, 20);
    }
    
    for (int f = 0; f < 4; f++) {
        float flamePhase = sin(angleAll * 0.25f + f * 1.2f);
        float fx = fireX + (f - 1.5f) * 0.008f + flamePhase * 0.003f;
        float flameHeight = 0.025f + 0.01f * sin(angleAll * 0.3f + f);
        
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.7f, 0.2f, 0.9f);
        glVertex2f(fx - 0.006f, fireY);
        glVertex2f(fx + 0.006f, fireY);
        glColor4f(1.0f, 0.4f, 0.0f, 0.0f);
        glVertex2f(fx + flamePhase * 0.005f, fireY + flameHeight);
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawHUD();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    switch (currentFrame) {
        case 1: drawFrame1(); break;
        case 2: drawFrame2(); break;
        case 3: drawFrame3(); break;
        case 4: drawFrame4(); break;
        default: drawFrame1(); break;
    }

    drawHelpOverlay();
    glutSwapBuffers();
}

void update(int value) {
    if (!isPaused) {
        angleAll += 0.5f * speedMultiplier;
        moonAngle += 2.0f * speedMultiplier;
        sunPulse += 0.12f * speedMultiplier;
        coronaAngle += 0.02f * speedMultiplier;
        heatwavePhase += 0.08f * speedMultiplier;
        cloudAngle += 0.3f * speedMultiplier;
        ceresAngle += 0.004f * speedMultiplier;

        for (int i = 0; i < 8; i++) {
            if (!planetPaused[i]) {
                planetRotation[i] += planetRotationSpeeds[i] * speedMultiplier;
            }
            ringAngle[i] += 0.2f * speedMultiplier;
        }

        cometX += 0.004f * speedMultiplier;
        if (cometX > 1.5f) cometX = -1.5f;

        aircraftX += 0.005f * speedMultiplier;
        if (aircraftX > 1.5f) aircraftX = -1.5f;

        starScroll -= 0.0002f * speedMultiplier;
        if (starScroll < -2.0f) starScroll = 0;

        for (auto& d : spaceDust) {
            d.x += d.vx * speedMultiplier;
            d.y += d.vy * speedMultiplier;
            if (d.x < -1.2f || d.x > 1.2f || d.y < -1.2f || d.y > 1.2f) {
                d.x = -1.0f + (rand() % 200) / 100.0f;
                d.y = -1.0f + (rand() % 200) / 100.0f;
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void initializeObjects() {
    srand(time(0));
    
    // Initialize stars
    for (int i = 0; i < 300; i++) {
        Star s;
        s.x = -1.2f + (rand() % 240) / 100.0f;
        s.y = -1.2f + (rand() % 240) / 100.0f;
        s.brightness = 0.3f + (rand() % 70) / 100.0f;
        s.twinkleSpeed = 0.5f + (rand() % 15) / 10.0f;
        stars.push_back(s);
    }

    // Initialize asteroids
    for (int i = 0; i < 150; i++) {
        Asteroid a;
        a.angle = (rand() % 360) * PI / 180.0f;
        a.distance = 0.58f + (rand() % 10) / 100.0f;
        a.size = 0.002f + (rand() % 3) / 1000.0f;
        a.speed = 0.001f + (rand() % 5) / 10000.0f;
        asteroids.push_back(a);
    }

    // Initialize space dust
    for (int i = 0; i < 100; i++) {
        SpaceDust d;
        d.x = -1.0f + (rand() % 200) / 100.0f;
        d.y = -1.0f + (rand() % 200) / 100.0f;
        d.vx = -0.001f + (rand() % 2) / 1000.0f;
        d.vy = -0.001f + (rand() % 2) / 1000.0f;
        d.size = 1.0f + (rand() % 2) / 10.0f;
        d.alpha = 0.2f + (rand() % 5) / 10.0f;
        spaceDust.push_back(d);
    }
}

void keyboard(unsigned char key, int x, int y) {
    // Handle frame switching (1-4)
    if (key >= '1' && key <= '4') {
        currentFrame = key - '0';
        glutPostRedisplay();
        return;
    }
    
    switch (key) {
        case 'p': case 'P': isPaused = !isPaused; break;
        case 'h': case 'H': showHelp = !showHelp; break;
        case '+': case '=': 
            speedMultiplier = min(speedMultiplier + 0.25f, 2.0f); 
            break;
        case '-': case '_': 
            speedMultiplier = max(speedMultiplier - 0.25f, 0.25f); 
            break;
        case 'e': case 'E': eclipseMode = !eclipseMode; break;
        case 'z': case 'Z': zoomPlanetIndex = -1; break;
        case '0': case '5': case '6': case '7':
            // Toggle individual planet pause (0-7, but 1-4 already used for frames)
            if (key >= '0' && key <= '7') {
                int idx = key - '0';
                if (idx < 8) planetPaused[idx] = !planetPaused[idx];
            }
            break;
        case 27: // ESC
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    if (currentFrame == 2 && zoomPlanetIndex == -1) {
        if (key >= GLUT_KEY_F1 && key <= GLUT_KEY_F8) {
            zoomPlanetIndex = key - GLUT_KEY_F1;
        }
    }
    glutPostRedisplay();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)width / (float)height;
    if (width >= height) {
        glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
    } else {
        glOrtho(-1.0, 1.0, -1.0/aspect, 1.0/aspect, -1.0, 1.0);
    }
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Solar System Explorer - Legacy OpenGL");

    // OpenGL initialization
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    initializeObjects();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, update, 0);

    cout << "=== SOLAR SYSTEM EXPLORER - LEGACY OPENGL ===" << endl;
    cout << "Controls:" << endl;
    cout << "  1-4: Switch between frames" << endl;
    cout << "  P: Pause/Resume animation" << endl;
    cout << "  H: Toggle help overlay" << endl;
    cout << "  +/-: Increase/Decrease speed" << endl;
    cout << "  E: Toggle eclipse mode" << endl;
    cout << "  0-7: Toggle individual planet pause" << endl;
    cout << "  Z: Exit zoom mode (Frame 2)" << endl;
    cout << "  ESC: Exit application" << endl;
    cout << "==============================================" << endl;

    glutMainLoop();
    return 0;
}
