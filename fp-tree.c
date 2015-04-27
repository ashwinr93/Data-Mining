#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils.h"

#define MAX_ITEMSET_SIZE 7
#define MIN_SUPPORT 0.1
#define MIN_CONF 0.4
#define ATTRIBUTES 24
#define TXNS 1728
#define INPUT_DATA "inputFP.txt"
#define OUTPUT_FILE "fp.out"

typedef struct FPnode *FPTreeNode;

typedef struct Childnode *childLink;

typedef struct Childnode {
	FPTreeNode node;
	childLink next;
} ChildNode;

typedef struct FPnode {
int item;
int count;
int numPath;
FPTreeNode parent;
childLink children;
FPTreeNode hlink;
} FPNode;

typedef struct Itemsetnode *LargeItemPtr;
typedef struct Itemsetnode {
	int support;
	int *itemset;
	LargeItemPtr next;
} ItemsetNode;

struct _ItemSet
{
  char *item_set;
  int count;
};

typedef struct _ItemSet ItemSet;

struct _ItemSetNode
{
  ItemSet *data;
  struct _ItemSetNode *next;
};

typedef struct _ItemSetNode ItemSetNode;

void FPgrowth(FPTreeNode T, FPTreeNode *headerTableLink, int headerSize, int *baseItems, int baseSize);

LargeItemPtr *largeItemset;
int *numLarge;
int *support1;
int *largeItem1;

int threshold;
int realK;

FPTreeNode root=NULL;
FPTreeNode *headerTableLink;

void append_to_list(ItemSetNode **list, ItemSet *item_set)
{
    ItemSetNode *tmp = malloc(sizeof(ItemSetNode));
    tmp->data = item_set;
    tmp->next = *list;

    *list = tmp;
}

ItemSet *new_item_set(char *item_set, int count)
{
  ItemSet *item = malloc(sizeof(ItemSet));

  item->item_set = item_set;
  item->count = count;

  return item;
}

void destroyTree(FPTreeNode node)
{
 childLink temp1, temp2;

 if (node == NULL) return;

 temp1 = node->children;
 while(temp1 != NULL) {
	temp2 = temp1->next;
	destroyTree(temp1->node);
	free(temp1);
	temp1 = temp2;
 }

 free(node);

 return;
}

void destroy()
{
 LargeItemPtr aLargeItemset;
 int i;

 for (i=0; i < realK; i++) {
	aLargeItemset = largeItemset[i];
	while (aLargeItemset != NULL) {
		largeItemset[i] = largeItemset[i]->next;
		free(aLargeItemset->itemset);
		free(aLargeItemset);
		aLargeItemset = largeItemset[i];
	}
 }
 free(largeItemset);

 free(numLarge);

 free(headerTableLink);

 destroyTree(root);

 return;
}

void swap(int *support, int *itemset, int x, int i)
{
 int temp;

 temp = support[x];
 support[x] = support[i];
 support[i] = temp;
 temp = itemset[x];
 itemset[x] = itemset[i];
 itemset[i] = temp;

 return;
}

void q_sortD(int *support, int *itemset, int low,int high, int size)
{
 int pass;
 int highptr=high++; 
 int pivot=low;

 if(low>=highptr) return;
 do {
	pass=1;
	while(pass==1) {
		if(++low<size) {
			if(support[low] > support[pivot])
				pass=1;
			else pass=0;
		} else pass=0;
	}

	pass=1;
	while(pass==1) {
		if(high-->0) {
			if(support[high] < support[pivot])
				pass=1;
			else pass=0;
		} else pass=0;
	}

	if(low<high)
		swap(support, itemset, low, high);
 } while(low<=high);

 swap(support, itemset, pivot, high);

 q_sortD(support, itemset, pivot, high-1, size);
 q_sortD(support, itemset, high+1, highptr, size);

 return;
}

void q_sortA(int *indexList, int *freqItemP, int low, int high, int size)
{
 int pass;
 int highptr=high++; 
 int pivot=low;

 if(low>=highptr) return;
 do {
	pass=1;
	while(pass==1) {
		if(++low<size) {
			if(indexList[low] < indexList[pivot])
				pass=1;
			else pass=0;
		} else pass=0;
	}

	pass=1;
	while(pass==1) {
		if(high-->0) {
			if(indexList[high] > indexList[pivot])
							pass=1;
			else pass=0;
		} else pass=0;
	}

	if(low<high)
		swap(indexList, freqItemP, low, high);
 } while(low<=high);

 swap(indexList, freqItemP, pivot, high);

 q_sortA(indexList, freqItemP, pivot, high-1, size);
 q_sortA(indexList, freqItemP, high+1, highptr, size);

 return;
}

