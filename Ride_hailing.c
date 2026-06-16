#include "Ride_hailing.h"

#define ORDER 4
#define MAX_KEYS (ORDER - 1)

typedef struct BPlusNode {
    int isLeaf;
    int keyCount;
    long long keys[MAX_KEYS];
    struct BPlusNode *children[ORDER];
    void *records[MAX_KEYS];
    struct BPlusNode *next;
} BPlusNode;

    typedef struct InsertResult {
        int hasSplit; //splited the value or not
        long long promotedKey; //the value that has moved up
        BPlusNode *rightNode; //pointer to the next right node
    } InsertResult;

typedef struct PairCount {
    int d_id;
    int p_id;
    int count;
} PairCount;

typedef struct DeleteResult {
    int deleted;
    int underflow;
    void *deletedRecord;
} DeleteResult;

BPlusNode *g_driverRoot = NULL;
BPlusNode *g_passengerRoot = NULL;
BPlusNode *g_bookingRoot = NULL;
int g_nextDriverId = 1;
int g_nextPassengerId = 1;
int g_nextBookingId = 1;

float distanceCalc(int x1, int y1, int x2, int y2) {
    float dx = (float)(x1 - x2);
    float dy = (float)(y1 - y2);
    return sqrtf(dx * dx + dy * dy);
}

void trimNewline(char *s) {
    size_t n;
    if (s == NULL) {
        return;
    }
    n = strlen(s);
    if (n > 0U && s[n - 1] == '\n') {
        s[n - 1] = '\0';
    }
}

BPlusNode *createNode(int isLeaf) {
    int i;
    BPlusNode *node = (BPlusNode *)malloc(sizeof(BPlusNode));
    if (node == NULL) {
        return NULL;
    }

    node->isLeaf = isLeaf;
    node->keyCount = 0;
    node->next = NULL;

    for (i = 0; i < ORDER; ++i) {
        node->children[i] = NULL;
        if (i < MAX_KEYS) {
            node->keys[i] = 0;
            node->records[i] = NULL;
        }
    }

    return node;
}

BPlusNode *findLeaf(BPlusNode *root, long long key) {
    BPlusNode *cur = root;

    while (cur != NULL && !cur->isLeaf) {
        int i = 0;
        while (i < cur->keyCount && key >= cur->keys[i]) {
            ++i;
        }
        cur = cur->children[i];
    }

    return cur;
}

void *searchInTree(BPlusNode *root, long long key) {
    int i;
    BPlusNode *leaf = findLeaf(root, key);

    if (leaf == NULL) {
        return NULL;
    }

    for (i = 0; i < leaf->keyCount; ++i) {
        if (leaf->keys[i] == key) {
            return leaf->records[i];
        }
    }

    return NULL;
}

BPlusNode *getLeftmostLeaf(BPlusNode *root) {
    BPlusNode *cur = root;
    while (cur != NULL && !cur->isLeaf) {
        cur = cur->children[0];
    }
    return cur;
}

