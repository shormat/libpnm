#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>

#include "pnm.h"

static void usage(const char *prog) {
    fprintf(stderr, "Usage : %s -f <PBM|PGM|PPM> <chemin_du_fichier>\n", prog);
    fprintf(stderr, "        %s -h\n", prog);
}

// compare deux chaînes sans tenir compte de la casse
static int same_ignore_case(const char *a, const char *b) {
    while (*a && *b) {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return *a == *b;
}

// vérifie que le format est PBM, PGM ou PPM (insensible à la casse)
static int is_valid_format(const char *format) {
    const char *formats[] = {"PBM", "PGM", "PPM"};
    size_t n = sizeof(formats) / sizeof(formats[0]);

    for (size_t i = 0; i < n; i++) {
        if (same_ignore_case(format, formats[i]))
            return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {

   /*
    *
    *    RAPPEL : argv[0] = "./NOMDUPROGRAMME"
    *             argv[1] = l'option "-f", "-h" etc...
    *             argv[2] = l'extention du fichier -> PPM, PGM ou PBM
    *             argv[3] = le chemin du fichier
    *
    */

    char *format = NULL;
    int opt;

    /*
     * ':' en tête de la chaîne : getopt n'affiche rien lui-même et renvoie
     * ':' si une option manque d'argument, '?' si l'option est inconnue.
     * Dans les deux cas, le caractère fautif est dans optopt.
     */
    opterr = 0;
    while ((opt = getopt(argc, argv, ":f:h")) != -1) {
        switch (opt) {
        case 'f':
            format = optarg; // argv[2]
            break;
        case 'h':
            usage(argv[0]);
            return EXIT_SUCCESS;
        case ':':
            fprintf(stderr, "L'option -%c attend un argument (PBM, PGM ou PPM)\n", optopt);
            usage(argv[0]);
            return EXIT_FAILURE;
        case '?':
            if (isprint(optopt))
                fprintf(stderr, "Option inconnue : -%c\n", optopt);
            else
                fprintf(stderr, "Caractère d'option inconnu : \\x%x\n", optopt);
            usage(argv[0]);
            return EXIT_FAILURE;
        default:
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (!format) {
        fprintf(stderr, "L'option -f est obligatoire\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (!is_valid_format(format)) {
        fprintf(stderr, "Format invalide : %s (attendu PBM, PGM ou PPM)\n", format);
        return EXIT_FAILURE;
    }

    // après getopt, optind pointe sur le premier argument non-option : argv[3]
    if (optind >= argc) {
        fprintf(stderr, "Chemin du fichier manquant\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (optind + 1 < argc) {
        fprintf(stderr, "Trop d'arguments\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    char *filename = argv[optind];

    // le format donné avec -f doit correspondre à l'extension du fichier
    const char *dot = strrchr(filename, '.');
    if (!dot || !same_ignore_case(dot + 1, format)) {
        fprintf(stderr, "Le format %s ne correspond pas au fichier %s\n", format, filename);
        return EXIT_FAILURE;
    }

    PNM *image = NULL;
    int ret = load_pnm(&image, filename);
    switch (ret) {
    case 0:
        printf("Image %s chargée (%s)\n", filename, format);
        break;
    case -1:
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        return EXIT_FAILURE;
    case -2:
        fprintf(stderr, "Nom de fichier malformé : %s\n", filename);
        return EXIT_FAILURE;
    case -3:
        fprintf(stderr, "Contenu du fichier malformé : %s\n", filename);
        return EXIT_FAILURE;
    default:
        fprintf(stderr, "Erreur inconnue (%d)\n", ret);
        return EXIT_FAILURE;
    }

    // nom de sortie : output.<ext>, ext en minuscules (ppm, pgm ou pbm)
    char output[sizeof("output.ppm")] = "output.";
    for (size_t i = 0; i < 3; i++)
        output[strlen("output.") + i] = (char)tolower((unsigned char)format[i]);
    output[sizeof(output) - 1] = '\0';

    ret = write_pnm(image, output);
    free_pnm(image);

    switch (ret) {
    case 0:
        printf("Image écrite dans %s\n", output);
        break;
    case -1:
        fprintf(stderr, "Nom du fichier de sortie malformé : %s\n", output);
        return EXIT_FAILURE;
    case -2:
        fprintf(stderr, "Erreur lors de l'écriture de %s\n", output);
        return EXIT_FAILURE;
    default:
        fprintf(stderr, "Erreur inconnue à l'écriture (%d)\n", ret);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
