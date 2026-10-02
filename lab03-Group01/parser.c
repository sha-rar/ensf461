#include "parser.h"
#include <ctype.h>

// Function to trim whitespace and ASCII control characters from buffer
// [Input] char* inputbuffer - input string to trim
// [Input] size_t bufferlen - size of input and output string buffers
// [Output] char* outputbuffer - output string after trimming
// [Return] size_t - size of output string after trimming
size_t trimstring(char* outputbuffer, const char* inputbuffer, size_t bufferlen)
{
    if (bufferlen == 0) {
        return 0;
    }

    size_t length = strnlen(inputbuffer, bufferlen - 1);
    memcpy(outputbuffer, inputbuffer, length);
    outputbuffer[length] = '\0';

    while (length > 0 && (unsigned char)outputbuffer[length - 1] < '!') {
        outputbuffer[length - 1] = '\0';
        length--;
    }

    return length;
}

// Parse the command line into an argv-style array.
int parseargs(char* inputbuffer, char* args[], size_t maxargs)
{
    if (maxargs == 0) {
        return -1;
    }

    size_t argc = 0;
    char* src = inputbuffer;
    char* dst = inputbuffer;

    while (*src != '\0') {
        while (isspace((unsigned char)*src)) {
            src++;
        }

        if (*src == '\0') {
            break;
        }

        if (argc >= maxargs - 1) {
            args[0] = NULL;
            return -1;
        }

        args[argc++] = dst;
        char quote = '\0';

        while (*src != '\0') {
            if (quote != '\0') {
                if (*src == quote) {
                    quote = '\0';
                    src++;
                } else {
                    *dst++ = *src++;
                }
            } else if (*src == '"' || *src == '\'') {
                quote = *src++;
            } else if (isspace((unsigned char)*src)) {
                break;
            } else {
                *dst++ = *src++;
            }
        }

        if (quote != '\0') {
            args[0] = NULL;
            return -2;
        }

        while (isspace((unsigned char)*src)) {
            src++;
        }

        *dst++ = '\0';
    }

    args[argc] = NULL;
    return (int)argc;
}

// Function to trim the input command to just be the first word
// [Input] char* inputbuffer - input string to trim
// [Input] size_t bufferlen - size of input and output string buffers
// [Output] char* outputbuffer - output string after trimming
// [Return] size_t - size of output string after trimming
size_t firstword(char* outputbuffer, const char* inputbuffer, size_t bufferlen)
{
    // TO DO: Implement this function
    return 0;
}

// Function to test that string only contains valid ascii characters (non-control and not extended)
// [Input] char* inputbuffer - input string to test
// [Input] size_t bufferlen - size of input buffer
// [Return] bool - true if no invalid ASCII characters present
bool isvalidascii(const char* inputbuffer, size_t bufferlen)
{
    // TO DO: Correct this function so that the second test string fails
    size_t testlen = bufferlen;
    size_t stringlength = strlen(inputbuffer);
    if (strlen(inputbuffer) < bufferlen) {
        testlen = stringlength;
    }

    bool isValid = true;
    for (size_t ii = 0; ii < testlen; ii++) {
        isValid &= ((unsigned char)inputbuffer[ii] <= '~');
    }

    return isValid;
}

// Function to find location of pipe character in input string
// [Input] char* inputbuffer - input string to test
// [Input] size_t bufferlen - size of input buffer
// [Return] int - location in the string of the pipe character, or -1 if not found
int findpipe(const char* inputbuffer, size_t bufferlen)
{
    // TO DO: Implement this function
    return -1;
}