InsertResult insertRecursive(BPlusNode *node, long long key, void *record) {
    InsertResult res;
    int i;

    res.hasSplit = 0;
    res.promotedKey = 0;
    res.rightNode = NULL;

    if (node->isLeaf) {
        int pos = 0;
        long long tempKeys[MAX_KEYS + 1];
        void *tempRecords[MAX_KEYS + 1];
        int total;
        int leftCount;
        BPlusNode *newLeaf;

        while (pos < node->keyCount && node->keys[pos] < key) {
            ++pos;
        }

        if (pos < node->keyCount && node->keys[pos] == key) {
            node->records[pos] = record;
            return res;
        }

        for (i = 0; i < pos; ++i) {
            tempKeys[i] = node->keys[i];
            tempRecords[i] = node->records[i];
        }

        tempKeys[pos] = key;
        tempRecords[pos] = record;

        for (i = pos; i < node->keyCount; ++i) {
            tempKeys[i + 1] = node->keys[i];
            tempRecords[i + 1] = node->records[i];
        }

        total = node->keyCount + 1;
        if (total <= MAX_KEYS) {
            node->keyCount = total;
            for (i = 0; i < total; ++i) {
                node->keys[i] = tempKeys[i];
                node->records[i] = tempRecords[i];
            }
            return res;
        }

        newLeaf = createNode(1);
        if (newLeaf == NULL) {
            return res;
        }

        leftCount = total / 2;
        node->keyCount = leftCount;
        for (i = 0; i < leftCount; ++i) {
            node->keys[i] = tempKeys[i];
            node->records[i] = tempRecords[i];
        }

        newLeaf->keyCount = total - leftCount;
        for (i = 0; i < newLeaf->keyCount; ++i) {
            newLeaf->keys[i] = tempKeys[leftCount + i];
            newLeaf->records[i] = tempRecords[leftCount + i];
        }

        newLeaf->next = node->next;
        node->next = newLeaf;

        res.hasSplit = 1;
        res.promotedKey = newLeaf->keys[0];
        res.rightNode = newLeaf;
        return res;
    }

    {
        int childIndex = 0;
        InsertResult childRes;
        long long tempKeys[MAX_KEYS + 1];
        BPlusNode *tempChildren[ORDER + 1];
        int total;
        int mid;
        BPlusNode *newInternal;

        while (childIndex < node->keyCount && key >= node->keys[childIndex]) {
            ++childIndex;
        }

        childRes = insertRecursive(node->children[childIndex], key, record);
        if (!childRes.hasSplit) {
            return childRes;
        }

        for (i = 0; i < childIndex; ++i) {
            tempKeys[i] = node->keys[i];
        }
        tempKeys[childIndex] = childRes.promotedKey;
        for (i = childIndex; i < node->keyCount; ++i) {
            tempKeys[i + 1] = node->keys[i];
        }

        for (i = 0; i <= childIndex; ++i) {
            tempChildren[i] = node->children[i];
        }
        tempChildren[childIndex + 1] = childRes.rightNode;
        for (i = childIndex + 1; i <= node->keyCount; ++i) {
            tempChildren[i + 1] = node->children[i];
        }

        total = node->keyCount + 1;
        if (total <= MAX_KEYS) {
            node->keyCount = total;
            for (i = 0; i < total; ++i) {
                node->keys[i] = tempKeys[i];
            }
            for (i = 0; i <= total; ++i) {
                node->children[i] = tempChildren[i];
            }

            childRes.hasSplit = 0;
            childRes.promotedKey = 0;
            childRes.rightNode = NULL;
            return childRes;
        }

        newInternal = createNode(0);
        if (newInternal == NULL) {
            childRes.hasSplit = 0;
            childRes.promotedKey = 0;
            childRes.rightNode = NULL;
            return childRes;
        }

        mid = total / 2;
        node->keyCount = mid;
        for (i = 0; i < mid; ++i) {
            node->keys[i] = tempKeys[i];
        }
        for (i = 0; i <= mid; ++i) {
            node->children[i] = tempChildren[i];
        }

        newInternal->keyCount = total - mid - 1;
        for (i = 0; i < newInternal->keyCount; ++i) {
            newInternal->keys[i] = tempKeys[mid + 1 + i];
        }
        for (i = 0; i <= newInternal->keyCount; ++i) {
            newInternal->children[i] = tempChildren[mid + 1 + i];
        }

        res.hasSplit = 1;
        res.promotedKey = tempKeys[mid];
        res.rightNode = newInternal;
        return res;
    }
}

BPlusNode *insert(BPlusNode *root, long long key, void *record) {
    if (root == NULL) {
        root = createNode(1);
        if (root == NULL) {
            return NULL;
        }
        root->keys[0] = key;
        root->records[0] = record;
        root->keyCount = 1;
        return root;
    }

    {
        InsertResult res = insertRecursive(root, key, record);
        if (res.hasSplit) {
            BPlusNode *newRoot = createNode(0);
            if (newRoot == NULL) {
                return root;
            }
            newRoot->keys[0] = res.promotedKey;
            newRoot->children[0] = root;
            newRoot->children[1] = res.rightNode;
            newRoot->keyCount = 1;
            return newRoot;
        }
    }

    return root;
}

int minLeafKeys(void) {
    return (MAX_KEYS + 1) / 2;
}

int minInternalKeys(void) {
    return ((ORDER + 1) / 2) - 1;
}

long long firstKeyInSubtree(BPlusNode *node) {
    BPlusNode *cur = node;
    while (cur != NULL && !cur->isLeaf) {
        cur = cur->children[0];
    }
    return (cur != NULL && cur->keyCount > 0) ? cur->keys[0] : 0;
}

void refreshInternalKeys(BPlusNode *node) {
    int i;
    if (node == NULL || node->isLeaf) {
        return;
    }
    for (i = 0; i < node->keyCount; ++i) {
        node->keys[i] = firstKeyInSubtree(node->children[i + 1]);
    }
}

