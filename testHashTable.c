#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "unitTest.h"
#include "hashTable.h"

#define TEST_ALL -1
#define NORMAL -2 
#define SPECIAL -3

/* Prototype of all test functions. This allows the creation of an array of
 * function pointers which makes the testing code shorter and more clear. It
 * also makes it easier/faster to add new tests - NICE!
 */
typedef void (*TEST_FUNC)();

typedef struct strPtr{
   char * data;
} dat;

static unsigned hashDat(const void *data)
{
   unsigned hash;
   dat * d = (dat*) data;
   const char *str = d->data;

   for (hash = 0; *str;  str++)
      hash = *str + 31 * hash;

   /* Return the raw hash value - the modulo must be done by caller */
   return hash;
}

static int compareDat(const void *data, const void *data2){
   dat *d1 = (dat*) data;
   dat *d2 = (dat*) data2;
   return strcmp(d1->data, d2->data);
}

static void destroyDat(const void *data){
   dat *d = (dat*) data;
   free(d->data);
}
/* Modified hash algorithm from K&R (page 144). This version does NOT mod the
 * calculated hash value by the table size because it does not know the table
 * size - the caller does though!
 *
 * Also note the following:
 *   * Not the best/fastest/most flexible hash - you'll want something better
 *     for the word frequency project.
 *   * Assumes the data is a C string (nul-terminated sequence of char values).
 *   * You should test with more than just C strings - that implies you will
 *     need to develop a different hash function for the different types.
 *     Writing one that works on a struct (like you'll be using in the word
 *     frequency project) would be a good idea!
 */
static unsigned hashString(const void *data)
{
   unsigned hash;
   const char *str = data;

   for (hash = 0; *str;  str++)
      hash = *str + 31 * hash;

   /* Return the raw hash value - the modulo must be done by caller */
   return hash;
}

static int compareString(const void *a, const void *b)
{
   return strcmp(a, b);
}

/* Helper function to make random strings for robust testing.
 */
static char* randomString()
{
   char *str; 
   int i, length = (rand() % 70) + 3; /* 3 to 72 character random strings  */

   if (NULL == (str = malloc(length + 1)))
   {
      perror("randomString()");
      exit(EXIT_FAILURE);
   }

   /* Put random but visible char values in the string */
   for (i = 0; i < length; i++)
      str[i] = (rand() % (' ' - '~' + 1)) + ' ';

   str[length] = 0; /* nul-terminate it! */

   return str; 
}

static dat* randomDat(){
   char * str = randomString();
   dat *d = calloc(1,sizeof(dat));
   d->data = calloc(1, sizeof(char*) * strlen(str));
   strcpy(d->data,str);
   free(str);
   return d;
}
/* Good practice suggests:
 *    1. Choosing descriptive test function names.
 *    2. Including a brief description of each test.
 * 
 * Test creating an empty hash table.
 */
static void testCreateEmpty()
{
   /* Set up the test */
   void *ht;

   /* Note that the arra is on the stack - htCreate must make a deep copy! */
   unsigned sizes[] = {7};
   
   /* Note that struct is on the stack - htCreate must make a deep copy! */
   HTFunctions functions = {hashString, compareString, NULL};

   /* Call the function being tested */
   ht = htCreate(&functions, sizes, 1, 0.73);

   /* Do the test(s) */
   TEST_UNSIGNED(htCapacity(ht), 7);
   TEST_UNSIGNED(htUniqueEntries(ht), 0);
   TEST_UNSIGNED(htTotalEntries(ht), 0);

   /* Clean up, destroyData is false because there is not data */
   htDestroy(ht, 0); /* Note: destroyData true should work here too! */
}

/* Test adding a single entry to a hash table
 */
static void testAddSingleEntry()
{
   /* Set up the test */
   void *ht;
   
   /* Note that the arra is on the stack - htCreate must make a deep copy! */
   unsigned sizes[] = {11, 23};

   /* Note that struct is on the stack - htCreate must make a deep copy! */
   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 1, 0.73);

   /* Call the function being tested */
   htAdd(ht, randomString());

   /* Do the test(s) */
   TEST_UNSIGNED(htCapacity(ht), 11);
   TEST_UNSIGNED(htUniqueEntries(ht), 1);
   TEST_UNSIGNED(htTotalEntries(ht), 1);

   /* Clean up */
   htDestroy(ht, 1); /* destroyData true: There is data and it's dynamic */ 
}

