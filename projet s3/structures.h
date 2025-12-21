#ifndef WILDWATER_TYPES_H
#define WILDWATER_TYPES_H

#define TAILLE_ID 128

typedef struct { // usine 
    char   id_usine[TAILLE_ID];
    double vol_capte_total;       // somme des volumes sources -> usine (avant fuites)
    double vol_achemine_total;    // somme des volumes arrivés à l’usine (après fuites)
    double vol_fuite_total;       // fuites source -> usine
    double capacite_max;          // capacité max de traitement
} StatsUsine;

typedef struct AVLUsine { // AVL usine
    StatsUsine      data; // donné usine
    int             hauteur; // equilibre
    struct AVLUsine *gauche; // fils gauche
    struct AVLUsine *droite; // fils droit
} AVLUsine;


typedef struct LienActeur { // permet lier parent->enfant
    struct Acteur *enfant;      // Pointeur vers l’acteur aval qui a lui meme sa liste d'enfant direct ...
    double       pct_fuite;   // pourcentage de fuites vers cette aval
    struct LienActeur  *suiv;   // pointe vers l'enfant d'après du parent (liste chainé)
} LienActeur;

typedef struct Acteur { // liste chainées acteurs aval avec id
    char       id[TAILLE_ID]; // id de l'acteur
    struct LienActeur *premier_enfant; // pointe vers le premier enfant direct (puis vers les autres avec suivant)
} Acteur;

typedef struct AVLActeur { // avl index rapide pour retrouver un acteur par id
    char              id[TAILLE_ID]; //Clé de recherche dans l’AVL
    struct Acteur      *acteur; //Pointe vers le vrai Acteur avec liste fils
    int               hauteur; // equilibre
    struct AVLActeur *gauche; // fils gauche
    struct AVLActeur *droite; // fils droit
} AVLActeur;

// L’AVL sert d’index pour retrouver rapidement un acteur a partir de son id 
// Acteur + LienActeur représente le réseau réel et permet de parcourir tous les acteurs en aval sans recherche
//On utilise l’AVL pour trouver le point de départ (usine), puis uniquement Acteur + LienActeur  pour calculer les fuites, ce qui est raccourci en temps

typedef struct { //bonus troncon avec plus grande fuite
    double fuite_max;    // fuite du troncon      
    char id_amont[TAILLE_ID]; //aval
    char id_aval[TAILLE_ID]; // amont
} MaxFuiteTroncon;


#endif