DeleteResult deleteRecursive(BPlusNode *node, long long key) {
    DeleteResult res;
    int i;

    res.deleted = 0;
    res.underflow = 0;
    res.deletedRecord = NULL;

    if (node == NULL) {
        return res;
    }

    if (node->isLeaf) {
        int pos = -1;
        for (i = 0; i < node->keyCount; ++i) {
            if (node->keys[i] == key) {
                pos = i;
                break;
            }
        }

        if (pos == -1) {
            return res;
        }

        res.deleted = 1;
        res.deletedRecord = node->records[pos];

        for (i = pos; i < node->keyCount - 1; ++i) {
            node->keys[i] = node->keys[i + 1];
            node->records[i] = node->records[i + 1];
        }
        node->keyCount -= 1;

        if (node->keyCount < minLeafKeys()) {
            res.underflow = 1;
        }

        return res;
    }

    {
        int childIndex = 0;
        DeleteResult childRes;

        while (childIndex < node->keyCount && key >= node->keys[childIndex]) {
            ++childIndex;
        }

        childRes = deleteRecursive(node->children[childIndex], key);
        if (!childRes.deleted) {
            return childRes;
        }

        if (childRes.underflow) {
            BPlusNode *child = node->children[childIndex];
            BPlusNode *leftSibling = (childIndex > 0) ? node->children[childIndex - 1] : NULL;
            BPlusNode *rightSibling = (childIndex < node->keyCount) ? node->children[childIndex + 1] : NULL;
            int minKeys = child->isLeaf ? minLeafKeys() : minInternalKeys();

            if (leftSibling != NULL && leftSibling->keyCount > minKeys) {
                if (child->isLeaf) {
                    for (i = child->keyCount; i > 0; --i) {
                        child->keys[i] = child->keys[i - 1];
                        child->records[i] = child->records[i - 1];
                    }
                    child->keys[0] = leftSibling->keys[leftSibling->keyCount - 1];
                    child->records[0] = leftSibling->records[leftSibling->keyCount - 1];
                    child->keyCount += 1;
                    leftSibling->keyCount -= 1;
                    node->keys[childIndex - 1] = child->keys[0];
                } else {
                    for (i = child->keyCount; i > 0; --i) {
                        child->keys[i] = child->keys[i - 1];
                    }
                    for (i = child->keyCount + 1; i > 0; --i) {
                        child->children[i] = child->children[i - 1];
                    }
                    child->keys[0] = node->keys[childIndex - 1];
                    child->children[0] = leftSibling->children[leftSibling->keyCount];
                    child->keyCount += 1;

                    node->keys[childIndex - 1] = leftSibling->keys[leftSibling->keyCount - 1];
                    leftSibling->keyCount -= 1;
                }
                childRes.underflow = 0;
            } else if (rightSibling != NULL && rightSibling->keyCount > minKeys) {
                if (child->isLeaf) {
                    child->keys[child->keyCount] = rightSibling->keys[0];
                    child->records[child->keyCount] = rightSibling->records[0];
                    child->keyCount += 1;

                    for (i = 0; i < rightSibling->keyCount - 1; ++i) {
                        rightSibling->keys[i] = rightSibling->keys[i + 1];
                        rightSibling->records[i] = rightSibling->records[i + 1];
                    }
                    rightSibling->keyCount -= 1;
                    node->keys[childIndex] = rightSibling->keys[0];
                } else {
                    child->keys[child->keyCount] = node->keys[childIndex];
                    child->children[child->keyCount + 1] = rightSibling->children[0];
                    child->keyCount += 1;

                    node->keys[childIndex] = rightSibling->keys[0];
                    for (i = 0; i < rightSibling->keyCount - 1; ++i) {
                        rightSibling->keys[i] = rightSibling->keys[i + 1];
                    }
                    for (i = 0; i < rightSibling->keyCount; ++i) {
                        rightSibling->children[i] = rightSibling->children[i + 1];
                    }
                    rightSibling->keyCount -= 1;
                }
                childRes.underflow = 0;
            } else {
                if (leftSibling != NULL) {
                    if (child->isLeaf) {
                        int base = leftSibling->keyCount;
                        for (i = 0; i < child->keyCount; ++i) {
                            leftSibling->keys[base + i] = child->keys[i];
                            leftSibling->records[base + i] = child->records[i];
                        }
                        leftSibling->keyCount += child->keyCount;
                        leftSibling->next = child->next;
                    } else {
                        int base = leftSibling->keyCount;
                        leftSibling->keys[base] = node->keys[childIndex - 1];
                        for (i = 0; i < child->keyCount; ++i) {
                            leftSibling->keys[base + 1 + i] = child->keys[i];
                        }
                        for (i = 0; i <= child->keyCount; ++i) {
                            leftSibling->children[base + 1 + i] = child->children[i];
                        }
                        leftSibling->keyCount += child->keyCount + 1;
                    }

                    free(child);

                    for (i = childIndex - 1; i < node->keyCount - 1; ++i) {
                        node->keys[i] = node->keys[i + 1];
                    }
                    for (i = childIndex; i < node->keyCount; ++i) {
                        node->children[i] = node->children[i + 1];
                    }
                    node->keyCount -= 1;
                } else if (rightSibling != NULL) {
                    if (child->isLeaf) {
                        int base = child->keyCount;
                        for (i = 0; i < rightSibling->keyCount; ++i) {
                            child->keys[base + i] = rightSibling->keys[i];
                            child->records[base + i] = rightSibling->records[i];
                        }
                        child->keyCount += rightSibling->keyCount;
                        child->next = rightSibling->next;
                    } else {
                        int base = child->keyCount;
                        child->keys[base] = node->keys[childIndex];
                        for (i = 0; i < rightSibling->keyCount; ++i) {
                            child->keys[base + 1 + i] = rightSibling->keys[i];
                        }
                        for (i = 0; i <= rightSibling->keyCount; ++i) {
                            child->children[base + 1 + i] = rightSibling->children[i];
                        }
                        child->keyCount += rightSibling->keyCount + 1;
                    }

                    free(rightSibling);

                    for (i = childIndex; i < node->keyCount - 1; ++i) {
                        node->keys[i] = node->keys[i + 1];
                    }
                    for (i = childIndex + 1; i < node->keyCount; ++i) {
                        node->children[i] = node->children[i + 1];
                    }
                    node->keyCount -= 1;
                }
            }
        }

        if (node->keyCount > 0) {
            refreshInternalKeys(node);
        }

        childRes.underflow = (node->keyCount < minInternalKeys());
        return childRes;
    }
}