/* Test that htCreate asserts when zero sizes specified 
 */
static void testErrorCreateZeroSizes()
{
   /* Set up the test */
   unsigned sizes[] = {7};
   HTFunctions functions = {hashString, compareString, NULL};

   /* Call the function being tested */
   TEST_ERROR(htCreate(&functions, sizes, 0, 0.73));

   /* No clean up necessary - expect assert/program abort by htCreate */
}

static void testAll(TEST_FUNC* testFuncs)
{
   int i;

   for (i = 0; testFuncs[i] != NULL; i++)
   {
      printf("Running normal test function %d\n", i);

      /* Call the test function via function pointer in the array */
      testFuncs[i]();
   }
}

static void runTests(TEST_FUNC *testFuncs, int testType, int testNumber)
{
   if (testNumber == TEST_ALL)
      testAll(testFuncs);
   else
   {
      if (testType == NORMAL)
         printf("Running normal test function %d\n", testNumber);
      else
         printf("Running special test function %d\n", testNumber);
      
      /* Call the test function via function pointer in the array */
      testFuncs[testNumber]();
   }
}

static int numberOfTests(TEST_FUNC* testFuncs)
{
   int count = 0;

   while(testFuncs[count] != NULL)
      count++;

   return count;
}

static int checkArgs(int argc, char *argv[], TEST_FUNC *normalFuncs,
   TEST_FUNC *specialFuncs, int *testType)
{
   int testNumber;

   if (argc == 1)
   {
      *testType = NORMAL;      
      testNumber =  TEST_ALL;
   }
   else if (argc == 2)
   {
      if (1 != sscanf(argv[1], "%d", &testNumber))
      {
         fprintf(stderr, "Invalid test number, must be an integer\n");
         exit(EXIT_FAILURE);
      }

      if (testNumber >= numberOfTests(normalFuncs) || testNumber < 0)
      {
         fprintf(stderr, "Invalid test number (%d)\n", testNumber);
         exit(EXIT_FAILURE);
      }
      
      *testType = NORMAL;
   }
   else if (argc == 3)
   {
      if (0 != strcmp(argv[1], "-special"))
      {
         fprintf(stderr, "Invalid option '%s'\n", argv[1]);
         exit(EXIT_FAILURE);
      }
      
      if (1 != sscanf(argv[2], "%d", &testNumber))
      {
         fprintf(stderr, "Invalid test number, must be an integer\n");
         exit(EXIT_FAILURE);
      }

      if (testNumber >= numberOfTests(specialFuncs) || testNumber < 0)
      {
         fprintf(stderr, "Invalid test number (%d)\n", testNumber);
         exit(EXIT_FAILURE);
      }

      *testType = SPECIAL;
   }
   else
   {
      fprintf(stderr, "Usage: %s [N|-special N]\n", argv[0]);
      exit(EXIT_FAILURE);
   }

   return testNumber;
}

/* Test driver for Hash Table Project.
 *
 * Usage: testHashTable [N|-special N]
 * Where: N is the zero-based index of the test to run.
 *
 * When no option is specified ALL of the tests in normalFuncs are run.
 * When N is specified the specified test in normalFuncs is run.
 * When "-special N" is specified the specified test from specialFuncs is run.
 *
 * TODO:
 *    1) Write a test function.
 *    2) Add its name to the appropriate TEST_FUNC array below.
 *    3) Compile and test your test by:
 *          1. Does it report failure when run on a function known to be
 *             incorrect?
 *          2. Does it pass when run on a function believed to be correct?
 *    4) Be sure use Valgrind, where appropriate, to check for memory errors
 *       and/or leaks!
 */

static void testZeroLoadFactor(){
   /* Set up the test */
   unsigned sizes[] = {10, 20};
   HTFunctions functions = {hashString, compareString, NULL};

   /* Call the function being tested */
   TEST_ERROR(htCreate(&functions, sizes, 2, 0.00));

   /* No clean up necessary - expect assert/program abort by htCreate */
}

