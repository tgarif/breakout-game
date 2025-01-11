#include "util.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

long getline(char** lineptr, size_t* n, FILE* stream) {
    if (!lineptr || !n || !stream) {
        return -1;  // Invalid arguments
    }

    if (*lineptr == NULL) {
        *n = 128;  // Initial buffer size
        *lineptr = malloc(*n);
        if (!*lineptr) {
            return -1;  // Memory allocation failure
        }
    }

    size_t pos = 0;
    int c;

    while ((c = fgetc(stream)) != EOF) {
        if (pos + 1 >= *n) {  // Resize buffer if needed
            size_t new_size = *n * 2;
            char* new_lineptr = realloc(*lineptr, new_size);
            if (!new_lineptr) {
                return -1;  // Memory allocation failure
            }
            *lineptr = new_lineptr;
            *n = new_size;
        }

        (*lineptr)[pos++] = (char)c;

        if (c == '\n') {  // Stop reading at newline
            break;
        }
    }

    if (pos == 0 && c == EOF) {
        return -1;  // End of file without reading anything
    }

    (*lineptr)[pos] = '\0';  // Null-terminate the string
    return pos;              // Return the number of characters read
}

char* readFile(const char* filename) {
    FILE* fp;
    long size = 0;
    char* content;

    /* Read File to get size */
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Couldn't open file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp) + 1;
    fclose(fp);

    /* Read File for Content */
    fp = fopen(filename, "r");
    content = malloc(size);
    fread(content, 1, size - 1, fp);
    content[size - 1] = '\0';
    fclose(fp);

    return content;
}

void readAndProcessLine(const char* filename, void (*processLine)(const char* line, void* context), void* context) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Couldn't open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    char* line = NULL;
    size_t len = 0;
    long read;

    while ((read = getline(&line, &len, fp)) != -1) {
        line[strcspn(line, "\n")] = '\0';

        processLine(line, context);
    }

    free(line);
    fclose(fp);
}

char* custom_strdup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char* copy = malloc(len);
    if (copy) strcpy(copy, str);
    return copy;
}

void initialize(DynamicArray* dynArray, size_t initialCapacity, size_t elementSize) {
    dynArray->array = malloc(initialCapacity * elementSize);
    dynArray->size = 0;
    dynArray->capacity = initialCapacity;
    dynArray->elementSize = elementSize;
}

void push(DynamicArray* dynArray, const void* value) {
    if (dynArray->size == dynArray->capacity) {
        dynArray->capacity *= 2;
        dynArray->array = realloc(dynArray->array, dynArray->capacity * dynArray->elementSize);
    }
    memcpy((char*)dynArray->array + dynArray->size * dynArray->elementSize, value, dynArray->elementSize);
    dynArray->size++;
}

void pushPtr(DynamicArray* dynArray, const void* value) {
    if (dynArray->size == dynArray->capacity) {
        dynArray->capacity *= 2;
        dynArray->array = realloc(dynArray->array, dynArray->capacity * dynArray->elementSize);
    }
    memcpy((char*)dynArray->array + dynArray->size * dynArray->elementSize, &value, dynArray->elementSize);
    dynArray->size++;
}

void insert(DynamicArray* dynArray, size_t position, const void* start, size_t count) {
    if (position > dynArray->size) {
        fprintf(stderr, "Error: Insert position out of bounds.\n");
        return;
    }

    while (dynArray->size + count > dynArray->capacity) {
        dynArray->capacity *= 2;
        dynArray->array = realloc(dynArray->array, dynArray->capacity * dynArray->elementSize);
    }

    // Shift elements to the right to make space for the new range
    char* base = (char*)dynArray->array;
    memmove(base + (position + count) * dynArray->elementSize,
        base + position * dynArray->elementSize,
        (dynArray->size - position) * dynArray->elementSize);

    memcpy(base + position * dynArray->elementSize, start, count * dynArray->elementSize);

    dynArray->size += count;
}

size_t remove_if(DynamicArray* dynArray, bool (*predicate)(const void*)) {
    char* base = (char*)dynArray->array;
    size_t writeIndex = 0;

    for (size_t i = 0; i < dynArray->size; i++) {
        void* element = base + i * dynArray->elementSize;

        if (!predicate(element)) {
            if (writeIndex != i) {
                memcpy(base + writeIndex * dynArray->elementSize, element, dynArray->elementSize);
            }
            writeIndex++;
        }
    }

    return writeIndex;
}

void erase(DynamicArray* dynArray, size_t start, size_t end) {
    if (start >= dynArray->size || end > dynArray->size || start > end) {
        fprintf(stderr, "Error: Invalid erase range.\n");
        return;
    }

    char* base = (char*)dynArray->array;

    for (size_t i = start; i < end; i++) {
        *(void**)(base + i * dynArray->elementSize) = NULL;
    }

    memmove(base + start * dynArray->elementSize,
        base + end * dynArray->elementSize,
        (dynArray->size - end) * dynArray->elementSize);

    dynArray->size -= (end - start);
}

