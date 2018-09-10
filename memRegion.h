typedef struct
{
  char startaddr[20];
  char endaddr[20];
  int size;
  int isReadable;
  int isExecutable;
  int isWritable;
}memRegion;
