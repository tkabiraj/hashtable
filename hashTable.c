#include <limits.h>
#include <stdlib.h>
#include "hashTable.h"
#include <assert.h>

typedef struct ListNode {
   HTEntry* entry;
   struct ListNode* next;
}ListNode;

typedef struct HashTable {
   HTFunctions* functions;
   unsigned* sizes;
   int numSizes;
   int index;
   int numItem;
   int total;
   float lf;
   ListNode** ht;
}HashTable;
unsigned addTail(HashTable* hti, int hi, void* value);

void hrehash(HashTable* hti, ListNode* curr, ListNode** nht) {
   int index;
   ListNode* temp;
   while(curr) {
      index = hti->functions->hash(curr->entry->data)%hti->sizes[hti->index];
      temp = curr->next;
      if(nht)
         curr->next = nht[index];
      else
         curr->next = NULL;
      nht[index] = curr;
      curr = temp;
   }
}

void failMalloc(){
   fprintf(stderr, "malloc failure");
   exit(EXIT_FAILURE);
}

void rehash(HashTable* hti) {
   int i;
   ListNode** nht;
   ListNode** oht;
   ListNode* curr;
   hti->index += 1;
   nht = calloc(hti->sizes[hti->index], sizeof(ListNode*));
   if (nht == NULL)
      failMalloc();
   for(i = 0; i < hti->sizes[hti->index-1]; i++) {
      curr = hti->ht[i];
      hrehash(hti, curr, nht);
   }
   oht = hti->ht;
   hti->ht = nht;
   free(oht);
}

void checkRehash(HashTable* hti) {
   float ratio = (float)hti->numItem/hti->sizes[hti->index];
   if(hti->lf != 1 && hti->index < hti->numSizes-1 && ratio > hti->lf) 
      rehash(hti);
}

void setSizeArray(HashTable* hti, unsigned sizes[], int numSizes) {
   int i, prev = 0;
   for(i  = 0; i < numSizes; i++) {
      assert(sizes[i] > 1);
      assert(sizes[i] > prev);
      prev = sizes[i];
      hti->sizes[i] = sizes[i];
   }
}

void setFunctions(HashTable* hti, HTFunctions* functions) {
   hti->functions->hash = functions->hash;
   hti->functions->compare = functions->compare;
   hti->functions->destroy = functions->destroy;
}

void* htCreate(HTFunctions *functions, unsigned sizes[],
   int numSizes, float rehashLoadFactor) {
   HashTable* hti;
   assert(numSizes >0); 
   assert(rehashLoadFactor> 0 && rehashLoadFactor <= 1);
   hti = (void*)malloc(sizeof(HashTable));
   if (hti == NULL)
      failMalloc();
   hti->functions = malloc(sizeof(HTFunctions));
   if (hti->functions == NULL)
      failMalloc();
   setFunctions(hti, functions);
   hti->sizes = malloc(sizeof(unsigned) * numSizes);
   if (hti->sizes == NULL)
      failMalloc();
   setSizeArray(hti, sizes, numSizes);
   hti->index = hti->numItem = hti->total = 0;
   hti->numSizes = numSizes;
   hti->lf = rehashLoadFactor;
   hti->ht = (ListNode**)calloc(sizes[0], sizeof(ListNode*));
   if (hti->ht == NULL)
      failMalloc();
   return hti;
}

void destroy(HashTable* hti, ListNode* curr, int destroyData) {
   ListNode* next;
   while (curr) {
      next = curr->next;
      if(destroyData) {
         if(hti->functions->destroy) 
            hti->functions->destroy(curr->entry->data);
         free(curr->entry->data);
         free(curr->entry);
      }
      free(curr);
      curr = next;
   }
}

void htDestroy(void *hashTable, int destroyData)
{
   HashTable* hti = hashTable;
   ListNode* curr;

   int i;
   for(i = 0; i < hti->sizes[hti->index]; i++) {
      curr = hti->ht[i];
      destroy(hti, curr, destroyData);
   }
   free(hti->functions);
   free(hti->sizes);
   free(hti->ht);
   free(hti);
}

unsigned htAdd(void *hashTable, void *data)
{
   HashTable* hti = hashTable;
   int hi, fr;
   assert(data != NULL);
   checkRehash(hti);
   hi = hti ->functions->hash(data)%hti->sizes[hti->index];
   fr = addTail(hti, hi, data);
   return fr;
}

HTEntry* addEntry(void* data) {
   HTEntry* entry = malloc(sizeof(HTEntry));
   if (entry == NULL)
      failMalloc();
   entry->data = data;
   entry->frequency = 1;
   return entry;
}

unsigned addTail(HashTable *hti, int hi, void* data) {
   ListNode* curr = hti->ht[hi];
   hti->numItem += 1;
   hti->total += 1;
   while(curr){
      if(!hti->functions->compare(curr->entry->data,data)){
         hti->numItem -=1;
         curr->entry->frequency+=1;
         return curr->entry->frequency;
      }
      curr = curr->next;
   }
   curr = calloc(1,sizeof(ListNode));
   if (curr == NULL)
      failMalloc();
   curr->entry = addEntry(data);
   curr->next = hti->ht[hi];
   hti->ht[hi] = curr;
   return 1;
}
HTEntry htLookUp(void *hashTable, void *data)
{
   HashTable* hti = hashTable;
   ListNode* curr;
   int i;
   HTEntry entry;
   assert(data != NULL);
   i = hti->functions->hash(data)%hti->sizes[hti->index];
   curr = hti->ht[i];
   while (curr) {
      if(!hti->functions->compare(curr->entry->data,data))
         return *curr->entry;
      curr = curr->next;
   }
   entry.data = NULL;
   entry.frequency = 0;
   return entry;
}

void toArray(HTEntry* arr, ListNode* curr, int* s) {
   while (curr) {
      arr[*s] = *curr->entry;
      curr = curr->next;
      *s = *s + 1;
   }
}

HTEntry* htToArray(void *hashTable, unsigned *size)
{
   HashTable* hti = hashTable;
   int i, s = 0;
   ListNode* curr;
   HTEntry* arr = calloc(hti->numItem, sizeof(HTEntry));
   if (arr == NULL)
      failMalloc();
   for(i = 0; i < hti->sizes[hti->index]; i++) {
      curr = hti->ht[i];
      toArray(arr, curr, &s);
   }
   *size = s;
   if(s == 0){
      free(arr);
      return NULL;
   }
   return arr;
}

unsigned htCapacity(void *hashTable)
{
   return ((HashTable*)hashTable)->sizes[((HashTable*)hashTable)->index];
}

unsigned htUniqueEntries(void *hashTable)
{
   return ((HashTable*)hashTable)->numItem;
}

unsigned htTotalEntries(void *hashTable)
{
   return ((HashTable*)hashTable)->total;
}

void updateChain(ListNode* curr, int* chain, int* count, int* total) {
   *count = *chain = 0;
   while(curr) {
      *chain = 1;
      *count += 1;
      *total += 1;
      curr = curr->next;
   }
}

HTMetrics htMetrics(void *hashTable)
{
   HTMetrics metrics;
   HashTable* hti = hashTable;
   int i, num = 0, max = 0, total = 0, count, chain;
   ListNode* curr;
   for(i = 0; i < hti->sizes[hti->index]; i++) {
      curr = hti->ht[i];
      updateChain(curr, &chain, &count, &total);
      if(chain)
         num++;
      if(count > max)
         max = count;
   }
   metrics.numberOfChains = num;
   metrics.maxChainLength = max;
   metrics.avgChainLength = (float)total/num;

   return metrics;
}
