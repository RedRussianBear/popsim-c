//
// Created by Misha on 9/15/2017.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include "SOIL/SOIL.h"
#include "main.h"

#define WIDTH 1357
#define HEIGHT 628
#define CLANS 11

GLuint background;
int num_clans = CLANS;
int num_families = 0;

int width = WIDTH;
int height = HEIGHT;
int refresh_mills = 30;

Plot* map[HEIGHT][WIDTH];
Clan* clans[CLANS];
List families;

Biome *Ocean, *Freshwater, *Rainforest, *Forest, *Taiga, *Savanna, *Grassland, *Desert, *Tundra, *Polar;


Family* addFamily(struct Clan* clan, int lat, int lon) {
    Family* new = newFamily(clan, lat, lon);
    Node* newNode = malloc(sizeof(Node));
    newNode->val = new;

    families.last->next = newNode;
    families.last = newNode;
    num_families++;

    newNode->next = NULL;

    return new;
}

Biome* newBiome(int capacity, int difficulty) {
    Biome* new = malloc(sizeof(Biome));

    new->capacity = capacity;
    new->difficulty = difficulty;

    return new;
}

Clan * newClan(int id, struct SimColor *color, int lat, int lon) {
    Clan* new = malloc(sizeof(Clan));

    new->color = color;
    new->id = id;
    new->spawn_lat = lat;
    new->spawn_lon = lon;

    return new;
}

Family* newFamily(struct Clan* clan, int lat, int lon) {
    Family* new = malloc(sizeof(Family));

    new->clan = clan;
    new->lat = lat;
    new->lon = lon;

    new->strength = 8;
    new->energy = 8;

    return new;
}

Plot* newPlot(struct Biome *biome, int lat, int lon) {
    Plot* new = malloc(sizeof(Plot));

    new->biome = biome;
    new->max_capacity = biome->capacity;
    new->capacity = biome->capacity;
    new->lat = lat;
    new->lon = lon;
    new->occupant = NULL;

    return new;
}

SimColor* newColor(int R, int G, int B) {
    SimColor* new = malloc(sizeof(SimColor));

    new->R = R;
    new->G = G;
    new->B = B;

    return new;
}

void cull() {
    while(((Family*)(families.first->val))->strength < 2) {
        printf("Culling head\n");
        Node* tmp = families.first;
        families.first = families.first->next;

        free(((Family *)tmp->val));
        free(tmp);
    }

    Node* nextnode = families.first;
    while(nextnode != NULL) {
        Node* current = nextnode;
        nextnode = nextnode->next;
        if(nextnode == NULL) break;
        Family* next_family = (Family *) nextnode->val;


        if(next_family->strength < 2) {
            printf("Culling\n");
            current->next = nextnode->next;
            if(nextnode->next == NULL) families.last = current;
            free(next_family);
            free(nextnode);

            nextnode = current->next;

        }

    }
}

void updateFamily(Family* self) {
    Plot* plot = map[self->lat][self->lon];

    self->strength *= 1.5;
    improve(plot);

    if(self->strength > plot->capacity && self->strength > 4) {
        int max_capacity = -1;
        Plot *moveto = NULL;

        char d[4][2] = {{-1, 0},
                        {0,  -1},
                        {1,  0},
                        {0,  1}};

        int i;
        for (i = 0; i < 4; i++) {
            if(d[i][0] + self->lat >= HEIGHT) continue;
            if(d[i][0] + self->lat < 0) continue;
            if(d[i][1] + self->lon >= WIDTH) continue;
            if(d[i][1] + self->lon < 0) continue;

            Plot *new_plot = map[d[i][0] + self->lat][d[i][1] + self->lon];

            if (new_plot->occupant == NULL && new_plot->capacity > max_capacity) {
                moveto = new_plot;
                max_capacity = new_plot->capacity;
            }
        }

        if (max_capacity > 0) {
            Family *new_family = addFamily(self->clan, moveto->lat, moveto->lon);
            new_family->strength = self->strength / 2;
            new_family->energy = new_family->strength - moveto->biome->difficulty;
            self->strength /= 2;
            moveto->occupant = new_family;
        }
    }

    int consume = self->strength;
    int energy = self->strength;

    int consumetmp = consume;
    consume = consume - plot->capacity > 0 ? consume - plot->capacity : 0;
    plot->capacity = plot->capacity - consumetmp > 0 ? plot->capacity - consumetmp : 0;

    while(consume > 0 && energy > 0) {
        energy = wander(self, energy);
        plot = map[self->lat][self->lon];

        consumetmp = consume;
        consume = consume - plot->capacity > 0 ? consume - plot->capacity : 0;
        plot->capacity = plot->capacity - consumetmp > 0 ? plot->capacity - consumetmp : 0;
    }

    if(consume > 0) self->strength -= consume;

    self->energy= self->strength;


}