BPlusNode *deleteFromTree(BPlusNode *root, long long key, void **deletedRecord) {
    DeleteResult res;

    if (deletedRecord != NULL) {
        *deletedRecord = NULL;
    }

    if (root == NULL) {
        return NULL;
    }

    res = deleteRecursive(root, key);

    if (deletedRecord != NULL) {
        *deletedRecord = res.deletedRecord;
    }

    if (!res.deleted) {
        return root;
    }

    if (root->isLeaf && root->keyCount == 0) {
        free(root);
        return NULL;
    }

    if (!root->isLeaf && root->keyCount == 0) {
        BPlusNode *newRoot = root->children[0];
        free(root);
        return newRoot;
    }

    return root;
}

void freeTreeNodes(BPlusNode *root) {
    int i;
    if (root == NULL) {
        return;
    }

    if (!root->isLeaf) {
        for (i = 0; i <= root->keyCount; ++i) {
            freeTreeNodes(root->children[i]);
        }
    }

    free(root);
}

void freeTreeRecords(BPlusNode *root) {
    BPlusNode *leaf = getLeftmostLeaf(root);
    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            free(leaf->records[i]);
        }
        leaf = leaf->next;
    }
}

Driver *searchDriver(int d_id) {
    return (Driver *)searchInTree(g_driverRoot, (long long)d_id);
}

Passenger *searchPassenger(int p_id) {
    return (Passenger *)searchInTree(g_passengerRoot, (long long)p_id);
}

Booking *searchBooking(int booking_id) {
    return (Booking *)searchInTree(g_bookingRoot, (long long)booking_id);
}

int isValidMobileNumber(const char *mobile) {
    int i;

    if (mobile == NULL || strlen(mobile) != 10U) {
        return 0;
    }

    for (i = 0; i < 10; ++i) {
        if (mobile[i] < '0' || mobile[i] > '9') {
            return 0;
        }
    }

    return 1;
}

int passengerMobileExistsInTree(const char *mobile) {
    BPlusNode *leaf = getLeftmostLeaf(g_passengerRoot);
    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            Passenger *p = (Passenger *)leaf->records[i];
            if (p != NULL && strcmp(p->mobile_no, mobile) == 0) {
                return 1;
            }
        }
        leaf = leaf->next;
    }
    return 0;
}

long long pairKey(int d_id, int p_id) {
    return ((long long)d_id << 32) | (unsigned int)p_id;
}

void freePairTree(BPlusNode *pairRoot) {
    BPlusNode *leaf = getLeftmostLeaf(pairRoot);
    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            free(leaf->records[i]);
        }
        leaf = leaf->next;
    }
    freeTreeNodes(pairRoot);
}

void saveDrivers(void) {
    FILE *f = fopen(DRIVER_DATA_FILE, "w");
    BPlusNode *leaf;

    if (f == NULL) {
        return;
    }

    leaf = getLeftmostLeaf(g_driverRoot);
    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            Driver *d = (Driver *)leaf->records[i];
            fprintf(f, "%d|%s|%d|%d|%d|%d|%.2f\n",
                d->d_ID,
                d->name,
                d->vehicle_type,
                d->current_location.x,
                d->current_location.y,
                d->status,
                d->total_earnings);
        }
        leaf = leaf->next;
    }

    fclose(f);
}