void clearArray(DynamicArray* dynArray, void (*cleanupCallback)(void*)) {
    if (cleanupCallback) {
        for (size_t i = 0; i < dynArray->size; i++) {
            void* element = (char*)dynArray->array + i * dynArray->elementSize;
            cleanupCallback(element);
        }
    }
    dynArray->size = 0;
}

void cleanup(DynamicArray* dynArray, void (*cleanupCallback)(void*)) {
    if (cleanupCallback) {
        for (size_t i = 0; i < dynArray->size; i++) {
            void* element = (char*)dynArray->array + i * dynArray->elementSize;
            cleanupCallback(element);
        }
    }
    free(dynArray->array);
    dynArray->array = NULL;
    dynArray->size = dynArray->capacity = dynArray->elementSize = 0;
}

void initMap(DynamicMap* map) {
    map->nil = malloc(sizeof(Node));
    map->nil->color = 0;  // Black
    map->nil->left = map->nil->right = map->nil->parent = NULL;
    map->root = map->nil;
}

int compareKeys(Key a, Key b) {
    if (a.type != b.type) return 0;

    float epsilon = 1e-6;
    switch (a.type) {
        case KEY_TYPE_STRING:
            return strcmp(a.strKey, b.strKey);
        case KEY_TYPE_INT:
            return a.intKey - b.intKey;
        case KEY_TYPE_FLOAT:
            if (fabs(a.floatKey - b.floatKey) < epsilon) {
                return 0;
            } else if (a.floatKey < b.floatKey) {
                return -1;
            } else {
                return 1;
            }
    }

    return 0;
}

void leftRotate(DynamicMap* map, Node* x) {
    Node* y = x->right;
    x->right = y->left;
    if (y->left != map->nil) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == map->nil) {
        map->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

void rightRotate(DynamicMap* map, Node* x) {
    Node* y = x->left;
    x->left = y->right;
    if (y->right != map->nil) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == map->nil) {
        map->root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

void insertFixup(DynamicMap* map, Node* z) {
    while (z->parent->color == 1) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right;
            if (y->color == 1) {
                z->parent->color = 0;
                y->color = 0;
                z->parent->parent->color = 1;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    leftRotate(map, z);
                }
                z->parent->color = 0;
                z->parent->parent->color = 1;
                rightRotate(map, z->parent->parent);
            }
        } else {
            Node* y = z->parent->parent->left;
            if (y->color == 1) {
                z->parent->color = 0;
                y->color = 0;
                z->parent->parent->color = 1;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(map, z);
                }
                z->parent->color = 0;
                z->parent->parent->color = 1;
                leftRotate(map, z->parent->parent);
            }
        }
    }
    map->root->color = 0;
}

void insertIntoMap(DynamicMap* map, Key key, void* value) {
    Node* z = malloc(sizeof(Node));
    z->key = key;
    z->value = value;
    z->color = 1;
    z->left = z->right = map->nil;

    Node* y = map->nil;
    Node* x = map->root;
    int cmp = 0;

    while (x != map->nil) {
        y = x;
        cmp = compareKeys(key, x->key);

        if (cmp == 0) {
            x->value = value;
            free(z);
            return;
        } else if (cmp < 0) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    z->parent = y;
    if (y == map->nil) {
        map->root = z;
    } else if (cmp < 0) {
        y->left = z;
    } else {
        y->right = z;
    }

    insertFixup(map, z);
}

void* getFromMap(DynamicMap* map, Key key) {
    Node* current = map->root;
    while (current != map->nil) {
        int cmp = compareKeys(key, current->key);
        if (cmp == 0) {
            return current->value;
        } else if (cmp < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    return NULL;
}

void reverseInOrder(Node* node, Node* nil, void (*callback)(Key key, void* value, void* context), void* context) {
    if (node == nil) return;
    reverseInOrder(node->right, nil, callback, context);
    callback(node->key, node->value, context);
    reverseInOrder(node->left, nil, callback, context);
}

void traverseInOrder(Node* node, Node* nil, void (*callback)(Key key, void* value, void* context), void* context) {
    if (node == nil) return;
    traverseInOrder(node->left, nil, callback, context);
    callback(node->key, node->value, context);
    traverseInOrder(node->right, nil, callback, context);
}

void clearMap(DynamicMap* map) {
    freeTree(map->root, map->nil);
    map->root = map->nil;
}

void freeTree(Node* node, Node* nil) {
    if (node == nil) return;
    freeTree(node->left, nil);
    freeTree(node->right, nil);
    free(node);
}

void freeMap(DynamicMap* map) {
    freeTree(map->root, map->nil);
    free(map->nil);
}
