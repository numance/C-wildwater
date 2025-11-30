#include <stdio.h>                                                                                      //
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TAILLE_ID 64

typedef struct {
    char   id_usine[TAILLE_ID];    // Identifiant complet type "Facility complex #RH400057F"
    double vol_capte_total;        // Somme des volumes provenant des sources avant fuite
    double vol_achemine_total;     // Somme des volumes arriver à l'usine après fuites Source->Usine
    double fuite_totale;           // Somme des fuites sur les tronçons Source->Usine 
    double capacite_max;           // Capacité max  traitement usine
} StatsUsine;

typedef struct NoeudAVL {
    StatsUsine data;
    int hauteur;
    struct NoeudAVL *gauche;
    struct NoeudAVL *droite;
} NoeudAVL;


void retirer_retour_ligne(char *s) { // Supprime le premier '\r' ou '\n' rencontré
    s[strcspn(s, "\r\n")] = '\0';
}


void trim(char *s) { // Trim : enlève les espaces au début et à la fin de la chaîne. 
    char *p = s;
    while (*p && isspace((unsigned char)*p)) { // check au debut si ' ', '\n', '\r'
        p++; // alors on passe a après et supp l'espace
    }
    if (p != s) {
        memmove(s, p, strlen(p) + 1); //copie le texte à partir de p dans s
    }
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) { // de meme qu'avant mais a la fin
        s[--len] = '\0'; // raccourci la chaine par la fin
    }
}


int est_vide_ou_tiret(const char *s) { // Teste si une "valeur" est vide ou marquée '-' 
    if (!s) return 1; //vide NULL
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return 1; // vide \0
    if (*s == '-' && s[1] == '\0') return 1; // vide '-'
    return 0; // sinon non vide
}

int hauteur(NoeudAVL *n) { // si n NULL return 0 sinon n->hauteur
    return n ? n->hauteur : 0; 
}

int max_int(int a, int b) { // retourne max
    return (a > b) ? a : b;
}


int commence_par(const char *s, const char *prefixe) {
    size_t n = strlen(prefixe);
    return strncmp(s, prefixe, n) == 0; // si les deux identique vrai (1) sinpn (0)
}


int est_source(const char *id) { // Acteurs considérés comme "sources" pour le captage si un correspond alors source
    return  commence_par(id, "Source ") || commence_par(id, "Spring ") || commence_par(id, "Well ") || commence_par(id, "Well field ") || commence_par(id, "Fountain "); 
}


int est_usine(const char *id) { // de meme pour usine
    return  commence_par(id, "Plant ") || commence_par(id, "Unit ") || commence_par(id, "Module ") || commence_par(id, "Facility complex ");
}

// differentes rotations

NoeudAVL *rotation_droite(NoeudAVL *y) {
    NoeudAVL *x = y->gauche;
    NoeudAVL *T2 = x->droite;

    x->droite = y;
    y->gauche = T2;

    y->hauteur = 1 + max_int(hauteur(y->gauche), hauteur(y->droite));
    x->hauteur = 1 + max_int(hauteur(x->gauche), hauteur(x->droite));

    return x;
}

NoeudAVL *rotation_gauche(NoeudAVL *x) {
    NoeudAVL *y = x->droite;
    NoeudAVL *T2 = y->gauche;

    y->gauche = x;
    x->droite = T2;

    x->hauteur = 1 + max_int(hauteur(x->gauche), hauteur(x->droite));
    y->hauteur = 1 + max_int(hauteur(y->gauche), hauteur(y->droite));

    return y;
}

int facteur_equilibre(NoeudAVL *n) {
    return n ? hauteur(n->gauche) - hauteur(n->droite) : 0; // si n NULL 0 sinon gauche - droite
}


NoeudAVL *creer_noeud_usine(const char *id_usine) { // créé un noeud avec donné usine
    NoeudAVL *n = malloc(sizeof(*n));
    if (!n) {
        perror("malloc");
        exit(1);
    }

    strncpy(n->data.id_usine, id_usine, TAILLE_ID - 1); // copie nom
    n->data.id_usine[TAILLE_ID - 1] = '\0'; // raccourci de 1 pour etre sur de ne pa depasser

    n->data.vol_capte_total    = 0.0; // met les stats a 0
    n->data.vol_achemine_total = 0.0;
    n->data.fuite_totale       = 0.0;
    n->data.capacite_max       = 0.0;

    n->hauteur = 1; // met l'équilibre a 1 et fils NULL
    n->gauche  = NULL;
    n->droite  = NULL;

    return n;
}