void savePassengers(void) {
    FILE *f = fopen(PASSENGER_DATA_FILE, "w");
    BPlusNode *leaf;

    if (f == NULL) {
        return;
    }

    leaf = getLeftmostLeaf(g_passengerRoot);
    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            Passenger *p = (Passenger *)leaf->records[i];
            fprintf(f, "%d|%s|%s|%d\n",
                p->p_ID,
                p->name,
                p->mobile_no,
                p->frequency);
        }
        leaf = leaf->next;
    }

    fclose(f);
}

void saveBookings(void) {
    FILE *f = fopen(BOOKING_DATA_FILE, "w");
    BPlusNode *leaf;

    if (f == NULL) {
        return;
    }

    leaf = getLeftmostLeaf(g_bookingRoot);
    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            Booking *b = (Booking *)leaf->records[i];
            fprintf(f, "%d|%d|%d|%d|%.2f|%.2f|%d\n",
                b->booking_id,
                b->d_ID,
                b->p_ID,
                b->vehicle_type,
                b->distance_travelled,
                b->fare,
                b->timestamp);
        }
        leaf = leaf->next;
    }

    fclose(f);
}

void loadData(void) {
    FILE *fp;
    char line[256];
    int maxDriverId = 0;
    int maxPassengerId = 0;

    fp = fopen(DRIVER_DATA_FILE, "a");
    if (fp != NULL) {
        fclose(fp);
    }
    fp = fopen(PASSENGER_DATA_FILE, "a");
    if (fp != NULL) {
        fclose(fp);
    }
    fp = fopen(BOOKING_DATA_FILE, "a");
    if (fp != NULL) {
        fclose(fp);
    }

    fp = fopen(DRIVER_DATA_FILE, "r");
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp) != NULL) {
            int id;
            char name[MAX_NAME_LEN];
            int type;
            int x;
            int y;
            int status;
            float earnings;

            trimNewline(line);
            if (sscanf(line, "%d|%49[^|]|%d|%d|%d|%d|%f", &id, name, &type, &x, &y, &status, &earnings) == 7) {
                Driver *d = (Driver *)malloc(sizeof(Driver));
                if (d != NULL) {
                    d->d_ID = id;
                    strncpy(d->name, name, MAX_NAME_LEN - 1);
                    d->name[MAX_NAME_LEN - 1] = '\0';
                    d->vehicle_type = type;
                    d->current_location.x = x;
                    d->current_location.y = y;
                    d->status = status;
                    d->total_earnings = earnings;
                    g_driverRoot = insert(g_driverRoot, (long long)d->d_ID, d);
                    if (d->d_ID > maxDriverId) {
                        maxDriverId = d->d_ID;
                    }
                }
            }
        }
        fclose(fp);
    }

    fp = fopen(PASSENGER_DATA_FILE, "r");
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp) != NULL) {
            int id;
            char name[MAX_NAME_LEN];
            char mobile[MAX_MOBILE_LEN];
            int frequency;

            trimNewline(line);
            if (sscanf(line, "%d|%49[^|]|%14[^|]|%d", &id, name, mobile, &frequency) == 4) {
                Passenger *p = (Passenger *)malloc(sizeof(Passenger));
                if (p != NULL) {
                    p->p_ID = id;
                    strncpy(p->name, name, MAX_NAME_LEN - 1);
                    p->name[MAX_NAME_LEN - 1] = '\0';
                    strncpy(p->mobile_no, mobile, MAX_MOBILE_LEN - 1);
                    p->mobile_no[MAX_MOBILE_LEN - 1] = '\0';
                    p->frequency = frequency;
                    g_passengerRoot = insert(g_passengerRoot, (long long)p->p_ID, p);
                    if (p->p_ID > maxPassengerId) {
                        maxPassengerId = p->p_ID;
                    }
                }
            }
        }
        fclose(fp);
    }

    {
        int maxBookingId = 0;
        fp = fopen(BOOKING_DATA_FILE, "r");
        if (fp != NULL) {
            while (fgets(line, sizeof(line), fp) != NULL) {
                int booking_id;
                int d_id;
                int p_id;
                int vehicle_type;
                float distance;
                float fare;
                int timestamp;

                trimNewline(line);
                if (sscanf(line, "%d|%d|%d|%d|%f|%f|%d", &booking_id, &d_id, &p_id, &vehicle_type, &distance, &fare, &timestamp) == 7) {
                    Booking *b = (Booking *)malloc(sizeof(Booking));
                    if (b != NULL) {
                        b->booking_id = booking_id;
                        b->d_ID = d_id;
                        b->p_ID = p_id;
                        b->vehicle_type = vehicle_type;
                        b->distance_travelled = distance;
                        b->fare = fare;
                        b->timestamp = timestamp;
                        g_bookingRoot = insert(g_bookingRoot, (long long)b->booking_id, b);
                        if (b->booking_id > maxBookingId) {
                            maxBookingId = b->booking_id;
                        }
                    }
                }
            }
            fclose(fp);
        }
        g_nextBookingId = maxBookingId + 1;
    }

    g_nextDriverId = maxDriverId + 1;
    g_nextPassengerId = maxPassengerId + 1;
}

