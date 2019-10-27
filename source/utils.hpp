#pragma once

#define INV_WHITE "\x1b[47m"
#define BLACK CONSOLE_BLACK
#define WHITE CONSOLE_WHITE
#define RESET CONSOLE_RESET
#define RED CONSOLE_RED
#define GREEN CONSOLE_GREEN
#define BLUE CONSOLE_BLUE
#define YELLOW CONSOLE_YELLOW
#define MAGENTA CONSOLE_MAGENTA
#define CYAN CONSOLE_CYAN

#define MAX_LINES 38

char* keyboard(char* message, size_t size);
void userAppInit(void);

extern "C" 
{
    void printarraynew(char *array[], int on[], int arraylength, int highlight, int offset, int starty);
    void Debug_pritnt(char * input);
    char * calculateSize(uint64_t size);
    size_t get_file_size(std::string filename);
    void NroInfo(char *path_);
    bool is_nro_file(const char *path);
    void *realloc_or_free(void *ptr, size_t size);
    int get_dirent_dir(char const *path, struct dirent **result, size_t *size);
    int cmp_dirent_aux(struct dirent const *a, struct dirent const *b);
    int cmp_dirent(void const *a, void const *b);
    char * g_get_capacity ( char * dev_path);
    char * g_get_free_space ( char * dev_path);
}
