#define SDL_MAIN_HANDLED
#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<cstdlib>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define FPS_CAP             60
#define SCREEN_WIDTH        800
#define SCREEN_HEIGHT       600
#define HEADER_HEIGHT       36
#define PLANTS_CONSTRAINT   10
#define NPCS_CONSTRAINT     10
#define ENEMIES_CONSTRAINT  4
#define BULLETS_CONSTRAINT  10
#define ROCKETS_CONSTRAINT  3
#define GRASS_WIDTH         SCREEN_WIDTH / 4 - 10
#define ENEMY_DISPLAY_RANGE SCREEN_HEIGHT * 5
#define NPC_DISPLAY_RANGE   SCREEN_HEIGHT * 3
#define PWRUP_DISPLAY_RANGE  SCREEN_HEIGHT * 3
#define PWRUP_DIST          10
#define ACCELERATION        8
#define MAX_SPEED           1000
#define MIN_SPEED           400
#define INITIAL_SPEED       500
#define SCORE_PER_TICK      1
#define SCORE_BONUS         50
#define FREEZE_TIME         5 // in seconds
#define SAFE_DISTANCE       100
#define ATTACK_DISTANCE     200
#define PLAYER_LIVES        3
#define NO_COLLISION_TIME   3
#define ROCKET_AMMO         3
#define STND_RANGE          200
#define ROCKET_RANGE        400

struct Player {
    int lives = PLAYER_LIVES, alive = 1, no_coll = 0;
    double speed = INITIAL_SPEED, turning_spd = 5, nc_time = 0;
    double pos_x = SCREEN_WIDTH / 2, pos_y = SCREEN_HEIGHT * 2 / 3, width = 20, height = 40;
    float score = 0;
    SDL_Surface* surface = SDL_LoadBMP("./bitmaps/car1.bmp");
    SDL_Surface* icon = SDL_LoadBMP("./bitmaps/smallcar1.bmp");
};

struct Enemy {
    double speed = 600, pos_x = 0, pos_y = 0, width = 25, height = 45, turning_spd = 0.3;
    SDL_Surface* surface = SDL_LoadBMP("./bitmaps/enemy1.bmp");
};

struct NPC_Car {
    double speed = 0, pos_x = 0, pos_y = 0, width = 25, height = 45;
    SDL_Surface* surface = SDL_LoadBMP("./bitmaps/npcCar.bmp");
};

struct Plant {
    double pos_x = 0, pos_y = 0, width = 80;
    SDL_Surface* surface;
};

struct Bullet {
    double speed = 1000;
    double initial_y = 0, pos_x = 0, pos_y = 0, width = 10, height = 10;
    double range = STND_RANGE;
    int appear = 0;
    SDL_Surface* surface_bullet = SDL_LoadBMP("./bitmaps/bullet.bmp");
    SDL_Surface* surface_rocket = SDL_LoadBMP("./bitmaps/rocket.bmp");
    SDL_Surface* surface = surface_bullet;
};

struct PowerUp {
    double pos_x = 0, pos_y = 0, width = 20, height = 20;
    SDL_Surface* surface = SDL_LoadBMP("./bitmaps/rocketIcon.bmp");
};

struct Colors {
    int red, green, blue, black;
};

void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
    // draw a text txt on surface screen, starting from the point (x, y)
    // charset is a 128x128 bitmap containing character images
    int px, py, c;
    SDL_Rect s, d;
    s.w = 8;
    s.h = 8;
    d.w = 8;
    d.h = 8;
    while (*text) {
        c = *text & 255;
        px = (c % 16) * 8;
        py = (c / 16) * 8;
        s.x = px;
        s.y = py;
        d.x = x;
        d.y = y;
        SDL_BlitSurface(charset, &s, screen, &d);
        x += 8;
        text++;
    };
};


