#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define TABLELEN 90000000
#define HEAP_START_SIZE 10000

typedef struct {  // struct to represent a board
    int move; //Up : 1 || Down : 2 || Left : 3 || Right : 4  move required to get to this board
    int h; // sum of unit distance from start + manhattan distance to goal
    int g;
    struct board *next; // for the single linked list of hashmap
    struct board *pred;
    int layout[];   
} board;

typedef struct {
    int size;
    int numitems;
    board ** data;
} heap;

/* ################### BOARD STUFF ###################### */
int manhattan(int arr1[],int N)
{
    int dim = sqrt(N);
    int result = 0;
    int goal[dim][dim];
    int arr2[dim][dim];
    for (int x=0; x<N; x++) {
        int row = x / dim;
        int col = x % dim;
        goal[row][col] = x + 1;
        arr2[row][col] = arr1[x];
    }
    // print2D(3,goal);
    // print2D(3,arr2);
    for (int i=0; i<dim; i++){
        for (int j=0; j<dim; j++){
            if (goal[i][j] == N) continue;
            for (int l=0; l<dim; l++){
                for (int m=0; m<dim; m++){
                    if (goal[i][j] == arr2[l][m]){
                        result += (abs(m - j) + abs(l - i));
                        // printf("%d %d %d %d %d\n", i,j,l,m,result);
                        break;
                    }
                }
            }
        }
    }
    // printarr(arr1,BOARD_SIZE);
    // printf("MD is %d\n",result);
    return result;
}

int findBlank(int arr1[], int n)
{
    for (int i=0; i<n; i++){
        if (arr1[i] == n){
           return i;
        }
    }
    return -1;
}

int isSolvable(int arr1[], int n)
{
    int numinv = 0;
    int blank = findBlank(arr1,n);
    int dim = sqrt(n);
    int tmp = blank / dim;
    tmp = dim - tmp; 
    for (int i=0; i<n-1; i++){
        for (int j=i+1; j<n; j++){
            if ((arr1[i] != n) && (arr1[j] != n) && (arr1[i] > arr1[j])) numinv += 1;
        }
    }
    if ((dim % 2 == 1) && (numinv % 2 == 0)) return 1;
    if (dim % 2 == 0){
        if ((tmp % 2 == 1) && (numinv % 2 == 0))return 1;
        if ((tmp % 2 == 0) && (numinv % 2 == 1))return 1;
    }
    return 0;
}

int * moveup(int arr1[], int n)
{
    int dim = sqrt(n);
    int blank = findBlank(arr1,n);
    if ((blank / dim - 1) < 0){
        return NULL;
    }
    int *temp = malloc(n*sizeof(int));
    for (int i=0; i<n; i++){
        temp[i] = arr1[i];
    }
    int new = (((blank / dim) - 1) * dim) + (blank % dim);
    temp[blank] = arr1[new];
    temp[new] = n;
    return temp;
}

int * movedown(int arr1[], int n)
{
    int dim = sqrt(n);
    int blank = findBlank(arr1,n);
    if ((blank / dim + 1) > (dim - 1)){
        return NULL;
    }
    int *temp = malloc(n*sizeof(int));
    for (int i=0; i<n; i++){
        temp[i] = arr1[i];
    }
    int new = (((blank / dim) + 1) * dim) + (blank % dim);
    temp[blank] = arr1[new];
    temp[new] = n;
    return temp;
}

int * moveleft(int arr1[], int n)
{
    int dim = sqrt(n);
    int blank = findBlank(arr1,n);
    if ((blank % dim - 1) < 0){
        return NULL;
    }
    int *temp = malloc(n*sizeof(int));
    for (int i=0; i<n; i++){
        temp[i] = arr1[i];
    }
    int new = ((blank / dim) * dim) + (blank % dim - 1);
    temp[blank] = arr1[new];
    temp[new] = n;
    return temp;
}

int * moveright(int arr1[], int n)
{
    int dim = sqrt(n);
    int blank = findBlank(arr1,n);
    if ((blank % dim + 1) > (dim - 1)){
        return NULL;
    }
    int *temp = malloc(n*sizeof(int));
    for (int i=0; i<n; i++){
        temp[i] = arr1[i];
    }
    int new = ((blank / dim) * dim) + (blank % dim + 1);
    temp[blank] = arr1[new];
    temp[new] = n;
    return temp;
}

int arrequal(int arr1[], int arr2[], int n) // check if 2 arrays are equal
{
    for (int i=0; i<n; i++){
        if (arr1[i] != arr2[i]){
            return 0;
        }
    }
    return 1;
}
/* ################### BOARD STUFF ###################### */

