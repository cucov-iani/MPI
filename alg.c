// Iani Cucov - Academic Writing //

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BUBBLE_SORT 0
#define QUICK_SORT 1
#define INSERTION_SORT 2
#define SELECTION_SORT 3
#define MERGE_SORT 4
#define RADIX_SORT 5

/* Display names for each algorithm, indexed by the ALGORITHM constants above. */
static const char* ALGORITHM_NAMES[] = {
    "bubble sort",
    "quick sort",
    "insertion sort",
    "selection sort",
    "merge sort",
    "radix sort"
};

/* Output file handle for results.csv. Opened once in main() and shared by all write functions. */
static FILE* resultsFile = NULL;

/* Seeds the random number generator with the current time so each run produces different data. */
void resetRandomSeed() {
    srand((unsigned int)time(NULL));
}

/* Records the current monotonic time. Call before the operation you want to time. */
struct timespec startOperationTimer() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

/* Returns the elapsed time in nanoseconds since startTime. */
double stopOperationTimerNs(struct timespec startTime) {
    struct timespec endTime;
    clock_gettime(CLOCK_MONOTONIC, &endTime);

    return (double)(endTime.tv_sec - startTime.tv_sec) * 1000000000.0 +
           (double)(endTime.tv_nsec - startTime.tv_nsec);
}

/* Converts nanoseconds to milliseconds. */
double nsToMs(double nanoseconds) {
    return nanoseconds / 1000000.0;
}

/* Allocates and returns an array of `size` random integers in the range [0, size].
   The caller is responsible for freeing the returned pointer. */
int* generateIntArray(unsigned long size) {
    int* array = malloc(sizeof(int) * size);
    if (array == NULL) {
        return NULL;
    }

    for (unsigned long i = 0; i < size; i++) {
        array[i] = (int)(rand() % (size + 1));
    }

    return array;
}

/* Allocates and returns a partially sorted array of `size` integers.
   Starts as a sorted sequence 0..size-1, then performs size/factor random swaps.
   A smaller factor means more swaps (less sorted); a larger factor means fewer swaps (more sorted).
   Example: factor=10 gives ~10% of elements swapped; factor=90 gives ~1% swapped.
   The caller is responsible for freeing the returned pointer. */
int* generatePartiallySortedIntArray(unsigned long size, int factor) {
    int* array = malloc(sizeof(int) * size);
    if (array == NULL) {
        return NULL;
    }

    for (unsigned long i = 0; i < size; i++) {
        array[i] = (int)i;
    }

    unsigned long swapCount = size / factor;
    if (swapCount == 0 && size > 1) {
        swapCount = 1;
    }

    for (unsigned long i = 0; i < swapCount; i++) {
        unsigned long indexA = (unsigned long)(rand() % size);
        unsigned long indexB = (unsigned long)(rand() % size);

        int temp = array[indexA];
        array[indexA] = array[indexB];
        array[indexB] = temp;
    }

    return array;
}

/* Allocates and returns an array of `size` integers in strictly descending order (size-1 down to 0).
   The caller is responsible for freeing the returned pointer. */
int* generateReverseSortedIntArray(unsigned long size) {
    int* array = malloc(sizeof(int) * size);
    if (array == NULL) {
        return NULL;
    }

    for (unsigned long i = 0; i < size; i++) {
        array[i] = (int)(size - 1 - i);
    }

    return array;
}

/* Sorts array in-place using bubble sort (O(n^2) average/worst, O(n) best).
   Includes an early-exit optimisation: stops if a full pass produces no swaps. */
void bubbleSort(int* array, unsigned long size) {
    if (array == NULL || size < 2) {
        return;
    }

    for (unsigned long i = 0; i < size - 1; i++) {
        int swapped = 0;

        for (unsigned long j = 0; j < size - 1 - i; j++) {
            if (array[j] > array[j + 1]) {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
                swapped = 1;
            }
        }

        if (!swapped) {
            break;
        }
    }
}

