#include "util.h"

int* read_next_line(FILE* fin) {
    int capacity = 8;
    int count = 0;

    int* values = (int*)malloc((capacity + 1) * sizeof(int));
    if ( values == NULL )
        return NULL;

    int c;
    int value = 0;
    int sign = 1;
    int reading_number = 0;

    while ( (c = fgetc(fin)) != EOF ) {

        if ( c == '-' && !reading_number ) {
            sign = -1;
            value = 0;
            reading_number = 1;
        }
        else if ( c >= '0' && c <= '9' ) {
            value = value * 10 + (c - '0');
            reading_number = 1;
        }
        else if ( c == ',' || c == '\n' || c == '\r' ) {

            if ( reading_number ) {
                if ( count >= capacity ) {
                    capacity *= 2;

                    int* temp = (int*)realloc(
                        values,
                        (capacity + 1) * sizeof(int)
                    );

                    if ( temp == NULL ) {
                        free(values);
                        return NULL;
                    }

                    values = temp;
                }

                count++;
                values[count] = sign * value;

                value = 0;
                sign = 1;
                reading_number = 0;
            }

            if ( c == '\n' )
                break;

            if ( c == '\r' ) {
                int next_char = fgetc(fin);

                if ( next_char != '\n' && next_char != EOF )
                    ungetc(next_char, fin);

                break;
            }
        }
    }

    if ( c == EOF && count == 0 && !reading_number ) {
        free(values);
        return NULL;
    }

    /* Last value if the file ends without a newline */
    if ( reading_number ) {
        if ( count >= capacity ) {
            capacity *= 2;

            int* temp = (int*)realloc(
                values,
                (capacity + 1) * sizeof(int)
            );

            if ( temp == NULL ) {
                free(values);
                return NULL;
            }

            values = temp;
        }

        count++;
        values[count] = sign * value;
    }

    values[0] = count;

    return values;
}


float compute_average(int* line) {
    int count = line[0];
    float sum = 0.0f;

    for ( int i = 1; i <= count; i++ ) {
        sum += line[i];
    }

    return sum / count;
}


float compute_stdev(int* line) {
    int count = line[0];
    float avg = compute_average(line);
    float sum = 0.0f;

    for ( int i = 1; i <= count; i++ ) {
        float difference = line[i] - avg;
        sum += difference * difference;
    }

    return sqrtf(sum / count);
}