#include "structures.h"
#include "leaks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


int max_int(int a, int b) {
    return (a > b) ? a : b; //retourne max
}

int est_tiret(char *s) { 
    if (*s == '-') return 1; //verifie que s est tiret
    return 0; 
}

// fonction de base équilibre et rotation

int hauteur_avl_usine(AVLUsine *n) {
    return n ? n->hauteur : 0; 
}

AVLUsine *rotation_droite(AVLUsine *y) {
    AVLUsine *x  = y->gauche;
    AVLUsine *T2 = x->droite;

    x->droite = y;
    y->gauche = T2;

    y->hauteur = 1 + max_int(hauteur_avl_usine(y->gauche), hauteur_avl_usine(y->droite));
    x->hauteur = 1 + max_int(hauteur_avl_usine(x->gauche), hauteur_avl_usine(x->droite));

    return x;
}

AVLUsine *rotation_gauche(AVLUsine *x) {
    AVLUsine *y  = x->droite;
    AVLUsine *T2 = y->gauche;

    y->gauche = x;
    x->droite = T2;

    x->hauteur = 1 + max_int(hauteur_avl_usine(x->gauche), hauteur_avl_usine(x->droite));
    y->hauteur = 1 + max_int(hauteur_avl_usine(y->gauche), hauteur_avl_usine(y->droite));

    return y;
}

int facteur_equilibre_avl_usine(AVLUsine *n) {
    return n ? hauteur_avl_usine(n->gauche) - hauteur_avl_usine(n->droite) : 0; //si n NULL 0 sinon fils gauche - droit
}