/* Swaps the two integers pointed to by a and b. */
void swapInts(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/* Chooses the median of array[low], array[mid], array[high] as pivot,
   swaps it into the array[high] position, then partitions array[low..high].
   Elements <= pivot are moved to the left; elements > pivot to the right.
   Returns the final index of the pivot. */
long partitionQuickSort(int* array, long low, long high) {
    long mid = low + (high - low) / 2;

    if (array[low] > array[mid])  swapInts(&array[low], &array[mid]);
    if (array[low] > array[high]) swapInts(&array[low], &array[high]);
    if (array[mid] > array[high]) swapInts(&array[mid], &array[high]);

    swapInts(&array[mid], &array[high]);

    int pivot = array[high];
    long i = low - 1;

    for (long j = low; j < high; j++) {
        if (array[j] <= pivot) {
            i++;
            swapInts(&array[i], &array[j]);
        }
    }

    swapInts(&array[i + 1], &array[high]);
    return i + 1;
}

/* Recursive helper for quickSort. Sorts array[low..high] in-place. */
void quickSortRecursive(int* array, long low, long high) {
    if (low < high) {
        long pivotIndex = partitionQuickSort(array, low, high);
        quickSortRecursive(array, low, pivotIndex - 1);
        quickSortRecursive(array, pivotIndex + 1, high);
    }
}

/* Sorts array in-place using quick sort. */
void quickSort(int* array, unsigned long size) {
    if (array == NULL || size < 2) {
        return;
    }

    quickSortRecursive(array, 0, (long)size - 1);
}

/* Sorts array in-place using insertion sort. */
void insertionSort(int* array, unsigned long size) {
    if (array == NULL || size < 2) {
        return;
    }

    for (unsigned long i = 1; i < size; i++) {
        int key = array[i];
        long j = (long)i - 1;

        while (j >= 0 && array[j] > key) {
            array[j + 1] = array[j];
            j--;
        }

        array[j + 1] = key;
    }
}

/* Sorts array in-place using selection sort. */
void selectionSort(int* array, unsigned long size) {
    if (array == NULL || size < 2) {
        return;
    }

    for (unsigned long i = 0; i < size - 1; i++) {
        unsigned long minIndex = i;

        for (unsigned long j = i + 1; j < size; j++) {
            if (array[j] < array[minIndex]) {
                minIndex = j;
            }
        }

        if (minIndex != i) {
            swapInts(&array[i], &array[minIndex]);
        }
    }
}

/* Merges two adjacent sorted sub-arrays: array[left..mid] and array[mid+1..right]. */
void merge(int* array, unsigned long left, unsigned long mid, unsigned long right) {
    unsigned long n1 = mid - left + 1;
    unsigned long n2 = right - mid;

    int* L = malloc(sizeof(int) * n1);
    int* R = malloc(sizeof(int) * n2);

    if (L == NULL || R == NULL) {
        free(L);
        free(R);
        return;
    }

    for (unsigned long i = 0; i < n1; i++) {
        L[i] = array[left + i];
    }

    for (unsigned long j = 0; j < n2; j++) {
        R[j] = array[mid + 1 + j];
    }

    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long k = left;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            array[k++] = L[i++];
        } else {
            array[k++] = R[j++];
        }
    }

    while (i < n1) {
        array[k++] = L[i++];
    }

    while (j < n2) {
        array[k++] = R[j++];
    }

    free(L);
    free(R);
}

/* Recursive helper for mergeSort. Sorts array[left..right] in-place. */
void mergeSortRecursive(int* array, long left, long right) {
    if (left < right) {
        long mid = left + (right - left) / 2;
        mergeSortRecursive(array, left, mid);
        mergeSortRecursive(array, mid + 1, right);
        merge(array, (unsigned long)left, (unsigned long)mid, (unsigned long)right);
    }
}

/* Sorts array in-place using merge sort. */
void mergeSort(int* array, unsigned long size) {
    if (array == NULL || size < 2) {
        return;
    }

    mergeSortRecursive(array, 0, (long)size - 1);
}

/* Returns the maximum value in array. Used by radixSort. */
int getMaxValue(int* array, unsigned long size) {
    int mx = array[0];

    for (unsigned long i = 1; i < size; i++) {
        if (array[i] > mx) {
            mx = array[i];
        }
    }

    return mx;
}

/* Performs a stable counting sort on array by the digit at position exp. */
void countSortByDigit(int* array, unsigned long size, int exp) {
    if (array == NULL || size == 0) {
        return;
    }

    int* output = malloc(sizeof(int) * size);
    if (output == NULL) {
        return;
    }

    int count[10] = {0};

    for (unsigned long i = 0; i < size; i++) {
        int digit = (array[i] / exp) % 10;
        count[digit]++;
    }

    for (int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
    }

    for (long i = (long)size - 1; i >= 0; i--) {
        int digit = (array[i] / exp) % 10;
        output[count[digit] - 1] = array[i];
        count[digit]--;
    }

    for (unsigned long i = 0; i < size; i++) {
        array[i] = output[i];
    }

    free(output);
}

/* Sorts array in-place using radix sort. */
void radixSort(int* array, unsigned long size) {
    if (array == NULL || size < 2) {
        return;
    }

    int maxValue = getMaxValue(array, size);

    for (int exp = 1; maxValue / exp > 0; exp *= 10) {
        countSortByDigit(array, size, exp);
    }
}

