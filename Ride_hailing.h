#ifndef RIDE_HAILING_H
#define RIDE_HAILING_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define MAX_NAME_LEN 50
#define MAX_MOBILE_LEN 15
#define CAB_RATE 10.0
#define BIKE_RATE 5.0
#define MAX_RADIUS 5.0

#define DRIVER_DATA_FILE "drivers.txt"
#define PASSENGER_DATA_FILE "passengers.txt"
#define BOOKING_DATA_FILE "bookings.txt"

typedef struct Location {
    int x;
    int y;
} Location;


typedef struct Driver{
    int d_ID;
    char name[MAX_NAME_LEN];
    int vehicle_type;
    Location current_location;
    int status;
    float total_earnings;
}Driver;

typedef struct Passenger {
    int p_ID;
    char name[MAX_NAME_LEN];
    char mobile_no[MAX_MOBILE_LEN];
    int frequency;
} Passenger;

typedef struct Booking {
    int booking_id;
    int d_ID;
    int p_ID;
    int vehicle_type;
    float distance_travelled;
    float fare;
    int timestamp;
} Booking;

void addDriver(char name[], int type, int x, int y);
void addPassenger(char name[], char mobile[]);
Driver* findNearestVehicle(int p_x, int p_y, int prefType);
int requestRide(int p_id, int p_x, int p_y, int prefType);
void completeRide(int booking_id, float distance);
float calculateDriverEarnings(int d_id);
void displayTopDrivers(void);
void displayFrequentPairs(void);
void displayAvailableVehicles(void);
void updateDriverLocation(int d_id, int new_x, int new_y);
void deleteDriver(int d_id);
void displayBookingHistory(void);
void range_search(int p_id1, int p_id2);

void loadData(void);
void saveDrivers(void);
void savePassengers(void);
void saveBookings(void);
void freeAllData(void);

#endif