static void testFindEntry(){
   /* Set up the test */
   void *ht;
   char *string;
   HTEntry resultOne;

   /* Note that the arra is on the stack - htCreate must make a deep copy! */
   unsigned sizes[] = {11, 23};

   /* Note that struct is on the stack - htCreate must make a deep copy! */
   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 2, 0.73);

   /* Call the function being tested */
   string = randomString();
   htAdd(ht, string);

   resultOne = htLookUp(ht, string);

   /* Do the test(s) */
   TEST_STRING(resultOne.data, string);
   TEST_UNSIGNED(resultOne.frequency, 1);

   /* Clean up */
   htDestroy(ht, 1); /* destroyData true: There is data and it's dynamic */ 
}

static void testRetrieveEntryFreq(){
   /* Set up the test */
   void *ht;
   char *string;
   HTEntry result;
   int i = 0;

   /* Note that the arra is on the stack - htCreate must make a deep copy! */
   unsigned sizes[] = {11, 23};

   /* Note that struct is on the stack - htCreate must make a deep copy! */
   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 2, 0.73);

   /* Call the function being tested */
   string = randomString();
   for(i = 0; i < 8; i++)
      htAdd(ht, string);

   result = htLookUp(ht, string);

   /* Do the test(s) */
   TEST_UNSIGNED(result.frequency, 8);

   /* Clean up */
   htDestroy(ht, 1); /* destroyData true: There is data and it's dynamic */ 
}

static void testAddRepeatedEntry()
{
   /* Set up the test */
   void *ht;
   char *string;
   /* Note that the arra is on the stack - htCreate must make a deep copy! */
   unsigned sizes[] = {11, 23};

   /* Note that struct is on the stack - htCreate must make a deep copy! */
   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 2, 0.73);

   /* Call the function being tested */
   string = randomString();
   htAdd(ht, string);
   htAdd(ht, string);
   htAdd(ht, string);

   /* Do the test(s) */
   TEST_UNSIGNED(htCapacity(ht), 11);
   TEST_UNSIGNED(htUniqueEntries(ht), 1);
   TEST_UNSIGNED(htTotalEntries(ht), 3);

   /* Clean up */
   htDestroy(ht, 1); /* destroyData true: There is data and it's dynamic */ 
}

static void testToArrayEmpty()
{
   /* Set up the test */
   void *ht;
   HTEntry *array;
   unsigned size;
   /* Note that the arra is on the stack - htCreate must make a deep copy! */
   unsigned sizes[] = {11, 23};

   /* Note that struct is on the stack - htCreate must make a deep copy! */
   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 2, 0.51);

   array = htToArray(ht, &size);
   /* Do the test(s) */
   TEST_UNSIGNED(size, 0);
   TEST_BOOLEAN((array == NULL), 1); /* If it gets this far */
   /* Clean up */
   htDestroy(ht, 1); /* destroyData true: There is data and it's dynamic */ 
   free(array);
}

static void testToArrayFull()
{
   /* Set up the test */
   void *ht;
   HTEntry *array;
   unsigned size, i;
   /* Note that the arra is on the stack - htCreate must make a deep copy! */
   unsigned sizes[] = {101};

   /* Note that struct is on the stack - htCreate must make a deep copy! */
   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 1, 0.51);
   
   for(i = 0; i < 15; i++)
      htAdd(ht, randomString());
   
   array = htToArray(ht, &size);
   /* Do the test(s) */
   TEST_UNSIGNED(size, 15);
   TEST_BOOLEAN((array == NULL), 0); /* If it gets this far */
   for(i = 0; i < size; i++)
      printf("array[%u]: %s, %u\n", i,
         (char *)(array[i].data), array[i].frequency);
   /* Clean up */
   htDestroy(ht, 1); /* destroyData true: There is data and it's dynamic */ 
   free(array);
}

