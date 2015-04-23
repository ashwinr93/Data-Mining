#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

void binarize_file(FILE *fp)
{
  FILE *out_file = fopen("binarized.data", "w");

  /*
  a -> parents=usual,
  b -> parents=pretentious,
  c -> parents=great_pret,
  d -> has_nurs=proper,
  e -> has_nurs=less_proper,
  f -> has_nurs=improper,
  g -> has_nurs=critical,
  h -> has_nurs=very_crit,
  i -> form=complete,
  j -> form=completed,
  k -> form=incomplete,
  l -> form=foster,
  m -> children=1,
  n -> children=2,
  o -> children=3,
  p -> children=more,
  q -> housing=convenient,
  r -> housing=less_conv,
  s -> housing=critical,
  t -> finance=convenient,
  u -> finance=inconv,
  v -> social=nonprob,
  w -> social=slightly_prob,
  x -> social=problematic,
  */
  
  fprintf(out_file, "abcdefghijklmnopqrstuvwx");
  char *line = NULL;

  while ((line = read_line(fp)) != NULL)
  {
    fprintf(out_file, "\n");
    char *line_pointer = line;
    char *attr = NULL;
    int i;
    for (i = 0; (attr = strsep(&line, ",")) != NULL; i++)
    {
      switch (i)
      {
        case 0: if (strcmp(attr, "usual") == 0)
                  fprintf(out_file, "100");
                else if (strcmp(attr, "pretentious") == 0)
                  fprintf(out_file, "010");
                else if (strcmp(attr, "great_pret") == 0)
                  fprintf(out_file, "001");
                break;
        case 1: if (strcmp(attr, "proper") == 0)
                  fprintf(out_file, "10000");
                else if (strcmp(attr, "less_proper") == 0)
                  fprintf(out_file, "01000");
                else if (strcmp(attr, "improper") == 0)
                  fprintf(out_file, "00100");
                else if (strcmp(attr, "critical") == 0)
                  fprintf(out_file, "00010");
                else if (strcmp(attr, "very_crit") == 0)
                  fprintf(out_file, "00001");
                break;
        case 2: if (strcmp(attr, "complete") == 0)
                  fprintf(out_file, "1000");
                else if (strcmp(attr, "completed") == 0)
                  fprintf(out_file, "0100");
                else if (strcmp(attr, "incomplete") == 0)
                  fprintf(out_file, "0010");
                else if (strcmp(attr, "foster") == 0)
                  fprintf(out_file, "0001");
                break;
        case 3: if (strcmp(attr, "1") == 0)
                  fprintf(out_file, "1000");
                else if (strcmp(attr, "2") == 0)
                  fprintf(out_file, "0100");
                else if (strcmp(attr, "3") == 0)
                  fprintf(out_file, "0010");
                else if (strcmp(attr, "more") == 0)
                  fprintf(out_file, "0001");
                break;
        case 4: if (strcmp(attr, "convenient") == 0)
                  fprintf(out_file, "100");
                else if (strcmp(attr, "less_conv") == 0)
                  fprintf(out_file, "010");
                else if (strcmp(attr, "critical") == 0)
                  fprintf(out_file, "001");
                break;
        case 5: if (strcmp(attr, "convenient") == 0)
                  fprintf(out_file, "10");
                else if (strcmp(attr, "inconv") == 0)
                  fprintf(out_file, "01");
                break;
        case 6: if (strcmp(attr, "nonprob") == 0)
                  fprintf(out_file, "100");
                else if (strcmp(attr, "slightly_prob") == 0)
                  fprintf(out_file, "010");
                else if (strcmp(attr, "problematic") == 0)
                  fprintf(out_file, "001");
                break;
      }
    }

    free(line_pointer);
  }

  fclose(out_file);
}

void process_file(FILE *fp)
{
  //FILE *out_file = fopen("processed_fp.data", "w");
  FILE *out_file = fopen("processed.data", "w");
  char *line = NULL;

  // Discard the 1st line contaning labels
  free(read_line(fp));
  while ((line = read_line(fp)) != NULL && *line != '\0')
  {
    int i;
    //fprintf(out_file, "%d ", 7);
    for (i = 0; i < strlen(line); i++)
    {
      if (*(line + i) == '1')
      {
        //fprintf(out_file, "%d ", 1 + i);
        fprintf(out_file, "%c", 97 + i);
      }
    }
    fprintf(out_file, "\n");
    free(line);
  }

  fclose(out_file);
}

int main(void)
{
  FILE *fp = fopen("nursery.data", "r");
  binarize_file(fp);
  fclose(fp);

  fp = fopen("binarized.data", "r");
  process_file(fp);
  fclose(fp);

  return 0;
}