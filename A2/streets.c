#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "streets.h"

struct node {
    int id;
    double lat;
    double lon;
    int num_ways;
    int *way_ids;
};

struct way {
    int id;
    char *name;
    float maxspeed;
    bool oneway;
    int num_nodes;
    int *node_ids;
};

struct ssmap {
    struct node *nodes;
    int num_nodes;
    struct way *ways; 
    int num_ways;
};

struct ssmap * ssmap_create(int nr_nodes, int nr_ways)
{
    // return if nr_nodes or nr_ways is zero
    if (nr_nodes == 0 || nr_ways == 0) {
        return NULL;
    }

    // memory for the ssmap structure
    struct ssmap *map = (struct ssmap *)malloc(sizeof(struct ssmap));
    if (!map) {
        return NULL;
    }

    // memory for the nodes array in  the ssmap structure
    map->nodes = (struct node *)malloc(nr_nodes * sizeof(struct node));
    if (!map->nodes) { 
        free(map); 
        return NULL;
    }
    map->num_nodes = nr_nodes;

    // memory for the ways array in the ssmap structure
    map->ways = (struct way *)malloc(nr_ways * sizeof(struct way));
    if (!map->ways) { 
        free(map->nodes);
        free(map);
        return NULL;
    }
    map->num_ways = nr_ways;

    return map;
}

bool
ssmap_initialize(struct ssmap * m)
{
    /* additional initialization code can be added here */
    return true;
}

void
ssmap_destroy(struct ssmap * m)
{
    // free all way objects and their fields
    for (int i = 0; i < m->num_ways; i++) {
        free(m->ways[i].name); 
        free(m->ways[i].node_ids);
    }

    // free all node objects and their fields
    for (int j = 0; j < m->num_nodes; j++) {
        free(m->nodes[j].way_ids); 
    }
    free(m->nodes);
    free(m->ways);

    free(m);
}

struct way * ssmap_add_way(struct ssmap * m, int id, const char * name, float maxspeed, bool oneway, 
              int num_nodes, const int node_ids[num_nodes])
{
    // memory for the new way
    struct way *new_way = (struct way *)malloc(sizeof(struct way));
    if (!new_way) { 
        return NULL;
    }

    new_way->name = (char *)malloc(strlen(name) + 1); // +1 for the null terminator
    if (!new_way->name) {
        free(new_way);
        return NULL;
    }
    strcpy(new_way->name, name);

    new_way->node_ids = (int *)malloc(num_nodes * sizeof(int));
    if (!new_way->node_ids) {
        free(new_way->name); 
        free(new_way);       
        return NULL;
    }
    for (int i = 0; i < num_nodes; i++) {
        new_way->node_ids[i] = node_ids[i];
    }

    // set the rest of the fields
    new_way->id = id;
    new_way->maxspeed = maxspeed;
    new_way->oneway = oneway;
    new_way->num_nodes = num_nodes;

    m->ways[id] = *new_way;

    return &(m->ways[id]);
}

struct node * 
ssmap_add_node(struct ssmap * m, int id, double lat, double lon, 
               int num_ways, const int way_ids[num_ways])
{
    // memory for new node
    struct node *new_node = (struct node *)malloc(sizeof(struct node));
    if (!new_node) {
        return NULL;
    }

    // memory for the way_ids array and copy
    new_node->way_ids = (int *)malloc(num_ways * sizeof(int));
    if (!new_node->way_ids) {
        free(new_node); 
        return NULL;
    }
    for (int i = 0; i < num_ways; i++) {
        new_node->way_ids[i] = way_ids[i];
    }

    new_node->id = id;
    new_node->lat = lat;
    new_node->lon = lon;
    new_node->num_ways = num_ways;

    m->nodes[id] = *new_node; 

    return new_node;
}

void
ssmap_print_way(const struct ssmap * m, int id)
{
    if (id < 0 || id >= m->num_ways) {
        printf("error: way %d does not exist.\n", id);
        return;
    }

    const struct way *way_obj = &(m->ways[id]);
    if (!way_obj) {
        printf("error: way %d does not exist.\n", id);
        return;
    }

    const char *way_name = way_obj->name ? way_obj->name : "Unnamed";

    printf("Way %d: %s\n", id, way_name);
    printf("Speed Limit: %.1f km/h\n", way_obj->maxspeed);
    printf("One-way: %s\n", way_obj->oneway ? "Yes" : "No");
    
    printf("Nodes: ");
    for (int i = 0; i < way_obj->num_nodes; i++) {
        printf("%d ", way_obj->node_ids[i]);
    }
    printf("\n");
}

