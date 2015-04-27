#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#define MIN_SUPPORT 0.05
#define MIN_CONF 0.80
#define DATA_FILE "inputHash.txt"
#define MAX_NODE_SIZE 2
#define TXNS 1728

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

struct _TreeNode
{
  struct _TreeNode *left;
  struct _TreeNode *middle;
  struct _TreeNode *right;
  ItemSetNode *item_sets;
  int level;
  int flag;
};

typedef struct _TreeNode TreeNode;

TreeNode *new_tree_node(void)
{
  TreeNode *root;
  root = malloc(sizeof(TreeNode));
  root->left = NULL;
  root->right = NULL;
  root->middle = NULL;
  root->level = -1;
  root->item_sets = NULL;
  root->flag = 1;

  return root;
}

void append_to_list(ItemSetNode **list, ItemSet *item_set)
{
    ItemSetNode *tmp = malloc(sizeof(ItemSetNode));
    tmp->data = item_set;
    tmp->next = *list;

    *list = tmp;
}

int get_size_of_list(ItemSetNode *list)
{
  int size = 0;

  for (size = 0; list != NULL; list = list->next, size++);

  return size;
}

ItemSet *new_item_set(char *item_set, int count)
{
  ItemSet *item = malloc(sizeof(ItemSet));

  item->item_set = item_set;
  item->count = count;

  return item;
}

void sort_string(char *s)
{
   int c, d = 0, length;
   char *pointer, *result, ch;

   length = strlen(s);

   result = malloc(length + 1);

   pointer = s;

   for (ch = 'a'; ch <= 'z'; ch++)
   {
      for (c = 0; c < length; c++)
      {
         if (*pointer == ch)
         {
            *(result + d) = *pointer;
            d++;
         }
         pointer++;
      }
      pointer = s;
   }
   *(result + d) = '\0';

   strcpy(s, result);
   free(result);
}

void insert_itemset_into_tree(TreeNode **root, ItemSet *item_set, int level, int k)
{
  if (*root == NULL)
  {
    *root = new_tree_node();
    (*root)->level = level;
    append_to_list(&((*root)->item_sets), item_set);
  }
  else if (get_size_of_list((*root)->item_sets) == MAX_NODE_SIZE && level < k)
  {
    ItemSetNode *tmp = NULL, *l;

    append_to_list(&tmp, (*root)->item_sets->data);
    append_to_list(&tmp, (*root)->item_sets->next->data);
    append_to_list(&tmp, item_set);

    (*root)->item_sets = NULL;
    (*root)->flag = 0;

    for (l = tmp; l != NULL; l = l->next)
    {
      if ((*(l->data->item_set + level)) % 3 == 0)
        insert_itemset_into_tree(&((*root)->left), l->data, 1 + level, k);
      else if ((*(l->data->item_set + level)) % 3 == 1)
        insert_itemset_into_tree(&((*root)->middle), l->data, 1 + level, k);
      else if ((*(l->data->item_set + level)) % 3 == 2)
        insert_itemset_into_tree(&((*root)->right), l->data, 1 + level, k);
    }
  }
  else if ((*root)->flag)
  {
    append_to_list(&((*root)->item_sets), item_set);
  }
  else
  {
    if ((*(item_set->item_set + level)) % 3 == 0)
      insert_itemset_into_tree(&((*root)->left), item_set, 1 + level, k);
    else if ((*(item_set->item_set + level)) % 3 == 1)
      insert_itemset_into_tree(&((*root)->middle), item_set, 1 + level, k);
    else if ((*(item_set->item_set + level)) % 3 == 2)
      insert_itemset_into_tree(&((*root)->right), item_set, 1 + level, k);
  }
}

void print_tree(TreeNode *root)
{
  if (root == NULL)
    return;

  ItemSetNode *l;

  printf("Level: %d - ", root->level);
  for (l = root->item_sets; l != NULL; l = l->next)
  {
    printf("Item Set: %s, Count: %d ", l->data->item_set, l->data->count);
  }
  printf("\n");

  print_tree(root->left);
  print_tree(root->middle);
  print_tree(root->right);
}

