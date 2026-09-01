// NEON ARENA - minimal FPS prototype
// SDL2 + legacy OpenGL fixed pipeline. Build: make
#include <SDL.h>
#include <GL/gl.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <list>
#include <algorithm>

static const int   WIN_W = 1280, WIN_H = 720;
static const float ARENA = 24.0f;      // half-extent of arena
struct Vec3 { float x,y,z; };

struct Enemy {
    Vec3 pos;
    float hp;
    float phase;
    bool alive;
    float hitFlash;
};

struct Tracer { Vec3 from, to; float life; };
struct Spark  { Vec3 pos, vel;  float life; };

static SDL_Window*   win = nullptr;
static SDL_GLContext gl = nullptr;
static bool running = true;

// player state
static float px=0, pz=0, pyaw=0, ppitch=0;
static float pvel_x=0, pvel_z=0;
static int   hp = 100;
static int   score = 0;
static Uint32 lastShot = 0;
static const int FIRE_MS = 180;
static float recoil = 0;

// enemies / fx
static std::list<Enemy> enemies;
static std::vector<Tracer> tracers;
static std::vector<Spark>  sparks;
static Uint32 lastSpawn = 0;
static int wave = 1;

static float t_now() { return SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency(); }
static double now_s = 0;

static void glu_perspective(double fovy,double aspect,double zn,double zf);
static void solid_cube(float s);
// minimal 3x5 pixelfont
static const char* FONT[36] = {
    "111101101101111", //0
    "010110010010111", //1
    "111001111100111", //2
    "111001111001111", //3
    "101101111001001", //4
    "111100111001111", //5
    "111100111101111", //6
    "111001001010010", //7
    "111101111101111", //8
    "111101111001111", //9
    "111101111101101", //A
    "110101110101110", //B
    "111100100100111", //C
    "110101101101110", //D
    "111100111100111", //E
    "111100111100100", //F
    "111100101101111", //G
    "101101111101101", //H
    "111010010010111", //I
    "001001001101111", //J
    "101101110101101", //K
    "100100100100111", //L
    "101111111101101", //M
    "110101101101101", //N
    "111101101101111", //O
    "111101111100100", //P
    "111101101111001", //Q
    "111101110101101", //R
    "111100111001111", //S
    "111010010010010", //T
    "101101101101111", //U
    "101101101101010", //V
    "101101111111101", //W
    "101101010101101", //X
    "101101010010010", //Y
    "111001010100111"  //Z
};
static void draw_text(const char* s, int x, int y, int scale) {
    for (; *s; s++) {
        int idx = -1;
        if (*s >= '0' && *s <= '9') idx = *s-'0';
        else if (*s >= 'A' && *s <= 'Z') idx = *s-'A'+10;
        else if (*s == '-') idx = 10+ ('N'-'A');
        if (idx < 0) { x += 4*scale; continue; }
        glBegin(GL_QUADS);
        for (int r=0;r<5;r++) for (int c=0;c<3;c++)
            if (FONT[idx][r*3+c]=='1') {
                glVertex2i(x+c*scale,       y-r*scale);
                glVertex2i(x+(c+1)*scale,   y-r*scale);
                glVertex2i(x+(c+1)*scale,   y-(r+1)*scale);
                glVertex2i(x+c*scale,       y-(r+1)*scale);
            }
        glEnd();
        x += 4*scale;
    }
}

static void spawn_enemy() {
    // spawn at arena edge away from player
    float a = (rand() % 3600) * 0.017453f * 0.1f;
    float r = ARENA * 0.9f;
    Enemy e;
    e.pos = { cosf(a)*r, 0.9f, sinf(a)*r };
    e.hp = 3; e.phase = rand()%628 * 0.01f; e.alive = true; e.hitFlash = 0;
    enemies.push_back(e);
}

static void burst(Vec3 p, int n) {
    for (int i=0;i<n;i++) {
        Spark s;
        s.pos = p;
        s.vel = { (rand()%2000-1000)*0.004f, (rand()%1500+200)*0.004f, (rand()%2000-1000)*0.004f };
        s.life = 0.5f + (rand()%30)*0.01f;
        sparks.push_back(s);
    }
}

