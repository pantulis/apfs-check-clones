//
//  clone_checker.c
//
//  To compile:
//    gcc clone_checker.c -o clone_checker
//
//  Juan Lupion
//  Based on code by  Dyorgio Nascimento on 2020-12-10. https://github.com/dyorgio/apfs-clone-checker
//  Human size in bytes by Jonathan Leffler // https://stackoverflow.com/questions/3898840/converting-a-number-of-bytes-into-a-file-size-in-c

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/attr.h>
#include <string.h>
#include <inttypes.h>

// declare methods
void printUsage(char* executable);
void check_disk_fs(char *filename, bool is_forced_mode);
struct stat check_file(char *filename, bool is_forced_mode);
void print_extended_flags(char* path, u_int64_t flags);
char *calculate_size(off_t);

int main(int args_count, char **args) {

  bool is_forced_mode = false;
  bool is_quick_mode = false;
  int opt;
  while ( (opt = getopt(args_count, args, "v?h")) != -1) {
       switch ( opt ) {
         case 'v': fprintf(stderr, "APFS Clone Checker - Version: 1.0.0.0\n"); exit(EXIT_SUCCESS); break;
         case '?':
         case 'h': printUsage(args[0]);
         default:
            printUsage(args[0]);
       }
  }
  if ( args_count - optind < 1 ) {
      printUsage(args[0]);
  }
  
  char* path = args[optind];
  
  // Assuming APFS filesystem
  //check_disk_fs(path, is_forced_mode); 

  struct AttrList {
    u_int32_t       length;
    // standard attributes 
    struct timespec st_crtime;
    struct timespec st_modtime;
    // file attributes
    off_t data_length;
    off_t data_alloc_size;
    // fork attributes (extended common)
    off_t private_size;
    u_int64_t clone_id;
    u_int64_t extendedFlags;
    u_int32_t clone_refcnt;
  } __attribute__((aligned(4), packed));
	  
  int result;
  struct attrlist attrList;
  struct AttrList myStat = {0};

  memset(&attrList, 0, sizeof(attrList));
  attrList.bitmapcount = ATTR_BIT_MAP_COUNT;
  attrList.commonattr = ATTR_CMN_CRTIME | ATTR_CMN_MODTIME;
  attrList.fileattr = ATTR_FILE_DATALENGTH | ATTR_FILE_DATAALLOCSIZE;
  attrList.forkattr = ATTR_CMNEXT_PRIVATESIZE | ATTR_CMNEXT_EXT_FLAGS | ATTR_CMNEXT_CLONEID | ATTR_CMNEXT_CLONE_REFCNT;

  result = getattrlist(path, &attrList, &myStat, sizeof(myStat), FSOPT_ATTR_CMN_EXTENDED);
  
  if (result == -1) {
    fprintf(stderr, "Can't stat getattrlist(%s), error: %s \n", path, strerror(errno));
    exit(EXIT_FAILURE);
  }

   off_t logical_size = myStat.data_length;
   off_t physical_size = myStat.data_alloc_size;
   u_int32_t clone_refcnt = myStat.clone_refcnt;
   u_int64_t clone_id = myStat.clone_id;
   off_t private_size = myStat.private_size;
   
   if (clone_refcnt > 1) {
     // 60 bytes memory leak here, let's not care
     printf("Clone ID:  %"PRIX64" ""%s"": clones: %u, size(logical): %s, size(physical): %s, size (private): %s\n",
	    clone_id, path, clone_refcnt, calculate_size(logical_size), calculate_size(physical_size), calculate_size(private_size));
   };
   
   exit(EXIT_SUCCESS);
}

void printUsage(char* executable){
  fprintf(stdout, "Usage: %s file\n", executable);
  exit(EXIT_FAILURE);
}

void check_disk_fs(char *filename, bool is_forced_mode) {
  struct statfs fs;
  if( statfs(filename, &fs) == 0 ) {
    if( strcmp(fs.f_fstypename, "apfs") != 0) {
      fprintf(stderr, "%s: Only APFS is supported: %s\n", filename, fs.f_fstypename);
      exit(EXIT_FAILURE);
    }
  }
}

struct stat check_file(char *filename, bool is_forced_mode) {
  struct stat st;
  if ( stat(filename, &st) < 0 ) {
    fprintf(stderr, "%s: No such file\n", filename);
    if ( is_forced_mode ) {
      fprintf(stdout,"0\n");
      exit(EXIT_SUCCESS);
    } else {
      exit(EXIT_FAILURE);
    }
  }

  if ( (st.st_mode & S_IFMT) != S_IFREG ) {
    fprintf(stderr, "%s: Not a regular file\n", filename);
    if ( is_forced_mode ) {
      fprintf(stdout,"0\n");
      exit(EXIT_SUCCESS);
    } else {
      exit(EXIT_FAILURE);
    }
  }
  return st;
}

// size calculation function from Jonathan Lefler

#define DIM(x) (sizeof(x)/sizeof(*(x)))
static const char *sizes[]   = { "EiB", "PiB", "TiB", "GiB", "MiB", "KiB", "B" };
static const uint64_t exbibytes = 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL;

char *calculate_size(off_t size) {   
    char     *result = (char *) malloc(sizeof(char) * 20);
    uint64_t  multiplier = exbibytes;
    int i;

    for (i = 0; i < DIM(sizes); i++, multiplier /= 1024)
    {   
        if (size < multiplier)
            continue;
        if (size % multiplier == 0)
            sprintf(result, "%" PRIu64 " %s", size / multiplier, sizes[i]);
        else
            sprintf(result, "%.1f %s", (float) size / multiplier, sizes[i]);
        return result;
    }
    strcpy(result, "0");
    return result;
}
  