void addToLargeList(int *pattern, int patternSupport, int index)
{
 LargeItemPtr aLargeItemset;
 LargeItemPtr aNode, previous=NULL;
 int i;

 aLargeItemset = (LargeItemPtr) malloc (sizeof(ItemsetNode));
 if (aLargeItemset == NULL) {
	printf("out of memory\n");
	exit(1);
 }
 aLargeItemset->itemset = (int *) malloc (sizeof(int) * (index+1));
 if (aLargeItemset->itemset == NULL) {
	printf("out of memory\n");
	exit(1);
 }

 aLargeItemset->support = patternSupport;

 for (i=0; i <= index; i++) {
	aLargeItemset->itemset[i] = pattern[i];
 }

 aLargeItemset->next = NULL;

 aNode = largeItemset[index];
 if (aNode == NULL) {
	largeItemset[index] = aLargeItemset;
 } else {
 	while ((aNode != NULL) && (aNode->support > patternSupport)) {
		previous = aNode;
		aNode = aNode->next;
 	}

 	if (previous != NULL) {
		previous->next = aLargeItemset;
		aLargeItemset->next = aNode;
	} else {

		aLargeItemset->next = largeItemset[index];
		largeItemset[index] = aLargeItemset;
	}
 }

 (numLarge[index])++;

 return;
}

void combine(int *itemList, int *support, int start, int itemListSize, int *base, int baseSize)
{
 int *pattern;
 int i, j;
 if (baseSize >= realK) return;
 if (start == itemListSize) return;
 pattern = (int *) malloc (sizeof(int) * (baseSize+1));
 if (pattern == NULL) {
	printf("out of memory\n");
	exit(1);
 }

 for (j=0; j < baseSize; j++)
	pattern[j] = base[j];

 for (i=start; i < itemListSize; i++) {

	pattern[baseSize] = itemList[i];
	addToLargeList(pattern , support[i], baseSize);

	combine(itemList, support, i+1, itemListSize, pattern, baseSize+1);
 }

 free(pattern);

 return;
}

void insert_tree(int *freqItemP, int *indexList, int count, int ptr, int length,
			FPTreeNode T, FPTreeNode *headerTableLink, int *path)
{
 childLink newNode;
 FPTreeNode hNode;
 FPTreeNode hPrevious;
 childLink previous;
 childLink aNode;

 if (ptr == length) return;

 if (T->children == NULL) {
	newNode = (childLink) malloc (sizeof(ChildNode));
	if (newNode == NULL) {
		printf("out of memory\n");
		exit(1);
	}

	newNode->node = (FPTreeNode) malloc (sizeof(FPNode));
	if (newNode->node == NULL) {
		printf("out of memory\n");
		exit(1);
	}

	newNode->node->item = freqItemP[ptr];
	newNode->node->count = count;
	newNode->node->numPath = 1;
	newNode->node->parent = T;
	newNode->node->children = NULL;
	newNode->node->hlink = NULL;
	newNode->next = NULL;
	T->children = newNode;

	hNode = headerTableLink[indexList[ptr]];
	if (hNode == NULL) {
		headerTableLink[indexList[ptr]] = newNode->node;
	} else {
		while (hNode != NULL) {
			hPrevious = hNode;
			hNode = hNode->hlink;
		}

		hPrevious->hlink = newNode->node;
	}

	insert_tree(freqItemP, indexList, count, ptr+1, length, T->children->node, headerTableLink, path);
	T->numPath += *path;

 } else {
	aNode = T->children;
	while ((aNode != NULL) && (aNode->node->item != freqItemP[ptr])) {
		previous = aNode;
		aNode = aNode->next;
	}

	if (aNode == NULL) {
		newNode = (childLink) malloc (sizeof(ChildNode));
		if (newNode == NULL) {
			printf("out of memory\n");
			exit(1);
		}
		newNode->node = (FPTreeNode) malloc (sizeof(FPNode));
		if (newNode->node == NULL) {
			printf("out of memory\n");
			exit(1);
		}

		newNode->node->item = freqItemP[ptr];
		newNode->node->count = count;
		newNode->node->numPath = 1;
		newNode->node->parent = T;
		newNode->node->children = NULL;
		newNode->node->hlink = NULL;
		newNode->next = NULL;
		previous->next = newNode;

		hNode = headerTableLink[indexList[ptr]];
		if (hNode == NULL) {
			headerTableLink[indexList[ptr]] = newNode->node;
		} else {
			while (hNode != NULL) {
				hPrevious = hNode;
				hNode = hNode->hlink;
			}
			hPrevious->hlink = newNode->node;
		}

		insert_tree(freqItemP, indexList, count, ptr+1, length, newNode->node, headerTableLink, path);

		(*path)++;
		T->numPath += *path;

	} else {
		aNode->node->count += count;

		insert_tree(freqItemP, indexList, count, ptr+1, length, aNode->node, headerTableLink, path);

		T->numPath += *path;
	}
 }

 return;
}

