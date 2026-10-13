#ifndef DATAFILE_H
#define DATAFILE_H

struct tDatafileIndex
{
	long offset;
	const struct GameDriver *driver;
};

#ifdef CMD_LIST
struct tMenuIndex
{
	long offset;
	char *menuitem;
};

#ifdef UI_TITLE_MENU
extern int load_driver_command_ex(const struct GameDriver *drv, char *buffer, int bufsize, const int menu_sel, const char *menu_item[]);
#else
 extern int load_driver_command_ex(const struct GameDriver *drv, char *buffer, int bufsize, const int menu_sel);
#endif /* UI_TITLE_MENU */

extern UINT8 command_sub_menu(const struct GameDriver *drv, const char *menu_item[]);
#endif /* CMD_LIST */

extern int load_driver_history (const struct GameDriver *drv, char *buffer, int bufsize);
#ifdef MASH_DATAFILE
extern int load_driver_mameinfo (const struct GameDriver *drv, char *buffer, int bufsize);
extern int load_driver_drivinfo (const struct GameDriver *drv, char *buffer, int bufsize);
extern int load_driver_statistics (char *buffer, int bufsize);
#endif /* MASH_DATAFILE */

#endif