void addDriver(char name[], int type, int x, int y) {
    Driver *d;
    int id;

    if (name == NULL || (type != 0 && type != 1) || x < 0 || y < 0) {
        printf("Invalid driver input.\n");
        return;
    }

    id = g_nextDriverId++;

    if (searchDriver(id) != NULL) {
        printf("Driver with ID %d already exists.\n", id);
        return;
    }

    d = (Driver *)malloc(sizeof(Driver));
    if (d == NULL) {
        printf("Memory allocation failed for driver.\n");
        return;
    }

    d->d_ID = id;
    strncpy(d->name, name, MAX_NAME_LEN - 1);
    d->name[MAX_NAME_LEN - 1] = '\0';
    d->vehicle_type = type;
    d->current_location.x = x;
    d->current_location.y = y;
    d->status = 0;
    d->total_earnings = 0.0f;

    g_driverRoot = insert(g_driverRoot, (long long)id, d);
    saveDrivers();
    printf("Driver added successfully. Generated Driver ID: %d\n", id);
}

void addPassenger(char name[], char mobile[]) {
    Passenger *p;
    int id;

    if (name == NULL || mobile == NULL) {
        printf("Invalid passenger input.\n");
        return;
    }

    id = g_nextPassengerId++;

    if (!isValidMobileNumber(mobile)) {
        printf("Invalid mobile number. Please enter exactly 10 digits.\n");
        return;
    }

    if (searchPassenger(id) != NULL || passengerMobileExistsInTree(mobile)) {
        printf("Passenger with same ID or mobile already exists.\n");
        return;
    }

    p = (Passenger *)malloc(sizeof(Passenger));
    if (p == NULL) {
        printf("Memory allocation failed for passenger.\n");
        return;
    }

    p->p_ID = id;
    strncpy(p->name, name, MAX_NAME_LEN - 1);
    p->name[MAX_NAME_LEN - 1] = '\0';
    strncpy(p->mobile_no, mobile, MAX_MOBILE_LEN - 1);
    p->mobile_no[MAX_MOBILE_LEN - 1] = '\0';
    p->frequency = 0;

    g_passengerRoot = insert(g_passengerRoot, (long long)id, p);
    savePassengers();
    printf("Passenger added successfully. Generated Passenger ID: %d\n", id);
}

Driver *findNearestVehicle(int p_x, int p_y, int prefType) {
    BPlusNode *leaf;
    Driver *best = NULL;
    float minDistance = MAX_RADIUS + 1.0f;

    if (prefType != -1 && prefType != 0 && prefType != 1) {
        return NULL;
    }

    leaf = getLeftmostLeaf(g_driverRoot);
    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            Driver *d = (Driver *)leaf->records[i];
            if (d->status == 0 && (prefType == -1 || d->vehicle_type == prefType)) {
                float dist = distanceCalc(p_x, p_y, d->current_location.x, d->current_location.y);
                if (dist <= MAX_RADIUS && dist < minDistance) {
                    minDistance = dist;
                    best = d;
                }
            }
        }
        leaf = leaf->next;
    }

    return best;
}

int requestRide(int p_id, int p_x, int p_y, int prefType) {
    Passenger *p;
    Driver *d;
    Booking *b;

    if (p_x < 0 || p_y < 0) {
        printf("Invalid passenger location.\n");
        return -1;
    }

    if (prefType != -1 && prefType != 0 && prefType != 1) {
        printf("Invalid preferred vehicle type.\n");
        return -1;
    }

    p = searchPassenger(p_id);
    if (p == NULL) {
        printf("Passenger with ID %d does not exist.\n", p_id);
        return -1;
    }

    d = findNearestVehicle(p_x, p_y, prefType);
    if (d == NULL) {
        printf("No free vehicle found within %.1f km radius.\n", MAX_RADIUS);
        return -1;
    }

    b = (Booking *)malloc(sizeof(Booking));
    if (b == NULL) {
        printf("Memory allocation failed for booking.\n");
        return -1;
    }

    b->booking_id = g_nextBookingId++;
    b->d_ID = d->d_ID;
    b->p_ID = p->p_ID;
    b->vehicle_type = d->vehicle_type;
    b->distance_travelled = 0.0f;
    b->fare = 0.0f;
    b->timestamp = b->booking_id;

    d->status = 1;

    g_bookingRoot = insert(g_bookingRoot, (long long)b->booking_id, b);
    saveDrivers();
    saveBookings();

    printf("Ride booked. Booking ID: %d, Driver ID: %d (%s).\n", b->booking_id, d->d_ID, d->name);
    return b->booking_id;
}

