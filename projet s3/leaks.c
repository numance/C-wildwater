#include "structures.h"
#include "histogramme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// fonction de base équilibre et rotation

int hauteur_avl_acteur(AVLActeur *n) { 
    return n ? n->hauteur : 0; 
}

AVLActeur *rotation_droite_acteur(AVLActeur *y) {
    AVLActeur *x  = y->gauche;
    AVLActeur *T2 = x->droite;

    x->droite = y;
    y->gauche = T2;

    y->hauteur = 1 + max_int(hauteur_avl_acteur(y->gauche), hauteur_avl_acteur(y->droite));
    x->hauteur = 1 + max_int(hauteur_avl_acteur(x->gauche), hauteur_avl_acteur(x->droite));

    return x;
}

AVLActeur *rotation_gauche_acteur(AVLActeur *x) {
    AVLActeur *y  = x->droite;
    AVLActeur *T2 = y->gauche;

    y->gauche = x;
    x->droite = T2;

    x->hauteur = 1 + max_int(hauteur_avl_acteur(x->gauche), hauteur_avl_acteur(x->droite));
    y->hauteur = 1 + max_int(hauteur_avl_acteur(y->gauche), hauteur_avl_acteur(y->droite));

    return y;
}

int facteur_equilibre_avl_acteur(AVLActeur *n) {
    return n ? hauteur_avl_acteur(n->gauche) - hauteur_avl_acteur(n->droite) : 0; 
}

AVLActeur *creer_noeud_avl_acteur(char *id, Acteur *acteur) {
    AVLActeur *n = malloc(sizeof(*n)); // créé noeud et rempli donné
    if (!n) { 
        perror("malloc AVL acteur");
        exit(EXIT_FAILURE);
    }
    strncpy(n->id, id, TAILLE_ID - 1); 
    n->id[TAILLE_ID - 1] = '\0';
    n->acteur = acteur; // enregistre l’adresse du NoeudActeur qui correspond à ID (avec sa liste chaines de fils)
    n->hauteur = 1; 
    n->gauche = n->droite = NULL; 
    return n;
}

AVLActeur *avl_inserer_acteur(AVLActeur *racine, char *id, Acteur *acteur) {
    if (!racine) {
        return creer_noeud_avl_acteur(id, acteur); // si arriver en fin d'arbre créé noeud
    }

    int cmp = strcmp(id, racine->id); // compare les id (ascii) si 0 deja existanst sinon va sous arbre

    if (cmp < 0) { 
        racine->gauche = avl_inserer_acteur(racine->gauche, id, acteur);
    } else if (cmp > 0) { 
        racine->droite = avl_inserer_acteur(racine->droite, id, acteur);
    } else { 
        return racine;
    }

    racine->hauteur = 1 + max_int(hauteur_avl_acteur(racine->gauche), hauteur_avl_acteur(racine->droite)); // met a jour equilibre

    int balance = facteur_equilibre_avl_acteur(racine); // calcul equilibre et rééquilibre si besoin selon cas

    if (balance > 1 && strcmp(id, racine->gauche->id) < 0) 
        return rotation_droite_acteur(racine);

    if (balance < -1 && strcmp(id, racine->droite->id) > 0)
        return rotation_gauche_acteur(racine);

    if (balance > 1 && strcmp(id, racine->gauche->id) > 0) { 
        racine->gauche = rotation_gauche_acteur(racine->gauche);
        return rotation_droite_acteur(racine);
    }

    if (balance < -1 && strcmp(id, racine->droite->id) < 0) { 
        racine->droite = rotation_droite_acteur(racine->droite);
        return rotation_gauche_acteur(racine);
    }

    return racine;
}

AVLActeur *avl_rechercher_acteur(AVLActeur *racine, char *id) { // fonction classique recherche dans avl
    if (!racine) return NULL; 
    int cmp = strcmp(id, racine->id); 
    if (cmp == 0) return racine; 
    if (cmp < 0)  return avl_rechercher_acteur(racine->gauche, id); 
    return avl_rechercher_acteur(racine->droite, id); 
}