/* ################### HASHMAP STUFF ###################### */
int arrComp(int arr1[], int arr2[], int n) // compare two arrays to see if equal
{
    if (arr1 == NULL || arr2 == NULL) return 0;
    for (int i=0; i<n; i++){
        if (arr1[i] != arr2[i]) return 0;
    }
    return 1;
}

char * toString(int arr[], int size) // return a string representation of array {1,2,3} ==> "1 2 3"
{
    if (arr==NULL) return NULL;
    char *temp = malloc(size * sizeof(char) + 1);
    temp[size * sizeof(char)] = "\0";
    for(int i=0; i<size; i++){
        temp[i] = arr[i] + '0';
    }
    return temp;
}

unsigned int hash1(unsigned char *str)
{
    unsigned int hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % TABLELEN;
}

void printarr(int arr[], int size) // print out elements of array of ints
{
    if (arr == NULL) return;
    for (int i=0; i<size; i++){
        printf("%d ",arr[i]);
    }
    printf("\n");
}

void init_hash_table(board *ht[]) // set all table pointers to NULL
{
    for (int i = 0; i<TABLELEN; i++){
        ht[i] = NULL;
    }
}

int hash_table_insert(board *hash_table[], board *b, int size) // insert a board to the hash table
{
    if (b == NULL) return 0;
    unsigned int idx = hash1(toString(b->layout,size));
    b->next = hash_table[idx];
    hash_table[idx] = b;
    return 1;
}

board * hash_table_lookup(board *hash_table[], int arr[], int size) // lookup a board
{
    if (arr == NULL) return NULL;
    unsigned int idx = hash1(toString(arr,size));
    board *tmp = hash_table[idx];
    while (tmp != NULL && arrComp(tmp->layout,arr,size)==0){
        tmp = tmp->next;
    }
    // if (tmp != NULL){
    //     printf("Found\n");
    // }
    // else {
    //     printf("Not Found\n");
    // }
    return tmp;
}

board * hash_table_delete(board *hash_table[],int arr[], int size) // delete entry in hash table
{
    if (arr == NULL) return NULL;
    unsigned int idx = hash1(toString(arr,size));
    board *tmp = hash_table[idx];
    board *prev = NULL;
    while (tmp != NULL && arrComp(tmp->layout,arr,size)==0){
        prev = tmp;
        tmp = tmp->next;
    }
    if (tmp == NULL) return NULL;
    if (prev == NULL){
        hash_table[idx] = tmp->next;
    }
    else {
        prev->next = tmp->next;
    }
    return tmp;
}

/* ################### HASHMAP STUFF ###################### */

/* ################### HEAP STUFF ######################### */
int parent(int a)
{
    return (a - 1) / 2;
}

int left_child(int a)
{
    return (a * 2) + 1;
}

int right_child(int a)
{
    return (a * 2) + 2;
}

void swap(board ** a, board ** b)
{
    board * temp = *a;
    *a = *b;
    *b = temp;
}

// insert the item at the appropriate position
void enqueue(heap *h, int move, int g, int arr[], int size, board *p)
{
    if (h->numitems == h->size){ // expand the array
        h->size = h->size * 2;
        board **t = realloc(h->data, sizeof(board *) * h->size);
        if (t == NULL){
            printf("Error. Cant expand heap..\n");
            exit(0);
        }
        else{
            h->data = t;
        }
        printf("Changed size of heap to %d since n is %d\n",h->size,h->numitems);
    }
    // first insert at the last position of the array and move it up
    board **a = h->data; // simpler notation to use a and n
    int n = h->numitems;
    board *tmp = malloc(sizeof(*tmp) + sizeof(int)*size);
    tmp->g=g;
    tmp->h=manhattan(arr,size)+g;
    tmp->move=move;
    tmp->next=NULL;
    tmp->pred=p;
    memcpy(tmp->layout, arr, size*sizeof(int));
    a[n]= tmp;
    n = n + 1;

    // move up until the heap property satisfiess
    int i = n - 1;
    while (i != 0 && (a[parent(i)]->h > a[i]->h)) {
        swap(&a[i],&a[parent(i)]);
        i = parent(i);
    }
    h->numitems = n;
}

// moves the item at position i of array a into its appropriate position
void min_heapify(heap *h, int i)
{
    // simpler notation to use a and n
    board ** a = h->data;
    int n = h->numitems;

    // find left child node
    int left = left_child(i);

    // find right child node
    int right = right_child(i);

    // find the largest among 3 nodes
    int smallest = i;

    // check if the left node is smaller than the current node
    if (left <= n && a[left]->h < a[smallest]->h) {
            smallest = left;
    }

    // check if the right node is smaller than the current node
    if (right <= n && a[right]->h < a[smallest]->h) {
            smallest = right;
    }

    // swap the smaller node with the current node 
    // and repeat this process until the current node is smaller than 
    // the right and the left node
    if (smallest != i) {
            swap(&a[i],&a[smallest]);
            min_heapify(h, smallest);
    }
}

