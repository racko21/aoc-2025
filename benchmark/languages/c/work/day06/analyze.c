#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    FILE *f = fopen("example.txt", "r");
    char lines[10][256];
    int nlines = 0;
    while (fgets(lines[nlines], 256, f)) {
        // remove newline
        int len = strlen(lines[nlines]);
        if (lines[nlines][len-1] == '\n') lines[nlines][len-1] = 0;
        nlines++;
    }
    fclose(f);
    
    int width = 0;
    for (int i = 0; i < nlines; i++) {
        int len = strlen(lines[i]);
        if (len > width) width = len;
    }
    
    printf("nlines=%d, width=%d\n", nlines, width);
    
    // Print column by column
    for (int col = 0; col < width; col++) {
        printf("col %2d: ", col);
        for (int row = 0; row < nlines; row++) {
            char c = (col < (int)strlen(lines[row])) ? lines[row][col] : ' ';
            printf("'%c' ", c);
        }
        printf("\n");
    }
    
    return 0;
}
