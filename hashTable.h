/* This header file is provided with the hash table project and the associated
 * unit testing exercise for the same project.
 *
 *    ! ! !  DO NOT MODIFY THIS FILE ! ! !
 *
 * An unmodified version of this file will be used during evaluation even if
 * you submit a different one. If you need additional header file features you
 * must create your own separate header file and include it with your
 * submission.
 */
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>

/* The array type returned by htToArray.
 */
typedef struct
{
   void *data;
   unsigned frequency;
} HTEntry;

/* The hash table metric structure returned by htMetrics.
 */
typedef struct
{
   unsigned numberOfChains;
   unsigned maxChainLength;
   float avgChainLength;
} HTMetrics;

/* Function types for the data-specific functions required by the hash table.
 *
 *    FNHash: Calculates and returns the hash value (not index!) for the
 *       specified data.
 *
 *    FNCompare: Compares two data items to determine data1 is less-than,
 *       equal-to, or greater than data2. The function returns a negative,
 *       zero, or positive value, respectively.
 *
 *    FNDestroy: May be NULL. This function pointer should be provided (not
 *       NULL) when the data being stored in the hash table, itself, has
 *       additional dynamically allocated memory that only the owner of the
 *       data knows about and can free, otherwise it should be NULL. When
 *       provided, this function frees all such sub-allocations BUT SHOULD NOT
 *       free the data itself - that is done by htDestroy (see the description
 *       for the htDestroy function for details).
 */
typedef unsigned (*FNHash)(const void *data);
typedef int (*FNCompare)(const void *data1, const void *data2);
typedef void (*FNDestroy)(const void *data);

/* Structure of function pointers provided to and used by the hash table.
 */
typedef struct
{
   FNHash hash;
   FNCompare compare;
   FNDestroy destroy;
} HTFunctions;

/* Description: Creates a new hash table as specified.
 *
 * Notes:
 *   1. The function asserts (man 3 assert) if sizes is not 1 or more.
 *   2. The function asserts (man 3 assert) if any of the sizes are not
 *      greater than the immediately preceding size.
 *   3. The function asserts (man 3 assert) If the load factor is not greater
 *      than 0.0 and less than or equal to 1.0.
 *   4. You must make a deep copy of the _functions_ pointers and _sizes_
 *      array since you don't know where the caller allocated them - they
 *      could be on the stack and you don't want to point there - that's a
 *      potential and likely _unlucky bug_ waiting to happen!
 *
 * Parameters:
 *    functions: A structure of function pointers to the data-specific
 *       functions required by the hash table.
 *    sizes: An array of hash table sizes. Must always be at least one size
 *       greater than zero specified, more if rehashing is desired. Prime
 *       numbers are best but not required/checked for. That being said, you
 *       should use prime numbers!
 *    numSizes: The number of values in the sizes array.
 *    rehashLoadFactor: The load factor to rehash at. Should be a value greater
 *       than 0.0 and less than or equal to 1.0. A value of 1.0 means
 *       "do not rehash".
 *
 * Return: A pointer to an anonymous (file-local) structure representing a
 *         hash table.
 */
void* htCreate(
   HTFunctions *functions,
   unsigned sizes[],
   int numSizes,
   float rehashLoadFactor 
); 

/* Description: Frees all of the dynamically allocated memory allocated by the
 *    hash table itself. If the provided flag (destroyData) is true then all
 *    of the data stored in the hash table is freed too.
 *
 * Notes:
 *   1. If the destroyData flag is true AND the hash table's destroy function
 *      pointer is not NULL then the destroy function is called BEFORE freeing
 *      the data. Read about the destroy function earlier in this header file.
 *   2. The arrays returned by htToArray are NOT freed by this function.
 *   3. The arrays returned by htToArray contain shallow copies of the data
 *      in the hash table so do not destroy the data until you are through with
 *      all such arrays.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *    destroyData: A boolean flag indicating whether or not the data stored in
 *       the hash table should be freed too.
 *
 * Return: None
 */