AVLUsine *creer_noeud_usine(char *id_usine) {
    AVLUsine *n = malloc(sizeof(*n)); // crééer une usine
    if (!n) { 
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strncpy(n->data.id_usine, id_usine, TAILLE_ID - 1); // copie id et donéé a zero
    n->data.id_usine[TAILLE_ID - 1] = '\0';
    n->data.vol_capte_total    = 0.0; 
    n->data.vol_achemine_total = 0.0;
    n->data.vol_fuite_total    = 0.0;
    n->data.capacite_max       = 0.0;

    n->hauteur = 1; // equilibre et fils
    n->gauche  = NULL;
    n->droite  = NULL;
    return n;
}

AVLUsine *avl_inserer_usine(AVLUsine *racine, char *id_usine) {
    if (!racine) {
        return creer_noeud_usine(id_usine); // creer usine
    }

    int cmp = strcmp(id_usine, racine->data.id_usine); // compare chaine avec tableau ascii = existe, sinon va dans sous arbre
    if (cmp < 0) { 
        racine->gauche = avl_inserer_usine(racine->gauche, id_usine);
    } else if (cmp > 0) { 
        racine->droite = avl_inserer_usine(racine->droite, id_usine);
    } else { 
        return racine;
    }

    racine->hauteur = 1 + max_int(hauteur_avl_usine(racine->gauche), hauteur_avl_usine(racine->droite)); //recalcule la hauteur du nœud

    int balance = facteur_equilibre_avl_usine(racine); // calcul équilibre et réequilibre si besoin

    if (balance > 1 && strcmp(id_usine, racine->gauche->data.id_usine) < 0) 
        return rotation_droite(racine);

    if (balance < -1 && strcmp(id_usine, racine->droite->data.id_usine) > 0) 
        return rotation_gauche(racine);

    if (balance > 1 && strcmp(id_usine, racine->gauche->data.id_usine) > 0) { 
        racine->gauche = rotation_gauche(racine->gauche);
        return rotation_droite(racine);
    }

    if (balance < -1 && strcmp(id_usine, racine->droite->data.id_usine) < 0) { 
        racine->droite = rotation_droite(racine->droite);
        return rotation_gauche(racine);
    }

    return racine; // retourne avl mis a jour
}

AVLUsine *avl_rechercher_usine(AVLUsine *racine, char *id_usine) { // cherche id dans l'avl en comparant id et retourne si trouvé
    if (!racine) return NULL; 
    int cmp = strcmp(id_usine, racine->data.id_usine); 
    if (cmp == 0) return racine; 
    if (cmp < 0)  return avl_rechercher_usine(racine->gauche, id_usine); 
    return avl_rechercher_usine(racine->droite, id_usine); 
}

void liberer_avl_usines(AVLUsine *r) { // fonction de liberation arbre classique
    if (!r) return; 
    liberer_avl_usines(r->gauche); 
    liberer_avl_usines(r->droite); 
    free(r); 
}

void traiter_une_ligne_histo(char *ligne_brute, AVLUsine **racine_usines) {
    char ligne[128];
    strncpy(ligne, ligne_brute, sizeof(ligne) - 1); // copie ligne brute vers ligne
    ligne[sizeof(ligne) - 1] = '\0';

    char *col1 = strtok(ligne,   ";"); // fragmente la ligne a chaque ; pour donner nos colonnes puis reprend a la suite
    char *col2 = strtok(NULL, ";");
    char *col3 = strtok(NULL, ";");
    char *col4 = strtok(NULL, ";");
    char *col5 = strtok(NULL, ";");

    //CAS : USINE seule (capacité) - ; Usine ; - ; capacite ; -
    if (est_tiret(col1) && !est_tiret(col2) && est_tiret(col3) && !est_tiret(col4) && est_tiret(col5)) {

        double capacite = atof(col4); // transforme une chaine de caractere en nombre ici double

        *racine_usines = avl_inserer_usine(*racine_usines, col2); // insère l'usine si pas encore existante
        AVLUsine *n = avl_rechercher_usine(*racine_usines, col2); // cherche l'usine 
        n->data.capacite_max = capacite; // ajoute cap max 

        return;
    }

    //CAS 1 : SOURCE -> USINE - ; Source ; Usine ; volume ; %fuite
    if (est_tiret(col1) && !est_tiret(col2) && !est_tiret(col3) && !est_tiret(col4) && !est_tiret(col5)) {

        double vol   = atof(col4); // meme principe qu'avant et calcul pour real et src
        double pct   = atof(col5);
        double fuite = vol * pct / 100.0; 
        double aval  = vol - fuite; 

        *racine_usines = avl_inserer_usine(*racine_usines, col3); 
        AVLUsine *u = avl_rechercher_usine(*racine_usines, col3); 
        if (u) {
            u->data.vol_capte_total    += vol; // met données a jour
            u->data.vol_achemine_total += aval;
            u->data.vol_fuite_total    += fuite;
        }

        return;
    }
}

int traiter_fichier(char *nom_fichier, AVLUsine **racine_usines, AVLActeur **racine_index_acteurs, char *mode, char *opt_ou_id_usine) /* opt_histo pour histo : "max"/"src"/"real", NULL pour leaks */
{
    FILE *f = fopen(nom_fichier, "r"); // ouvre fichier
    if (!f) { 
        perror("Erreur ouverture fichier");
        return 1;
    }

    int mode_histo = (strcmp(mode, "histo") == 0); // cherche le mode voulu
    int mode_leaks = (strcmp(mode, "leaks") == 0);

    char ligne[128];

    while (fgets(ligne, sizeof(ligne), f)) { // copie ligne jusqu'a \n \r ou taille dépassé, et s'arrete definitivement a EOF
        if (mode_histo && !mode_leaks) { // traite en fonction de ce que demandé par le shell
            traiter_une_ligne_histo(ligne, racine_usines);
        } else { 
            traiter_une_ligne_leaks(ligne, racine_usines, racine_index_acteurs, opt_ou_id_usine);
        }
    }

    fclose(f); // ferme le fichier
    return 0;
}

void exporter_orde_lexinv(AVLUsine *n, const char *opt, FILE *f)
{
    if (!n) return;

    exporter_orde_lexinv(n->droite, opt, f); // ordre lexicographique inverse (infixe inverse)

    double valeur = 0.0;

    if (strcmp(opt, "max") == 0) { // trouve mode pour retourner bonne valeur sauf bonus
        valeur = n->data.capacite_max;
    } 
    else if (strcmp(opt, "src") == 0) {
        valeur = n->data.vol_capte_total;
        if (n->data.capacite_max > 0.0 && valeur > n->data.capacite_max)
            valeur = n->data.capacite_max;
    } 
    else if (strcmp(opt, "real") == 0) {
        valeur = n->data.vol_achemine_total;
        if (n->data.capacite_max > 0.0 && valeur > n->data.capacite_max)
            valeur = n->data.capacite_max;
    }

    valeur = valeur / 1000.0; // 1000 k.m3 = 1 M.m3 dut a maj 1.1 sujet

    fprintf(f, "%s;%.3f\n", n->data.id_usine, valeur);

    exporter_orde_lexinv(n->gauche, opt, f);
}



int exporter_histogramme(AVLUsine *racine_usines, char *opt, char *chemin_sortie_dat)
{
    FILE *f = fopen(chemin_sortie_dat, "w"); // ecrase puis ouvre 
    if (!f) {
        perror("Impossible d'ouvrir le fichier d'histogramme");
        return 1;
    }

    fprintf(f, "id_usine;valeur\n"); // en-tête du fichier

    exporter_orde_lexinv(racine_usines, opt, f); // écris dans l'ordre voulu

    fclose(f);
    return 0;
}