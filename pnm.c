#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "pnm.h"
#include "utils.h"

typedef enum {
    UNKNOWN = 0, 
    PBM = 1, // P1
    PGM = 2, // P2
    PPM = 3 // P3
} TYPE_PNM;

struct PNM_t
{
    TYPE_PNM type;
    size_t row;
    size_t col;
    int max_val;
    int *matrix;
};

static bool is_pnm(const char* input_ext){
    assert(input_ext);
    if(!input_ext) return false;

    char *exts[] = {"pbm", "pgm", "ppm"};
    size_t num_exts = sizeof(exts) / sizeof(exts[0]);
    for(size_t i = 0; i < num_exts; i++){
        if(!strcmp(input_ext, exts[i]))
            return true;
    }

    return false;
}

static PNM* init_pnm(void){
    PNM *image = malloc(sizeof(PNM));
    if(!image) return NULL;

    image->type = UNKNOWN;
    image->row = 0;
    image->col = 0;
    image->max_val = 0;
    image->matrix = NULL;
 
    return image;
}

static int get_number_channels(TYPE_PNM type){
    if(type == PPM) return 3;

    return 1;
}

static int allocate_matrix(PNM *image){
    if(!image) return -1;

    int channel_sz = get_number_channels(image->type);
    image->matrix = malloc(image->row * (image->col * channel_sz) * sizeof(int));
    if(!image->matrix) return -1;

    return 0;
}

void free_pnm(PNM *image){
    if(!image) return;

    if(image->matrix){
        free(image->matrix);
    }

    free(image);
}

int load_pnm(PNM **image, char* filename){
    assert(image && filename);
    if(!image || !filename) return -2;

    // check the extention name
    const char *extension = get_file_extension(filename);
    if(!is_pnm(extension)){
        fprintf(stderr, "le nom de l'extension est incorrect %s\n", extension);
        return -2;
    }

    *image = init_pnm();
    if(!(*image)){
        fprintf(stderr, "soucis init\n");
        return -1;
    }

    FILE *fp;
    fp = fopen(filename, "r");

    if(!fp){
        free_pnm(*image);
        *image = NULL;
        return -2;
    }

    // boucle de lecture du fichier 
    char magic[3];
    if(fscanf(fp, "%2s", magic) != 1 || magic[0] != 'P'){
        fprintf(stderr, "Format invalide\n");
        fclose(fp);
        free_pnm(*image);
        *image = NULL;
        return -3;
    }

    switch (magic[1])
    {
    case '1':
        (*image)->type = PBM;
        break;
    case '2':
        (*image)->type = PGM;
        break;
    case '3':
        (*image)->type = PPM;
        break;
    default:
        fprintf(stderr, "Format non supporté : %s\n", magic);
        fclose(fp);
        free_pnm(*image);
        *image = NULL;
        return -3;
    }

    skip_comments_and_spaces(fp);
    if (fscanf(fp, "%zu %zu", &(*image)->col, &(*image)->row) != 2) {
        fprintf(stderr, "Erreur de lecture des dimensions\n");
        fclose(fp);
        free_pnm(*image);
        *image = NULL;
        return -3;
    }

    if ((*image)->col == 0 || (*image)->row == 0) {
        fprintf(stderr, "Dimensions invalides : %zu x %zu\n", (*image)->col, (*image)->row);
        fclose(fp);
        free_pnm(*image);
        *image = NULL;
        return -3;
    }

    if ((*image)->type != PBM) {
        skip_comments_and_spaces(fp);
        if (fscanf(fp, "%d", &(*image)->max_val) != 1) {
            fprintf(stderr, "Erreur de lecture de la valeur max\n");
            fclose(fp);
            free_pnm(*image);
            *image = NULL;
            return -3;
        }

        if ((*image)->max_val < 1 || (*image)->max_val > 65535) {
            fprintf(stderr, "Valeur max invalide : %d (attendu entre 1 et 65535)\n", (*image)->max_val);
            fclose(fp);
            free_pnm(*image);
            *image = NULL;
            return -3;
        }
    }
    
    if(allocate_matrix(*image)){
        fprintf(stderr, "Erreur alloc de la matrice\n");
        fclose(fp);
        free_pnm(*image);
        *image = NULL;
        return -1;
    }

    int channel_sz = get_number_channels((*image)->type);
    size_t total_elements = (*image)->row * (*image)->col * channel_sz;

    for (size_t i = 0; i < total_elements; i++) {
        skip_comments_and_spaces(fp);
        if (fscanf(fp, "%d", &((*image)->matrix[i])) != 1) {
            fprintf(stderr, "Erreur de lecture des pixels (données manquantes)\n");
            fclose(fp);
            free_pnm(*image);
            *image = NULL;
            return -3;
        }
    }


    fclose(fp);
    return 0;
}

int write_pnm(PNM *image, char* filename){
    assert(image && filename);
    if(!image || !filename) return -1;

    if(!is_pnm(get_file_extension(filename))){
        fprintf(stderr, "le nom de l'extension est incorrect %s\n", filename);
        return -1;
    }

    FILE *fp = fopen(filename, "w");
    if(!fp) return -2;

    fprintf(fp, "P%d\n", image->type);

    fprintf(fp, "%zu %zu\n", image->col, image->row);

    if(image->type != PBM){
        fprintf(fp, "%d\n", image->max_val);
    }

    int channel_sz = get_number_channels(image->type);
    size_t row_width = image->col * channel_sz;

    // format PNM ASCII : aucune ligne ne doit dépasser 70 caractères
    for (size_t i = 0; i < image->row; i++) {
        size_t line_len = 0;
        for(size_t j = 0; j < row_width; j++) {
            size_t index = i * row_width + j;
            char value[12];
            size_t value_len = (size_t)snprintf(value, sizeof(value), "%d", image->matrix[index]);

            if (line_len > 0 && line_len + 1 + value_len > 70) {
                fputc('\n', fp);
                line_len = 0;
            }
            if (line_len > 0) {
                fputc(' ', fp);
                line_len++;
            }
            fputs(value, fp);
            line_len += value_len;
        }
        fputc('\n', fp);
    }

    fclose(fp);
    return 0;
}