#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

const char* get_file_extension(const char* filename){
    assert(filename);
    if(!filename) return "";

    const char *dot = strrchr(filename, '.');

    if(!dot || dot == filename) return "";

    return dot+1;
}

void skip_comments_and_spaces(FILE *fp) {
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '#') {
            // C'est un commentaire : on lit jusqu'à la fin de la ligne
            while ((ch = fgetc(fp)) != EOF && ch != '\n');
        } else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            // C'est un espace blanc : on l'ignore
            continue;
        } else {
            // C'est un caractère utile (chiffre ou lettre) : on le remet dans le flux
            ungetc(ch, fp);
            break;
        }
    }
}