void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
    // draw a surface sprite on a surface screen in point (x, y)
    // (x, y) is the center of sprite on screen
    SDL_Rect dest;
    dest.x = x - sprite->w / 2;
    dest.y = y - sprite->h / 2;
    dest.w = sprite->w;
    dest.h = sprite->h;
    SDL_BlitSurface(sprite, NULL, screen, &dest);
};


void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
    // draw a single pixel
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32*)p = color;
};


void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
    // draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
    for (int i = 0; i < l; i++) {
        DrawPixel(screen, x, y, color);
        x += dx;
        y += dy;
    };
};


void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
    // draw a rectangle of size l by k
    int i;
    DrawLine(screen, x, y, k, 0, 1, outlineColor);
    DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
    DrawLine(screen, x, y, l, 1, 0, outlineColor);
    DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
    for (i = y + 1; i < y + k - 1; i++)
        DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};


void DestroyScreen(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Window* window) {
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(scrtex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


void FreeSurfaces(SDL_Surface* charset, Player* player1, PowerUp* pwrUp, Bullet* bullet, Enemy* enemy, NPC_Car* npc, Plant* plant) {
    SDL_FreeSurface(charset);
    SDL_FreeSurface((*player1).surface);
    SDL_FreeSurface((*pwrUp).surface);
    for (int i = 0; i < BULLETS_CONSTRAINT; i++)
        SDL_FreeSurface((*(bullet + i)).surface);
    for (int i = 0; i < ENEMIES_CONSTRAINT; i++)
        SDL_FreeSurface((*(enemy + i)).surface);
    for (int i = 0; i < NPCS_CONSTRAINT; i++)
        SDL_FreeSurface((*(npc + i)).surface);
    for (int i = 0; i < PLANTS_CONSTRAINT; i++)
        SDL_FreeSurface((*(plant + i)).surface);
}


int CheckLoadingError(char* name, SDL_Surface* screen, SDL_Surface* srfc, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Window* window) {
    if (srfc == NULL) {
        printf("loading %s surface error: %s\n", name, SDL_GetError());
        DestroyScreen(screen, scrtex, renderer, window);
        SDL_Quit();
        return 1;
    };
    return 0;
}


void DisplayUpperInfo(SDL_Surface* screen, SDL_Surface* charset, Colors palette, Player player1, double time, double fps, int frozen) {
    // function for displaying upper info text
    char text[128];
    DrawRectangle(screen, 0, 0, SCREEN_WIDTH, 56, palette.red, palette.blue);
    // player's speed is multiplied by 2/10 just to imitate real speed in km/h
    sprintf(text, "Speed: %.0lf km/h  Score: %.0lf  Time elapsed: %.1lf s  %.0lf FPS", player1.speed * 2 / 10, player1.score, time, fps);
    DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 35, text, charset);
    // draw icons representing how many lives the player has
    for (int i = 0; i < player1.lives; i++) DrawSurface(screen, player1.icon, (SCREEN_WIDTH - 130) + 30 * i, 30);
    if (frozen) DrawString(screen, 50, 20, "SCORE FROZEN!", charset);
}


void DisplayPauseInfo(SDL_Surface* screen, SDL_Surface* charset, Colors palette) {
    // draw rectangle informing about pause
    DrawRectangle(screen, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 3 - 50, 200, 56, palette.red, palette.blue);
    DrawString(screen, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 3 - 25, "GAME PAUSED", charset);
}


void GenerateGrassland(SDL_Surface* screen, Colors palette, Player player1, Plant* plant, double delta, int paused) {
    int random_x;
    // draw grass on the left and right side of the road
    DrawRectangle(screen, 0, 36, GRASS_WIDTH, SCREEN_HEIGHT - HEADER_HEIGHT, palette.green, palette.green);
    DrawRectangle(screen, SCREEN_WIDTH * 3 / 4 + 10, 36, GRASS_WIDTH, SCREEN_HEIGHT - HEADER_HEIGHT, palette.green, palette.green);

    // draw plants on the grass and 'generate new ones
    for (int i = 0; i < PLANTS_CONSTRAINT; i++) {
        if (!paused) (*(plant + i)).pos_y += delta * player1.speed;
        if ((*(plant + i)).pos_y > SCREEN_HEIGHT) {
            //When a plant goes below the screen, it will be respawned in a random x position.
            //The following condition will also randomly determine
            //whether a plant is spawned on the left or the right side of the road.
            random_x = rand() % (GRASS_WIDTH - 40);
            if (rand() % 2) (*(plant + i)).pos_x = random_x;
            else (*(plant + i)).pos_x = random_x + SCREEN_WIDTH * 3 / 4 + 40;
            (*(plant + i)).pos_y = 0 + HEADER_HEIGHT;
        }
        DrawSurface(screen, (*(plant + i)).surface, (*(plant + i)).pos_x, (*(plant + i)).pos_y);
    }
}


int CheckCollision(double pos_y1, double pos_x1, double w1, double h1, double pos_y2, double pos_x2, double w2, double h2) {
    // Check if object1 collides with object2. 
    // h1, h2 - height, w1, w2 - width
    if ((pos_y1 - h1 / 2) < (pos_y2 + h2 / 2) && (pos_y1 + h1 / 2) > (pos_y2 - h2 / 2) &&
        (pos_x1 - w1 / 2) < (pos_x2 + w2 / 2) && (pos_x1 + w1 / 2) > (pos_x2 - w2 / 2)) {
        return 1;
    }
    else return 0;
}


double RandomSpeed() {
    // random speed for npc cars
    double speed = 150 + (rand() % 20) * 10;
    return speed;
}


void RandomCoords(double* x, double* y, int car_width) {
    // random coords for objects on the road, i.e. cars nad powerups
    *x = GRASS_WIDTH + car_width / 2 + rand() % (2 * GRASS_WIDTH - car_width / 2);
    *y = (rand() % 15) * (rand() % 15) * 100 * (-1);
}


void InitCoords(Plant* plant, NPC_Car* npc, Enemy* enemy, PowerUp* pwrUp) {
    // generate random coords for all objects
    for (int i = 0; i < PLANTS_CONSTRAINT; i++) {
        // randomly determine if it's spawning on the right or left side of the road
        if (i < PLANTS_CONSTRAINT / 2) (*(plant + i)).pos_x = rand() % (GRASS_WIDTH - 40);
        else (*(plant + i)).pos_x = rand() % (GRASS_WIDTH - 40) + SCREEN_WIDTH * 3 / 4;
        (*(plant + i)).pos_y = rand() % SCREEN_HEIGHT;
    }
    for (int i = 0; i < NPCS_CONSTRAINT; i++) {
        RandomCoords(&(*(npc + i)).pos_x, &(*(npc + i)).pos_y, (*(npc + i)).width);
        (*(npc + i)).speed = RandomSpeed();
    }
    for (int i = 0; i < ENEMIES_CONSTRAINT; i++)
        RandomCoords(&(*(enemy + i)).pos_x, &(*(enemy + i)).pos_y, (*(enemy + i)).width);
    RandomCoords(&(*pwrUp).pos_x, &(*pwrUp).pos_y, (*pwrUp).width);
}


void ResetGame(Player* player1, PowerUp* pwrUp, Bullet* bullet, Plant* plant, NPC_Car* npc, Enemy* enemy, double* time) {
    (*player1).alive = 1;
    (*player1).no_coll = 0;
    (*player1).lives = PLAYER_LIVES;
    (*player1).score = 0;
    (*player1).speed = INITIAL_SPEED;
    (*player1).pos_x = SCREEN_WIDTH / 2;
    *time = 0;
    for (int i = 0; i < BULLETS_CONSTRAINT; i++) {
        (*(bullet + i)).surface = (*(bullet + i)).surface_bullet;
    }
    InitCoords(plant, npc, enemy, pwrUp);
}


void Crash(Player* player1) {
    (*player1).no_coll = 1;
    (*player1).lives--;
    // reset position and speed of the player
    (*player1).pos_x = SCREEN_WIDTH / 2;
    (*player1).speed = INITIAL_SPEED;
}


void GameOver(SDL_Surface* screen, SDL_Surface* charset, Player* player1) {
    (*player1).alive = 0;
    (*player1).speed = 0;
    DrawString(screen, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 50, "GAME OVER", charset);
    DrawString(screen, SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2, "press ESC to quit", charset);
    DrawString(screen, SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 20, "press 'n' to restart", charset);
}


void ManageNoCollision(SDL_Surface* screen, SDL_Surface* charset, Player* player1, double delta, int paused) {
    // after player crashes and loses 1 life, they have a few seconds to avoid immidiate collision after respawning
    if ((*player1).no_coll && (*player1).lives != 0) {
        if (!paused) (*player1).nc_time += delta;
        DrawString(screen, 50, 40, "NO COLLISION!", charset);
    }
    if ((*player1).nc_time >= NO_COLLISION_TIME) {
        (*player1).no_coll = 0;
        (*player1).nc_time = 0;
    }
}


void PlayerMove(SDL_Surface* screen, Player* player1, PowerUp* pwrUp, Bullet* bullet, Plant* plant, Enemy* enemy, NPC_Car* npc,
    int bullet_iter, double* time, int* to_be_frozen) {
    // check collisions of player's car with others and draw the car on the screen

    if ((*player1).alive) {
        // when player's car drives off the road it crashes and loses 1 life
        if ((*player1).pos_x < GRASS_WIDTH || (*player1).pos_x + (*player1).width > SCREEN_WIDTH * 3 / 4 + 40) {
            Crash(player1);
        }
        // collision with enemies
        for (int i = 0; i < ENEMIES_CONSTRAINT; i++) {
            if (!(*player1).no_coll && CheckCollision((*player1).pos_y, (*player1).pos_x, (*player1).width, (*player1).height,
                (*(enemy + i)).pos_y, (*(enemy + i)).pos_x, (*(enemy + i)).width, (*(enemy + i)).height)) {
                Crash(player1);
            }
        }
        // collision with npcs
        for (int i = 0; i < NPCS_CONSTRAINT; i++) {
            if (!(*player1).no_coll && CheckCollision((*player1).pos_y, (*player1).pos_x, (*player1).width, (*player1).height,
                (*(npc + i)).pos_y, (*(npc + i)).pos_x, (*(npc + i)).width, (*(npc + i)).height)) {
                RandomCoords(&(*(npc + i)).pos_x, &(*(npc + i)).pos_y, (*(npc + i)).width);
                *to_be_frozen = 1;
            }
        }
        // 'collision' with powerup
        if (CheckCollision((*player1).pos_y, (*player1).pos_x, (*player1).width, (*player1).height,
            (*pwrUp).pos_y, (*pwrUp).pos_x, (*pwrUp).width, (*pwrUp).height)) {
            RandomCoords(&(*pwrUp).pos_x, &(*pwrUp).pos_y, (*pwrUp).width);
            (*pwrUp).pos_y *= PWRUP_DIST;
            for (int i = 0; i < ROCKET_AMMO; i++) {
                (*(bullet + bullet_iter + i)).surface = (*bullet).surface_rocket;
                (*(bullet + bullet_iter + i)).range = ROCKET_RANGE;
            }
        }
        DrawSurface(screen, (*player1).surface, (*player1).pos_x, (*player1).pos_y);
    }
}


void PowerUpMove(SDL_Surface* screen, PowerUp* pwrUp, Player player1, int paused, double delta) {
    // move and draw powerups on the screen
    if (!paused) (*pwrUp).pos_y += delta * player1.speed;
    DrawSurface(screen, (*pwrUp).surface, (*pwrUp).pos_x, (*pwrUp).pos_y);
    if ((*pwrUp).pos_y > PWRUP_DISPLAY_RANGE) {
        RandomCoords(&(*pwrUp).pos_x, &(*pwrUp).pos_y, (*pwrUp).width);
        (*pwrUp).pos_y *= PWRUP_DIST;
    }
}


void NPCMove(SDL_Surface* screen, NPC_Car* npc, double player_speed, double delta, int paused) {
    // move and draw npc cars on the screen
    double distance;
    for (int i = 0; i < NPCS_CONSTRAINT; i++) {
        // slow down to avoid collision with other npcs
        for (int j = 0; j < NPCS_CONSTRAINT; j++) {
            if (j != i) {
                distance = (*(npc + i)).pos_y - (*(npc + j)).pos_y;
                if (distance < SAFE_DISTANCE + (*(npc + i)).height / 2 && distance > 0) (*(npc + i)).speed = (*(npc + j)).speed;
            }
        }
        for (int j = 0; j < NPCS_CONSTRAINT; j++) {
            if (j != i) {
                if (CheckCollision((*(npc + i)).pos_y, (*(npc + i)).pos_x, (*(npc + i)).width, (*(npc + i)).height,
                    (*(npc + j)).pos_y, (*(npc + j)).pos_x, (*(npc + j)).width, (*(npc + j)).height)) {
                    RandomCoords(&(*(npc + i)).pos_x, &(*(npc + i)).pos_y, (*(npc + i)).width);
                }
            }
        }
        // determining y position on the screen, dependent of player's speed
        if (!paused) (*(npc + i)).pos_y = (*(npc + i)).pos_y + (player_speed - (*(npc + i)).speed) * delta;
        // respawning if it's too far behind the player
        if ((*(npc + i)).pos_y > NPC_DISPLAY_RANGE) {
            RandomCoords(&(*(npc + i)).pos_x, &(*(npc + i)).pos_y, (*(npc + i)).width);
            (*(npc + i)).speed = RandomSpeed();
        }
        DrawSurface(screen, (*(npc + i)).surface, (*(npc + i)).pos_x, (*(npc + i)).pos_y);
    }
}


void EnemiesMove(SDL_Surface* screen, Player player1, Enemy* enemy, NPC_Car* npc, double delta, int paused) {
    // move and draw enemies on the screen
    for (int i = 0; i < ENEMIES_CONSTRAINT; i++) {
        // determining y position on the screen, dependent of player's speed
        if (!paused) (*(enemy + i)).pos_y = (*(enemy + i)).pos_y + (player1.speed - (*(enemy + i)).speed) * delta;
        // 'attacking' the player
        // enemy attacks the player if they're close
        if (!paused) {
            if (player1.pos_y - (*(enemy + i)).pos_y < ATTACK_DISTANCE) {
                if ((*(enemy + i)).pos_x > player1.pos_x) (*(enemy + i)).pos_x -= (*(enemy + i)).turning_spd;
                else if ((*(enemy + i)).pos_x < player1.pos_x) (*(enemy + i)).pos_x += (*(enemy + i)).turning_spd;
            }
        }
        // check collision with npcs
        for (int j = 0; j < NPCS_CONSTRAINT; j++) {
            if (CheckCollision((*(enemy + i)).pos_y, (*(enemy + i)).pos_x, (*(enemy + i)).width, (*(enemy + i)).height,
                (*(npc + j)).pos_y, (*(npc + j)).pos_x, (*(npc + j)).width, (*(npc + j)).height))
                RandomCoords(&(*(npc + j)).pos_x, &(*(npc + j)).pos_y, (*(npc + j)).width);
        }
        // respawning if it's too far behind the player
        if ((*(enemy + i)).pos_y > ENEMY_DISPLAY_RANGE) RandomCoords(&(*(enemy + i)).pos_x, &(*(enemy + i)).pos_y, (*(enemy + i)).width);

        DrawSurface(screen, (*(enemy + i)).surface, (*(enemy + i)).pos_x, (*(enemy + i)).pos_y);
    }
}


void BulletsMove(SDL_Surface* screen, Player* player1, Enemy* enemy, NPC_Car* npc, Bullet* bullet, double delta, int paused, int* to_be_frozen) {
    // move and draw bullets, check collisions
    for (int i = 0; i < BULLETS_CONSTRAINT; i++) {
        if ((*(bullet + i)).initial_y - (*(bullet + i)).pos_y > (*(bullet + i)).range) {
            if ((*(bullet + i)).surface = (*(bullet + i)).surface_rocket) {
                (*(bullet + i)).surface = (*(bullet + i)).surface_bullet;
                (*(bullet + i)).range = STND_RANGE;
            }
            (*(bullet + i)).appear = 0;
        }
        if (!paused && (*(bullet + i)).appear) {
            (*(bullet + i)).pos_y = (*(bullet + i)).pos_y + (((*player1).speed - (*(bullet + i)).speed) - (*(bullet + i)).speed) * delta;
            // check collisions with npcs
            for (int j = 0; j < NPCS_CONSTRAINT; j++) {
                if (CheckCollision((*(bullet + i)).pos_y, (*(bullet + i)).pos_x, (*(bullet + i)).width, (*(bullet + i)).height,
                    (*(npc + j)).pos_y, (*(npc + j)).pos_x, (*(npc + j)).width, (*(npc + j)).height)) {
                    RandomCoords(&(*(npc + j)).pos_x, &(*(npc + j)).pos_y, (*(npc + j)).width);
                    (*(npc + j)).speed = RandomSpeed();
                    *to_be_frozen = 1;
                    (*(bullet + i)).appear = 0;
                }
            }
            // check collisions with enemies
            for (int j = 0; j < ENEMIES_CONSTRAINT; j++) {
                if (CheckCollision((*(bullet + i)).pos_y, (*(bullet + i)).pos_x, (*(bullet + i)).width, (*(bullet + i)).height,
                    (*(enemy + j)).pos_y, (*(enemy + j)).pos_x, (*(enemy + j)).width, (*(enemy + j)).height)) {
                    RandomCoords(&(*(enemy + j)).pos_x, &(*(enemy + j)).pos_y, (*(enemy + j)).width);
                    if (!(*to_be_frozen)) (*player1).score += SCORE_BONUS;
                    (*(bullet + i)).appear = 0;
                }
            }
            DrawSurface(screen, (*(bullet + i)).surface, (*(bullet + i)).pos_x, (*(bullet + i)).pos_y);
        }
    }
}


void IncreaseScore(Player* player1, int* to_be_frozen, double* frozen_time, double delta) {
    // score increasing (dependent of time and speed of the car), doesn't increase when it's 'frozen'
    if (*to_be_frozen) (*frozen_time) += delta;
    if ((*frozen_time) >= FREEZE_TIME) {
        *to_be_frozen = 0;
        *frozen_time = 0;
    }
    if (!(*to_be_frozen)) (*player1).score += (SCORE_PER_TICK * delta) * ((*player1).speed * 0.01);
}


void Shoot(Player* player1, Bullet* bullet_ptr, int* bullet_iter) {
    // initiate bullets after shooting, and move bullet iterator to allow next bullet to be shot
    if ((*player1).alive) {
        (*(bullet_ptr + *bullet_iter)).pos_x = (*player1).pos_x;
        (*(bullet_ptr + *bullet_iter)).pos_y = (*player1).pos_y;
        (*(bullet_ptr + *bullet_iter)).initial_y = (*player1).pos_y;
        (*(bullet_ptr + *bullet_iter)).appear = 1;
        (*bullet_iter)++;
        if (*bullet_iter == BULLETS_CONSTRAINT) (*bullet_iter) = 0;
    }
}


void HandleInput(Player* player1, PowerUp* pwrUp, Plant* plant, Enemy* enemy, NPC_Car* npc, Bullet* bullet,
    int* bullet_iter, int* quit, double* time, int* paused) {
    // steering the car
    const Uint8* key = SDL_GetKeyboardState(NULL);
    if (!(*paused) && (*player1).alive) {
        if (key[SDL_SCANCODE_UP]) {
            if ((*player1).speed < MAX_SPEED) (*player1).speed += ACCELERATION;
        }
        if (key[SDL_SCANCODE_DOWN]) {
            if ((*player1).speed > MIN_SPEED) (*player1).speed -= ACCELERATION;
        }
        if (key[SDL_SCANCODE_LEFT]) (*player1).pos_x = (*player1).pos_x - (*player1).turning_spd;
        if (key[SDL_SCANCODE_RIGHT]) (*player1).pos_x = (*player1).pos_x + (*player1).turning_spd;
    }
    SDL_Event event;

    // other functionalities
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) *quit = 1;
            else if (event.key.keysym.sym == SDLK_SPACE) Shoot(player1, bullet, bullet_iter);
            else if (event.key.keysym.sym == 'p') {
                if (*paused == 0) *paused = 1;
                else *paused = 0;
            }
            else if (event.key.keysym.sym == 'n') ResetGame(player1, pwrUp, bullet, plant, npc, enemy, time);
            break;
        case SDL_KEYUP:
            break;
        case SDL_QUIT:
            *quit = 1;
            break;
        };
    }
}


