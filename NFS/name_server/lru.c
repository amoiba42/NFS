#include "lru.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
int COUNT = 0;

long long int powerOfThree(int n) {
    long long int result = 1;
    while (n--) {
        result *= 3;
    }
    return result;
}


int hashFunction(const char *key) {
    unsigned long long sum = 0, i = 0;
    while (key[i] != '\0') {
        unsigned long long k = (unsigned long long)key[i++];
        sum += k * k * powerOfThree(k - 97);
    }
    return sum % CACHE_SIZE;
}


int cacheSize(LRUCache *cache) {
    return COUNT;
}


LRUCache *createCache() {
    LRUCache *cache = (LRUCache *)malloc(sizeof(LRUCache));
    if (!cache) {
        perror("Failed to allocate memory for cache");
        exit(EXIT_FAILURE);
    }
    cache->head = NULL;
    cache->tail = NULL;
    memset(cache->hashmap, 0, sizeof(cache->hashmap));
    return cache;
}


Node *createNode(const char *key, void *value) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) {
        perror("Failed to allocate memory for node");
        exit(EXIT_FAILURE);
    }
    strcpy(newNode->key, key);
    newNode->value = value;
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}


void removeFromList(LRUCache *cache, Node *node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        cache->head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        cache->tail = node->prev;
    }
    COUNT--;
}


void moveToHead(LRUCache *cache, Node *node) {
    removeFromList(cache, node);

    node->next = cache->head;
    node->prev = NULL;

    if (cache->head) {
        cache->head->prev = node;
    }

    cache->head = node;

    if (!cache->tail) {
        cache->tail = node;
    }
}


void put(LRUCache *cache, const char *key, void *value) {
    int index = hashFunction(key);
    Node *existingNode = cache->hashmap[index];

    if (existingNode) {
        // Key exists: Update value and move to head
        existingNode->value = value;
        moveToHead(cache, existingNode);
    } else {
        // Key doesn't exist: Create a new node
        Node *newNode = createNode(key, value);
        cache->hashmap[index] = newNode;

        // Add to the head of the list
        if (!cache->head) {
            cache->head = cache->tail = newNode;
        } else {
            newNode->next = cache->head;
            cache->head->prev = newNode;
            cache->head = newNode;
        }

        // Check for capacity overflow
        if (cacheSize(cache) > CACHE_SIZE) {
            Node *oldTail = cache->tail;
            cache->tail = oldTail->prev;
            if (cache->tail) cache->tail->next = NULL;

            int tailIndex = hashFunction(oldTail->key);
            cache->hashmap[tailIndex] = NULL;
            free(oldTail);
        }
        COUNT++;
    }
}

void *get(LRUCache *cache, const char *key) {
    int index = hashFunction(key);
    Node *node = cache->hashmap[index];

    if (node) {
        moveToHead(cache, node);
        return node->value;
    }
    return NULL;
}


void printCache(LRUCache *cache) {
    Node *current = cache->head;
    while (current) {
        printf("(%s, %p) ", current->key, current->value);
        current = current->next;
    }
    printf("\n");
}


void freeCache(LRUCache *cache) {
    Node *current = cache->head;
    while (current) {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    free(cache);
}

void flushCache(LRUCache *cache) {
    memset(cache, 0, sizeof(LRUCache));
}
