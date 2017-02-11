
typedef char *VA;

typedef struct MemoryRegions
{
	VA startAddr;
	VA endAddr;
	size_t size;
	int isReadable, isWritable, isExecutable, isPrivate;
	off_t offset;
	unsigned int long device_id_major, device_id_minor, inode_number;
	char name[1024];
} __attribute__((packed)) MemoryRegion;

char mtcp_readchar(int fd)
{
	char c;
	ssize_t rc;
	do{
		rc = read(fd, &c, 1);
	}while(rc == -1);
	if (rc <= 0) return 0;
	return c;
}

char mtcp_readdec (int fd, VA *value)
{
  char c;
  unsigned long int v;
  v = 0;
  while (1) {
    c = mtcp_readchar (fd);
    if ((c >= '0') && (c <= '9')) c -= '0';
    else break;
    v = v * 10 + c;
  }
  *value = (VA)v;
  return (c);
}

char mtcp_readhex(int fd, VA *value)
{
	char c;
	unsigned long int v;

	v = 0;
	while(1)
	{
		c = mtcp_readchar(fd);
		if ((c >= '0') && (c <= '9')) c -= '0';
    	else if ((c >= 'a') && (c <= 'f')) c -= 'a' - 10;
    	else if ((c >= 'A') && (c <= 'F')) c -= 'A' - 10;
    	else break;
    	v = v * 16 + c;
	}
  	*value = (VA)v;
   	return (c);
}

//read hex characters from the file into local variables
int readline (int fd, MemoryRegion *proc)
{
	char c, r, w, x, p;
	off_t offset;
	unsigned long int device_id_major, device_id_minor, inode_number;
	VA startAddr, endAddr;
	int i;

	c = mtcp_readhex(fd, &startAddr);
	if (c != '-') return 0;
	
	c = mtcp_readhex(fd, &endAddr);
	if (c != ' ') return 0;
	
	r = mtcp_readchar(fd);
	if ((r != 'r') && (r != '-')) return 0;
	w = mtcp_readchar(fd);
	if ((w != 'w') && (w != '-')) return 0;
	x = mtcp_readchar(fd);
	if ((x != 'x') && (x != '-')) return 0;
	p = mtcp_readchar(fd);
	if ((p != 'p') && (p != 's')) return 0;

	c = mtcp_readchar(fd);
	if (c != ' ') return 0;

	c = mtcp_readhex(fd, (VA *)&offset);
	if (c != ' ') return 0;

	c = mtcp_readhex(fd, (VA *)&device_id_major);
	if (c != ':') return 0;

	c = mtcp_readhex(fd, (VA *)&device_id_minor);
	if (c != ' ') return 0;

	c = mtcp_readdec(fd, (VA *)&inode_number);
	if (c != ' ') return 0;

	proc->startAddr = startAddr;
	//printf("%p\t", proc->startAddr);
	proc->endAddr = endAddr;
	//printf("%p\t", proc->endAddr);
	proc->size = endAddr - startAddr;
	//printf("%ld\n", proc->size);
	
	if (r == 'r')
		proc->isReadable = 1;
	else
		proc->isReadable = 0;
	//printf("%d\t", proc->isReadable);
	
	if (w == 'w')
		proc->isWritable = 1;
	else
		proc->isWritable = 0;
	//printf("%d\t", proc->isWritable);
	
	if (x == 'x')
		proc->isExecutable = 1;
	else
		proc->isExecutable = 0;
	//printf("%d\t", proc->isExecutable);
	
	if (p == 'p')
		proc->isPrivate = 1;
	else
		proc->isPrivate = 0;
	//printf("%d\n", proc->isPrivate);

	proc->offset = offset;
	proc->device_id_major = device_id_major;
	proc->device_id_minor = device_id_minor;
	proc->inode_number = inode_number;
	

	proc->name[0] = '\0';
  	while (c == ' ') c = mtcp_readchar (fd);
  	if (c == '/' || c == '[') 
  	{ /* absolute pathname, or [stack], [vdso], etc. */
    	i = 0;
    	do {
      		proc->name[i++] = c;
      		if (i == sizeof proc->name) return 0;
      		c = mtcp_readchar (fd);
    	} while (c != '\n');
    	proc->name[i] = '\0';
  	}
   	return 1;
}