/* Runs the sorting algorithm identified by `type` on `data` and returns the elapsed time in nanoseconds. */
double measureSortAlgNs(int type, int* data, unsigned long size) {
    struct timespec startTime = startOperationTimer();

    switch (type) {
        case BUBBLE_SORT:
            bubbleSort(data, size);
            break;
        case QUICK_SORT:
            quickSort(data, size);
            break;
        case INSERTION_SORT:
            insertionSort(data, size);
            break;
        case SELECTION_SORT:
            selectionSort(data, size);
            break;
        case MERGE_SORT:
            mergeSort(data, size);
            break;
        case RADIX_SORT:
            radixSort(data, size);
            break;
        default:
            printf("Algorithm not implemented\n");
            break;
    }

    return stopOperationTimerNs(startTime);
}

/* Prints the CSV column header to stdout. */
void printHeader() {
    printf("Elements,Runs,Algorithm,Variant,Elapsed time(ms)\n");
    if (resultsFile) {
        fprintf(resultsFile, "Elements,Runs,Algorithm,Variant,Elapsed time(ms)\n");
    }
}

/* Writes one result row to stdout and to resultsFile (if open).
   Format: size, count, algorithmName, variant, elapsedMs */
void writeResultRow(unsigned long size, int count, const char* algorithmName, const char* variant, double elapsedMs) {
    char line[256];

    /* 9 zecimale in ms = precizie pana la nanosecunde la nivel de afisare */
    snprintf(line, sizeof(line), "%lu,%d,%s,%s,%.9f", size, count, algorithmName, variant, elapsedMs);

    printf("%s\n", line);

    if (resultsFile) {
        fprintf(resultsFile, "%s\n", line);
    }
}

/* Benchmarks algorithm `type` on arrays of length `size` across all input variants. */
void analyzeSortAlg(int type, unsigned long size, int count) {
    double elapsedNs;
    double elapsedMs;
    double sumNs;
    const char* algorithmName = ALGORITHM_NAMES[type];
    int* x;

    /* reverse sorted */
    sumNs = 0.0;
    for (int i = 0; i < count; i++) {
        x = generateReverseSortedIntArray(size);
        if (x == NULL) {
            printf("Memory allocation failed for reversed array\n");
            return;
        }

        sumNs += measureSortAlgNs(type, x, size);
        free(x);
    }

    elapsedNs = sumNs / count;
    elapsedMs = nsToMs(elapsedNs);
    writeResultRow(size, count, algorithmName, "reversed", elapsedMs);

    /* random data */
    sumNs = 0.0;
    for (int i = 0; i < count; i++) {
        x = generateIntArray(size);
        if (x == NULL) {
            printf("Memory allocation failed for random array\n");
            return;
        }

        sumNs += measureSortAlgNs(type, x, size);
        free(x);
    }

    elapsedNs = sumNs / count;
    elapsedMs = nsToMs(elapsedNs);
    writeResultRow(size, count, algorithmName, "random", elapsedMs);

    /* partially sorted */
    for (int factor = 10; factor <= 100; factor += 20) {
        sumNs = 0.0;

        for (int i = 0; i < count; i++) {
            x = generatePartiallySortedIntArray(size, factor);
            if (x == NULL) {
                printf("Memory allocation failed for partially sorted array\n");
                return;
            }

            sumNs += measureSortAlgNs(type, x, size);
            free(x);
        }

        elapsedNs = sumNs / count;
        elapsedMs = nsToMs(elapsedNs);

        char variantType[64];
        snprintf(variantType, sizeof(variantType), "partially sorted %d%%", factor);

        writeResultRow(size, count, algorithmName, variantType, elapsedMs);
    }

    printf("\n");
}

/* Runs analyzeSortAlg for the selected algorithms at the given array size and run count. */
void analyzeAlgo(unsigned long size, int count) {
    // analyzeSortAlg(BUBBLE_SORT, size, count);
    // analyzeSortAlg(QUICK_SORT, size, count);
    // analyzeSortAlg(INSERTION_SORT, size, count);
    // analyzeSortAlg(SELECTION_SORT, size, count);
    analyzeSortAlg(MERGE_SORT, size, count);
    // analyzeSortAlg(RADIX_SORT, size, count);
}

int main() {
    resetRandomSeed();

    resultsFile = fopen("results.csv", "a");
    if (resultsFile == NULL) {
        printf("Could not open results.csv\n");
        return 1;
    }

    printHeader();

    analyzeAlgo(10, 1000000);
     analyzeAlgo(50, 1000000);
     analyzeAlgo(100, 1000000);
    // analyzeAlgo(1000, 10000);
    // analyzeAlgo(10000, 100);
    // analyzeAlgo(100000, 10);
    // analyzeAlgo(1000000, 1);

    fclose(resultsFile);
    return 0;
} 