board * dequeue(heap *h)
{
    // simpler notation to use a and n
    board **a = h->data; 
    int n = h->numitems;

    if (n == 0)
    {
        printf("%s","Cannot remove any item. Queue is empty\n");
        return;
    }

    board * min_item = a[0];
    a[0] = a[n - 1];
    n = n - 1;
    h->numitems = n;

    min_heapify(h, 0);
    return min_item;
}

// intialize the heap with size start_size
void heap_init(heap *h, int start_size)
{
    h->data = malloc(start_size * sizeof(board *));
    h->numitems=0;
    h->size=start_size;
}
// free memory of the heap
void heap_end(heap *h)
{
    free(h->data);
}
/* ################### HEAP STUFF ######################### */

int main()
{
    clock_t begin = clock();
    // int start[] = {1,9,2,4,5,3,7,8,6}; // 4 moves
    // int start[] = {6,5,4,1,7,3,9,8,2}; // 26 moves
    // int start[] = {8,6,7,2,5,4,3,9,1}; // 31 moves
    int start[] = {6,4,7,8,5,9,3,2,1}; // 31 moves
    // int start[] = {5,1,2,3,6,16,7,4,9,10,11,8,13,14,15,12}; // 8 moves
    // int start[] = {7,8,4,11,12,14,10,15,16,5,3,13,2,1,9,6}; // 50 moves
    // int start[] = {7,9,13,1,14,16,8,15,3,6,10,5,2,11,12,4}; // 58 moves
    // int start[] = {15,14,1,6,9,11,4,12,16,10,7,3,13,8,5,2}; // 52 moves
    // int start[] = {7,14,16,9,10,2,11,13,6,15,4,12,5,1,8,3}; // 54 moves
    int arrlen = *(&start + 1) - start;
    #define BOARD_SIZE arrlen
    assert(isSolvable(start,BOARD_SIZE) == 1);
    // initilaise goal array based on input length
    int goal[arrlen];
    for (int i=0; i<arrlen; i++){
        goal[i] = i+1;
    }
    heap h;
    int g = 0;
    heap_init(&h,HEAP_START_SIZE);
    board * ht = malloc(TABLELEN * sizeof(board*));
    init_hash_table(ht);
    enqueue(&h,-1,g,start,BOARD_SIZE,NULL);
    int pos_seen = 0;
    int moves = 0;
    while (h.numitems > 0){
        board * current = dequeue(&h);
        hash_table_insert(ht,current,BOARD_SIZE);
        pos_seen++;
        if (arrequal(goal,current->layout,BOARD_SIZE) == 1){
            while (current->pred != NULL){
                for (int i=0; i<BOARD_SIZE; i++){
                    printf("%d ",current->layout[i]);
                }
                printf("Move %d || G %d || H %d\n",current->move,current->g,current->h);
                moves++;
                current = current->pred;
            }
            printf("Solved in %d moves looking at %d positions\n",moves,pos_seen);
            // printf("Solved\n");
            break;
        }
        // // check if current board layout in closed set
        board *check = hash_table_lookup(ht,current->layout,BOARD_SIZE);
        if (check->h > current->h){
            // hash_table_delete(ht, current->layout, BOARD_SIZE);
            continue;
        }
        g = current->g + 1;
        int *up = moveup(current->layout,BOARD_SIZE);
        if (up != NULL && hash_table_lookup(ht,up,BOARD_SIZE)==NULL){
            enqueue(&h,1,g,up,BOARD_SIZE,current);
        }
        int *down = movedown(current->layout,BOARD_SIZE);
        if (down != NULL && hash_table_lookup(ht,down,BOARD_SIZE)==NULL){
            enqueue(&h,2,g,down,BOARD_SIZE,current);
        }
        int *left = moveleft(current->layout,BOARD_SIZE);
        if (left != NULL && hash_table_lookup(ht,left,BOARD_SIZE)==NULL){
            enqueue(&h,3,g,left,BOARD_SIZE,current);
        }
        int *right = moveright(current->layout,BOARD_SIZE);
        if (right != NULL && hash_table_lookup(ht,right,BOARD_SIZE)==NULL){
            enqueue(&h,4,g,right,BOARD_SIZE,current);
        }
    }
    heap_end(&h);
    clock_t end = clock();
    double time_spent = (end - begin) / CLOCKS_PER_SEC;
    printf("Ending.....\nElapsed: %.3f seconds\n", (double)(end - begin) / CLOCKS_PER_SEC);
    return 0;
}

