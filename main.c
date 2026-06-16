#include "Ride_hailing.h"

void clearInputBuffer(void) {
    int ch;
    do {
        ch = getchar();
    } while (ch != '\n' && ch != EOF);
}

void trimNewlineMain(char *s) {
    size_t n;
    if (s == NULL) {
        return;
    }
    n = strlen(s);
    if (n > 0U && s[n - 1] == '\n') {
        s[n - 1] = '\0';
    }
}

int main(void) {
    int choice;

    loadData();

    while (1) {
        int id;
        int type;
        int x;
        int y;
        int p_id;
        int prefType;
        int bookingId;
        int d_id;
        int p1;
        int p2;
        float distance;
        float earnings;
        char name[MAX_NAME_LEN];
        char mobile[MAX_MOBILE_LEN];

        printf("\n===== Ride-Hailing Menu =====\n");
        printf("1. Add Driver\n");
        printf("2. Add Passenger\n");
        printf("3. Request Ride\n");
        printf("4. Complete Ride\n");
        printf("5. Display Top Drivers\n");
        printf("6. Display Frequent Passenger-Driver Pair\n");
        printf("7. Display Available Vehicles\n");
        printf("8. Update Driver Location\n");
        printf("9. Delete Driver\n");
        printf("10. Display Booking History\n");
        printf("11. Range Search Passengers by ID\n");
        printf("12. Calculate Driver Earnings\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Exiting.\n");
            break;
        }
        clearInputBuffer();

        switch (choice) {
            case 1:
                printf("Enter Name: ");
                if (fgets(name, sizeof(name), stdin) == NULL) {
                    printf("Invalid name input.\n");
                    break;
                }
                trimNewlineMain(name);
                printf("Enter Vehicle Type (0 = Cab, 1 = Bike): ");
                scanf("%d", &type);
                printf("Enter Current Location (x y): ");
                scanf("%d %d", &x, &y);
                clearInputBuffer();
                addDriver(name, type, x, y);
                break;

            case 2:
                printf("Enter Name: ");
                if (fgets(name, sizeof(name), stdin) == NULL) {
                    printf("Invalid name input.\n");
                    break;
                }
                trimNewlineMain(name);
                printf("Enter Mobile Number: ");
                if (fgets(mobile, sizeof(mobile), stdin) == NULL) {
                    printf("Invalid mobile input.\n");
                    break;
                }
                trimNewlineMain(mobile);
                addPassenger(name, mobile);
                break;

            case 3:
                printf("Enter Passenger ID: ");
                scanf("%d", &p_id);
                printf("Enter Passenger Location (x y): ");
                scanf("%d %d", &x, &y);
                printf("Enter Preferred Vehicle Type (-1 = Any, 0 = Cab, 1 = Bike): ");
                scanf("%d", &prefType);
                clearInputBuffer();
                requestRide(p_id, x, y, prefType);
                break;

            case 4:
                printf("Enter Booking ID: ");
                scanf("%d", &bookingId);
                printf("Enter Distance Travelled (km): ");
                scanf("%f", &distance);
                clearInputBuffer();
                completeRide(bookingId, distance);
                break;

            case 5:
                displayTopDrivers();
                break;

            case 6:
                displayFrequentPairs();
                break;

            case 7:
                displayAvailableVehicles();
                break;

            case 8:
                printf("Enter Driver ID: ");
                scanf("%d", &d_id);
                printf("Enter New Location (x y): ");
                scanf("%d %d", &x, &y);
                clearInputBuffer();
                updateDriverLocation(d_id, x, y);
                break;

            case 9:
                printf("Enter Driver ID to delete: ");
                scanf("%d", &d_id);
                clearInputBuffer();
                deleteDriver(d_id);
                break;

            case 10:
                displayBookingHistory();
                break;

            case 11:
                printf("Enter P_ID1 and P_ID2: ");
                scanf("%d %d", &p1, &p2);
                clearInputBuffer();
                range_search(p1, p2);
                break;

            case 12:
                printf("Enter Driver ID: ");
                scanf("%d", &d_id);
                clearInputBuffer();
                earnings = calculateDriverEarnings(d_id);
                printf("Total earnings of driver %d = %.2f\n", d_id, earnings);
                break;

            case 0:
                saveDrivers();
                savePassengers();
                saveBookings();
                freeAllData();
                printf("Exiting program.\n");
                return 0;

            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }

    saveDrivers();
    savePassengers();
    saveBookings();
    freeAllData();
    return 0;
}
