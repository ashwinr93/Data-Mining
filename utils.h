char *read_line(FILE *fp)
{
  if (feof(fp))
    return NULL;

  char c;
  int count = 0;
  char *buffer = NULL;
  while ((c = fgetc(fp)) != '\n' && c != EOF)
  {
    buffer = realloc(buffer, (count + 1) * sizeof(char));
    *(buffer + count) = c;
    count++;
  }

  buffer = realloc(buffer, (count + 1) * sizeof(char));
  *(buffer + count) = '\0';

  return buffer;
}