void buildConTree(FPTreeNode *conRoot, FPTreeNode **conHeader, int conHeaderSize, int *conLargeItem,
		int *conLargeItemSupport, FPTreeNode T, FPTreeNode *headerTable, int baseIndex, int headerSize)
{
 FPTreeNode aNode;
 FPTreeNode ancestorNode;
 int *freqItemP;
 int *indexList;
 int path;
 int count;
 int i;

 *conHeader = (FPTreeNode *) malloc (sizeof(FPTreeNode) * conHeaderSize);
 if (*conHeader == NULL) {
        printf("out of memory\n");
        exit(1);
 }
 for (i=0; i < conHeaderSize; i++)
        (*conHeader)[i] = NULL;

 (*conRoot) = (FPTreeNode) malloc (sizeof(FPNode));
 if (*conRoot == NULL) {
        printf("out of memory\n");
        exit(1);
 }

 (*conRoot)->numPath = 1;
 (*conRoot)->parent = NULL;
 (*conRoot)->children = NULL;
 (*conRoot)->hlink = NULL;

 freqItemP = (int *) malloc (sizeof(int) * conHeaderSize);
 if (freqItemP == NULL) {
        printf("out of memory\n");
        exit(1);
 }

 indexList = (int *) malloc (sizeof(int) * conHeaderSize);
 if (indexList == NULL) {
        printf("out of memory\n");
        exit(1);
 }

 aNode = headerTable[baseIndex];

 while (aNode != NULL) {
	ancestorNode = aNode->parent;
	count = 0;

	while (ancestorNode != T) {
		for (i=0; i < conHeaderSize; i++) {
			if (ancestorNode->item == conLargeItem[i]) {
				freqItemP[count] = ancestorNode->item;
				indexList[count] = i;
				count++;
				break;
			}
		}
		ancestorNode = ancestorNode->parent;
	}

	q_sortA(indexList, freqItemP, 0, count-1, count);

	path = 0;

	insert_tree(&(freqItemP[0]), &(indexList[0]), aNode->count, 0, count, *conRoot, *conHeader, &path);

	aNode = aNode->hlink;
 }

 free(freqItemP);
 free(indexList);

 return;
}

void genConditionalPatternTree(int *pattern, int baseSize, int patternSupport,
				int *conLargeItem, int *conLargeItemSupport, FPTreeNode T,
				int headerIndex, int headerSize, FPTreeNode *headerTableLink)
{
 int conHeaderSize;
 FPTreeNode *conHeader;
 FPTreeNode conRoot;
 FPTreeNode aNode, ancestorNode;
 int j;

 for (j=0; j < headerSize; j++)
	conLargeItemSupport[j] = 0;

 aNode = headerTableLink[headerIndex];
 conHeaderSize = 0;

 while (aNode != NULL) {
	ancestorNode = aNode->parent;

	while (ancestorNode != T) {

		for (j=0; j < headerSize; j++) {
			if (ancestorNode->item == headerTableLink[j]->item) {
				conLargeItemSupport[j] += aNode->count;

				if ((conLargeItemSupport[j] >= threshold) &&
				   (conLargeItemSupport[j] - aNode->count <
					threshold)) {

					conLargeItem[j] = ancestorNode->item;
					conHeaderSize++;
				}
				break;
			}
		}
		ancestorNode = ancestorNode->parent;
	}

	aNode = aNode->hlink;
 }

 q_sortD(conLargeItemSupport, conLargeItem, 0, headerSize-1, headerSize);


 if (conHeaderSize > 0) {

	buildConTree(&conRoot, &conHeader, conHeaderSize, conLargeItem, conLargeItemSupport,
			T, headerTableLink, headerIndex, headerSize);

	FPgrowth(conRoot, conHeader, conHeaderSize, pattern, baseSize+1);

 	free(conHeader);
 	destroyTree(conRoot);
 }

 return;
}

