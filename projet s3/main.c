#include "structures.h"
#include "histogramme.h"
#include "leaks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// 0 : succès  1 : erreur de syntaxe / arguments  2 : erreur de traitement 3 : erreur génération fichier sortie 

int main(int argc, char *argv[])
{
    if (argc != 5) { // mauvais nbr d'argument erreur return
        fprintf(stderr, "Pas assez d'arguments.\n");
        return 1;
    }

    char *mode = argv[1];  // histo ou leaks

    AVLUsine *racine_usines = NULL;
    AVLActeur *racine_index_acteurs = NULL;

    int code_retour;

    if (strcmp(mode, "histo") == 0) { // Mode histo 

        char *opt            = argv[2]; 
        char *fichier_csv    = argv[3]; 
        char *fichier_sortie = argv[4]; 


        int res = traiter_fichier(fichier_csv, &racine_usines, &racine_index_acteurs, "histo", opt); // traite le fichier dans le mode demandé
        if (res != 0) {
            fprintf(stderr,"Erreur lors du traitement du fichier CSV \"%s\" (code = %d).\n", fichier_csv, res);
            code_retour = 2; 
            goto fin; // reprend le code apres fin
        }

        res = exporter_histogramme(racine_usines, opt, fichier_sortie); // créé fichier avec info usines dans un fichier créér
        if (res != 0) {
            fprintf(stderr,"Erreur lors de la génération du fichier d'histogramme \"%s\" (code = %d).\n", fichier_sortie, res);
            code_retour = 3;
            goto fin; 
        }

        code_retour = 0;  

    } else if (strcmp(mode, "leaks") == 0) {


        char *id_usine       = argv[2]; 
        char *fichier_csv    = argv[3]; 
        char *fichier_sortie = argv[4]; 

        int res = traiter_fichier(fichier_csv, &racine_usines, &racine_index_acteurs, "leaks", id_usine);
        if (res != 0) {
            fprintf(stderr, "Erreur lors du traitement du fichier CSV \"%s\" (code = %d).\n", fichier_csv, res);
            code_retour = 2; 
            goto fin; 
        }

        double leaks=calculer_fuites_usine(id_usine, racine_usines, racine_index_acteurs); // calcul fuite

        res = exporter_fuites_usine(racine_usines, id_usine, fichier_sortie, leaks); // exporte fuite dans un fichier historique
        if (res != 0) {
            fprintf(stderr,"Erreur lors de la génération du fichier de fuites \"%s\" pour l'usine \"%s\" (code = %d).\n", fichier_sortie, id_usine, res);
            code_retour = 3; 
            goto fin; 
        }

        code_retour = 0;  // succès

    } else {
        fprintf(stderr,"Mode \"%s\" inconnu. Attendu : histo ou leaks.\n", mode);
        return 1;
    }

fin: // reprise du code pour les (goto fin)

    if (racine_usines != NULL) { // libère allocations memoires
        liberer_avl_usines(racine_usines);
    }
    if (racine_index_acteurs != NULL) {
        liberer_reseau_acteurs(racine_index_acteurs);
    }

    return code_retour; // retour en fonction de erreur ou réussite
}