void
ssmap_print_node(const struct ssmap * m, int id)
{
    if (id < 0 || id >= m->num_nodes) {
        printf("error: node %d does not exist.\n", id);
        return;
    }

    const struct node *node_obj = &(m->nodes[id]);
    if (!node_obj) {
        printf("error: node %d does not exist.\n", id);
        return;
    }

    printf("Node %d: (%.7f, %.7f)\n", id, node_obj->lat, node_obj->lon);
    printf("Associated with %d ways\n", node_obj->num_ways);

    printf("Way IDs: ");
    for (int i = 0; i < node_obj->num_ways; i++) {
        printf("%d ", node_obj->way_ids[i]);
    }
    printf("\n");
}

void 
ssmap_find_way_by_name(const struct ssmap * m, const char * name)
{
    int found = 0; // track if any matches are found

    for (int i = 0; i < m->num_ways; i++) {
        if (m->ways[i].name && strstr(m->ways[i].name, name) != NULL) {
            printf("%d ", m->ways[i].id);
            found = 1; 
        }
    }

    if (!found) {
        printf("No way found with the name containing \"%s\"\n", name);
    } else {
        printf("\n");
    }
}


void 
ssmap_find_node_by_names(const struct ssmap * m, const char * name1, const char * name2) 
{

    for (int i = 0; i < m->num_nodes; i++) {
        struct node *node = &m->nodes[i];
        int found_1 = -1, found_2 = -1;

        for (int j = 0; j < node->num_ways; j++) {
            struct way *way = &m->ways[node->way_ids[j]];

            if (found_1 == -1 && strstr(way->name, name1) != NULL) {
                found_1 = way->id;
            }

            if (name2 != NULL && found_2 == -1 && strstr(way->name, name2) != NULL) {
                found_2 = way->id;
            }

            if (found_1 != -1 && (name2 == NULL || (found_2 != -1 && found_1 != found_2))) {
                printf("%d ", node->id);
                break;
            }
        }
    }

    printf("\n");
}

/**
 * Converts from degree to radian
 *
 * @param deg The angle in degrees.
 * @return the equivalent value in radian
 */
#define d2r(deg) ((deg) * M_PI/180.)

/**
 * Calculates the distance between two nodes using the Haversine formula.
 *
 * @param x The first node.
 * @param y the second node.
 * @return the distance between two nodes, in kilometre.
 */
static double
distance_between_nodes(const struct node * x, const struct node * y) {
    double R = 6371.;       
    double lat1 = x->lat;
    double lon1 = x->lon;
    double lat2 = y->lat;
    double lon2 = y->lon;
    double dlat = d2r(lat2-lat1); 
    double dlon = d2r(lon2-lon1); 
    double a = pow(sin(dlat/2), 2) + cos(d2r(lat1)) * cos(d2r(lat2)) * pow(sin(dlon/2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a)); 
    return R * c; 
}


double 
ssmap_path_travel_time(const struct ssmap * m, int size, int node_ids[size])
{
    double total_travel_time = 0.0;

    for (int i = 0; i < size - 1; i++) {
        int node_id_a = node_ids[i];
        int node_id_b = node_ids[i + 1];

        // Error 1: check if both nodes exist
        if (node_id_a < 0 || node_id_a >= m->num_nodes || node_id_b < 0 || node_id_b >= m->num_nodes) {
            printf("error: node %d does not exist.\n", node_id_a < 0 || node_id_a >= m->num_nodes ? node_id_a : node_id_b);
            return -1.0;
        }

        // Error 5: check if a node appears more than once
        for (int j = i + 1; j < size; j++) {
            if (node_ids[i] == node_ids[j]) {
                printf("error: node %d appeared more than once.\n", node_ids[i]);
                return -1.0;
            }
        }

        // Error 2: check if there is a way between the nodes
        bool shared_way = false;
        for (int j = 0; j < m->nodes[node_id_a].num_ways; j++) {
            for (int y = 0; y < m->nodes[node_id_b].num_ways; y++) {
                if (m->nodes[node_id_a].way_ids[j] == m->nodes[node_id_b].way_ids[y]) {
                    shared_way = true;
                    break;
                } 
            }
        }

        if (!shared_way) {
            printf("error: there are no roads between node %d and node %d.\n", node_id_a, node_id_b);
            return -1.0;
        }


        struct way* connecting_way =  NULL;

        bool reverse = false;
        for (int i = 0; i < m->nodes[node_id_a].num_ways; i++) {
            int way_id = m->nodes[node_id_a].way_ids[i];
            struct way *way = &m->ways[way_id];

            for (int j = 0; j < way->num_nodes - 1; j++) {
                if (way->node_ids[j] == node_id_a) {
                    if (way->node_ids[j + 1] == node_id_b) {
                        connecting_way = way;
                    }
                } else if (way->oneway && way->node_ids[j + 1] == node_id_a && way->node_ids[j] == node_id_b) {
                    reverse = true;
                }

                // If the way is not one-way, also check the reverse direction
                if (!way->oneway && way->node_ids[j + 1] == node_id_a) {
                    if (way->node_ids[j] == node_id_b) {
                        connecting_way = way;
                    }
                }
            }
        }

        // Error 4: check if the path respects the one-way rule
        if (!connecting_way && reverse) {
            printf("error: cannot go in reverse from node %d to node %d on one-way.\n", node_id_a, node_id_b);
            return -1.0;
        }
        // Error 3: check if there is a way between the nodes
        if (!connecting_way) {
           printf("error: cannot go directly from node %d to node %d.\n", node_id_a, node_id_b);
            return -1.0;
        }

        double distance = distance_between_nodes(&m->nodes[node_id_a], &m->nodes[node_id_b]);
        total_travel_time += distance / connecting_way->maxspeed;
    }

    return total_travel_time * 60;
}