void FPgrowth(FPTreeNode T, FPTreeNode *headerTableLink, int headerSize, int *baseItems, int baseSize)
{
 int count;
 int i, j;
 int *pattern;
 int patternSupport;
 FPTreeNode aNode = NULL;
 int *conLargeItem;
 int *conLargeItemSupport;


 if (baseSize >= realK) return;
 if (T == NULL) return;
 conLargeItem = (int *) malloc (sizeof(int) * headerSize);
 conLargeItemSupport = (int *) malloc (sizeof(int) * headerSize);
 if ((conLargeItem == NULL) || (conLargeItemSupport == NULL)) {
	printf("out of memory\n");
	exit(1);
 }

 if (T->numPath == 1) {

	count = 0;
	if (T->children != NULL) aNode = T->children->node;

	while (aNode != NULL) {
		conLargeItem[count] = aNode->item;
		conLargeItemSupport[count] = aNode->count;
		count++;
		if (aNode->children != NULL)
			aNode = aNode->children->node;
		else	aNode = NULL;
	}

	combine(conLargeItem, conLargeItemSupport, 0, count, baseItems, baseSize);
	free(conLargeItem);
	free(conLargeItemSupport);

 } else {
	pattern = (int *) malloc (sizeof(int) * (baseSize + 1));
	if (pattern == NULL) {
		printf("out of memory\n");
		exit(1);
	}

	for (i=0; i < headerSize; i++) {

		pattern[0] = headerTableLink[i]->item;

		for (j=0; j < baseSize; j++) {
			pattern[j+1] = baseItems[j];
		}

		aNode = headerTableLink[i];
		patternSupport = 0;

		while (aNode != NULL) {
			patternSupport += aNode->count;
			aNode = aNode->hlink;
		}

		addToLargeList(pattern, patternSupport, baseSize);

		genConditionalPatternTree(pattern, baseSize, patternSupport,
				conLargeItem, conLargeItemSupport, T,
				i, headerSize, headerTableLink);
	}

	free(pattern);
	free(conLargeItem);
	free(conLargeItemSupport);
 }

 return;
}

void pass1()
{
 int transSize;
 int item;
 int maxSize=0;
 FILE *fp;
 int i, j;

 /* Initialize the 1-itemsets list and support list */
 support1 = (int *) malloc (sizeof(int) * ATTRIBUTES);
 largeItem1 = (int *) malloc (sizeof(int) * ATTRIBUTES);
 if ((support1 == NULL) || (largeItem1 == NULL)) {
	printf("out of memory\n");
	exit(1);
 }

 for (i=0; i < ATTRIBUTES; i++) {
	support1[i] = 0;
	largeItem1[i] = i;
 }

 if ((fp = fopen(INPUT_DATA, "r")) == NULL) {
        printf("Can't open data file, %s.\n", INPUT_DATA);
        exit(1);
 }

 for (i=0; i < TXNS; i++) {

	fscanf(fp, "%d", &transSize);

	if (transSize > maxSize)
		maxSize = transSize;

	for (j=0; j < transSize; j++) {
		fscanf(fp, "%d", &item);
		support1[item]++;
	}
 }
 fclose(fp);

 realK = MAX_ITEMSET_SIZE;
 if ((maxSize < MAX_ITEMSET_SIZE) || (MAX_ITEMSET_SIZE <= 0))
	realK = maxSize;

 largeItemset = (LargeItemPtr *) malloc (sizeof(LargeItemPtr) * realK);
 numLarge = (int *) malloc (sizeof(int) * realK);

 if ((largeItemset == NULL) || (numLarge == NULL)) {
	printf("out of memory\n");
	exit(1);
 }

 for (i=0; i < realK; i++)  {
	largeItemset[i] = NULL;
	numLarge[i] = 0;
 }

 q_sortD(&(support1[0]), largeItem1, 0, ATTRIBUTES-1, ATTRIBUTES);

 numLarge[0] = 0;
 while ((numLarge[0] < ATTRIBUTES) && (support1[numLarge[0]] >= threshold))
	(numLarge[0])++;

 return;
}