static void testRehash(){
   void *ht;
   unsigned i, sizes[] = {10, 30, 999};

   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 3, 0.49);

   for(i = 0; i < 5; i++)
      htAdd(ht, randomString());

   TEST_UNSIGNED(htCapacity(ht), 10);
   TEST_UNSIGNED(htTotalEntries(ht), 5);

   htAdd(ht, randomString());

   TEST_UNSIGNED(htCapacity(ht), 30);
   TEST_UNSIGNED(htTotalEntries(ht), 6);

   for(i = 0; i < 9; i++)
      htAdd(ht, randomString());
   
   TEST_UNSIGNED(htCapacity(ht), 30);
   TEST_UNSIGNED(htTotalEntries(ht), 15);

   htAdd(ht, randomString());

   TEST_UNSIGNED(htCapacity(ht), 999);
   TEST_UNSIGNED(htTotalEntries(ht), 16);

   htDestroy(ht, 1);
}

static void testLarge(){
   void *ht;
   HTMetrics metrics;
   unsigned i, sizes[] = {10000};

   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 1, 0.99);

   for(i = 0; i < 500000; i++)
      htAdd(ht, randomString());
   
   metrics = htMetrics(ht);
   TEST_UNSIGNED(htCapacity(ht), 10000);
   TEST_UNSIGNED(htTotalEntries(ht), 500000);
   TEST_UNSIGNED(metrics.numberOfChains, 10000);
   TEST_UNSIGNED(metrics.maxChainLength, 79);
   TEST_REAL(metrics.avgChainLength,49.997501, .000001);

   htDestroy(ht, 1); 
}

void test11(){
   void *ht;
   HTFunctions funcs = {hashString, compareString, NULL};
   unsigned sizes[] = {9};
   ht = htCreate(&funcs, sizes, 1, .90);
   TEST_ERROR(htAdd(ht, NULL));
}
void test14(){
   void *ht;
   unsigned i, sizes[] = {10000};

   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 1, 0.99);

   for(i = 0; i < 5000; i++)
      htAdd(ht, randomString());
   TEST_ERROR(htLookUp(ht, NULL));
}
void test17(){
   void *ht;
   unsigned i, sizes[] = {10,100};
   HTMetrics metrics;
   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 2, 1);

   for(i = 0; i < 5000; i++)
      htAdd(ht, randomString());
   metrics = htMetrics(ht);

   TEST_UNSIGNED(htCapacity(ht), 10);
   TEST_UNSIGNED(htTotalEntries(ht), 5000);
   TEST_UNSIGNED(metrics.numberOfChains, 10);
   TEST_UNSIGNED(metrics.maxChainLength, 536);
   TEST_REAL(metrics.avgChainLength, 500.000000, .0000001)
   htDestroy(ht,1);
}

void test27(){
   void *ht;
   HTMetrics metrics;
   unsigned i, sizes[] = {3, 7};
   dat *d = randomDat();
#ifdef parr
   unsigned size;
   HTEntry * array;
#endif
   HTFunctions functions = {hashDat, compareDat, destroyDat};

   ht = htCreate(&functions, sizes, 2, .73);

   for(i = 0; i < 5000; i++)
      htAdd(ht, randomDat());
   for(i = 0; i < 10; i++){
      htAdd(ht, d);
   }
   
   metrics = htMetrics(ht);
   TEST_UNSIGNED(htCapacity(ht), 7);
   TEST_UNSIGNED(htTotalEntries(ht), 5010);
   TEST_UNSIGNED(metrics.numberOfChains, 7);
   TEST_UNSIGNED(metrics.maxChainLength, 757);
   TEST_REAL(metrics.avgChainLength, 714.428589, .000001);
#ifdef parr
   array = htToArray(ht, &size);
   for(i = 0; i < size; i++){
      printf("array[%u]: %s, %u\n", i,
         ((dat *)(array[i].data))->data, array[i].frequency);
      
   }
#endif
   htDestroy(ht, 1); 
}
static void test16(){
   void *ht;
   HTMetrics metrics;
   HTEntry lookup;
   char * str;
   char ** arr;
   unsigned i,j, sizes[] = {3,5,7,11,13,17,19,23,37,100, 200};

   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 11, 0.721);
   metrics = htMetrics(ht);
   TEST_UNSIGNED(metrics.numberOfChains, 0);
   TEST_UNSIGNED(metrics.maxChainLength, 0);
   TEST_REAL(metrics.avgChainLength, 0.000000, 0.00001);
   TEST_UNSIGNED(htCapacity(ht), 3);
   TEST_UNSIGNED(htUniqueEntries(ht), 0);
   TEST_UNSIGNED(htTotalEntries(ht), 0);
   for(j = 0; j < 10; j++){
      arr = malloc(sizeof(char *) * 100);
      for(i = 0; i < 100; i++){
         str = randomString();
         arr[i] = malloc(sizeof(char) * (strlen(str)+1));
         strcpy(arr[i],str);
         free(str);
      }

      for(i = 0; i < 100; i++)
         htAdd(ht, arr[i]);

      for(i = 0; i < 100; i++){
         lookup = htLookUp(ht, arr[i]);
         TEST_BOOLEAN((lookup.data != NULL), 1);
#ifdef parr
         printf("LookUp Results for entry %d: \ndata: %s, \nfrequency: %d\n", 100*j+i,
               (char*)lookup.data, lookup.frequency);
#endif
      }
      free(arr);
   }
   metrics = htMetrics(ht);
   TEST_UNSIGNED(htCapacity(ht), 200);
   TEST_UNSIGNED(htUniqueEntries(ht), 1000);
   TEST_UNSIGNED(htTotalEntries(ht), 1000);
   TEST_UNSIGNED(metrics.numberOfChains, 199);
   TEST_UNSIGNED(metrics.maxChainLength, 12);
   TEST_REAL(metrics.avgChainLength, 5.025126, 0.000001);
   htDestroy(ht, 1); 
}