Acteur *trouver_ou_creer_acteur(AVLActeur **racine_index, char *id) { // Trouver ou créer un acteur dans l'arbre classique + l'AVL d'index 
    AVLActeur *na = avl_rechercher_acteur(*racine_index, id); 
    if (na) return na->acteur; // si existe le retourne

    Acteur *a = malloc(sizeof(*a)); // sinon noeud créé et rentre données
    if (!a) {
        perror("malloc NoeudActeur");
        exit(EXIT_FAILURE);
    }
    strncpy(a->id, id, TAILLE_ID - 1); 
    a->id[TAILLE_ID - 1] = '\0';
    a->premier_enfant = NULL; // liste d'enfant vide au début

    *racine_index = avl_inserer_acteur(*racine_index, id, a); // ajoute l'index dans l'avl
    return a;
}

void ajouter_enfant_acteur(Acteur *parent, Acteur *enfant, double pct_fuite) { // relie parent a enfant 
    LienActeur *l = malloc(sizeof(*l)); // créé lien et rempli donnée
    if (!l) { 
        perror("malloc LienActeur");
        exit(EXIT_FAILURE);
    }
    l->enfant    = enfant; // acteur aval
    l->pct_fuite = pct_fuite; // rajoute donné % fuite
    l->suiv      = parent->premier_enfant; // insére au début liste des acteurs aval
    parent->premier_enfant = l; 
}

void liberer_reseau_acteurs(AVLActeur *racine) { // libère index avl et chaque liste chainé des fils
    if (!racine) return; 

    liberer_reseau_acteurs(racine->gauche); // libere sous arbres
    liberer_reseau_acteurs(racine->droite); 

    Acteur *a = racine->acteur; //récupère noeud acteur

    LienActeur *l = a->premier_enfant; // prend début liste et libere toout les liens
    while (l) { 
        LienActeur *suiv = l->suiv; 
        free(l); 
        l = suiv; 
    }
    free(a); // libère enfin le noeud et index correspondant 
    free(racine);
}

void propager_fuites(Acteur *acteur, double vol_entree, double *fuites_totales) {

    // parcours enfants et compte pour répartir
    int nb_enfants = 0; 
    for (LienActeur *l = acteur->premier_enfant; l; l = l->suiv) { 
        nb_enfants++; 
    }

    if (nb_enfants == 0) return; // si aucun enfant, fin de propagation

    double part = vol_entree / (double)nb_enfants; // partage eau fils

    // propager vers chaque enfant avec la même part
    for (LienActeur *l = acteur->premier_enfant; l; l = l->suiv) { 
        double fuite = part * l->pct_fuite / 100.0; 
        double aval  = part - fuite; 

        *fuites_totales += fuite; 

        propager_fuites(l->enfant, aval, fuites_totales); // appel recursif enfant a aval
    }
}

double calculer_fuites_usine(char *id_usine, AVLUsine *racine_usines, AVLActeur *racine_index_acteurs) {
    AVLUsine *nu = avl_rechercher_usine(racine_usines, id_usine); // cherche l'usine demandé pour fuite
    if (!nu) {
        fprintf(stderr, "Erreur : usine \"%s\" inconnue (AVL usines).\n", id_usine); // si non trouvé retourne erreur
        return -1.0;
    }

    double vol_traite = nu->data.vol_achemine_total; //Déterminer le volume réellement traité capacité max comparé à acheminé
    if (nu->data.capacite_max > 0.0 && vol_traite > nu->data.capacite_max) {
        vol_traite = nu->data.capacite_max;
    }

    AVLActeur *na = avl_rechercher_acteur(racine_index_acteurs, id_usine); //récupères le NoeudActeur* correspondant, donc l’accès liste

    double fuites = 0.0; // met fuite a 0
    propager_fuites(na->acteur, vol_traite, &fuites); // calcul pour tout les enfant de l'usine
    return fuites;
}

