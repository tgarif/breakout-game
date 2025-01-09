#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>
#include <stdint.h>

/*
 * Basic utilities
 */
char* readFile(const char* filename);
char* custom_strdup(const char* str);

/*
 * DynamicArray
 */
typedef struct {
    void* array;
    size_t size;
    size_t capacity;
    size_t elementSize;
} DynamicArray;

void initialize(DynamicArray* dynArray, size_t initialCapacity, size_t elementSize);
void push(DynamicArray* dynArray, const void* value);
void insert(DynamicArray* dynArray, size_t position, const void* start, size_t count);
void cleanup(DynamicArray* dynArray);

/*
 * DynamicMap using Red Black Tree data structure
 */
typedef enum {
    KEY_TYPE_STRING,
    KEY_TYPE_INT,
    KEY_TYPE_FLOAT,
} KeyType;

typedef struct {
    KeyType type;  // Type of the key
    union {
        char* strKey;
        int intKey;
        float floatKey;
    };
} Key;

typedef struct Node {
    Key key;
    void* value;
    struct Node* left;
    struct Node* right;
    struct Node* parent;
    uint8_t color;  // 0 for black, 1 for red
} Node;

typedef struct {
    Node* root;
    Node* nil;  // Sentinel node
} DynamicMap;

void initMap(DynamicMap* map);
int compareKeys(Key a, Key b);
void leftRotate(DynamicMap* map, Node* x);
void rightRotate(DynamicMap* map, Node* x);
void insertFixup(DynamicMap* map, Node* z);
void insertIntoMap(DynamicMap* map, Key key, void* value);
void* getFromMap(DynamicMap* map, Key key);
void reverseInOrder(Node* node, Node* nil, void (*callback)(Key key, void* value, void* context), void* context);
void freeTree(Node* node, Node* nil);
void freeMap(DynamicMap* map);

#endif