NoeudAVL *avl_inserer_id(NoeudAVL *racine, const char *id_usine) { // Insertion d'un nouveau identifiant d'usine dans l'AVL
    if (racine == NULL) { // a la fin donc cree nouveau noeud
        return creer_noeud_usine(id_usine);
    }

    int cmp = strcmp(id_usine, racine->data.id_usine); // compare lettre par lettre avec tableau ascii pour savoir si égal, inf ou sup

    if (cmp < 0) { // sous arbre gauche
        racine->gauche = avl_inserer_id(racine->gauche, id_usine);
    } else if (cmp > 0) { // sous arbre droit
        racine->droite = avl_inserer_id(racine->droite, id_usine);
    } else { // Déjà présente, on ne change rien. 
        return racine;
    }

    // Mise à jour de la hauteur 
    racine->hauteur = 1 + max_int(hauteur(racine->gauche), hauteur(racine->droite));

    // Rééquilibrage AVL 
    int balance = facteur_equilibre(racine);

    // gauche gauche
    if (balance > 1 && strcmp(id_usine, racine->gauche->data.id_usine) < 0)
        return rotation_droite(racine);

    // droite droite
    if (balance < -1 && strcmp(id_usine, racine->droite->data.id_usine) > 0)
        return rotation_gauche(racine);

    // gauche droite 
    if (balance > 1 && strcmp(id_usine, racine->gauche->data.id_usine) > 0) {
        racine->gauche = rotation_gauche(racine->gauche);
        return rotation_droite(racine);
    }

    // droite gauche
    if (balance < -1 && strcmp(id_usine, racine->droite->data.id_usine) < 0) {
        racine->droite = rotation_droite(racine->droite);
        return rotation_gauche(racine);
    }

    return racine; // avl a jour
}

// Recherche d'une usine dans l'AVL. 
NoeudAVL *avl_rechercher(NoeudAVL *racine, const char *id_usine) {
    if (!racine) return NULL; // pas trouvé

    int cmp = strcmp(id_usine, racine->data.id_usine); // comme avant comparaison caractere par caractere

    if (cmp == 0) { // égal donc usine trouvé
        return racine;
    } else if (cmp < 0) { // cherche dans sous arbre gauche
        return avl_rechercher(racine->gauche, id_usine);
    } else { // cherche dan sous arbre droit
        return avl_rechercher(racine->droite, id_usine);
    }
}



void afficher_stats_usines_inorder(NoeudAVL *r) {
    if (!r) return; // fin d'affichage r NULL

    afficher_stats_usines_inorder(r->gauche); // affichage infixe

    double vol_traite = r->data.vol_achemine_total;
    if (r->data.capacite_max > 0.0 && vol_traite > r->data.capacite_max) { // L’usine reçoit plus que ce qu’elle peut traiter
        vol_traite = r->data.capacite_max; // donc elle ne peux traiter que son max
    }

    printf("Usine %-30s\n", r->data.id_usine); // afiche différente donné
    printf("    Volume capte total    : %.3f (k m3)\n", r->data.vol_capte_total);
    printf("    Volume achemine total : %.3f (k m3)\n", r->data.vol_achemine_total);
    printf("    Volume traite (limite capacite) : %.3f (k m3)\n", vol_traite);
    printf("    Fuites totales        : %.3f (k m3)\n", r->data.fuite_totale);
    printf("    Capacite max traite   : %.3f (k m3)\n\n", r->data.capacite_max);

    afficher_stats_usines_inorder(r->droite);
}

void liberer_avl(NoeudAVL *r) { // libere tout l'arbre ou une partie
    if (!r) return; 
    liberer_avl(r->gauche); 
    liberer_avl(r->droite);
    free(r);
}

/*
   On considère ici les cas principaux pour l'histogramme des usines :

   1) Lignes SOURCE -> USINE (captage), par ex :
      -;Spring #MQ001991L;Facility complex #RH400057F;20892;0.997

      col1 = "-"
      col2 = source (Spring/Source/...)
      col3 = usine
      col4 = volume annuel capte (en k m3)
      col5 = % fuites sur le tronçon source->usine

      => on met à jour, pour l'usine de col3 :
         - vol_capte_total    += volume
         - fuite_totale       += fuite
         - vol_achemine_total += volume - fuite

   2) Lignes USINE seule (capacité), par ex :
      -;Facility complex #RH400057F;-;4749292;-

      col1 = "-"
      col2 = usine
      col3 = "-"
      col4 = capacite max (en k m3)
      col5 = "-"

      => on met à jour, pour l'usine de col2 :
         - capacite_max = col4

   3) Les autres lignes (USINE->STOCKAGE, etc.) ne fournissent pas de volume annuel
      utilisable directement pour l'histogramme (souvent col4 = "-").
      On les ignore ici, ou on pourra les utiliser plus tard pour d'autres analyses.
*/