int HandleGameplay(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Texture* scrtex, SDL_Surface* charset, Colors palette,
    Player* player1, PowerUp* pwrUp, Bullet* bullet, Enemy* enemy, NPC_Car* npc, Plant* plant) {
    int t1, t2, quit = 0, frames = 0, to_be_frozen = 0, paused = 0, bullet_iter = 0;
    double delta, random_x, worldTime = 0, fpsTimer = 0, frozen_time = 0, fps = 0;
    const int frameDelay = 1000 / FPS_CAP; // Calculate the frame delay in milliseconds
    t1 = SDL_GetTicks();
    while (!quit) {
        t2 = SDL_GetTicks();
        // here t2-t1 is the time in milliseconds since
        // the last screen was drawn
        // delta is the same time in seconds
        delta = (t2 - t1) * 0.001;
        t1 = t2;
        if (!paused) worldTime += delta;
        if (!paused) IncreaseScore(player1, &to_be_frozen, &frozen_time, delta);
        SDL_FillRect(screen, NULL, palette.black);
        GenerateGrassland(screen, palette, *player1, plant, delta, paused);
        PlayerMove(screen, player1, pwrUp, bullet, plant, enemy, npc, bullet_iter, &worldTime, &to_be_frozen);
        PowerUpMove(screen, pwrUp, *player1, paused, delta);
        NPCMove(screen, npc, (*player1).speed, delta, paused);
        EnemiesMove(screen, *player1, enemy, npc, delta, paused);
        BulletsMove(screen, player1, enemy, npc, bullet, delta, paused, &to_be_frozen);

        fpsTimer += delta;
        if (fpsTimer > 0.5) {
            fps = frames * 2;
            frames = 0;
            fpsTimer -= 0.5;
        }

        DisplayUpperInfo(screen, charset, palette, *player1, worldTime, fps, to_be_frozen);
        if (to_be_frozen) DrawString(screen, 50, 20, "SCORE FROZEN!", charset);
        ManageNoCollision(screen, charset, player1, delta, paused);
        if (paused) DisplayPauseInfo(screen, charset, palette);
        if ((*player1).lives == 0) GameOver(screen, charset, player1);

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
        HandleInput(player1, pwrUp, plant, enemy, npc, bullet, &bullet_iter, &quit, &worldTime, &paused);
        frames++;

        // cap the frame rate
        int frameTime = SDL_GetTicks() - t1;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    };
    return 1;
}


// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
    char text[128];
    SDL_Surface* screen, * charset;
    SDL_Texture* scrtex;
    SDL_Window* window;
    SDL_Renderer* renderer;
    Plant plants[PLANTS_CONSTRAINT];
    NPC_Car npcs[NPCS_CONSTRAINT];
    Enemy enemies[ENEMIES_CONSTRAINT];
    Bullet bullets[BULLETS_CONSTRAINT];
    Bullet rockets[ROCKETS_CONSTRAINT];
    PowerUp pwrUp;
    Player player1;
    Colors palette;

    // console window is not visible, to see the printf output
    // the option:
    // project -> szablon2 properties -> Linker -> System -> Subsystem
    // must be changed to "Console"
    printf("printf output goes here\n");

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    int rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    if (rc != 0) {
        SDL_Quit();
        printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
        return 1;
    };

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_SetWindowTitle(window, "Spy Hunter by Bartosz Sobczak");

    screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    palette.black = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    palette.green = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
    palette.red = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
    palette.blue = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

    // loading charset
    charset = SDL_LoadBMP("./bitmaps/cs8x8.bmp");
    if (CheckLoadingError("charset", screen, charset, scrtex, renderer, window)) return 1;
    SDL_SetColorKey(charset, true, 0x000000);

    // loading plants, select one of the plant types
    for (int i = 0; i < PLANTS_CONSTRAINT; i++) {
        if (i % 3 == 0) plants[i].surface = SDL_LoadBMP("./bitmaps/tree1.bmp");
        if (i % 3 == 1) plants[i].surface = SDL_LoadBMP("./bitmaps/tree2.bmp");
        if (i % 3 == 2) plants[i].surface = SDL_LoadBMP("./bitmaps/plant1.bmp");

        if (plants[i].surface == NULL) {
            switch (i % 3) {
            case 1:
                printf("loading tree1 surface error: %s\n", SDL_GetError());
                break;
            case 2:
                printf("loading tree2 surface error: %s\n", SDL_GetError());
                break;
            case 3:
                printf("loading tree3 surface error: %s\n", SDL_GetError());
                break;
            }
            DestroyScreen(screen, scrtex, renderer, window);
            SDL_Quit();
        }
    }

    // check for loading errors for other sprites
    if (CheckLoadingError("player1 car", player1.surface, screen, scrtex, renderer, window)) return 1;
    if (CheckLoadingError("power up", pwrUp.surface, screen, scrtex, renderer, window)) return 1;
    for (int i = 0; i < NPCS_CONSTRAINT; i++) {
        if (CheckLoadingError("npc car", npcs[i].surface, screen, scrtex, renderer, window)) return 1;
    }
    for (int i = 0; i < ENEMIES_CONSTRAINT; i++) {
        if (CheckLoadingError("enemy car", enemies[i].surface, screen, scrtex, renderer, window)) return 1;
    }
    for (int i = 0; i < BULLETS_CONSTRAINT; i++) {
        if (CheckLoadingError("bullet", bullets[i].surface_bullet, screen, scrtex, renderer, window)) return 1;
        if (CheckLoadingError("bullet", bullets[i].surface_rocket, screen, scrtex, renderer, window)) return 1;
    }

    InitCoords(&plants[0], &npcs[0], &enemies[0], &pwrUp);
    HandleGameplay(screen, renderer, scrtex, charset, palette, &player1, &pwrUp, &bullets[0], &enemies[0], &npcs[0], &plants[0]);
    FreeSurfaces(charset, &player1, &pwrUp, &bullets[0], &enemies[0], &npcs[0], &plants[0]);
    DestroyScreen(screen, scrtex, renderer, window);
    SDL_Quit();
    return 0;
};