int wander(Family* self, int energy) {
    int max_capacity = -1;
    Plot* moveto = NULL;

    char d[4][2] = {{-1,0},{0,-1},{1,0},{0,1}};
    int i;
    for(i = 0; i < 4; i++) {
        if(d[i][0] + self->lat >= HEIGHT) continue;
        if(d[i][0] + self->lat < 0) continue;
        if(d[i][1] + self->lon >= WIDTH) continue;
        if(d[i][1] + self->lon < 0) continue;

        Plot* new_plot = map[d[i][0] + self->lat][d[i][1] + self->lon];

        if(new_plot->occupant == NULL && new_plot->capacity > max_capacity){
            moveto = new_plot;
            max_capacity = new_plot->capacity;
        }
    }

    if(moveto != NULL) {
        map[self->lat][self->lon]->occupant = NULL;

        self->lat = moveto->lat;
        self->lon = moveto->lon;

        map[self->lat][self->lon]->occupant = self;
        return energy - map[self->lat][self->lon]->biome->difficulty;
    }
    else return -1;
}

void updatePlot(Plot* self) {
    int new_val = self->capacity + 0.2 * self->max_capacity;
    self->capacity = new_val < self->max_capacity ? new_val : self->max_capacity;
}

void improve(Plot* self) {
    if(self->max_capacity == self->biome->capacity*2) return;
    self->max_capacity = (int) (self->max_capacity * 1.5 < self->biome->capacity * 2 ?
                                self->max_capacity * 1.5 : self->biome->capacity * 2);
}

void genWorld() {
    unsigned char* map_img = SOIL_load_image("C:\\Users\\Misha\\CLionProjects\\popsim-c\\world-map.png",
                                             &width, &height, 0, SOIL_LOAD_RGB);

    unsigned int i, id;
    for(i = 0; i < WIDTH*HEIGHT*3; i += 3) {
        id = (map_img[i] << 16) | (map_img[i+1] << 8) | (map_img[i+2]);
        Biome* biome;

        switch(id) {
            case OCEAN:
                biome = Ocean;
                break;
            case FRESHWATER:
                biome = Freshwater;
                break;
            case RAINFOREST:
                biome = Rainforest;
                break;
            case FOREST:
                biome = Forest;
                break;
            case TAIGA:
                biome = Taiga;
                break;
            case SAVANNA:
                biome = Savanna;
                break;
            case GRASSLAND:
                biome = Grassland;
                break;
            case DESERT:
                biome = Desert;
                break;
            case TUNDRA:
                biome = Tundra;
                break;
            case POLAR:
                biome = Polar;
                break;
            default:
                biome = Ocean;
                break;
        }

        map[(i/3)/WIDTH][(i/3)%WIDTH] = newPlot(biome, (i/3)/WIDTH, (i/3)%WIDTH);
    }

    SOIL_free_image_data(map_img);
}