// ray vs axis-aligned box (enemy cube), returns dist or -1
static float ray_box(Vec3 o, Vec3 d, Vec3 c, float h) {
    float tmin = 0, tmax = 1e9f;
    float lo[3] = {c.x-h,c.y-h,c.z-h}, hi[3] = {c.x+h,c.y+h,c.z+h};
    float oo[3] = {o.x,o.y,o.z}, dd[3] = {d.x,d.y,d.z};
    for (int i=0;i<3;i++) {
        if (fabsf(dd[i]) < 1e-6f) { if (oo[i]<lo[i]||oo[i]>hi[i]) return -1; continue; }
        float t1=(lo[i]-oo[i])/dd[i], t2=(hi[i]-oo[i])/dd[i];
        if (t1>t2) { float tmp=t1;t1=t2;t2=tmp; }
        tmin = tmin>t1?tmin:t1; tmax = tmax<t2?tmax:t2;
        if (tmin>tmax) return -1;
    }
    return tmin;
}

static void shoot() {
    Uint32 t = SDL_GetTicks();
    if (t - lastShot < FIRE_MS) return;
    lastShot = t; recoil = 1.0f;

    Vec3 o = { px, 1.5f, pz };
    Vec3 d = { sinf(pyaw)*cosf(ppitch), -sinf(ppitch), -cosf(pyaw)*cosf(ppitch) };

    // nearest hit enemy
    Enemy* best = nullptr; float bestT = 1e9f;
    for (auto& e : enemies) {
        if (!e.alive) continue;
        float tt = ray_box(o, d, e.pos, 0.8f);
        if (tt >= 0 && tt < bestT) { bestT = tt; best = &e; }
    }
    Vec3 end = d; end.x*=bestT; end.y*=bestT; end.z*=bestT;
    Vec3 hit = { o.x+end.x, o.y+end.y, o.z+end.z };
    if (bestT > 60) { hit = { o.x+d.x*60, o.y+d.y*60, o.z+d.z*60 }; }
    tracers.push_back({ o, hit, 0.12f });
    if (best) {
        best->hp--; best->hitFlash = 1;
        burst(hit, 10);
        if (best->hp <= 0) { best->alive=false; score += 10; burst(best->pos, 25); }
    } else if (bestT <= ARENA*2) {
        burst(hit, 4);
    }
}

static void update(float dt) {
    const Uint8* k = SDL_GetKeyboardState(nullptr);

    // look handled via relative mouse in event loop
    // move
    float fwd = (k[SDL_SCANCODE_W]?1:0) - (k[SDL_SCANCODE_S]?1:0);
    float str = (k[SDL_SCANCODE_D]?1:0) - (k[SDL_SCANCODE_A]?1:0);
    float sp = 9.0f;
    float sx = sinf(pyaw), cz = cosf(pyaw);
    float ax = (-sx*fwd + cz*str) * sp;
    float az = ( cz*fwd + sx*str) * sp;
    pvel_x += (ax - pvel_x) * fminf(1, dt*12);
    pvel_z += (az - pvel_z) * fminf(1, dt*12);
    px += pvel_x*dt; pz += pvel_z*dt;
    float lim = ARENA-1.0f;
    px = px<-lim?-lim:px>lim?lim:px;
    pz = pz<-lim?-lim:pz>lim?lim:pz;

    if (k[SDL_SCANCODE_SPACE]) shoot();
    recoil *= powf(0.001f, dt);

    // spawn waves: keep N alive, ramp up
    Uint32 tms = SDL_GetTicks();
    int target = 3 + wave/2; if (target > 12) target = 12;
    int alive = 0; for (auto&e:enemies) if (e.alive) alive++;
    if (alive == 0 && enemies.empty()) wave++;
    if ((tms - lastSpawn > 1500 && alive < target)) { spawn_enemy(); lastSpawn = tms; }

    // enemies chase player
    float speed = 2.2f + wave*0.15f;
    for (auto& e : enemies) {
        if (!e.alive) continue;
        e.phase += dt*4;
        e.hitFlash *= powf(0.01f, dt);
        float dx = px-e.pos.x, dz = pz-e.pos.z;
        float l = sqrtf(dx*dx+dz*dz); if (l>0.01f) { dx/=l; dz/=l; }
        e.pos.x += dx*speed*dt; e.pos.z += dz*speed*dt;
        e.pos.y = 0.9f + sinf(e.phase)*0.15f;
        // touch damage
        if (l < 1.4f) {
            static Uint32 lastHurt = 0;
            if (tms - lastHurt > 600) { hp -= 10; lastHurt = tms;
                pvel_x -= dx*8; pvel_z -= dz*8; }
        }
    }

    // fx decay
    for (auto& t2 : tracers) t2.life -= dt;
    for (auto& s : sparks) { s.life -= dt; s.vel.y -= 9*dt; s.pos.x+=s.vel.x*dt; s.pos.y+=s.vel.y*dt; s.pos.z+=s.vel.z*dt; if(s.pos.y<0.05f){s.pos.y=0.05f;s.vel.y*=-0.4f;} }
    tracers.erase(std::remove_if(tracers.begin(), tracers.end(), [](const Tracer&t){return t.life<=0;}), tracers.end());
    sparks.erase (std::remove_if(sparks.begin(),  sparks.end(),  [](const Spark&s){return s.life<=0;}),  sparks.end());

    if (hp <= 0) {
        printf("GAME OVER - Score: %d, Wave: %d\n", score, wave);
        hp = 100; score = 0; wave = 1;
        enemies.clear(); px=pz=0;
    }
}

