#define FUSE_USE_VERSION 28

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>

static  const  char *dirpath = "/home/pan/Downloads";
char *en_cap = "ZYXWVUTSRQPONMLKJIHGFEDCBA";
char *en = "zyxwvutsrqponmlkjihgfedcba";
char *de_cap = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char *de = "abcdefghijklmnopqrstuvwxyz";

int isDirectory(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

void encrypt(char* str)
{
    for (int i = 0 ; i < strlen(str) ; i++)
    {
        if ('a' <= str[i] && str[i] <= 'z')
        {
            str[i] = en[str[i] - 'a'];
        }
        else
        if ('A' <= str[i] && str[i] <= 'Z')
        {
            str[i] = en_cap[str[i] - 'a'];
        }
    }
}

void decrypt(char* str)
{
    for (int i = 0 ; i < strlen(str) ; i++)
    {
        if ('a' <= str[i] && str[i] <= 'z')
        {
            str[i] = de[str[i] - 'a'];
        }
        else
        if ('A' <= str[i] && str[i] <= 'Z')
        {
            str[i] = de_cap[str[i] - 'a'];
        }
    }
}

void listFilesRecursively(char *basePath,int status)
{
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            printf("%s\n",path);
            if (dp->d_name[0] != '.')
            {
                listFilesRecursively(path,status);
                char *fileName;
                char tmp[510];
                strcpy(tmp,dp->d_name);
                char s[2] = ".";
                fileName = strtok(tmp,s);
                encrypt(fileName);
                char *ext = strrchr(dp->d_name,'.');
                char oldName[510];
                char newName[510];
                strcpy(oldName,path);
                if (dp->d_type == DT_REG)
                    sprintf(newName,"%s/%s%s",basePath,fileName,ext);
                else
                    sprintf(newName,"%s/%s",basePath,fileName);
                rename(oldName,newName);
                printf("Renamed %s -> %s\n",oldName,newName);
            }
        }
    }

    closedir(dir);
}

void info (char *string, char *path){
    char* info = "INFO";
    char log[1000];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(log, "%s::%02d%02d%04d-%02d:%02d:%02d::%s::%s", info, tm.tm_mday, tm.tm_mon, tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec, string, path);
	FILE *fl = fopen("/home/arsyad/SinSeiFS.log", "a");  
   fprintf(fl, "%s\n", log);  
    fclose(fl);  
    return;
}

void warning (char *string, char *path)
{
    char* info = "WARNING";
    char log[1000];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(log, "%s::%02d%02d%04d-%02d:%02d:%02d::%s::%s", info, tm.tm_mday, tm.tm_mon, tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec, string, path);
	FILE *fl = fopen("/home/arsyad/SinSeiFS.log", "a");
    fprintf(fl, "%s\n", log);  
    fclose(fl);  
    return;
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
    info("LS", path);
	int res;
    char fpath[1000];

    sprintf(fpath,"%s%s",dirpath,path);

    res = lstat(fpath, stbuf);

    if (res == -1) return -errno;

    return 0;
}

int is_atoz(const char *name)
{
    ++name;
    printf("%s name\n",name);
    if (strlen(name) < 5) return 0;
    return (name[0] == 'A' && name[1] == 't' && name[2] == 'o' && name[3] == 'Z' && name[4] == '_');
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    info("CD", path);
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    info("CAT", path);
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
    info("MKDIR", path);
	int res;


    char fpath[1000];

    if(strcmp(path,"/") == 0)
    {
        path=dirpath;
        sprintf(fpath,"%s",path);
    } else sprintf(fpath, "%s%s",dirpath,path);
    printf("%s\n",fpath);
	res = mkdir(fpath, mode);


	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
    info("CREATE", path);
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{

    warning("UNLINK", path);
	int res;

	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
    warning("RMDIR", path);
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
    info("RENAME", from);
	int res;
    char completePathFrom[1010];
    sprintf(completePathFrom,"%s%s",dirpath,from);

    char completePathTo[1010];
    sprintf(completePathTo,"%s%s",dirpath,to);

    if (isDirectory(completePathFrom))
    {
        // listFilesRecursively(completePathFrom,-1);
        // printf("Directory\n");
        int atoz_before = is_atoz(from);
        int atoz_after = is_atoz(to);
        if (!atoz_before && atoz_after)
        {
            listFilesRecursively(completePathFrom,1);
        }
        else
        if (atoz_before && !atoz_after)
        {
            listFilesRecursively(completePathFrom,-1);
        }
    }

	res = rename(completePathFrom, completePathTo);
    printf("%s -- %s\n",completePathFrom,completePathTo);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
    info("TRUNCATE", path);
	int res;

	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
    info("OPEN", path);
	int res;

	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
    info("WRITE", path);
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}
static struct fuse_operations xmp_oper = {
	.getattr = xmp_getattr,
	.readdir = xmp_readdir,
	.read = xmp_read,
	.mkdir = xmp_mkdir,
	.mknod = xmp_mknod,
	.unlink = xmp_unlink,
	.rmdir = xmp_rmdir,
	.rename = xmp_rename,
	.truncate = xmp_truncate,
	.open = xmp_open,
	.read = xmp_read,
	.write = xmp_write,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}