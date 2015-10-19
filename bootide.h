/* Routines for reading from the hard disk. These are shared by
 * the stage1 and stage2 bootloaders */

#define SECTSIZE 512

void waitdisk(void);
void readsects(uchar *addr, uint lba, uchar count);
