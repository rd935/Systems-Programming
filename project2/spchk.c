#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_WORD_LENGTH 100
#define CHUNK_SIZE 1024 //adjust as needed

// Function to read the dictionary file and load it into memory
char **loadDictionary(const char *filename, int *numWords) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening dictionary file.\n");
        exit(1);
    }

    // Allocate initial memory for the dictionary array
    int capacity = CHUNK_SIZE;
    char **dictionary = (char **)malloc(capacity * sizeof(char *));
    if (dictionary == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    // Read words from the file and store them in the dictionary array
    char word[MAX_WORD_LENGTH];
    int count = 0;
    while (fscanf(file, "%s", word) != EOF) {
        // Expand dictionary array if needed
        if (count >= capacity) {
            capacity += CHUNK_SIZE;
            dictionary = (char **)realloc(dictionary, capacity * sizeof(char *));
            if (dictionary == NULL) {
                printf("Memory reallocation failed.\n");
                exit(1);
            }
        }

        // Allocate memory for the word and store it in the dictionary array
        dictionary[count] = strdup(word);
        if (dictionary[count] == NULL) {
            printf("Memory allocation failed.\n");
            exit(1);
        }
        count++;
    }

    fclose(file);
    *numWords = count;
    return dictionary;
}

// Function to check if a sub-word is in the dictionary
int checkSubWord(char *word, char **dictionary, int numWords) {
    int i;
    for (i = 0; i < numWords; i++) {
        if (strcmp(word, dictionary[i]) == 0) {
            return 1; // Sub-word found in dictionary
        }
    }
    return 0; // Sub-word not found in dictionary
}

// Function to check if a word is in the dictionary
int checkWord(char *word, char **dictionary, int numWords) {
    // Check if the word contains hyphens
    if (strchr(word, '-') != NULL) {
        // Word contains hyphens, check each sub-word separately
        char *token = strtok(word, "-");
        while (token != NULL) {
            if (!checkSubWord(token, dictionary, numWords)) {
                return 0; // Sub-word not found in dictionary
            }
            token = strtok(NULL, "-");
        }
        return 1; // All sub-words found in dictionary
    } else {
        // Word does not contain hyphens, check as a single word
        return checkSubWord(word, dictionary, numWords);
    }
}

// Function to process a text file
void processTextFile(char *filename, char **dictionary, int numWords) {
    FILE *textFile = fopen(filename, "r");
    if (textFile == NULL) {
        printf("Error opening text file: %s\n", filename);
        return;
    }

    char line[MAX_WORD_LENGTH];
    int lineNum = 0;
    while (fgets(line, MAX_WORD_LENGTH, textFile) != NULL) {
        lineNum++;

        // Tokenize the line into words
        char *token = strtok(line, " \t\n");
        int columnNum = 0;
        while (token != NULL) {
            columnNum++;

            // Remove punctuation marks from the word
            int len = strlen(token);
            while (len > 0 && !isalpha(token[len - 1])) {
                token[--len] = '\0';
            }

            // Convert word to lowercase for case-insensitive comparison
            for (int i = 0; token[i]; i++) {
                token[i] = tolower(token[i]);
            }

            if (!checkWord(token, dictionary, numWords)) {
                printf("%s (%d, %d): %s\n", filename, lineNum, columnNum, token);
            }

            token = strtok(NULL, " \t\n");
        }
    }

    fclose(textFile);
}

// Function to recursively process directories
void processDirectory(char *dirname, char **dictionary, int numWords) {
    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        printf("Error opening directory: %s\n", dirname);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char path[MAX_WORD_LENGTH];
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) == -1) {
            printf("Error statting %s\n", path);
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Recursively process subdirectories
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                processDirectory(path, dictionary, numWords);
            }
        } else {
            // Process text files
            processTextFile(path, dictionary, numWords);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <dictionary_file> <text_file(s) or directory(s)>\n", argv[0]);
        exit(1);
    }

    // Load dictionary
    int numWords;
    char **dictionary = loadDictionary(argv[1], &numWords);

    // Process text files or directories
    for (int i = 2; i < argc; i++) {
        struct stat statbuf;
        if (stat(argv[i], &statbuf) == -1) {
            printf("Error statting %s\n", argv[i]);
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            processDirectory(argv[i], dictionary, numWords);
        } else {
            processTextFile(argv[i], dictionary, numWords);
        }
    }

    // Free memory allocated for dictionary
    for (int i = 0; i < numWords; i++) {
        free(dictionary[i]);
    }
    free(dictionary);

    return 0;
}