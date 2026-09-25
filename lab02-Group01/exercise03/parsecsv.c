#include <stdio.h>
#include <string.h>
#include "record_list.h"
#include "util.h"


int main(int argc, char** argv) {

    char usage[] = "Usage: parsecsv.out <input CSV file> <output CSV file>\n\n";
    char foerr[] = "Error: unable to open/create file\n\n";

    if ( argc != 3 ) {
        fprintf(stderr, "%s", usage);
        return -1;
    }

    FILE* fin = fopen(argv[1], "r");
    if ( fin == NULL ) {
        fprintf(stderr, "Error: unable to open file %s\n\n", argv[1]);
        return -2;
    }

    int* newline = read_next_line(fin);
    record_t* head = NULL;
    record_t* curr = NULL;
    
    while ( newline != NULL ) {
        float avg = compute_average(newline);
        float sdv = compute_stdev(newline);

        curr = append(curr, avg, sdv);

        if ( head == NULL )
            head = curr;

        free(newline);
        newline = read_next_line(fin);
    }

    fclose(fin);


    /* Write the list to the output file */
    FILE* fout = fopen(argv[2], "w");

    if ( fout == NULL ) {
        fprintf(stderr, "%s", foerr);

        curr = head;

        while ( curr != NULL ) {
            record_t* temp = next(curr);
            free(curr);
            curr = temp;
        }

        return -2;
    }

    curr = head;

    while ( curr != NULL ) {
        fprintf(fout, "%f,%f\n", curr->avg, curr->sdv);
        curr = next(curr);
    }

    fclose(fout);


    /* Free all memory allocated for the list */
    curr = head;

    while ( curr != NULL ) {
        record_t* temp = next(curr);
        free(curr);
        curr = temp;
    }


    return 0;
}