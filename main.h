//
// Created by Misha on 9/15/2017.
//

#ifndef POPSIM_C_POPSIM_H
#define POPSIM_C_POPSIM_H

#define OCEAN       (0<<16) | (148<<8) | (255)
#define FRESHWATER  (164<<16) | (215<<8) | (252)
#define RAINFOREST  (0<<16) | (249<<8) | (20)
#define FOREST      (0<<16) | (187<<8) | (89)
#define TAIGA       (0<<16) | (136<<8) | (124)
#define SAVANNA     (255<<16) | (152<<8) | (0)
#define GRASSLAND   (230<<16) | (252<<8) | (121)
#define DESERT      (255<<16) | (213<<8) | (0)
#define TUNDRA      (151<<16) | (253<<8) | (248)
#define POLAR       (255<<16) | (255<<8) | (255)

typedef struct Family;
typedef struct Clan;
typedef struct SimColor;
typedef struct Biome;
typedef struct Plot;
typedef struct Node;
typedef struct List;


struct Family* newFamily(struct Clan* clan, int lat, int lon);
struct Clan* newClan(int number, struct SimColor* color, int lat, int lon);
struct SimColor* newColor(int R, int G, int B);
struct Plot* newPlot(struct Biome* biome, int lat, int lon);
struct Biome* newBiome(int capacity, int difficulty);

struct Family* addFamily(struct Clan* tribe, int lat, int lon);
int wander(struct Family* self, int energy);
void drawBackground(void);
void drawFamilies(void);

void display(void);
void update(void);
void timer(int val);
void cull();
void improve(struct Plot* self);
void genClans();
void genWorld();
void updateFamily(struct Family* self);
void updatePlot(struct Plot* self);

typedef struct SimColor {
    unsigned char R, G, B;
} SimColor;

typedef struct Family {
    struct Clan* clan;
    int lat, lon;
    int strength, energy;
} Family;

typedef struct Clan {
    int id;
    struct SimColor* color;
    int spawn_lat, spawn_lon;
} Clan;

typedef struct Biome {
    int capacity, difficulty;
} Biome;

typedef struct Plot {
    struct Biome* biome;
    struct Family* occupant;
    int lat, lon;
    int capacity, max_capacity;
} Plot;

typedef struct Node {
    struct Node* next;
    void *val;
} Node;

typedef struct List {
    Node *first, *last;
} List;

#endif //POPSIM_C_POPSIM_H