void completeRide(int booking_id, float distance) {
    Booking *b;
    Driver *d;
    Passenger *p;
    float rate;

    if (distance < 0.0f) {
        printf("Distance cannot be negative.\n");
        return;
    }

    b = searchBooking(booking_id);
    if (b == NULL) {
        printf("Booking ID %d not found.\n", booking_id);
        return;
    }

    if (b->fare > 0.0f || b->distance_travelled > 0.0f) {
        printf("Booking ID %d is already completed.\n", booking_id);
        return;
    }

    b->distance_travelled = distance;
    rate = (b->vehicle_type == 0) ? CAB_RATE : BIKE_RATE;
    b->fare = distance * rate;

    d = searchDriver(b->d_ID);
    if (d != NULL) {
        d->status = 0;
        d->total_earnings += b->fare;
    }

    p = searchPassenger(b->p_ID);
    if (p != NULL) {
        p->frequency += 1;
    }

    saveDrivers();
    savePassengers();
    saveBookings();

    printf("Ride completed for booking ID %d. Fare: %.2f\n", booking_id, b->fare);
}

float calculateDriverEarnings(int d_id) {
    float sum = 0.0f;
    BPlusNode *leaf = getLeftmostLeaf(g_bookingRoot);

    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            Booking *b = (Booking *)leaf->records[i];
            if (b->d_ID == d_id) {
                sum += b->fare;
            }
        }
        leaf = leaf->next;
    }

    return sum;
}

void displayTopDrivers(void) {
    BPlusNode *leaf = getLeftmostLeaf(g_driverRoot);
    Driver *top[3] = {NULL, NULL, NULL};
    int i;

    if (leaf == NULL) {
        printf("No drivers found.\n");
        return;
    }

    while (leaf != NULL) {
        for (i = 0; i < leaf->keyCount; ++i) {
            Driver *curr = (Driver *)leaf->records[i];

            if (top[0] == NULL || curr->total_earnings > top[0]->total_earnings) {
                top[2] = top[1];
                top[1] = top[0];
                top[0] = curr;
            } else if (top[1] == NULL || curr->total_earnings > top[1]->total_earnings) {
                top[2] = top[1];
                top[1] = curr;
            } else if (top[2] == NULL || curr->total_earnings > top[2]->total_earnings) {
                top[2] = curr;
            }
        }
        leaf = leaf->next;
    }

    if (top[0] == NULL) {
        printf("No drivers found.\n");
        return;
    }

    printf("Top 3 drivers by earnings:\n");
    for (i = 0; i < 3 && top[i] != NULL; ++i) {
        printf("%d. %s (ID: %d) - Earnings: %.2f\n", i + 1, top[i]->name, top[i]->d_ID, top[i]->total_earnings);
    }
}

void displayFrequentPairs(void) {
    BPlusNode *pairRoot = NULL;
    BPlusNode *leaf;
    PairCount *best = NULL;

    leaf = getLeftmostLeaf(g_bookingRoot);
    if (leaf == NULL) {
        printf("No booking history available.\n");
        return;
    }

    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            Booking *b = (Booking *)leaf->records[i];
            if (b->distance_travelled > 0.0f) {
                long long key = pairKey(b->d_ID, b->p_ID);
                PairCount *entry = (PairCount *)searchInTree(pairRoot, key);
                if (entry == NULL) {
                    entry = (PairCount *)malloc(sizeof(PairCount));
                    if (entry != NULL) {
                        entry->d_id = b->d_ID;
                        entry->p_id = b->p_ID;
                        entry->count = 1;
                        pairRoot = insert(pairRoot, key, entry);
                    }
                } else {
                    entry->count += 1;
                }
            }
        }
        leaf = leaf->next;
    }

    leaf = getLeftmostLeaf(pairRoot);
    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            PairCount *entry = (PairCount *)leaf->records[i];
            if (entry != NULL && (best == NULL || entry->count > best->count)) {
                best = entry;
            }
        }
        leaf = leaf->next;
    }

    if (best == NULL) {
        printf("No completed rides found.\n");
        freePairTree(pairRoot);
        return;
    }

    {
        Driver *d = searchDriver(best->d_id);
        Passenger *p = searchPassenger(best->p_id);

        printf("Most frequent passenger-driver pair:\n");
        printf("Driver: %s (ID: %d)\n", (d != NULL) ? d->name : "Unknown", best->d_id);
        printf("Passenger: %s (ID: %d)\n", (p != NULL) ? p->name : "Unknown", best->p_id);
        printf("Rides together: %d\n", best->count);
    }

    freePairTree(pairRoot);
}

