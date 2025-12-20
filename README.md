./script.sh data.csv histo src

cette commande permet de traiter tout le .dat et retourné un fichier dans l'ordre lexicographique inverse des id d'usine avec pour chacune leur quantité d'eau
acheminé depuis les sources avant tout fuites et 2 histogramme sont créés un pour les 10 plus grandes usines et l'autre pour les 50 plus petites à partir des
données traités


./script.sh data.csv histo max

cette commande permet de traiter tout le .dat et retourné un fichier dans l'ordre lexicographique inverse des id d'usine avec pour chacune leur quantité d'eau
maximale pouvant être traiter et 2 histogramme sont créés un pour les 10 plus grandes usines et l'autre pour les 50 plus petites à partir des données traités


./script.sh data.csv histo real

cette commande permet de traiter tout le .dat et retourné un fichier dans l'ordre lexicographique inverse des id d'usine avec pour chacune leur quantité d'eau
acheminé depuis les sources après les fuites sur le troncon et 2 histogramme sont créés un pour les 10 plus grandes usines et l'autre pour les 50 plus petites
à partir des données traités


./script.sh data.csv leaks "id_usine"

cette commande permet de traiter tout le .dat et met à jour un fichier contenant l'historique des commandes leaks avec pour chacune id usine et fuite totale sur
réseau aval