void traiter_une_ligne_leaks(char *ligne_brute, AVLUsine **racine_usines, AVLActeur **racine_index_acteurs, char* id_usine) {
    char ligne[128];
    strncpy(ligne, ligne_brute, sizeof(ligne) - 1); // copie ligne brute vers ligne
    ligne[sizeof(ligne) - 1] = '\0';

    char *col1 = strtok(ligne,   ";"); // fragmente la ligne a chaque ; pour donner nos colonnes puis reprend a la suite
    char *col2 = strtok(NULL, ";");
    char *col3 = strtok(NULL, ";");
    char *col4 = strtok(NULL, ";");
    char *col5 = strtok(NULL, ";");

    // CAS 1 : SOURCE -> USINE  - ; Source ; Usine ; volume ; %fuite
    if (est_tiret(col1) && !est_tiret(col2) && !est_tiret(col3) && !est_tiret(col4) && !est_tiret(col5)) {

        if (strcmp(col3, id_usine) != 0) return;

        // transformation et calcul des données
        double vol   = atof(col4); 
        double pct   = atof(col5);
        double fuite = vol * pct / 100.0; 
        double aval  = vol - fuite; 

        *racine_usines = avl_inserer_usine(*racine_usines, col3); // ajoute usine sauf si existante deja
        AVLUsine *u = avl_rechercher_usine(*racine_usines, col3); // cherche noeud correspondant et maj des données
        if (u) {
            u->data.vol_capte_total    += vol;
            u->data.vol_achemine_total += aval;
            u->data.vol_fuite_total    += fuite;
        }

        return;
    }

    //CAS 2: USINE - ; Usine ; - ; capacite ; -
    if (est_tiret(col1) && !est_tiret(col2) && est_tiret(col3) && !est_tiret(col4) && est_tiret(col5)) {

        if (strcmp(col2, id_usine) != 0) return;

        double capacite = atof(col4); 

        *racine_usines = avl_inserer_usine(*racine_usines, col2); // insère usine sauf si deja existante
        AVLUsine *n = avl_rechercher_usine(*racine_usines, col2); // cherche le noeud et maj max
        n->data.capacite_max = capacite; 

        (void)trouver_ou_creer_acteur(racine_index_acteurs, col2); //créer acteur dans l'AVL d'index ainsi que en tant qu'acteur
        return;
    }

    // CAS 3: USINE -> STOCKAGE  - ; Usine ; Stockage ; - ; %fuite
    if (est_tiret(col1) && !est_tiret(col2) && !est_tiret(col3) && est_tiret(col4) && !est_tiret(col5)) {         

        if (strcmp(col2, id_usine) != 0) return; // si ce n'est pas l'usine demandée on ignore

        double pct = atof(col5); 

        // amont -> aval 
        Acteur *amont = trouver_ou_creer_acteur(racine_index_acteurs, col2); // récupère/crée acteur amont et aval
        Acteur *aval  = trouver_ou_creer_acteur(racine_index_acteurs, col3); 
        ajouter_enfant_acteur(amont, aval, pct); // ajoute a la liste chainé de fils l'acteur aval

        return;
    }


    // CAS 4 (meme principe que 3) : les cas restant avec tjr meme forme    usine ; amont ; aval ; - ; %fuite
    if (!est_tiret(col1) && !est_tiret(col2) && !est_tiret(col3) && est_tiret(col4) && !est_tiret(col5)) {

        if (strcmp(col1, id_usine) != 0) return;


        double pct = atof(col5);


        Acteur *amont = trouver_ou_creer_acteur(racine_index_acteurs, col2); 
        Acteur *aval  = trouver_ou_creer_acteur(racine_index_acteurs, col3); 
        ajouter_enfant_acteur(amont, aval, pct);

        return;
    }
}


int exporter_fuites_usine(AVLUsine *racine_usines, char *id_usine, char *chemin_sortie_dat, double leaks) {
    AVLUsine *u = avl_rechercher_usine(racine_usines, id_usine);

    FILE *f = fopen(chemin_sortie_dat, "a+"); // lecture + ajout
    if (!f) {
        perror("Ouverture fichier fuites");
        return 1;
    }
    // Aller à la fin pour connaître la taille
    fseek(f, 0, SEEK_END);
    long taille = ftell(f);
    // Si fichier vide → écrire l’en-tête
    if (taille == 0) {
        fprintf(f, "id_usine;leaks\n");
    }
    // si l'usine n'existe pas erreur
    if (!u) {
        fclose(f);
        return -1; 
    }
    // Écriture de la ligne de résultat
    leaks = leaks/1000.0;
    fprintf(f, "%s;%.3f\n", u->data.id_usine, leaks);

    fclose(f);
    return 0;
}