ItemSetNode *get_frequent_one_itemsets(void)
{
  const char* item[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y"};
  int count[25] = {0};

  FILE *fp = fopen(DATA_FILE, "r");

  char *line = NULL;
  int i;

  while ((line = read_line(fp)) != NULL && *line != '\0')
  {
    for (i = 0; i < 25; i++)
    {
      if (strstr(line, item[i]) != NULL)
      {
        count[i]++;
      }
    }
    free(line);
  }

  ItemSetNode *list = NULL;
  for (i = 0; i < 25; i++)
  {
    float support = (float) count[i] / TXNS;
    if (support >= MIN_SUPPORT)
    {
      char *tmp = malloc(sizeof(char) * 2);
      *tmp = *item[i];
      *(tmp + 1) = '\0';
      append_to_list(&list, new_item_set(tmp, count[i]));
    }
  }

  fclose(fp);

  return list;
}

void traverse_tree(TreeNode *root, char *txn)
{
  if (root == NULL)
    return;

  ItemSetNode *l;

  for (l = root->item_sets; l != NULL; l = l->next)
  {
    if (strstr(txn, l->data->item_set) != NULL)
      l->data->count = l->data->count + 1;
  }

  traverse_tree(root->left, txn);
  traverse_tree(root->middle, txn);
  traverse_tree(root->right, txn);
}

void get_leaves(TreeNode *root, ItemSetNode **list)
{
  if (root == NULL)
    return;

  ItemSetNode *l;

  for (l = root->item_sets; l != NULL; l = l->next)
  {
    if ((float)l->data->count / TXNS >= MIN_SUPPORT)
    {
      append_to_list(list, l->data);
    }
  }

  get_leaves(root->left,list);
  get_leaves(root->middle, list);
  get_leaves(root->right, list);
}

ItemSetNode *get_frequent_itemsets(TreeNode *root)
{
  FILE *fp = fopen(DATA_FILE, "r");

  char *line = NULL;

  while ((line = read_line(fp)) != NULL && *line != '\0')
  {
    traverse_tree(root, line);
    free(line);
  }

  fclose(fp);

  ItemSetNode *tmp = NULL;

  get_leaves(root, &tmp);

  return tmp;

}

ItemSetNode *get_frequent_k_plus_one_itemsets(ItemSetNode *frequent_k, ItemSetNode *frequent_1, int k_plus_one)
{
  TreeNode *root = NULL;

  ItemSetNode *l, *i;

  for (l = frequent_k; l != NULL; l = l->next)
  {
    for (i = frequent_1; i != NULL; i = i->next)
    {
      if (*(i->data->item_set) > *(l->data->item_set + strlen(l->data->item_set) - 1))
      {
        char *old_item_set = l->data->item_set;
        char *item_set = malloc(sizeof(char) * (strlen(old_item_set) + 2));
        strcpy(item_set, old_item_set);
        strcat(item_set, i->data->item_set);
        sort_string(item_set);

        ItemSet *tmp = new_item_set(item_set, 0);
        insert_itemset_into_tree(&root, tmp, 0, k_plus_one);
      }
    }
  }
  ItemSetNode *frequent_k_plus_1 = get_frequent_itemsets(root);

  return frequent_k_plus_1;
}

void print_list(ItemSetNode *list)
{
  ItemSetNode *l;
  for (l = list; l != NULL; l = l->next)
  {
    printf("(Itemset: %s, Count: %d)\n", l->data->item_set, l->data->count);
  }
}

void print_frequent_k(ItemSetNode *frequent_k, int k)
{
  printf("Frequent %d\n", k);
  print_list(frequent_k);
}

void swap(char *x, char *y)
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
      swap((a+i), (a+j));
      permute(a, i+1, n);
      swap((a+i), (a+j));
    }
  }
}

void apriori(void)
{
  ItemSetNode *frequent[8] = {NULL};
  frequent[1] = get_frequent_one_itemsets();
  int i, j, k;
  for (i = 2; i < 8; i++)
  {
    frequent[i] = get_frequent_k_plus_one_itemsets(frequent[i - 1], frequent[1], i);
  }

  ItemSetNode *all_frequent = NULL, *l, *m;
  for (i = 1; i < 8; i++)
  {
    for (l = frequent[i]; l != NULL; l = l->next)
    {
      append_to_list(&all_frequent, l->data);
    }
  }
  printf("Frequent Itemsets: \n");
  print_list(all_frequent);

  for (l = all_frequent; l != NULL; l = l->next)
  {
    for (i = 0; i < 256; i++)
      permutations[i] = NULL;
    count = 0;

    char *tmp = malloc(sizeof(char) * (strlen(l->data->item_set) + 1));
    strcpy(tmp, l->data->item_set);
    
    permute(tmp, 0, strlen(tmp) - 1);
    for (i = 0; i < 256; i++)
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
          for (m = all_frequent; m != NULL; m = m->next)
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
}

int main(void)
{
  printf("Support = %f, Confidence = %f\n", MIN_SUPPORT, MIN_CONF);
  apriori();
  return 0;
}