// solid cube via immediate mode
static void solid_cube(float s) {
    float h=s*0.5f;
    glBegin(GL_QUADS);
    // -z
    glVertex3f(-h,-h,-h); glVertex3f( h,-h,-h); glVertex3f( h, h,-h); glVertex3f(-h, h,-h);
    // +z
    glVertex3f(-h,-h, h); glVertex3f( h,-h, h); glVertex3f( h, h, h); glVertex3f(-h, h, h);
    // -x
    glVertex3f(-h,-h,-h); glVertex3f(-h,-h, h); glVertex3f(-h, h, h); glVertex3f(-h, h,-h);
    // +x
    glVertex3f( h,-h,-h); glVertex3f( h,-h, h); glVertex3f( h, h, h); glVertex3f( h, h,-h);
    // -y
    glVertex3f(-h,-h,-h); glVertex3f( h,-h,-h); glVertex3f( h,-h, h); glVertex3f(-h,-h, h);
    // +y
    glVertex3f(-h, h,-h); glVertex3f( h, h,-h); glVertex3f( h, h, h); glVertex3f(-h, h, h);
    glEnd();
}

static void wire_cube(float s) {
    glBegin(GL_LINES);
    for (int i=0;i<2;i++) for (int j=0;j<2;j++) {
        glVertex3f(i?s:-s, j?s:-s, -s*0.55f*2); glVertex3f(i?s:-s, j?s:-s, s*0.55f*2);
        glVertex3f(i?s:-s, -s*0.55f*2, j?s:-s); glVertex3f(i?s:-s, s*0.55f*2, j?s:-s);
        glVertex3f(-s*0.55f*2, i?s:-s, j?s:-s); glVertex3f(s*0.55f*2, i?s:-s, j?s:-s);
    }
    glEnd();
}