void buildTree()
{
 int *freqItemP;
 int *indexList;
 int count;		/* Number of frequent items in a transaction */
 FILE *fp;		/* Pointer to the database file */
 int transSize;		/* Transaction size */
 int item;		/* An item in the transaction */
 int i, j, m;
 int path;		/* Number of new tree paths (i.e. new leaf nodes) created so far */


 /* Create header table */
 headerTableLink = (FPTreeNode *) malloc (sizeof(FPTreeNode) * numLarge[0]);
 if (headerTableLink == NULL) {
	printf("out of memory\n");
	exit(1);
 }
 for (i=0; i < numLarge[0]; i++)
	headerTableLink[i] = NULL;

 /* Create root of the FP-tree */
 root = (FPTreeNode) malloc (sizeof(FPNode));
 if (root == NULL) {
	printf("out of memory\n");
	exit(1);
 }

 /* Initialize the root node */
 root->numPath = 1;
 root->parent = NULL;
 root->children = NULL;
 root->hlink = NULL;

 /* Create freqItemP to store frequent items of a transaction */
 freqItemP = (int *) malloc (sizeof(int) * ATTRIBUTES);
 if (freqItemP == NULL) {
	printf("out of memory\n");
	exit(1);
 }

 indexList = (int *) malloc (sizeof(int) * ATTRIBUTES);
 if (indexList == NULL) {
	printf("out of memory\n");
	exit(1);
 }


 /* scan DB and insert frequent items into the FP-tree */
 if ((fp = fopen(INPUT_DATA, "r")) == NULL) {
        printf("Can't open data file, %s.\n", INPUT_DATA);
        exit(1);
 }


 for (i=0; i < TXNS; i++) {

	/* Read the transaction size */
	fscanf(fp, "%d", &transSize);

	count = 0;
 	path = 0;

	for (j=0; j < transSize; j++) {

		/* Read a transaction item */
		fscanf(fp, "%d", &item);

		for (m=0; m < numLarge[0]; m++) {
			if (item == largeItem1[m]) {
				freqItemP[count] = item;
				indexList[count] = m;
				count++;
				break;
			}
		}
	}

	q_sortA(indexList, freqItemP, 0, count-1, count);

	insert_tree(&(freqItemP[0]), &(indexList[0]), 1, 0, count, root, headerTableLink, &path);
 }
 fclose(fp);

 free(freqItemP);
 free(indexList);
 free(largeItem1);
 free(support1);

 return;
}

void displayResult()
{
 FILE *fp = fopen(OUTPUT_FILE, "w");
 LargeItemPtr aLargeItemset;
 int i, j;

 for (i=0; i < realK; i++) {

	if (numLarge[i] == 0) break;

	aLargeItemset = largeItemset[i];

	while (aLargeItemset != NULL) {
		for (j=0; j <= i; j++) {
			fprintf(fp, "%d ", aLargeItemset->itemset[j]);
		}
		fprintf(fp, "%d\n", aLargeItemset->support);

		aLargeItemset = aLargeItemset->next;
	}
 }

 fclose(fp);
 return;
}

void print_list(ItemSetNode *list)
{
  ItemSetNode *l;
  for (l = list; l != NULL; l = l->next)
  {
    printf("(Itemset: %s, Count: %d)\n", l->data->item_set, l->data->count);
  }
}

