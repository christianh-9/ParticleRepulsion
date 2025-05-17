#include "raylib.h"
#include <vector>
#include <raymath.h>

//particle with position, velocity, size and color
struct Particle {
    Vector2 pos;
    Vector2 vel;
    float   radius;
    Color   color;
};

void ResolveCollision(Particle &a, Particle &b) {
    Vector2 diff = Vector2Subtract(b.pos, a.pos);
    float dist = Vector2Length(diff);
    float minDist = a.radius + b.radius;

    if (dist < minDist && dist > 0.0f) {
        //Vector direction
        Vector2 normal = Vector2Scale(diff, 1.0f / dist);

        //So particles don't overlap
        float overlap = 0.5f * (minDist - dist);
        a.pos = Vector2Subtract(a.pos, Vector2Scale(normal, overlap));
        b.pos = Vector2Add(b.pos, Vector2Scale(normal, overlap));

        //Velocity
        Vector2 relVel = Vector2Subtract(b.vel, a.vel);
        float velAlongNormal = Vector2DotProduct(relVel, normal);

        //If velocities are separate
        if (velAlongNormal > 0) return;

        //To bounce
        Vector2 impulse = Vector2Scale(normal, velAlongNormal);
        a.vel = Vector2Add(a.vel, impulse);
        b.vel = Vector2Subtract(b.vel, impulse);
    }
}

enum AppState { MENU, SIMULATE };

int main() {
    const int screenWidth   = 800;
    const int screenHeight  = 600;

    //For slow-motion, how strong particles get pushed, how they slow down, and the radius for mouse
    const float slowMo       = 0.5f;
    const float repulsion    = 120.0f;
    const float dragStrength = 1.2f;
    const float influenceRad = 75.0f;

    InitWindow(screenWidth, screenHeight, "Particle Repulsion");
    InitAudioDevice();
    SetTargetFPS(60);

    //Start at menu
    AppState state = MENU;

    //Default particle counter
    int particleCount = 500;
    std::vector<Particle> particles;

    Music music{};

    //Main loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLUE);

        if (state == MENU) {
            //Menu
            ClearBackground(BLACK);

            //Menu Panel
            int pw = 600, ph = 400;
            int px = screenWidth/2 - pw/2;
            int py = screenHeight/2 - ph/2;
            DrawRectangle(px, py, pw, ph, Color{30, 30, 60, 220});

            //Prompt user
            const char* title = "Select Particle Count (100-2500)";
            int ts = 32;
            int tw = MeasureText(title, ts);
            DrawText(title, screenWidth/2 - tw/2, py + 40, ts, SKYBLUE);

            //Count
            const char* countText = TextFormat("%d", particleCount);
            int cs = 72;
            int cw = MeasureText(countText, cs);
            DrawText(countText, screenWidth/2 - cw/2, py + 120, cs, SKYBLUE);

            //How to increase/decrease
            const char* instr1 = "Use UP / DOWN to adjust";
            const char* instr2 = "Press ENTER to start";
            int isz = 20;
            int iw1 = MeasureText(instr1, isz);
            int iw2 = MeasureText(instr2, isz);
            DrawText(instr1, screenWidth/2 - iw1/2, py + 220, isz, LIGHTGRAY);
            DrawText(instr2, screenWidth/2 - iw2/2, py + 250, isz, LIGHTGRAY);

            //Input
            if (IsKeyPressed(KEY_UP))   particleCount += 100;
            if (IsKeyPressed(KEY_DOWN)) particleCount -= 100;
            particleCount = Clamp(particleCount, 100, 2500);

            //Start the simulation
            if (IsKeyPressed(KEY_ENTER)) {
                //Initialize particles
                particles.clear();
                particles.reserve(particleCount);
                for (int i = 0; i < particleCount; ++i) {
                    Particle p;
                    //random position
                    p.pos.x   = (float)GetRandomValue(0, screenWidth);
                    p.pos.y   = (float)GetRandomValue(0, screenHeight);
                    //rest velocity
                    p.vel     = { 0, 0 };
                    //random sizing
                    p.radius  = (float)GetRandomValue(3, 6);
                    //random color
                    p.color   = {
                            (unsigned char)GetRandomValue(100,255),
                            (unsigned char)GetRandomValue(100,255),
                            (unsigned char)GetRandomValue(100,255),
                            255
                    };
                    particles.push_back(p);
                }
                // Load & play music
                music = LoadMusicStream("../song.ogg");
                PlayMusicStream(music);
                SetMusicVolume(music, 1.0f);

                //Switch to simulation
                state = SIMULATE;
            }
        }
        else {
            //Simulation Screen
            ClearBackground(BLACK);
            UpdateMusicStream(music);

            //For slow-motion
            float dt = GetFrameTime() * slowMo;

            //Mouse position
            Vector2 mouse = GetMousePosition();

            for (auto &p : particles) {
                //vector & distance to mouse
                Vector2 diff = { p.pos.x - mouse.x, p.pos.y - mouse.y };
                float   dist = sqrtf(diff.x*diff.x + diff.y*diff.y);

                //Push away if inside iRad
                if (dist < influenceRad) {
                    Vector2 dir = { diff.x/dist, diff.y/dist };
                    float strength = (influenceRad - dist)/influenceRad * repulsion;

                    //Accelerate
                    p.vel.x += dir.x * strength * dt;
                    p.vel.y += dir.y * strength * dt;
                }

                //So particles lose speed
                p.vel.x -= p.vel.x * dragStrength * dt;
                p.vel.y -= p.vel.y * dragStrength * dt;

                //Velocity for particles
                p.pos.x += p.vel.x * dt;
                p.pos.y += p.vel.y * dt;

                //Bouncing for particles
                if (p.pos.x < p.radius) {
                    p.pos.x = p.radius; p.vel.x *= -1;
                }
                if (p.pos.x > screenWidth - p.radius) {
                    p.pos.x = screenWidth - p.radius; p.vel.x *= -1;
                }
                if (p.pos.y < p.radius) {
                    p.pos.y = p.radius; p.vel.y *= -1;
                }
                if (p.pos.y > screenHeight - p.radius) {
                    p.pos.y = screenHeight - p.radius; p.vel.y *= -1;
                }
            }
            // Handle collisions
            for (int i = 0; i < particles.size(); ++i) {
                for (int j = i + 1; j < particles.size(); ++j) {
                    ResolveCollision(particles[i], particles[j]);
                }
            }

            // Draw simulation
            DrawText("Particle Repulsion", 10, 10, 20, RAYWHITE);
            for (auto &p : particles) {
                DrawCircleV(p.pos, p.radius, p.color);
            }
        }
        EndDrawing();
    }
    // Cleanup and end program
    if (state == SIMULATE) UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
