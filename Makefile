# Programme à créer
PROG = wildwater

# Sources
SRC = main.c \
      leaks.c \
      histogramme.c

# Règle par défaut : compile
all: $(PROG)

# Compilation du programme
$(PROG): $(SRC)
	gcc -Wall -Wextra -std=c99 -O2 -o $(PROG) $(SRC)

# Nettoyage
clean:
	rm -f $(PROG)
