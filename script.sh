#!/bin/bash

EXEC="./wildwater"       # nom de l'exécutable C
LEAKS_FILE="leaks_history.dat"  # fichier historique fuites 

# Fonction pour récupérer le temps en millisecondes universel
temp_ms() {
    python3 - <<'PY' #Lance l’interpréteur Python 3
import time #Donne accès à l’heure système
print(int(time.time() * 1000)) #temps actuel en secondes * 1000 arrondi car int
PY
}

# Fonction pour afficher un message d'erreur et sortir 
die() {
    local code="$1" # déclare variable local
    shift #enlève l'arg et decale vers le 2eme (message d'erreur)
    echo "Erreur : $*" >&2 #affiche le message d’erreur qu’on a passé à la fonction
    end=$(temp_ms) #mesure la durée (fin) en millisecondes
    echo "Durée totale : $((end - start)) ms" #affiche temp traitement fin - début en ms
    exit "$code" # termine l’exécution du script courant
}

start=$(temp_ms)   # début chrono en millisecondes

if [ $# -lt 3 ]; then # il faut au minimum chemin_fichier + mode donc 3
    die 1 "pas assez d'arguments.
Usage :
  $0 chemin_fichier.csv histo {max|src|real}
  $0 chemin_fichier.csv leaks \"id_usine\""
fi

DATA_CSV="$1" # v1.1 : Le tout premier argument sera le chemin du fichier de données
mode="$2"     # mode = second argument saisi

case "$mode" in # différent cas si mode = histo ou leaks sinon erreur
    histo)
        if [ $# -ne 3 ]; then  # v1.1 : data + histo + option
            die 1 "usage correct : $0 chemin_fichier.csv histo {max|src|real}"
        fi
        opt="$3" #opt=troisième arg
        case "$opt" in # cas correctes
            max)  OUT_BASE="histo_max"  ;;
            src)  OUT_BASE="histo_src"  ;;
            real) OUT_BASE="histo_real" ;;
            all)  OUT_BASE="histo_all"  ;;
            *)
                die 1 "troisième argument pour 'histo' doit être : max, src ou real."
                ;;
        esac

        OUT_DATA="${OUT_BASE}.dat" # créé nom fichier données (id_usine;valeur)
        # v1.1 : 2 images (50 plus petites / 10 plus grandes)
        OUT_PNG_SMALL="${OUT_BASE}_50_small.png"  # image 50 plus petites
        OUT_PNG_LARGE="${OUT_BASE}_10_large.png"  # image 10 plus grandes

        TMP_SMALL="tmp_${OUT_BASE}_50_small.dat" # fichier temporaire 50 plus petites
        TMP_LARGE="tmp_${OUT_BASE}_10_large.dat" # fichier temporaire 10 plus grandes
        ;;

    leaks)
        if [ $# -ne 3 ]; then # v1.1 : data + leaks + "id_usine" uniquement
            die 1 "usage correct : $0 chemin_fichier.csv leaks \"identifiant complet de l'usine\""
        fi
        USINE_ID="$3" #usine=troisième arg
        ;;

    *)
        die 1 "deuxième argument doit être 'histo' ou 'leaks'."
        ;;
esac

# Compiler le C 
make # compile
if [ $? -ne 0 ]; then # diff 0 si le make a échoué
        die 2 "échec de la compilation (make)."
fi

# Appeler le programme C selon le mode
RET=0 #on pose valeur retour a 0 pour le moment

