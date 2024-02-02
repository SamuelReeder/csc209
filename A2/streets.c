#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "streets.h"

struct node {
    double lat;
    double lon;
    /* TODO: task 1 */
};

struct way {
    /* TODO: task 1 */
};

struct ssmap {
    /* TODO: task 1 */
};

struct ssmap * 
ssmap_create(int nr_nodes, int nr_ways)
{
    /* TODO: task 2 */
    return NULL;
}

bool
ssmap_initialize(struct ssmap * m)
{
    /* TODO: task 2
     * additional initialization code can be added here */
    return true;
}

void
ssmap_destroy(struct ssmap * m)
{
    /* TODO: task 2 */
}

struct way * 
ssmap_add_way(struct ssmap * m, int id, const char * name, float maxspeed, bool oneway, 
              int num_nodes, const int node_ids[num_nodes])
{
    /* TODO: task 2 */
    return NULL;
}

struct node * 
ssmap_add_node(struct ssmap * m, int id, double lat, double lon, 
               int num_ways, const int way_ids[num_ways])
{
    /* TODO: task 2 */
    return NULL;
}

void
ssmap_print_way(const struct ssmap * m, int id)
{
    /* TODO: task 3 */
}

void
ssmap_print_node(const struct ssmap * m, int id)
{
    /* TODO: task 3 */
}

void 
ssmap_find_way_by_name(const struct ssmap * m, const char * name)
{
    /* TODO: task 3 */
}

void 
ssmap_find_node_by_names(const struct ssmap * m, const char * name1, const char * name2)
{
    /* TODO: task 3 */
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
    /* TODO: task 4 */
    (void)distance_between_nodes;
    return 0.0;
}

void 
ssmap_path_create(const struct ssmap * m, int start_id, int end_id)
{
    /* TODO: task 4 */
}