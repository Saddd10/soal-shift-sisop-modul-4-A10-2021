# soal-shift-sisop-modul-4-A10-2021

## Soal 1
Di suatu jurusan, terdapat admin lab baru yang super duper gabut, ia bernama Sin. Sin baru menjadi admin di lab tersebut selama 1 bulan. Selama sebulan tersebut ia bertemu orang-orang hebat di lab tersebut, salah satunya yaitu Sei. Sei dan Sin akhirnya berteman baik. Karena belakangan ini sedang ramai tentang kasus keamanan data, mereka berniat membuat filesystem dengan metode encode yang mutakhir. Berikut adalah filesystem rancangan Sin dan Sei : 

![Soal1](https://user-images.githubusercontent.com/73766214/121810713-1bdeb500-cc8c-11eb-8d65-1f0d1ee9103d.png)


#### Solusi
##### A dan B. Jika sebuah direktori dibuat dengan awalan “AtoZ_”, maka direktori tersebut akan menjadi direktori ter-encode. Jika sebuah direktori di-rename dengan awalan “AtoZ_”, maka direktori tersebut akan menjadi direktori ter-encode.

- Melakukan pengecekan jika ada folder berawalan ```AtoZ_```.
- Membuat fungsi ```encrypt``` dan ```decrypt``` untuk mengenkripsi dan mendekripsi nama folder pada direktori sesuai dengan cipher atbash.


```c
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
```

```c
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
```

##### C. Apabila direktori yang terenkripsi di-rename menjadi tidak ter-encode, maka isi direktori tersebut akan terdecode.

##### D. Setiap pembuatan direktori ter-encode (mkdir atau rename) akan tercatat ke sebuah log. Format : /home/[USER]/Downloads/[Nama Direktori] → /home/[USER]/Downloads/AtoZ_[Nama Direktori].

##### E. Metode encode pada suatu direktori juga berlaku terhadap direktori yang ada di dalamnya.(rekursif).

- Membuat file ```listFileRecursively``` yang akan terpanggil apabila ada folder yang di-rename menjadi ```AtoZ_```
- Memodifikasi fungsi struct ```xmp_rename``` untuk mengecek apabila ada folder yang di-rename menjadi ```AtoZ_``` 

```c
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
```

```c
static int xmp_rename(const char *from, const char *to)
{
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

    info("RENAME", completePathTo);
	if (res == -1)
		return -errno;

	return 0;
}
```
Output folder berawalan "AtoZ_":
![Soal1_1](https://user-images.githubusercontent.com/73766214/121810574-a8d53e80-cc8b-11eb-94d8-cc6d90090a1c.png)
![Soal1_2](https://user-images.githubusercontent.com/73766214/121810580-ad99f280-cc8b-11eb-9244-17ac899b9eb9.png)

Output folder tidak berawalan "AtoZ_":
![Soal1_3](https://user-images.githubusercontent.com/73766214/121810678-f94c9c00-cc8b-11eb-94cb-89fdc6345471.png)
![Soal1_4](https://user-images.githubusercontent.com/73766214/121810680-fc478c80-cc8b-11eb-9f1c-7e3029fb7aa0.png)


#### Kendala :

## Soal 4
Untuk memudahkan dalam memonitor kegiatan pada filesystem mereka Sin dan Sei membuat sebuah log system dengan spesifikasi sebagai berikut.

#### Solusi
##### Log system yang akan terbentuk bernama “SinSeiFS.log” pada direktori home pengguna (/home/[user]/SinSeiFS.log). Log system ini akan menyimpan daftar perintah system call yang telah dijalankan pada filesystem. Karena Sin dan Sei suka kerapian maka log yang dibuat akan dibagi menjadi dua level, yaitu INFO dan WARNING. Untuk log level WARNING, digunakan untuk mencatat syscall rmdir dan unlink. Sisanya, akan dicatat pada level INFO. Format untuk logging yaitu:
[Level]::[dd][mm][yyyy]-[HH]:[MM]:[SS]:[CMD]::[DESC :: DESC]

Level : Level logging, dd : 2 digit tanggal, mm : 2 digit bulan, yyyy : 4 digit tahun, HH : 2 digit jam (format 24 Jam),MM : 2 digit menit, SS : 2 digit detik, CMD : System Call yang terpanggil, DESC : informasi dan parameter tambahan

- Membuat fungsi baru yaitu ```info``` dan ```warning``` untuk membuat file log dan menulis daftar perintah system call yang telah dijalankan.
- Passing parameter kata "Info" dan "Warning" serta pathfile dari fungsi struct yang dijalankan
- ```year + 1900``` dan ```month + 1``` untuk menghasilkan tahun dan bulan saat ini
- ```sprintf()``` digunakan untuk mencetak string pada log sesuai dengan format pada soal

```c
void info (char *string, const char *path){
    char* info = "INFO";
    char log[1000];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(log, "%s::%02d%02d%d-%02d:%02d:%02d::%s::%s", info, tm.tm_mday, 1 + tm.tm_mon, 1900 + tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec, string, path);
	FILE *fl = fopen("/home/arsyad/SinSeiFS.log", "a");  
   fprintf(fl, "%s\n", log);  
    fclose(fl);  
    return;
}
```

```c
void warning (char *string, const char *path)
{
    char* info = "WARNING";![soal4_1](https://user-images.githubusercontent.com/73766214/121805869-4f631480-cc77-11eb-9a27-b90ee5fb3627.png)

    char log[1000];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(log, "%s::%02d%02d%d-%02d:%02d:%02d::%s::%s", info, tm.tm_mday, 1 + tm.tm_mon, 1900 + tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec, string, path);
	FILE *fl = fopen("/home/arsyad/SinSeiFS.log", "a");
    fprintf(fl, "%s\n", log);  
    fclose(fl);  
    return;
}
```

Output:

![soal4_1](https://user-images.githubusercontent.com/73766214/121805883-66a20200-cc77-11eb-92a2-5f55049ea418.png)
![Soal4_2](https://user-images.githubusercontent.com/73766214/121805885-6bff4c80-cc77-11eb-8dd9-821d45d1b4c2.png)

#### Kendala 