if [ "$mode" = "histo" ]; then
    echo "Traitement histogramme ($opt)"
    echo "Fichier d'entrée : $DATA_CSV"
    echo "Fichier de sortie données : $OUT_DATA"

    # appelle le C : sortie avec UNE seule valeur par usine (id_usine;valeur) en M.m3 (géré côté C)
    "$EXEC" histo "$opt" "$DATA_CSV" "$OUT_DATA"
    RET=$?

    if [ $RET -ne 0 ]; then
        echo "Le programme C a retourné le code $RET (erreur pendant le traitement)."
    else
        echo "Préparation des données (50 plus petites valeurs) : $TMP_SMALL" # construction des donnés pour graphe

        head -n 1 "$OUT_DATA" > "$TMP_SMALL" # créé le fichier ou écrasé et copie 1er ligne de histo (nom colonne)

        tail -n +2 "$OUT_DATA" \
        | LC_ALL=C sort -s -t';' -k2,2g \
        | head -n 50 \
        >> "$TMP_SMALL"

         # ignore 1ere ligne et tri numérique 2eme colonne et copie les 50 premier (ordre croissant)

        echo "Préparation des données (10 plus grandes valeurs) : $TMP_LARGE"

        head -n 1 "$OUT_DATA" > "$TMP_LARGE"

        tail -n +2 "$OUT_DATA" \
        | LC_ALL=C sort -s -t';' -k2,2g \
        | tail -n 10 \
        >> "$TMP_LARGE"

        echo "Génération de l'image PNG (50 plus petites) : $OUT_PNG_SMALL"

        gnuplot <<EOF
set terminal png size 1400,700      #dimensions en pxl
set output "${OUT_PNG_SMALL}"       #fichier qui va stocker histo
set datafile separator ";"          #séparateur col dans fichier csv
set style data histograms           #diagramme barre
set style fill solid 1.0 border -1  #barre pleine
set boxwidth 0.9                    #largeur des barres

set xtics rotate by -90             #titre usine aff -90 degré pour lisibilité (0 fait superposition illisible)                     

set title "Histogramme - ${opt} (50 plus petites)" #defini titre histo
set ylabel "Volume (M.m3)"          # valeur axe y
set key off                         #legend désactivé


set yrange [0:*]                    #met y a 0 et va jusqu'au max des barres
set format y "%.1f"                 #unité avec 1 chiffre après virgule pour petite usine

# every ::1                         #ignore la ligne d'en-tête (commence a 0 en temp normal)
plot "${TMP_SMALL}" every ::1 using 2:xtic(1) #fichier traité ignore en tete utilise la colone 2 pour val num et 1 pour nom usine abscisse
EOF

        if [ $? -ne 0 ]; then
            echo "Attention : échec lors de la génération de l'image PNG (50 petites) avec gnuplot." >&2
        fi

        echo "Génération de l'image PNG (10 plus grandes) : $OUT_PNG_LARGE"

        gnuplot <<EOF 
set terminal png size 1400,700
set output "${OUT_PNG_LARGE}"
set datafile separator ";"
set style data histograms
set style fill solid 1.0 border -1
set boxwidth 0.9

set xtics rotate by -90

set title "Histogramme - ${opt} (10 plus grandes)"
set ylabel "Volume (M.m3)"
set key off

set yrange [0:*]
set format y "%.f"     # pas de notation scientifique, juste des chiffres

plot "${TMP_LARGE}" every ::1 using 2:xtic(1)
EOF

        if [ $? -ne 0 ]; then
            echo "Attention : échec lors de la génération de l'image PNG (10 grandes) avec gnuplot." >&2
        fi

        rm -f "$TMP_SMALL" "$TMP_LARGE" # supprime les fichiers temporaires (plus propre)
    fi

elif [ "$mode" = "leaks" ]; then #mode leak
    echo "Calcul des fuites pour l'usine : $USINE_ID"
    echo "Fichier d'entrée : $DATA_CSV"
    echo "Fichier historique des fuites : $LEAKS_FILE"

    # ici, LEAKS_FILE est le fichier historique qui sera créé ou mis a jour selon cas
    "$EXEC" leaks "$USINE_ID" "$DATA_CSV" "$LEAKS_FILE"
    RET=$? #retourn en fonction de probleme ou non

    if [ $RET -ne 0 ]; then # si dif de 0 erreur
        echo "Le programme C a retourné le code $RET (erreur pendant le traitement)."
    else
        echo "Traitement des fuites terminé. Résultat ajouté à : $LEAKS_FILE"
    fi
fi

end=$(temp_ms) # prend le temps actuel en ms
echo "Durée totale : $((end - start)) ms" # affiche temp traite en ms

exit $RET # sortie avec le code du programme C