void testRehashCycles(){
   void *ht;
   unsigned i, sizes[] = {10, 30, 999};
   char * str;
   HTMetrics metrics;
   HTFunctions functions = {hashString, compareString, NULL};

   ht = htCreate(&functions, sizes, 3, 0.49);
   
   str = randomString();
   for(i = 0; i < 5; i++)
      htAdd(ht, randomString());

   TEST_UNSIGNED(htCapacity(ht), 10);
   TEST_UNSIGNED(htTotalEntries(ht), 5);
   TEST_UNSIGNED(htUniqueEntries(ht), 5);
   htAdd(ht, randomString());



   TEST_UNSIGNED(htCapacity(ht), 30);
   TEST_UNSIGNED(htTotalEntries(ht), 6);
   TEST_UNSIGNED(htUniqueEntries(ht), 6);
   for(i = 0; i < 9; i++)
      htAdd(ht, str);
   
   TEST_UNSIGNED(htCapacity(ht), 30);
   TEST_UNSIGNED(htTotalEntries(ht), 15);
   TEST_UNSIGNED(htUniqueEntries(ht), 7);

   htAdd(ht, randomString());

   TEST_UNSIGNED(htCapacity(ht), 30);
   TEST_UNSIGNED(htTotalEntries(ht), 16);
   TEST_UNSIGNED(htUniqueEntries(ht), 8);
   metrics = htMetrics(ht);
   TEST_UNSIGNED(metrics.numberOfChains, 7);
   TEST_UNSIGNED(metrics.maxChainLength, 2);
   TEST_REAL(metrics.avgChainLength, 1.142857, 0.000001);
   htDestroy(ht, 1);
}

int main(int argc, char *argv[])
{
   int testNumber, testType;

   /* The array of "normal" test functions, ALL of these run by default */
   TEST_FUNC normalFuncs[] = {
      testCreateEmpty,
      testAddSingleEntry,
      testAddRepeatedEntry,
      testRetrieveEntryFreq,
      testFindEntry,
      testToArrayEmpty,
      testToArrayFull,
      testRehash,
      testLarge,
      test17,
      test27,
      test16,
      testRehashCycles,
      NULL
   };

   /* The array of "special" test functions - MUST be run one at a time */
   TEST_FUNC specialFuncs[] = {
      testErrorCreateZeroSizes,
      testZeroLoadFactor,
      test11,
      test14,
      NULL
   };

   /* Random numbers used to produce "interesting" strings for testing */
   srand(182955);

   /* Make stdout unbuffered so that test output is synchronous on signals */
   setbuf(stdout, NULL);

   /* Get the test number and test type */
   testNumber = checkArgs(argc, argv, normalFuncs, specialFuncs, &testType);
 
   /* Run the test(s)... */
   if (testType == NORMAL)
      runTests(normalFuncs, NORMAL, testNumber);
   else
      runTests(specialFuncs, SPECIAL, testNumber);
   
   return 0;
}
