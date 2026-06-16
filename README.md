# Ride Hailing Management System (C) - B+ Tree Only

This project is a console-based ride-hailing simulator in C.

It manages:
- Drivers
- Passengers
- Bookings

It supports:
- Add driver and passenger
- Request and complete rides
- Track driver earnings
- Top drivers
- Most frequent driver-passenger pair
- Available vehicles listing
- Driver location updates
- Driver deletion
- Booking history
- Passenger ID range search

The implementation now uses only B+ trees for in-memory storage and lookup.

## 1. Project Files

- main.c: Console menu and user input flow
- Ride_hailing.h: Structs, constants, function declarations
- Ride_hailing.c: Full implementation, including B+ tree storage/indexing
- drivers.txt: Driver persistence
- passengers.txt: Passenger persistence
- bookings.txt: Booking persistence

## 2. Core Design

Primary runtime storage:
- In-memory B+ trees for Driver, Passenger, Booking records

Global roots in Ride_hailing.c:
- g_driverRoot for driver ID lookup
- g_passengerRoot for passenger ID lookup
- g_bookingRoot for booking ID lookup

Booking ID generation:
- g_nextBookingId is derived from max booking ID while loading file data

## 3. B+ Tree Implementation Details

B+ tree configuration:
- ORDER = 4
- Max keys per node = ORDER - 1 = 3

Node layout:
- isLeaf: leaf/internal node flag
- keyCount: number of keys currently stored
- keys[]: sorted keys
- children[]: child pointers for internal nodes
- records[]: payload pointers for leaf nodes
- next: leaf-to-leaf pointer for ordered traversal

Implemented B+ operations:
- createNode(isLeaf)
- findLeaf(root, key)
- searchInTree(root, key)
- insert(root, key, record)
- insertRecursive(...)
- getLeftmostLeaf(root)
- freeTreeNodes(root)

Current deletion approach:
- Generic B+ delete is not implemented.
- Driver delete is handled by rebuilding the driver B+ tree excluding the deleted key.

## 4. Business Logic Flow

addDriver:
- Validates input and duplicate ID via B+ search
- Allocates a Driver record and inserts into driver B+ tree
- Persists with saveDrivers()

addPassenger:
- Validates mobile number and uniqueness
- Allocates Passenger record and inserts into passenger B+ tree
- Persists with savePassengers()

requestRide:
- Finds passenger by ID in passenger B+ tree
- Scans driver leaf chain to find nearest free eligible driver
- Creates booking and marks driver busy
- Inserts booking into booking B+ tree
- Persists driver and booking data

completeRide:
- Finds booking by ID in booking B+ tree
- Prevents duplicate completion
- Computes fare from vehicle type rate
- Updates driver earnings/status and passenger frequency
- Persists all affected datasets

displayFrequentPairs:
- Builds a temporary B+ tree keyed by (driver_id, passenger_id)
- Counts completed rides per pair
- Finds maximum frequency by leaf scan

range_search:
- Uses passenger B+ tree leaf-chain traversal from low ID to high ID

## 5. Persistence Format

drivers.txt rows:
- d_ID|name|vehicle_type|x|y|status|total_earnings

passengers.txt rows:
- p_ID|name|mobile_no|frequency

bookings.txt rows:
- booking_id|d_ID|p_ID|vehicle_type|distance_travelled|fare|timestamp

## 6. Public API (Current)

From Ride_hailing.h:
- addDriver(int id, char name[], int type, int x, int y)
- addPassenger(int id, char name[], char mobile[])
- findNearestVehicle(int p_x, int p_y, int prefType)
- requestRide(int p_id, int p_x, int p_y, int prefType)
- completeRide(int booking_id, float distance)
- calculateDriverEarnings(int d_id)
- displayTopDrivers(void)
- displayFrequentPairs(void)
- displayAvailableVehicles(void)
- updateDriverLocation(int d_id, int new_x, int new_y)
- deleteDriver(int d_id)
- displayBookingHistory(void)
- range_search(int p_id1, int p_id2)
- loadData(void)
- saveDrivers(void), savePassengers(void), saveBookings(void)
- freeAllData(void)

## 7. Build and Run

Compile with GCC:
- gcc main.c Ride_hailing.c -o ride_hailing -lm

Run:
- .\ride_hailing.exe

## 8. Complexity Notes

- B+ search/insert: O(log n)
- Passenger range query: O(log n + k)
- Nearest vehicle matching: O(D) by scanning driver leaves
- Frequent pair counting: O(B log B)

Where:
- D = number of drivers
- B = number of bookings
- k = number of passengers returned in range search

## 9. Important Behavior

- Passenger mobile must be exactly 10 digits
- Driver cannot be deleted if currently booked
- Booking completion is one-time only
- MAX_RADIUS filter applies in ride matching
- Timestamp currently follows booking ID sequence

## 10. Left Out in the Sample Code (Checklist)

Compared to a full production-quality B+ tree system, the sample code still leaves out or weakly handles:

- Complete and fully-correct generic B+ tree deletion across all node cases
- Parent separator-key updates after borrow/merge in delete path
- Full internal-node underflow propagation handling
- Consistent memory ownership and deallocation strategy for record pointers
- Safe handling of names containing spaces (sample mostly uses %s scanning)
- Unified in-memory reporting path (sample mixes file-based and index-based analytics)
- Full cleanup path for all allocated records and nodes on exit
- Robust input validation and user-facing error messages for every failure path