void swap_char(char *x, char *y)
{
    char temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

char *permutations[128];
int count;
void permute(char *a, int i, int n)
{
  int j;
  if (i == n)
  {
    char *tmp = malloc(sizeof(char) * (strlen(a) + 1));
    strcpy(tmp, a);
    permutations[count++] = tmp;
  }
  else
  {
    for (j = i; j <= n; j++)
    {
      swap_char((a+i), (a+j));
      permute(a, i+1, n);
      swap_char((a+i), (a+j));
    }
  }
}

void main(int argc, char *argv[])
{
 int headerTableSize;
 float time1, time2, time3;


 threshold = MIN_SUPPORT * TXNS;
 if (threshold == 0)
	threshold = 1;

 pass1();

 if (numLarge[0] > 0) {
 	buildTree();

	headerTableSize = numLarge[0];
	numLarge[0] = 0;
 	FPgrowth(root, headerTableLink, headerTableSize, NULL, 0);

 	displayResult();
 }

 destroy();

 FILE *fp = fopen(OUTPUT_FILE, "r");

 int n;
 ItemSetNode *list = NULL;
 char *tmp = malloc(sizeof(char) * 8);
 int i, j;

 char *line = NULL;
 char *attr = NULL;

 while ((line = read_line(fp)) != NULL && *line != '\0')
 {
	char *line_pointer = line;
	for (i = 0; (attr = strsep(&line, " ")) != NULL; i++)
	{
		if (strcmp(attr, "1") == 0)
			*(tmp + i) = 'a';
		else if (strcmp(attr, "2") == 0)
			*(tmp + i) = 'b';
		else if (strcmp(attr, "3") == 0)
			*(tmp + i) = 'c';
		else if (strcmp(attr, "4") == 0)
			*(tmp + i) = 'd';
		else if (strcmp(attr, "5") == 0)
			*(tmp + i) = 'e';
		else if (strcmp(attr, "6") == 0)
			*(tmp + i) = 'f';
		else if (strcmp(attr, "7") == 0)
			*(tmp + i) = 'g';
		else if (strcmp(attr, "8") == 0)
			*(tmp + i) = 'h';
		else if (strcmp(attr, "9") == 0)
			*(tmp + i) = 'i';
		else if (strcmp(attr, "10") == 0)
			*(tmp + i) = 'j';
		else if (strcmp(attr, "11") == 0)
			*(tmp + i) = 'k';
		else if (strcmp(attr, "12") == 0)
			*(tmp + i) = 'l';
		else if (strcmp(attr, "13") == 0)
			*(tmp + i) = 'm';
		else if (strcmp(attr, "14") == 0)
			*(tmp + i) = 'n';
		else if (strcmp(attr, "15") == 0)
			*(tmp + i) = 'o';
		else if (strcmp(attr, "16") == 0)
			*(tmp + i) = 'p';
		else if (strcmp(attr, "17") == 0)
			*(tmp + i) = 'q';
		else if (strcmp(attr, "18") == 0)
			*(tmp + i) = 'r';
		else if (strcmp(attr, "19") == 0)
			*(tmp + i) = 's';
		else if (strcmp(attr, "20") == 0)
			*(tmp + i) = 't';
		else if (strcmp(attr, "21") == 0)
			*(tmp + i) = 'u';
		else if (strcmp(attr, "22") == 0)
			*(tmp + i) = 'v';
		else if (strcmp(attr, "23") == 0)
			*(tmp + i) = 'w';
		else if (strcmp(attr, "24") == 0)
			*(tmp + i) = 'x';
		else
		{
			*(tmp + i) = '\0';
			append_to_list(&list, new_item_set(tmp, atoi(attr)));
			tmp = malloc(sizeof(char) * 8);
		}
	}
	free(line_pointer);
 }
 fclose(fp);
 printf("Support = %f, Confidence = %f\n", MIN_SUPPORT, MIN_CONF);
 printf("Frequent Itemsets: \n");
 print_list(list);
 ItemSetNode *l, *m;
 for (l = list; l != NULL; l = l->next)
 {
	for (i = 0; i < 128; i++)
		permutations[i] = NULL;
	count = 0;

	char *tmp = malloc(sizeof(char) * (strlen(l->data->item_set) + 1));
	strcpy(tmp, l->data->item_set);
	
	permute(tmp, 0, strlen(tmp) - 1);
	for (i = 0; i < 128; i++)
	{
		if (permutations[i] != NULL)
		{
			for (j = 1; j < strlen(permutations[i]); j++)
			{
				char *LHS = malloc(sizeof(char) * (j + 1));
				strncpy(LHS, permutations[i], j);
				*(LHS + j) = '\0';
				char *total = permutations[i];
				int lhs_support, total_support;
				for (m = list; m != NULL; m = m->next)
				{
					if (strcmp(LHS, m->data->item_set) == 0)
						lhs_support = m->data->count;
					if (strcmp(total, m->data->item_set) == 0)
						total_support = m->data->count;
				}
				float conf = (float) total_support / lhs_support;
				if (conf >= MIN_CONF)
					printf("Rule: %s -> %s, Confidence: %f\n", LHS, j + permutations[i], conf);
			}
		}
	}

 }
 return;
}