static void render() {
    int w,h; SDL_GL_GetDrawableSize(win,&w,&h);
    glViewport(0,0,w,h);
    glClearColor(0.02f,0.02f,0.05f,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glu_perspective(75.0f + recoil*4, (float)w/h, 0.1f, 200);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    // camera
    float bob = sqrtf(pvel_x*pvel_x+pvel_z*pvel_z)*0.012f;
    float cy = 1.6f + bob*sinf(now_s*14);
    glRotatef(ppitch*57.2958f, 1,0,0);
    glRotatef(-pyaw*57.2958f - 90.0f, 0,1,0);
    glTranslatef(-(px+sinf(pyaw)*recoil*-0.06f), -(cy - recoil*0.03f), -(pz-cosf(pyaw)*recoil*-0.06f));

    // --- floor grid ---
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (float i=-ARENA;i<=ARENA;i+=2.0f) {
        float fade = 1.0f - fabsf(i)/ARENA;
        glColor4f(0.0f, 0.7f*fade+0.1f, 0.9f*fade+0.15f, 0.5f);
        glVertex3f(i,0,-ARENA); glVertex3f(i,0,ARENA);
        glVertex3f(-ARENA,0,i); glVertex3f(ARENA,0,i);
    }
    glEnd();
    // dark floor
    glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset(1,1);
    glBegin(GL_QUADS);
    glColor3f(0.03f,0.04f,0.08f);
    glVertex3f(-ARENA,0,-ARENA); glVertex3f(ARENA,0,-ARENA);
    glVertex3f(ARENA,0,ARENA);   glVertex3f(-ARENA,0,ARENA);
    glEnd();
    glDisable(GL_POLYGON_OFFSET_FILL);

    // arena walls (neon pillars at corners + boundary lines)
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glColor3f(1.0f,0.2f,0.6f);
    glVertex3f(-ARENA,0,-ARENA); glVertex3f(ARENA,0,-ARENA);
    glVertex3f(ARENA,0, ARENA);  glVertex3f(-ARENA,0, ARENA);
    glEnd();

    // --- enemies: neon magenta cubes with glow shell ---
    for (auto& e : enemies) {
        if (!e.alive) continue;
        float fl = e.hitFlash;
        glPushMatrix();
        glTranslatef(e.pos.x, e.pos.y, e.pos.z);
        glRotatef(now_s*40, 0,1,0);
        glColor3f(0.25f+fl*0.75f, 0.02f+fl*0.6f, 0.35f+fl*0.6f);
        solid_cube(1.1f);
        glColor4f(1.0f, 0.25f, 0.7f, 0.9f);
        glLineWidth(2.0f);
        wire_cube(0.62f);
        glPopMatrix();
    }

    // --- tracers ---
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    for (auto& t : tracers) {
        glColor4f(0.2f, 1.0f, 1.0f, t.life/0.12f);
        glVertex3f(t.from.x,t.from.y,t.from.z);
        glVertex3f(t.to.x,t.to.y,t.to.z);
    }
    glEnd();

    // --- sparks ---
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (auto& s : sparks) {
        glColor4f(1.0f, 0.8f, 0.2f, s.life);
        glVertex3f(s.pos.x,s.pos.y,s.pos.z);
    }
    glEnd();

    // --- crosshair ---
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0,w,0,h,-1,1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);
    float cx=w*0.5f, cyy=h*0.5f, g=8+recoil*14;
    glBegin(GL_LINES);
    glColor3f(0.2f,1.0f,1.0f);
    glVertex2f(cx-g,cyy); glVertex2f(cx-g-8,cyy);
    glVertex2f(cx+g,cyy); glVertex2f(cx+g+8,cyy);
    glVertex2f(cx,cyy-g); glVertex2f(cx,cyy-g-8);
    glVertex2f(cx,cyy+g); glVertex2f(cx,cyy+g+8);
    glEnd();

    // --- HUD text (bitmap font) ---
    char buf[128];
    snprintf(buf,sizeof buf,"HP %d   SCORE %d   WAVE %d", hp, score, wave);
    glRasterPos2i(16, h-28);
    glColor3f(0.2f,1.0f,1.0f);
    for (char* c=buf; *c; c++) if(*c>='a'&&*c<='z') *c = *c-'a'+'A';
    draw_text(buf, 16, h-28, 3);
    if (SDL_GetTicks()-lastShot < FIRE_MS)
        { glColor3ub(255,255,255); glBegin(GL_QUADS); glVertex2f(cx-2,cyy-2); glVertex2f(cx+2,cyy-2); glVertex2f(cx+2,cyy+2); glVertex2f(cx-2,cyy+2); glEnd(); }
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);

    SDL_GL_SwapWindow(win);
}
int main(int argc, char** argv) {
    (void)argc;(void)argv;
    srand(SDL_GetTicks());
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) { fprintf(stderr,"SDL_Init: %s\n", SDL_GetError()); return 1; }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    win = SDL_CreateWindow("NEON ARENA - WASD + Maus, Klick/Space = Schiesen, ESC = Ende",
                           SDL_WINDOWPOS_CENTERED_DISPLAY(0), SDL_WINDOWPOS_CENTERED_DISPLAY(0),
                           WIN_W, WIN_H, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
    if (!win) { fprintf(stderr,"window: %s\n", SDL_GetError()); return 1; }
    gl = SDL_GL_CreateContext(win);
    if (!gl) { fprintf(stderr,"gl context: %s\n", SDL_GetError()); return 1; }
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_GL_SetSwapInterval(1);

    Uint64 pc = SDL_GetPerformanceCounter();
    Uint64 prev = pc;
    while (running) {
        pc = SDL_GetPerformanceCounter();
        double dt = (double)(pc-prev)/SDL_GetPerformanceFrequency();
        if (dt > 0.1) dt = 0.1;
        prev = pc; now_s = t_now();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT: running=false; break;
            case SDL_KEYDOWN:
                if (ev.key.keysym.sym == SDLK_ESCAPE) running=false;
                break;
            case SDL_MOUSEMOTION:
                pyaw   += ev.motion.xrel * 0.0022f;
                ppitch -= ev.motion.yrel * 0.0022f;
                if (ppitch >  1.45f) ppitch =  1.45f;
                if (ppitch < -1.45f) ppitch = -1.45f;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT) shoot();
                break;
            }
        }
        update((float)dt);
        render();
    }

    printf("Exit clean. Final score: %d\n", score);
    if (gl) SDL_GL_DeleteContext(gl);
    if (win) SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

// ---- perspective helper (no GLU dependency) ----
void glu_perspective(double fovy,double aspect,double zn,double zf) {
    double fh = tan(fovy/360.0*M_PI)*zn, fw = fh*aspect;
    glFrustum(-fw,fw,-fh,fh,zn,zf);
}