void htDestroy(void *hashTable, int destroyData);

/* Description: Adds a shallow copy of the data to the hash table.
 * 
 * Notes:
 *    1. The function asserts (man 3 assert) if data is NULL. 
 *    2. The function rehashes to the next size when:
 *          1. The rehash load factor is NOT 1.0, a value of indicates
 *             rehashing is not desired.
 *          2. And there is a next size, otherwise continue with the current
 *             size.
 *          3. And the ratio of unique entries TO the current hash table size
 *             (BEFORE adding the new data) exceeds the rehash load factor.
 *    3. The caller is responsible for freeing the data when it is a duplicate.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *    data: The data to look add.
 *
 * Return: The frequency of the data in the hash table. A value of 1 means it
 *    is a new entry, values greater than 1 mean it is a duplicate with the
 *    indicated frequency.
 */
unsigned htAdd(void *hashTable, void *data);

/* Description: Determines if the data is in the hash table or not.
 * 
 * Notes:
 *    1. The function asserts (man 3 assert) if data is NULL. 
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *    data: The data to look for.
 *
 * Return: Returns an HTEntry with a shallow copy of the data and its
 *    frequency if found, otherwise NULL data and frequency 0.
 */
HTEntry htLookUp(void *hashTable, void *data);

/* Description: Returns a dynamically allocated array with shallow copies of
 * all of the hash table entries.
 *
 * Notes:
 *    1. The caller is responsible for freeing the memory for the returned
 *       array BUT MUST NOT free the data in each HTEntry in the array - that
 *       memory is still in use by the hash table! To free the data call
 *       htDestroy when you are completely done with the hash table AND all
 *       arrays returned by this function.
 * 
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *    size: Output parameter updated with the array's size.
 *
 * Return: A dynamically allocated array with all of the hash table entries or
 *    NULL if the hash table is empty (note that free can be called on NULL).
 */
HTEntry* htToArray(void *hashTable, unsigned *size);

/* Description: Reports the current capacity of the hash table.
 * 
 * Notes:
 *    1. This function should always return one of the sizes passed to htCreate.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *
 * Return: The current capacity of the hash table.
 */
unsigned htCapacity(void *hashTable);

/* Description: Returns the number of unique entries in the hash table as a 
 *    convenience and to avoid the O(N) pass through all entries that would
 *    otherwise be necessary to calculate the value.
 * 
 * Notes: 
 *    1. The description implies the value should be stored and maintained by
 *       the hash table as data is added - NOT calculated when this function
 *       is called.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *
 * Return: The number of unique entries in the hash table.
 */
unsigned htUniqueEntries(void *hashTable);

/* Description: Returns the sum of the frequencies of all entries in the hash
 *    table as a convenience and to avoid the O(N) pass through all entries
 *    that would otherwise be necessary to calculate the value.
 * 
 * Notes: 
 *    1. The description implies the value should be stored and maintained by
 *       the hash table as data is added - NOT calculated when this function
 *       is called.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *
 * Return: The sum of the frequencies of all entries in the hash table.
 */
unsigned htTotalEntries(void *hashTable);

/* Description: Returns various metrics on the hash table to aid in performance
 *    tuning of CPU and memory usage for a particular problem domain. 
 * 
 * Notes:
 *    1. The number of chains is a count of the non-empty chains in the
 *       hash table.
 *    2. The maximum chain length is the length of the longest chain in the
 *       hash table.
 *    3. The average chain length is the sum of the length of all chains
 *       in the hash table divided by the number of non-zero length chains.
 *
 * Parameters:
 *    hashTable: A pointer returned by htCreate.
 *
 * Return: An HTMetric struct with the current metrics for the specified hash
 *    table.
 */
HTMetrics htMetrics(void *hashTable);

#endif
