#ifndef USINES_H
#define USINES_H

#include "structures.h"
#include <stdio.h>

// fonction utiliser pour leak et histo

int max_int(int a, int b);

int est_tiret(char *s);

// fonction concernant AVLUsine

int hauteur_avl_usine(AVLUsine *n);

AVLUsine *rotation_droite(AVLUsine *y);

AVLUsine *rotation_gauche(AVLUsine *x);

int facteur_equilibre_avl_usine(AVLUsine *n);

AVLUsine *creer_noeud_usine(char *id_usine);

AVLUsine *avl_inserer_usine(AVLUsine *racine, char *id_usine);

AVLUsine *avl_rechercher_usine(AVLUsine *racine, char *id_usine);

void liberer_avl_usines(AVLUsine *r);

//traitement et exportation

void traiter_une_ligne_histo(char *ligne_brute, AVLUsine **racine_usines);

int traiter_fichier(char *nom_fichier, AVLUsine **racine_usines, AVLActeur **racine_index_acteurs, char *mode, char *opt_ou_id_usine); // commun a leaks et histo aussi

void exporter_orde_lexinv(AVLUsine *n, char *opt, FILE *f);

int exporter_histogramme(AVLUsine *racine_usines, char *opt, char *chemin_sortie_dat);

#endif