void traiter_une_ligne(const char *ligne_brute, NoeudAVL **racine_usines) {
    char ligne[128];

    strncpy(ligne, ligne_brute, sizeof(ligne) - 1); //copie au max ligne-1 caractères de la chaîne src dans dest.
    ligne[sizeof(ligne) - 1] = '\0'; //dernier caractere supp pour eviter erreur affichage

    retirer_retour_ligne(ligne); // supp les \n \r pour le traitement

    char *col1 = strtok(ligne,   ";"); // Découpage des colonnes s'arrete a chaque ; puis reprend apres
    char *col2 = strtok(NULL, ";"); // NULL pour qu'il ne recommence pas au début
    char *col3 = strtok(NULL, ";");
    char *col4 = strtok(NULL, ";");
    char *col5 = strtok(NULL, ";");

    // si ligne incorecte ou info manquante
    if (!col1 || !col2 || !col3 || !col4 || !col5) {
        fprintf(stderr, "Ligne mal formée, ignorée : %s\n", ligne_brute); // erreur return
        return;
    }

    // Trim de base pour enlever espace a la fin et début gagne stockage et temps de traitement
    trim(col1);
    trim(col2);
    trim(col3);
    trim(col4);
    trim(col5);

    // def d'une usine col 1, 3 et 5 vide et 2 identifiant
    if (est_vide_ou_tiret(col1) && est_usine(col2) && est_vide_ou_tiret(col3) && !est_vide_ou_tiret(col4) && est_vide_ou_tiret(col5)) {

        double capacite = atof(col4); // converti en double la chaine de caractere du texte

        
        *racine_usines = avl_inserer_id(*racine_usines, col2); // On s'assure que l'usine existe avl sinon ajoute
        NoeudAVL *n = avl_rechercher(*racine_usines, col2); // recupere le noeud
        if (n) {
            n->data.capacite_max = capacite; // ajoute sa capacité max transformé juste avant
        }
        return; // fin de traitement ligne
    }

    // def source-> usine col 1 vide, 2 source, 3 id et 4, 5 data
    if (est_vide_ou_tiret(col1) && est_source(col2) && est_usine(col3) && !est_vide_ou_tiret(col4)&& !est_vide_ou_tiret(col5))) {

        double vol = atof(col4); // converti en double
        double pct = atof(col5);
        double fuite = vol * pct / 100.0; // calcul des fuite
        double aval  = vol - fuite; // calcul quantité réellement transmis

        // On met à jour l'usine de la colonne 3 
        *racine_usines = avl_inserer_id(*racine_usines, col3);
        NoeudAVL *n = avl_rechercher(*racine_usines, col3);
        if (n) {
            n->data.vol_capte_total    += vol;
            n->data.fuite_totale       += fuite;
            n->data.vol_achemine_total += aval;
        }
        return;
    }
}


int traiter_fichier(const char *nom_fichier, NoeudAVL **racine_usines) { // Lecture et traitement de l'ensemble du fichier 
    FILE *f = fopen(nom_fichier, "r"); // ouvrir le fichier mode read
    if (!f) { // si fichier vide affiche message erreur et retourne erreur (1)
        perror("Erreur lors de l'ouverture du fichier");
        return 1;
    }

    char ligne[128]; //taille ligne max dans fichier 64 (impossible avec 64-1) donc par convention 128
    long numero_ligne = 0; //numero ligne fichier lu

    while (fgets(ligne, sizeof(ligne), f)) { // lit une ligne de taille s'arrete a un \n, taille max ligne ou EOF ( fin de boucle)
        numero_ligne++;


        
        if (ligne[0] == '\n' || ligne[0] == '\r') // si premier carcatere de la ligne un retour ligne suivante ou retour debut ligne
            continue; // alors on saute la ligne


        traiter_une_ligne(ligne, racine_usines); // traite le texte obtenu
    }

    fclose(f); //ferme fichier
    return 0; // retourne succes
}


int main() {
    const char *nom_fichier = "c-wildwater_v3.dat"; // fichier data
    NoeudAVL *racine_usines = NULL; // creer arbre vide

    int resultat = traiter_fichier(nom_fichier, &racine_usines); 
    if (resultat != 0) {
        fprintf(stderr, "Erreur pendant le traitement.\n"); // en cas probleme message erreur
        return resultat;
    }

    printf("Stats par usine :\n\n");
    afficher_stats_usines_inorder(racine_usines); // affichage stat chaque usine avec source->usine et traitement (usine)

    liberer_avl(racine_usines); // liberer espace avl

    printf("Traitement terminé avec succès.\n"); // fin du traitement et programme
    return 0;
}
