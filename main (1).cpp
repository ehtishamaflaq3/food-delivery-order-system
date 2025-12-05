#include <iostream>
#include <ctime>
#include <cstdlib>
using namespace std;
// ----------------- ORDER QUEUE -----------------
class OrderQueue
{
    static const int MAX_ORDERS = 100;
    int orders[MAX_ORDERS];
    int front;
    int rear;
    int count;
public:
    OrderQueue()
    {
        front = 0;
        rear = -1;
        count = 0;
    }
    // for checking its empty or not
    bool isEmpty()
    {
        return count == 0;
    };
    // for checking its full or not
    bool isFull()
    {
        return count == MAX_ORDERS;
    };
    // adding orders of the users with id because id helps us to locate its eta and tracking
    void enqueueOrder(int orderID)
    {
        if (isFull())
        {
            cout << "Queue is full! Cannot add order " << orderID << endl;
            return;
        }
        rear = (rear + 1) % MAX_ORDERS;
        orders[rear] = orderID;
        count++;
        cout << "Order " << orderID << " added." << endl;
    };
    // for removing orders from start acc to the fifo principle
    void dequeueOrder()
    {
        if (isEmpty())
        {
            cout << "Queue is empty! Cannot remove order." << endl;
            return;
        }
        cout << "Order " << orders[front] << " removed." << endl;
        front = (front + 1) % MAX_ORDERS;
        count--;
    };
    // for checking which order is first to be completed
    int peekOrder()
    {
        if (isEmpty())
        {
            cout << "Queue is empty!" << endl;
            return -1;
        }
        return orders[front];
    }
};
// now our first step is completed now we work on riders
//  ----------------- RIDER LIST -----------------
//  we stroe riders in list for managing
struct Rider
{
    int riderID;
    int currentZone;
    bool available;
    Rider *next;
};
class RiderList
{
    Rider *head;

public:
    RiderList()
    {
        head = nullptr;
    }
    // add rider in system
    void addRider(int riderID, int zone)
    {
        // this line create new rider
        Rider *newRider = new Rider{riderID, zone, true, nullptr};
        // if list is empty then new rider is first rider
        if (!head)
        {
            head = newRider;
        }
        else
        {
            // if list has riders
            Rider *temp = head;
            while (temp->next) // go to the end of the list
                temp = temp->next;
            temp->next = newRider; // add new rider at end
        }
        cout << "Rider " << riderID << " added at zone " << zone << endl;
    };
    // for remoiving rider from system................................
    void removeRider(int riderID)
    {
        // if there is noe rider in list edge case
        if (!head)
            return;
        // if we want to remove the first rider to remove
        if (head->riderID == riderID)
        {
            Rider *temp = head;
            head = head->next;
            delete temp;
            cout << "Rider " << riderID << " removed." << endl;
            return;
        }
        // if ur rider is in middle or end in list
        Rider *temp = head;
        while (temp->next && temp->next->riderID != riderID)
            temp = temp->next;
        if (temp->next)
        {
            Rider *toDelete = temp->next;
            temp->next = temp->next->next;
            delete toDelete;
            cout << "Rider " << riderID << " removed." << endl;
        }
    }
    // look all riders and find the one who is available and closest to the order zone
    Rider *findNearestRider(int orderZone)
    {
        Rider *temp = head;
        Rider *nearest = nullptr;
        int minDistance = 1000;
        while (temp)
        {
            if (temp->available)
            {
                // for safety of minus value we use abs which means absolute value (2-3=-1) but abs gives answer 1
                int dist = abs(temp->currentZone - orderZone);

                if (dist < minDistance)
                {
                    minDistance = dist;
                    nearest = temp;
                }
            }
            temp = temp->next;
        }
        return nearest;
    };
    // now aftere finding distance we assign order to the selected rider
    void assignRider(int orderID, int orderZone)
    {
        Rider *rider = findNearestRider(orderZone);
        if (rider)
        {
            rider->available = false;
            cout << "Order " << orderID << " assigned to Rider " << rider->riderID << endl;
        }
        else
        {
            cout << "No available rider for order " << orderID << endl;
        }
    }
    // all riders
    void showRiders()
    {
        Rider *temp = head;
        cout << "Riders List:\n";
        while (temp)
        {
            cout << "Rider " << temp->riderID
                 << " | Zone: " << temp->currentZone
                 << " | " << (temp->available ? "Available" : "Busy") << endl;
            temp = temp->next;
        }
    }
};
//
// ----------------- ZONE TREE -----------------
struct ZoneNode
{
    int zoneID;
    string zoneName;
    ZoneNode *left;
    ZoneNode *right;
    ZoneNode(int id) : zoneID(id), left(nullptr), right(nullptr) {}
};
class ZoneTree
{
    ZoneNode *root;

public:
    ZoneTree() { root = nullptr; }
    // insert zone
    void insertZone(int id)
    {
        root = insertZoneHelper(root, id);
    }
    // search zone
    bool searchZone(int id)
    {
        return searchZoneHelper(root, id);
    }

private:
    ZoneNode *insertZoneHelper(ZoneNode *node, int id)
    {
        if (!node)
            return new ZoneNode(id);
        if (id < node->zoneID)
            node->left = insertZoneHelper(node->left, id);
        else if (id > node->zoneID)
            node->right = insertZoneHelper(node->right, id);
        return node;
    }
    bool searchZoneHelper(ZoneNode *node, int id)
    {
        if (!node)
            return false;
        if (node->zoneID == id)
            return true;
        if (id < node->zoneID)
            return searchZoneHelper(node->left, id);
        else
            return searchZoneHelper(node->right, id);
    }
};
// eta
// ----------------- ETA -----------------
class ETA
{
public:
    ETA()
    {
        srand(time(0));
    }
    int calculateETA(int zoneDistance, int prepTime)
    {
        int speed = 2; // units per minute
        return (zoneDistance / speed) + prepTime;
    }
    int randomDistance()
    {
        return rand() % 10 + 1; // random distance 1-10
    }
};
// dispatch manager
// ----------------- DISPATCH MANAGER -----------------
class DispatchManager {
    OrderQueue orders;
    RiderList riders;
    ZoneTree zones;
    ETA etaCalc;
public:
    // Add rider
    void addRider(int riderID, int zone) {
        riders.addRider(riderID, zone);
    }
    // Add zone
    void addZone(int zoneID) {
        zones.insertZone(zoneID);
    }
    // Add order
    void createOrder(int orderID,string orderName, int zoneID, int prepTime) {
        cout << "\nCreating order " << orderID << " for zone " << zoneID << endl;
        // Check if zone exists
        if (!zones.searchZone(zoneID)) {
            cout << "Zone " << zoneID << " does not exist! Cannot place order.\n";
            return;
        }
        // Add order to queue
        orders.enqueueOrder(orderID);
        // Assign nearest rider
        Rider* rider = riders.findNearestRider(zoneID);
        if (!rider) {
            cout << "No available rider for order " << orderID << endl;
            return;
        }
        riders.assignRider(orderID, zoneID);
        // Calculate ETA
        int distance = etaCalc.randomDistance(); 
        int etaMinutes = etaCalc.calculateETA(distance, prepTime);
        cout << "Estimated delivery time for order " << orderID
             << " = " << etaMinutes << " minutes\n";
    }
    // Show all riders
    void showRiders() {
        riders.showRiders();
    }
};
// ----------------- MAIN -----------------
int main()
{
    /*
    OrderQueue q;
    q.enqueueOrder(101);
    q.enqueueOrder(106);
    q.enqueueOrder(109);
    cout << "First order in queue: " << q.peekOrder() << endl;
    q.dequeueOrder();
    cout << "First order now: " << q.peekOrder() << endl;
    */
    // ///////////////////////////////////////
    /*
    RiderList riders;
    // we add riders with its number iin list and zones
    riders.addRider(1, 2);
    riders.addRider(2, 4);
    riders.addRider(3, 6);
    // its shows all riders
    riders.showRiders();
    // assigning rider with food id order and zone
    riders.assignRider(101, 4);
    riders.assignRider(109, 7);
    riders.showRiders();
    */
    ////////////////////////////////////////////
    /*
    ZoneTree zones;
    zones.insertZone(1);
    zones.insertZone(2);
    zones.insertZone(3);
    zones.insertZone(4);
    zones.insertZone(5);
    cout << "Zone 3 exists? " << (zones.searchZone(3) ? "Yes" : "No") << endl;
    cout << "Zone 7 exists? " << (zones.searchZone(7) ? "Yes" : "No") << endl;
    */
    // ETA calculation
   /*
    ETA etaCalc;
    int distance = etaCalc.randomDistance();
    int prepTime = 5;
    cout << "ETA for distance " << distance << " and prep " << prepTime<< " = " << etaCalc.calculateETA(distance, prepTime) << " minutes" << endl;
    */
    DispatchManager dm;
    // Add zones
    dm.addZone(1);
    dm.addZone(2);
    dm.addZone(3);
    dm.addZone(4);
    dm.addZone(5);
    // Add riders
    dm.addRider(1, 5);
    dm.addRider(2, 2);
    dm.addRider(3, 4);
    dm.addRider(4, 8);
    dm.showRiders();
    // Create orders
    dm.createOrder(101,"pizza", 4, 5); // orderID, zoneID, prepTime
    dm.createOrder(102,"pratha", 7, 3); // zone 7 doesn't exist
    dm.createOrder(103,"chai" ,2, 4);
    dm.showRiders();
    return 0;
}