#ifndef ACTEURS_H
#define ACTEURS_H

#include "structures.h"

// fonction concernant avl acteur pour index

int hauteur_avl_acteur(AVLActeur *n);

int facteur_equilibre_avl_acteur(AVLActeur *n);

AVLActeur *rotation_droite_acteur(AVLActeur *y);

AVLActeur *rotation_gauche_acteur(AVLActeur *x); // 4 premieres fonction equilibre

AVLActeur *creer_noeud_avl_acteur(const char *id, Acteur *acteur); // alloue esapce et créé

AVLActeur *avl_inserer_acteur(AVLActeur *racine, char *id, Acteur *acteur); // insérer nouveau noeud sauf si deja existant

AVLActeur *avl_rechercher_acteur(AVLActeur *racine, char *id); // recherche acteur dans l'arbre et le retourne si trouvé

// lienacteur + nouedacteur

Acteur *trouver_ou_creer_acteur(AVLActeur **racine_index, char *id); // trouve acteur ou le créé et ajoute index

void ajouter_enfant_acteur(Acteur *parent, Acteur *enfant, double pct_fuite); // ajoute enfant a la liste chainé des fils du parent (fuite et id)

void liberer_reseau_acteurs(AVLActeur *racine); // libère tout les structures

// calcul fuites 

void maj_max_troncon(MaxFuiteTroncon* maxT, char *id_amont, char *id_aval, double fuite);

void propager_fuites(Acteur *acteur, double vol_entree, double *fuites_totales, MaxFuiteTroncon* maxT); // calcul nbr fils acteur / l'eau et calcule pour chaque fils et ses fils lui meme... (recursivité)

double calculer_fuites_usine(char *id_usine, AVLUsine *racine_usines, AVLActeur *racine_index_acteurs, MaxFuiteTroncon* maxT); // retrouve l'usine et ses info ainsi que strct acteur et lance propagation

// traitement fichier et export

void traiter_une_ligne_leaks(char *ligne_brute, AVLUsine **racine_usines, AVLActeur **racine_index_acteurs, char *id_usine); // traite une ligne en analysant structure ligne, créé des noeuds et des liens entre acteurs

int exporter_fuites_usine(AVLUsine *racine_usines, char *id_usine, char *chemin_sortie_dat, double leaks, MaxFuiteTroncon* maxT); // exporte dzns historique les données demandé sauf si usine non trouvé

#endif
