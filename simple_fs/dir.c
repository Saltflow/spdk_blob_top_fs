
#include "dir.h"
#include "blob_op.h"

static const struct spdk_dir_operations simplefs_dir_ops = {
	.spdk_mkdir			= simple_dir_create,
	.spdk_readdir	 	= simple_dir_read,
	.spdk_writedir 		= simple_dir_write,
	.spdk_closedir	 	= simple_dir_close,
};

// This function is expected to read once in the directory
void simple_dir_read(struct spdkfs_dir *dir)
{
	assert(dir->fs);
	size_t len;
	spdk_blob_get_xattr_value(dir->blob, "dir_persistent", &dir->dir_persist, &len);
	assert(len ==  sizeof(struct spdkfs_dir_persist_ctx));
	int io_unit = spdk_bs_get_io_unit_size(dir->fs->bs);
	dir->dirents = spdk_malloc(dir->dir_persist->d_size, io_unit, NULL, SPDK_ENV_SOCKET_ID_ANY,
				   SPDK_MALLOC_SHARE);
	generic_blob_io(dir->fs, dir->blob, dir->dir_persist->d_size, 0, dir->dirents, true);

	// In-memory data
	dir->initialized = true;
	dir->dirty = false;
}
// Always append to the bottom
void simple_dir_write(struct spdkfs_dir *dir)
{
	if (!dir->dirty) {
		return;
	}
	dir->dirty = false;
	spdk_set_thread(dir->fs->op_thread);
	spdk_blob_set_xattr(dir->blob, "dir_persistent", dir->dir_persist,
			    sizeof(struct spdkfs_dir_persist_ctx));
	generic_blob_io(dir->fs, dir->blob, dir->dir_persist->d_size, 0, dir->dirents, false);
}

void simple_dir_close(struct spdkfs_dir *dir)
{
	if (dir->dirty) {
		if(dir->dir_persist->d_size > get_blob_size(dir->blob, dir->fs->bs))
			generic_blob_resize(dir->fs, dir->blob, dir->dir_persist->d_size);
		generic_blob_io(dir->fs, dir->blob, dir->dir_persist->d_size, 0, dir->dirents, false);
		spdk_blob_set_xattr(dir->blob, "dir_persistent", dir->dir_persist, sizeof(struct spdkfs_dir_persist_ctx));
	}
	blob_close(dir->blob);
}
void simple_dir_create(struct spdkfs_dir *dir)
{
	dir->dir_persist = malloc(sizeof(struct spdkfs_dir_persist_ctx));
	// Persist data
	dir->dir_persist->d_dirent_count = 0;
	dir->dir_persist->_blob_id = spdk_blob_get_id(dir->blob);
	dir->dir_persist->i_ctime = time(NULL);
	dir->dir_persist->d_size = 0;
	spdk_blob_set_xattr(dir->blob, "dir_persistent", dir->dir_persist,
			    sizeof(struct spdkfs_dir_persist_ctx));

	// In-memory data
	dir->initialized = true;
	dir->dirty = false;

}

/* Extracts a file name part from *SRCP into PART, and updates *SRCP so that the
 * next call will return the next file name part. Returns 1 if successful, 0 at
 * end of string, -1 for a too-long file name part. */
static int
get_next_part (char part[SPDK_MAX_NAME_COUNT + 1], const char **srcp) 
{
  const char *src = *srcp;
  char *dst = part;
  /* Skip leading slashes. If its all slashes, were done. */
  while (*src == '/')
    src++;
  if (*src == '\0')
    return 0;
  /* Copy up to SPDK_MAX_NAME_COUNT character from SRC to DST. Add null terminator. */
  while (*src != '/' && *src != '\0') 
    {
      if (dst < part + SPDK_MAX_NAME_COUNT)
        *dst++ = *src; 
        else 
          return -1;
        src++;
    }
    *dst = '\0';
    /* Advance source pointer. */
    *srcp = src;
    return 1;
}

int spdkfs_dir_lookup (const struct spdkfs_dir *dir, const char *name)
{
  ASSERT (dir != NULL);
  ASSERT (name != NULL);
 for(int i=0; i < dir->dir_persist->d_dirent_count; ++i) {
     if(!strcmp(dir->dirents[i]._name, name)) {
         return i;
     }
 }
 return -1;
}

/*find if there is any file on the given path,
  which should invoke lookup for muitiple times,
  return the tailed name of the path, and the 
  path of working directory.
  it is the caller`s responsibilty to close the
  directory
  once it failed, it will return false. */
bool
find_path(const struct spdkfs_dir *dir, const char *name,
          char *tail_name, struct spdkfs_dir *file_dir)
{
  bool abs_path = false;

  assert(dir != NULL);
  ASSERT(name != NULL);

  /*for a removed dir, always return false*/
//   if(inode_removed(dir -> inode))
//     return false;

  /*if root path, return root with . entry*/
  if(!strcmp (name, "/"))
    {
      tail_name[0] = '.';
      tail_name[1] = '\0';
      file_dir = dir->fs->super_blob->root;
      return true;
    }

  /* if absolute path, then user root directory, and close it afterward*/
  if(name[0] == '/')
    abs_path = true;
  char file_name[SPDK_MAX_NAME_COUNT+1];  
  struct spdkfs_dir *now_dir;
  if(abs_path)
    now_dir = dir->fs->super_blob->root;
  else
    now_dir = dir; 
  int result;
  while (result = get_next_part (file_name, &(name)))
  {
    /* before the function could get 0, it is iterating through the path*/
    if (!strlen (name))
      break;
    if(result == -1)
      return false;
    spdk_blob_id blob_id = spdkfs_dir_lookup(now_dir, file_name);
     if(blob_id == -1)
       {
          return false;
       }
     if (now_dir != dir)
       now_dir->d_op->spdk_closedir(now_dir);
     blob_open(&now_dir->blob, blob_id);
    now_dir->d_op->spdk_readdir(now_dir);
  }
  strlcpy (tail_name, file_name, SPDK_MAX_NAME_COUNT+1);
  *(file_dir) = *(now_dir); 
  return true;
}


void bind_dir_ops(struct spdkfs_dir *dir)
{
	dir->d_op = &simplefs_dir_ops;
}