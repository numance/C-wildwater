Il faut ajouter le fichier brut et le renommer data.csv et le mettre dans le dossier contenant les fichiers .c, .h, makefile et scriptshell
et il génèrera vos fichiers de sortie automatiquement en fonction de la commande parmis celles qui suivent:

nb: avant éxecution dans le dossier où se trouve le script shell rentrer la commande: chmod +x script.sh

./script.sh data.csv histo src

cette commande permet de traîter tout le .dat et retourner un fichier dans l'ordre lexicographique inverse des id d'usines avec pour chacune leur quantité d'eau
acheminée depuis les sources avant toute fuite et 2 histogrammes sont créés: un pour les 10 plus grandes usines et l'autre pour les 50 plus petites à partir des
données traitées


./script.sh data.csv histo max

cette commande permet de traîter tout le .dat et retourner un fichier dans l'ordre lexicographique inverse des id d'usines avec pour chacune leur quantité d'eau
maximale pouvant être traîtée et 2 histogrammes sont créés: un pour les 10 plus grandes usines et l'autre pour les 50 plus petites à partir des données traitées


./script.sh data.csv histo real

cette commande permet de traîter tout le .dat et retourner un fichier dans l'ordre lexicographique inverse des id d'usines avec pour chacune leur quantité d'eau
acheminée depuis les sources après les fuites sur le troncon et 2 histogrammes sont créés: un pour les 10 plus grandes usines et l'autre pour les 50 plus petites
à partir des données traitées


./script.sh data.csv leaks "id_usine"

cette commande permet de traîter tout le .dat et met à jour un fichier contenant l'historique des commandes leaks avec pour chacune l'id usine et la fuite totale sur
le réseau aval ainsi que le tronçon qui provoque la plus grosse fuite en quantité sur le réseau aval avec son id amont, id aval et quantité en m.M3