double 
ssmap_path_travel_time_no_print(const struct ssmap * m, int size, int node_ids[size])
{
    double total_travel_time = 0.0;

    for (int i = 0; i < size - 1; i++) {
        int node_id_a = node_ids[i];
        int node_id_b = node_ids[i + 1];

        // Error 1: check if both nodes exist
        if (node_id_a < 0 || node_id_a >= m->num_nodes || node_id_b < 0 || node_id_b >= m->num_nodes) {
            return -1.0;
        }

        // Error 5: check if a node appears more than once
        for (int j = i + 1; j < size; j++) {
            if (node_ids[i] == node_ids[j]) {
                return -1.0;
            }
        }

        // Error 2: check if there is a way between the nodes
        bool shared_way = false;
        for (int j = 0; j < m->nodes[node_id_a].num_ways; j++) {
            for (int y = 0; y < m->nodes[node_id_b].num_ways; y++) {
                if (m->nodes[node_id_a].way_ids[j] == m->nodes[node_id_b].way_ids[y]) {
                    shared_way = true;
                    break;
                } 
            }
        }

        if (!shared_way) {
            return -1.0;
        }


        struct way* connecting_way =  NULL;

        bool reverse = false;
        for (int i = 0; i < m->nodes[node_id_a].num_ways; i++) {
            int way_id = m->nodes[node_id_a].way_ids[i];
            struct way *way = &m->ways[way_id];

            for (int j = 0; j < way->num_nodes - 1; j++) {
                if (way->node_ids[j] == node_id_a) {
                    if (way->node_ids[j + 1] == node_id_b) {
                        connecting_way = way;
                    }
                } else if (way->oneway && way->node_ids[j + 1] == node_id_a && way->node_ids[j] == node_id_b) {
                    reverse = true;
                }

                // If the way is not one-way, also check the reverse direction
                if (!way->oneway && way->node_ids[j + 1] == node_id_a) {
                    if (way->node_ids[j] == node_id_b) {
                        connecting_way = way;
                    }
                }
            }
        }

        // Error 4: check if the path respects the one-way rule
        if (!connecting_way && reverse) {
            return -1.0;
        }
        // Error 3: check if there is a way between the nodes
        if (!connecting_way) {
            return -1.0;
        }

        double distance = distance_between_nodes(&m->nodes[node_id_a], &m->nodes[node_id_b]);
        total_travel_time += distance / connecting_way->maxspeed;
    }

    return total_travel_time * 60;
}


// MINHEAP PRIORITY QUEUE

typedef struct MinHeapNode {
    int node_id;
    double distance;
} MinHeapNode;

typedef struct MinHeap {
    int size;
    int capacity;
    int *pos;
    MinHeapNode **array;
} MinHeap;

MinHeapNode* newMinHeapNode(int node_id, double distance) {
    MinHeapNode* node = (MinHeapNode*) malloc(sizeof(MinHeapNode));
    node->node_id = node_id;
    node->distance = distance;
    return node;
}

MinHeap* createMinHeap(int capacity) {
    MinHeap* minHeap = (MinHeap*) malloc(sizeof(MinHeap));
    minHeap->pos = (int *)malloc(capacity * sizeof(int));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (MinHeapNode**) malloc(capacity * sizeof(MinHeapNode*));
    return minHeap;
}