void genClans() {
    int i = 0;
    SimColor* color = newColor((unsigned char)rand() %256, (unsigned char)rand() %256, (unsigned char)rand() %256);
    clans[i] = newClan(i, color, rand() %HEIGHT, rand() %WIDTH);
    Node* first_node = malloc(sizeof(Node));
    families.first = first_node;
    families.last = first_node;
    first_node->val = newFamily(clans[i], clans[i]->spawn_lat, clans[i]->spawn_lon);

    num_families++;

    for(i = 1; i < num_clans; i++) {
        color = newColor((unsigned char) rand() % 256, (unsigned char) rand() % 256, (unsigned char) rand() % 256);
        int lat = rand() % HEIGHT;
        int lon = rand() % WIDTH;

        while (map[lat][lon]->biome->difficulty > 10) {
            lat = (lat + 10) % HEIGHT;
            lon = (lon + 10) % WIDTH;
        }

        clans[i] = newClan(i, color, lat, lon);

        Family* init_family = addFamily(clans[i], clans[i]->spawn_lat, clans[i]->spawn_lon);
        map[init_family->lat][init_family->lon]->occupant = init_family;

    }
}

void update() {
    printf("Started Updating Plots\n");
    int i, j;
    for(i = 0; i < HEIGHT; i++) {
        for(j = 0; j < WIDTH; j++) {
            updatePlot(map[i][j]);
        }
    }
    printf("Finished Updating Plots\n");

    printf("Started Updating Families\n");
    Node* nextnode = families.first;
    while(nextnode != NULL) {
        Node* current = nextnode;
        nextnode = nextnode->next;

        Family* curfam = (Family*) (current->val);
        updateFamily(curfam);
    }
    printf("Finished Updating Families\n");

    cull();


    printf("Total families now: %d\n", num_families);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    printf("Started Logic\n");
    update();
    printf("Finished Logic\n");

    printf("Started Drawing Background\n");
    drawBackground();
    printf("Finished Drawing Background\n");
    printf("Started Drawing Families\n");
    drawFamilies();
    printf("Finished Drawing Families\n");


    glutSwapBuffers();
    printf("Swapped buffers\n");

    glutTimerFunc(refresh_mills, timer, 0);
}


void drawFamilies() {

    Node* curnode = families.first;
    while(curnode->next != NULL) {
        glBegin(GL_POINTS);
        curnode = curnode->next;
        Family* current = curnode->val;
        SimColor* color = current->clan->color;
        glColor3ub(color->R,color->G,color->B);
        glVertex2i(current->lon,current->lat);
        glEnd();
    }

}

void drawBackground() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, 1, -1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glColor4f(1,1,1,1);
    glBindTexture(GL_TEXTURE_2D, background);

    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(0, 0); // Upper left

    glTexCoord2f(1, 0);
    glVertex2f(WIDTH, 0); // Upper right

    glTexCoord2f(1, 1);
    glVertex2f(WIDTH, HEIGHT); // Lower right

    glTexCoord2f(0, 1);
    glVertex2f(0, HEIGHT); // Lower left
    glEnd();
    glPopMatrix();
}

void timer(int val) {
    glutPostRedisplay();      // Post re-paint request to activate display()
//    glutTimerFunc(refresh_mills, timer, 0); // next Timer call milliseconds later
}

void initGL() {
    // Set "clearing" or background color
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f); // Black and opaque
}

/* Main function: GLUT runs as a console application starting at main()  */
int main(int argc, char** argv) {
    glutInit(&argc, argv);                  // Initialize GLUT
    srand((unsigned int) time(NULL));


    glutInitWindowSize(WIDTH, HEIGHT);      // Set the window's initial width & height
    glutInitWindowPosition(50, 50);           // Position the window's initial top-left corner
    glutCreateWindow("Population Simulation");  // Create a window with the given title

    //Load our texture
    background = SOIL_load_OGL_texture(
            "C:\\Users\\Misha\\CLionProjects\\popsim-c\\world-map.png",
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_MULTIPLY_ALPHA
    );

    if(0 == background) printf( "SOIL loading error: '%s'\n", SOIL_last_result() );

    Ocean = newBiome(2, 50);
    Freshwater = newBiome(20, 1);
    Rainforest = newBiome(12, 20);
    Forest = newBiome(15, 5);
    Taiga = newBiome(8, 8);
    Savanna = newBiome(15, 2);
    Grassland = newBiome(20, 2);
    Desert = newBiome(2, 2);
    Tundra = newBiome(5, 2);
    Polar = newBiome(0, 2);

    genWorld();
    genClans();

    glutDisplayFunc(display);

    initGL();
    glutMainLoop();
    return 0;
}