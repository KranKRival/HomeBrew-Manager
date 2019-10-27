#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <malloc.h>
#include <ostream>
#include <unistd.h>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <string>
#include <thread>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string.h>
#include <inttypes.h>
#include <switch.h>
#include "utils.hpp"

#define DIM(x) (sizeof(x)/sizeof(*(x)))

static const char     *sizes[]   = { "EiB", "PiB", "TiB", "GiB", "MiB", "KiB", "B" };
static const uint64_t  exbibytes = 1024ULL * 1024ULL * 1024ULL *
                                   1024ULL * 1024ULL * 1024ULL;

NroHeader header;
NroAssetHeader asset_header;
NacpStruct nacp;

int n;

char* keyboard(char* message, size_t size)
{
	SwkbdConfig	skp; 
	Result keyrc = swkbdCreate(&skp, 0);
	char* out = NULL;
	out = (char *)calloc(sizeof(char), size + 1);

	if (R_SUCCEEDED(keyrc) && out != NULL){
		swkbdConfigMakePresetDefault(&skp);
		swkbdConfigSetGuideText(&skp, message);
		keyrc = swkbdShow(&skp, out, size);
		swkbdClose(&skp);	
	}

	else 
	{
	free(out);
	out = NULL;
	}

	return (out);
}

void userAppInit(void)
{
	void *addr = NULL;
	if (svcSetHeapSize(&addr, 0x4000000) == (Result)-1) fatalSimple(0);
}



extern "C" 
{
	void printarraynew(char *array[], int on[], int arraylength, int highlight, int offset, int starty)
	{
		int max = arraylength - offset;
		
		if (max > MAX_LINES)
		max = MAX_LINES;
		
		printf("\x1b[%d;1H", starty);
		
		for (int i2 = 0; i2 < MAX_LINES * 2; i2++)
		printf("                                        ");
		
		printf("\x1b[%d;1H", starty);
		
		for (int i = 0; i < max; i++)
		{
			switch(on[i + offset]){
				case 0:
				printf(BLACK);
				break;
				case 1:
				printf(RED);
				break;
				case 2:
				printf(CYAN);
				break;
				case 3:
				printf(MAGENTA);
				break;
		    }  
			if (i == highlight - 1)
			printf(INV_WHITE "%s", array[i + offset]);
			else
			printf( WHITE "%s", array[i + offset]);
			printf("\n" RESET);
		}
	}

	void Debug_pritnt(char * input)
	{
		printf(INV_WHITE BLACK "\x1b[1;1HHomeBrew Manager " BLUE "\x1b[45;1HDEBUG: %s" RESET, input); 
	}

	char * calculateSize(uint64_t size)
	{ 
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

	size_t get_file_size(std::string filename)
	{
		FILE *p_file = NULL;
		p_file = fopen(filename.c_str(),"rb");
		fseek(p_file,0,SEEK_END);
		size_t size = ftell(p_file);
		fclose(p_file);
		return size;
    }
	
	void NroInfo(char *path_) 
	{
		FILE *fp = fopen(path_, "rb");
		
		if (fp == NULL) 
		{
			 printf(INV_WHITE BLACK "\x1b[1;1H" RED "\x1b[40;8H Bad file" RESET);
			 fclose(fp);
		}
		
		fseek(fp, sizeof(NroStart), SEEK_SET);
		if (fread(&header, sizeof(header), 1, fp) != 1) 
		{
			printf(INV_WHITE BLACK "\x1b[1;1H" RED "\x1b[40;8H Bad header read" RESET );
			fclose(fp);
		}
		
		fseek(fp, header.size, SEEK_SET);
		if (fread(&asset_header, sizeof(asset_header), 1, fp) != 1) 
		{
			printf(INV_WHITE BLACK "\x1b[1;1H" RED "\x1b[40;8H Bad asset header read" RESET );
			fclose(fp);
		}
		
		fseek(fp, header.size + asset_header.nacp.offset, SEEK_SET);
		if (fread(&nacp, sizeof(nacp), 1, fp) != 1) 
		{
			printf(INV_WHITE BLACK "\x1b[1;1H" RED "\x1b[40;8H Bad nacp read" RESET );
			fclose(fp);
		}
		
		size_t appsize = get_file_size(path_);
		printf(INV_WHITE BLACK "\x1b[1;1H" RED "\x1b[40;8H App Info: \x1b[42;1HAuthor  - %s \x1b[43;1HName    - %s \x1b[44;1HVersion - %s \x1b[45;1HSize    - %s" RESET, nacp.lang[0].author, nacp.lang[0].name,nacp.version,  calculateSize(appsize)  );
        fclose(fp);
	}

	bool is_nro_file(const char *path)
	{
		std::string fn = path;
		if(fn.substr(fn.find_last_of(".") + 1) == "nro") 
		{
			return true;
		} 
		else 
		{
			return false;
		}
		return false;
	}

	void *realloc_or_free(void *ptr, size_t size) 
	{
		void *tmp = realloc(ptr, size);
		if (tmp == NULL) 
		{
			free(ptr);
		}
		return tmp;
	}

	int get_dirent_dir(char const *path, struct dirent **result, size_t *size) 
	{
		DIR *dir = opendir(path);
		if (dir == NULL) 
		{
			closedir(dir);
			return -1;
		}
		
		struct dirent *array = NULL;
		size_t i = 0;
		size_t used = 0;
		struct dirent *dirent;
		
		while ((dirent = readdir(dir)) != NULL) 
		{
			if (used == i) 
			{
				i += 42; // why not?
				array = realloc_or_free(array, sizeof *array * i);
				if (array == NULL) 
				{
					closedir(dir);
					return -1;
				}
			}
			array[used++] = *dirent;
        }
		
		struct dirent *tmp = realloc(array, sizeof *array * used);
		if (tmp != NULL) 
		{
			array = tmp;
		}
		
		*result = array;
		*size = used;
		closedir(dir);
		return 0;
	}
    
	int cmp_dirent_aux(struct dirent const *a, struct dirent const *b) 
	{
		return strcmp(a->d_name, b->d_name);
	}
	
	int cmp_dirent(void const *a, void const *b) 
	{
		return cmp_dirent_aux(a, b);
	}

	char * g_get_capacity ( char * dev_path)
	{
		unsigned long long result = 0;
		char s_cap[50];
		char * ss_cap = "N/A";
		struct statvfs sfs;
		
		if ( statvfs ( dev_path, &sfs) != -1 )
		{
			result = (unsigned long long)sfs.f_bsize * sfs.f_blocks;
		}
		if (result > 0)
		{
			double f_cap = (double)result/(1024*1024);
			n = sprintf(s_cap, "%.2f Mb", f_cap);
			ss_cap = strdup(s_cap);
		}
		return ss_cap;
	}
	
	char * g_get_free_space ( char * dev_path)
	{
		unsigned long long result = 0;
		char s_cap[50];
		char * ss_cap = "N/A";
		struct statvfs sfs;
		
		if ( statvfs ( dev_path, &sfs) != -1 )
		{
			result = (unsigned long long)sfs.f_bsize * sfs.f_bfree;
		}
		if (result > 0)
		{
			double f_cap = (double)result/(1024*1024);
			n = sprintf(s_cap, "%.2f Mb", f_cap);
			ss_cap = strdup(s_cap);
		}
		return ss_cap;
	}
}