void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b) {
    MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap* minHeap, int idx) {
    int smallest, left, right;
    smallest = idx;
    left = 2 * idx + 1;
    right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->distance < minHeap->array[smallest]->distance)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right]->distance < minHeap->array[smallest]->distance)
        smallest = right;

    if (smallest != idx) {
        MinHeapNode *smallestNode = minHeap->array[smallest];
        MinHeapNode *idxNode = minHeap->array[idx];
        minHeap->pos[smallestNode->node_id] = idx;
        minHeap->pos[idxNode->node_id] = smallest;

        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

int isEmpty(MinHeap* minHeap) {
    return minHeap->size == 0;
}

void freeMinHeap(MinHeap* minHeap) {
    if (minHeap == NULL) {
        return;
    }

    for (int i = 0; i < minHeap->size; i++) {
        free(minHeap->array[i]);
    }

    free(minHeap->array);
    free(minHeap->pos);
    free(minHeap);
}

MinHeapNode* extractMin(MinHeap* minHeap) {
    if (isEmpty(minHeap))
        return NULL;

    MinHeapNode* root = minHeap->array[0];
    MinHeapNode* lastNode = minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;

    minHeap->pos[root->node_id] = minHeap->size-1;
    minHeap->pos[lastNode->node_id] = 0;

    --minHeap->size;
    minHeapify(minHeap, 0);

    return root;
}

void decreaseKey(MinHeap* minHeap, int node_id, double new_val) {
    int i = minHeap->pos[node_id];

    MinHeapNode* node = minHeap->array[i];
    node->distance = new_val;

    while (i && node->distance < minHeap->array[(i - 1) / 2]->distance) {
        minHeap->pos[node->node_id] = (i-1)/2;
        minHeap->pos[minHeap->array[(i-1)/2]->node_id] = i;
        swapMinHeapNode(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void minHeapInsert(MinHeap* minHeap, int node_id, double distance) {
    if (minHeap->size == minHeap->capacity) {
        printf("\nOverflow: Could not insertKey\n");
        return;
    }

    int i = minHeap->size;
    MinHeapNode* node = newMinHeapNode(node_id, distance);
    minHeap->array[i] = node;
    minHeap->pos[node_id] = i;

    // fix min heap property
    while (i != 0 && minHeap->array[i]->distance < minHeap->array[(i - 1) / 2]->distance) {
        minHeap->pos[minHeap->array[i]->node_id] = (i-1)/2;
        minHeap->pos[minHeap->array[(i-1)/2]->node_id] = i;
        swapMinHeapNode(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }

    minHeap->size++;
}

int isInMinHeap(MinHeap *minHeap, int node_id) {
    if (minHeap->pos[node_id] < minHeap->size)
        return 1;
    return 0;
}

void ssmap_path_create(const struct ssmap * m, int start_id, int end_id) {
    if (start_id < 0 || start_id >= m->num_nodes || end_id < 0 || end_id >= m->num_nodes) {
        printf("error: node %d does not exist.\n", start_id < 0 || start_id >= m->num_nodes ? start_id : end_id);
        return;
    }

    if (start_id == end_id) {
        printf("%d\n", start_id);
        return;
    }

    MinHeap *pq = createMinHeap(m->num_nodes);
    minHeapInsert(pq, start_id, 0.0);

    double dist[m->num_nodes];
    int prev[m->num_nodes];
    dist[start_id] = 0.0;
    prev[start_id] = -1;
    for (int i = 0; i < m->num_nodes; i++) {
        if (i != start_id) {
            dist[i] = INFINITY;
            prev[i] = -1;
            minHeapInsert(pq, i, dist[i]);  
        }
              
    }

    while (!isEmpty(pq)) {
        int u = extractMin(pq)->node_id;
        if (u == end_id) {
            break; // found
        }

        for (int i = 0; i < m->nodes[u].num_ways; i++) {
            struct way *way = &m->ways[m->nodes[u].way_ids[i]];

            // find the position of u in the way
            int pos;
            for (pos = 0; pos < way->num_nodes; pos++) {
                if (way->node_ids[pos] == u) {
                    break;
                }
            }

            // only consider the nodes adjacent to u
            int neighbors[2] = {pos - 1, pos + 1};
            for (int k = 0; k < 2; k++) {
                int j = neighbors[k];
                if (j < 0 || j >= way->num_nodes) {
                    continue;
                }

                int neighbor = way->node_ids[j];

                if (neighbor == u) {
                    continue;
                }

                // redacted
                // double alt = dist[u] + distance_between_nodes(&m->nodes[u], &m->nodes[neighbor]);
                double time = ssmap_path_travel_time_no_print(m, 2, (int[]){u, neighbor});
                double alt = dist[u] + time;
                if (time == -1) {
                    continue;
                } else if (alt < dist[neighbor]) {
                    dist[neighbor] = alt;
                    prev[neighbor] = u;
                    decreaseKey(pq, neighbor, alt);
                }
            }
        }
    }

    int path[m->num_nodes];
    int length = 0;

    // follow prev array from the end node to the start node (-1 as defined)
    for (int v = end_id; v != -1; v = prev[v]) {
        path[length] = v;
        length++;
    }

    // check for path
    if (path[length - 1] != start_id) {
        printf("No path found from node %d to node %d\n", start_id, end_id);
        freeMinHeap(pq);
        return;
    }

    // print it
    for (int i = length - 1; i >= 0; i--) {
        printf("%d ", path[i]);
    }
    printf("\n");
    freeMinHeap(pq);
}