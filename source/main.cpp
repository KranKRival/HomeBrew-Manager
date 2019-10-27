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

int  apps_active[200];
char *apps[200];
int amount = 0;
bool noapps = false, keyboardinit = false;
int highlight = 1;
struct dirent *result;
size_t size;
bool activecursor = true;

char HBABuff[2000];
char itembuff[2000];
char infobuff[300];
char exbuff[1000];

void additem(const char *item, int spot);
void CleanItem();
void Cleanlist();
void RefreshApps();
void apps_main();

int main(int argc, char* argv[])
{
    consoleInit(NULL);
    apps_main();
    consoleExit(NULL);
    exit(EXIT_SUCCESS); 
    return 0;    
}

void apps_main()
{
    bool update = true;
    bool _delete = false;

    RefreshApps();
    
    printf(INV_WHITE BLACK "\x1b[1;1HHomeBrew Manager " RED "\x1b[42;64H(X) Delete App" RED "\x1b[43;68H(+) Exit" RESET);
    
    while(appletMainLoop())
    {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

        if (!noapps)
        {
            if (!_delete && activecursor)
            {
                if (kDown & KEY_LSTICK_DOWN || kDown & KEY_DDOWN) 
                {
                    highlight++;
                    update = true;
                    
                }
                if (kDown & KEY_LSTICK_UP || kDown & KEY_DUP) 
                {
                    highlight--;
                    update = true;
                }
                if (kHeld & KEY_RSTICK_DOWN)
                {
                    highlight++;
                    update = true;
                }
                if (kHeld & KEY_RSTICK_UP)
                {
                    highlight--;
                    update = true;
                }
                
            }
            if (highlight > amount) 
            {
                highlight = 1;
                update = true;
            }
            if (highlight < 1) 
            {
                highlight = amount;
                update = true;
            }
            if (kDown & KEY_X)
            {
                if (strcmp(result[highlight - 1].d_name, "HomeBrew-Manager.nro") == 0 || strcmp(result[highlight - 1].d_name, "HomeBrew-Manager") == 0)
                {
                    printf(INV_WHITE BLACK "\x1b[1;1H" RED "\x1b[42;35H Error: " WHITE "%s %s" RED "\x1b[43;35H     (A) / (B) Continue" RESET , "Cant Delete own self", "!");
                    _delete = false;
                    activecursor = false;
                }
                else
                {
                    printf(INV_WHITE BLACK "\x1b[1;1H" RED "\x1b[42;35H Delete: " WHITE "%s %s" RED "\x1b[43;35H(A) Confirm / (B) Cancel" RESET , result[highlight - 1].d_name, "?");
                    _delete = true;
                    activecursor = false;
                }
                update = false;
                
            }

            if (kDown & KEY_A)
            {
                //consoleClear();
                if (_delete)
                {
                    char _deletebuff[300];
                    sprintf(_deletebuff, "/switch/%s",result[highlight - 1].d_name);
                    if (is_nro_file(result[highlight - 1].d_name))
                    {
                        remove(_deletebuff); 
                    }
                    else
                    {
                        fsdevDeleteDirectoryRecursively(_deletebuff);
                        //additem(_deletebuff, amount +1);
                        sprintf(HBABuff, "/switch/appstore/.get/packages/%s", result[highlight - 1].d_name);
                        DIR* HBA_DIR = opendir(HBABuff);
                        if (HBA_DIR != NULL) 
                        {
                            closedir(HBA_DIR);
                            fsdevDeleteDirectoryRecursively(HBABuff);
                            
                        }

                    }
                    
                    CleanItem();
                    Cleanlist();
                    RefreshApps();

                }

                 update = true;
                _delete = false;
                activecursor = true;
            }
            else if (kDown & KEY_B)
            {
                //consoleClear();
                update = true;
                _delete = false;
                activecursor = true;
            }

            if (update) 
            {
                consoleClear();
                Cleanlist();
                RefreshApps();

                sprintf(infobuff, "/switch/%s", result[highlight - 1].d_name);
                if (is_nro_file(result[highlight - 1].d_name))
                {
                    NroInfo(infobuff);
                }
                else
                {
                    DIR *dir_x = opendir(infobuff);
                    if (dir_x != NULL) 
                    {
                        struct dirent *dirent_x;
                        while ((dirent_x = readdir(dir_x)) != NULL) 
                        {
                            if (is_nro_file(dirent_x->d_name))
                            {
                                sprintf(exbuff, "%s/%s", infobuff, dirent_x->d_name);
                                NroInfo(exbuff);
                            }
                        } 
                    }
                    closedir(dir_x);
                }
                printf(INV_WHITE BLACK "\x1b[1;1HHomeBrew Manager " RED "\x1b[42;64H(X) Delete App" RED "\x1b[43;68H(+) Exit" RESET);
                printarraynew(apps, apps_active, amount, highlight, 0, 3);
                printf(INV_WHITE BLACK "\x1b[1;18H- Capacity: %s / Free: %s - %d Apps" RESET,g_get_capacity("."), g_get_free_space("."), amount);
                update = false;
            }
        }
        else
        {
            printf(RED "\x1b[3;1HNo apps detected!" RESET);
        }

        if (kDown & KEY_PLUS)
        {
            break;
        } 
        
        consoleUpdate(NULL);
    }
}

void additem(const char *item, int spot)
{
    size_t size = strlen(item) + 1;
    apps[spot] = (char*) malloc (size);
    strcpy(apps[spot], item);
}

void CleanItem()
{
    int i;

    if (amount == 1){
        noapps = true;
        //additem("No Apps detected", 0);
        //apps_active[0] = 9;
        return;
    }

    for (i = 0; i < amount - 1; i++){
        additem(apps[i + 1], i);
        apps_active[i] = apps_active[i + 1];
    }
    
    free(apps[i]);
    amount--;   
}

void Cleanlist()
{
    int i;
    for (i = 0; i < amount; i++)
    {
        apps_active[i] = 0;
        free(apps[i]);
    }
    amount = 0;
       
}

void RefreshApps()
{
    
    if (get_dirent_dir("/switch", &result, &size) != 0) 
    {
        perror("get_file_dir()");
    }
    
    qsort(result, size, sizeof *result, &cmp_dirent);
    
    
    for (size_t i = 0; i < size; i++) 
    {
        sprintf(HBABuff, "/switch/appstore/.get/packages/%s", result[i].d_name);
        DIR* HBA_DIR = opendir(HBABuff);
        if (HBA_DIR != NULL) 
        {
            sprintf(itembuff, "%s %s %s", result[i].d_name, " - ", BLUE "(HomeBrew AppStore)");
        }
        else
        {
            sprintf(itembuff, "%s %s %s", result[i].d_name,  " - ", RED "(Manually Installed)");
        }
        closedir(HBA_DIR);
        additem(itembuff, i);
        amount++;
        
    }
    
    free(result);
}