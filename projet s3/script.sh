#!/bin/bash

EXEC="./wildwater"       # nom de l'exécutable C
LEAKS_FILE="leaks_history.dat"  # fichier historique fuites 

# Fonction pour récupérer le temps en millisecondes universel
temp_ms() {
    python3 - <<'PY' 
import time 
print(int(time.time() * 1000)) #temps actuel en secondes * 1000 arrondi car int v1.1 sujet
PY
}

# Fonction pour afficher un message d'erreur temps traitement et sortir 
die() {
    local code="$1" 
    shift 
    echo "Erreur : $*" >&2 #affiche le message d’erreur qu’on a passé à la fonction
    end=$(temp_ms) 
    echo "Durée totale : $((end - start)) ms" 
    exit "$code" 
}

start=$(temp_ms)   # début chrono en millisecondes

if [ $# -lt 3 ]; then # il faut au minimum chemin_fichier + mode donc 3 argument
    die 1 "pas assez d'arguments.
Usage :
  $0 chemin_fichier.csv histo {max|src|real}
  $0 chemin_fichier.csv leaks \"id_usine\""
fi

DATA_CSV="$1" # v1.1 : Le tout premier argument le chemin du fichier de données
mode="$2"     # mode = second argument saisi

case "$mode" in # différent cas si mode = histo ou leaks sinon erreur
    histo)
        if [ $# -ne 3 ]; then  # v1.1 : data + histo + option
            die 1 "usage correct : $0 chemin_fichier.csv histo {max|src|real}"
        fi
        opt="$3" #opt=troisième arg
        case "$opt" in 
            max)  OUT_BASE="histo_max"  ;; #nomme fichier en fct de opt
            src)  OUT_BASE="histo_src"  ;;
            real) OUT_BASE="histo_real" ;;
            *)
                die 1 "troisième argument pour 'histo' doit être : max, src ou real."
                ;;
        esac

        OUT_DATA="${OUT_BASE}.dat" # créé nom de tout les fichier 
        
        OUT_PNG_PETIT="${OUT_BASE}_50_petit.png"  
        OUT_PNG_GRAND="${OUT_BASE}_10_grand.png"  

        TMP_PETIT="tmp_${OUT_BASE}_50_petit.dat" 
        TMP_GRAND="tmp_${OUT_BASE}_10_grand.dat" 
        ;;

    leaks)
        if [ $# -ne 3 ]; then 
            die 1 "usage correct : $0 chemin_fichier.csv leaks \"identifiant complet de l'usine\""
        fi
        USINE_ID="$3" #usine=troisième arg
        ;;

    *)
        die 1 "deuxième argument doit être 'histo' ou 'leaks'."
        ;;
esac

# Compiler le C 
make 
if [ $? -ne 0 ]; then 
        die 2 "échec de la compilation (make)."
fi


RET=0 #on pose valeur retour a 0 pour le moment

if [ "$mode" = "histo" ]; then
    echo "Traitement histogramme ($opt)"
    echo "Fichier d'entrée : $DATA_CSV"
    echo "Fichier de sortie données : $OUT_DATA"

    # appelle le C (avec tout les arg neccesaire): sortie avec UNE seule valeur par usine (id_usine;valeur) en M.m3 (géré côté C)
    "$EXEC" histo "$opt" "$DATA_CSV" "$OUT_DATA"
    RET=$?

    if [ $RET -ne 0 ]; then
        echo "Le programme C a retourné le code $RET (erreur pendant le traitement)."
    else
        echo "Préparation des données (50 plus petites valeurs) : $TMP_PETIT" # construction des donnés pour graphe

        head -n 1 "$OUT_DATA" > "$TMP_PETIT" # créé le fichier ou écrasé et copie 1er ligne de histo (nom colonne)

        tail -n +2 "$OUT_DATA" \
        | LC_ALL=C sort -s -t';' -k2,2g \
        | head -n 50 \
        >> "$TMP_PETIT"

         # ignore 1ere ligne et tri numérique 2eme colonne et copie les 50 premier (ordre croissant)

        echo "Préparation des données (10 plus grandes valeurs) : $TMP_GRAND" #de meme pour grand 

        head -n 1 "$OUT_DATA" > "$TMP_GRAND"

        tail -n +2 "$OUT_DATA" \
        | LC_ALL=C sort -s -t';' -k2,2g \
        | tail -n 10 \
        >> "$TMP_GRAND"

        # juste on copie a la fin cette fois ci

        echo "Génération de l'image PNG (50 plus petites) : $OUT_PNG_PETIT"

        gnuplot <<EOF
set terminal png size 1400,700      #dimensions en pxl
set output "${OUT_PNG_PETIT}"       #fichier qui va stocker histo
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
plot "${TMP_PETIT}" every ::1 using 2:xtic(1) #fichier traité ignore en tete utilise la colone 2 pour val num et 1 pour nom usine abscisse
EOF

        if [ $? -ne 0 ]; then
            echo "Attention : échec lors de la génération de l'image PNG (50 petites) avec gnuplot." >&2
        fi

        echo "Génération de l'image PNG (10 plus grandes) : $OUT_PNG_GRAND" # meme principe que pour petit

        gnuplot <<EOF 
set terminal png size 1400,700
set output "${OUT_PNG_GRAND}"
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

plot "${TMP_GRAND}" every ::1 using 2:xtic(1)
EOF

        if [ $? -ne 0 ]; then
            echo "Attention : échec lors de la génération de l'image PNG (10 grandes) avec gnuplot." >&2
        fi

        rm -f "$TMP_PETIT" "$TMP_GRAND" # supprime les fichiers temporaires 
    fi

elif [ "$mode" = "leaks" ]; then #mode leak
    echo "Calcul des fuites pour l'usine : $USINE_ID"
    echo "Fichier d'entrée : $DATA_CSV"
    echo "Fichier historique des fuites : $LEAKS_FILE"

    # ici, LEAKS_FILE est le fichier historique qui sera créé ou mis a jour selon cas
    "$EXEC" leaks "$USINE_ID" "$DATA_CSV" "$LEAKS_FILE"
    RET=$? 

    if [ $RET -ne 0 ]; then # si RET dif de 0 erreur
        echo "Le programme C a retourné le code $RET (erreur pendant le traitement)."
    else
        echo "Traitement des fuites terminé. Résultat ajouté à : $LEAKS_FILE"
    fi
fi

end=$(temp_ms) 
echo "Durée totale : $((end - start)) ms" # affiche temp traite en ms

exit $RET # sortie avec le code du programme C