void displayAvailableVehicles(void) {
    BPlusNode *leaf = getLeftmostLeaf(g_driverRoot);
    int found = 0;

    if (leaf == NULL) {
        printf("No drivers in database.\n");
        return;
    }

    printf("Available vehicles:\n");
    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            Driver *d = (Driver *)leaf->records[i];
            if (d->status == 0) {
                printf("ID: %d | Name: %s | Type: %s | Location: (%d, %d)\n",
                    d->d_ID,
                    d->name,
                    (d->vehicle_type == 0) ? "Cab" : "Bike",
                    d->current_location.x,
                    d->current_location.y);
                found = 1;
            }
        }
        leaf = leaf->next;
    }

    if (!found) {
        printf("No free vehicles currently available.\n");
    }
}

void updateDriverLocation(int d_id, int new_x, int new_y) {
    Driver *d;

    if (new_x < 0 || new_y < 0) {
        printf("Location cannot be negative.\n");
        return;
    }

    d = searchDriver(d_id);
    if (d == NULL) {
        printf("Driver with ID %d not found.\n", d_id);
        return;
    }

    if (d->status == 1) {
        printf("Cannot update location for a booked driver.\n");
        return;
    }

    d->current_location.x = new_x;
    d->current_location.y = new_y;

    saveDrivers();
    printf("Driver location updated successfully.\n");
}

void deleteDriver(int d_id) {
    Driver *target = searchDriver(d_id);
    void *deletedRecord = NULL;

    if (target == NULL) {
        printf("Driver with ID %d not found.\n", d_id);
        return;
    }

    if (target->status == 1) {
        printf("Cannot delete: driver is currently booked.\n");
        return;
    }

    g_driverRoot = deleteFromTree(g_driverRoot, (long long)d_id, &deletedRecord);
    if (deletedRecord == NULL) {
        printf("Driver with ID %d not found.\n", d_id);
        return;
    }

    free(deletedRecord);

    saveDrivers();
    printf("Driver deleted successfully.\n");
}

void displayBookingHistory(void) {
    BPlusNode *leaf = getLeftmostLeaf(g_bookingRoot);

    if (leaf == NULL) {
        printf("No bookings found.\n");
        return;
    }

    printf("Booking history:\n");
    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            Booking *b = (Booking *)leaf->records[i];
            printf("Booking ID: %d | Driver ID: %d | Passenger ID: %d | Type: %s | Distance: %.2f km | Fare: %.2f\n",
                b->booking_id,
                b->d_ID,
                b->p_ID,
                (b->vehicle_type == 0) ? "Cab" : "Bike",
                b->distance_travelled,
                b->fare);
        }
        leaf = leaf->next;
    }
}

void range_search(int p_id1, int p_id2) {
    int low = (p_id1 < p_id2) ? p_id1 : p_id2;
    int high = (p_id1 < p_id2) ? p_id2 : p_id1;
    BPlusNode *leaf = findLeaf(g_passengerRoot, (long long)low);
    int found = 0;

    if (g_passengerRoot == NULL) {
        printf("Passenger database is empty.\n");
        return;
    }

    printf("Passengers with IDs in range [%d, %d]:\n", low, high);
    while (leaf != NULL) {
        int i;
        for (i = 0; i < leaf->keyCount; ++i) {
            if (leaf->keys[i] > (long long)high) {
                if (!found) {
                    printf("No passengers found in range.\n");
                }
                return;
            }
            if (leaf->keys[i] >= (long long)low) {
                Passenger *p = (Passenger *)leaf->records[i];
                printf("ID: %d | Name: %s | Mobile: %s | Frequency: %d\n", p->p_ID, p->name, p->mobile_no, p->frequency);
                found = 1;
            }
        }
        leaf = leaf->next;
    }

    if (!found) {
        printf("No passengers found in range.\n");
    }
}

void freeAllData(void) {
    freeTreeRecords(g_driverRoot);
    freeTreeRecords(g_passengerRoot);
    freeTreeRecords(g_bookingRoot);

    freeTreeNodes(g_driverRoot);
    freeTreeNodes(g_passengerRoot);
    freeTreeNodes(g_bookingRoot);

    g_driverRoot = NULL;
    g_passengerRoot = NULL;
    g_bookingRoot = NULL;
    g_nextBookingId = 1;
}
