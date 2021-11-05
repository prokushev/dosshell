/* DOSShell Loader/Task list API open source prototype */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>

extern void far cdecl test(void);

void interrupt (*oldvect)(void) = NULL;

#define MAXENTRIES 16
#define ENTRIELENGTH 80

#define F_DOSSHELL	0x0040
#define F_UNUSED	0x8000

typedef
  struct
  {
    char far name[ENTRIELENGTH];
    unsigned int flags;
  } tentrie;

typedef
  struct
  {
    unsigned char id;		// Switcher id
    unsigned char unk1[4];
    unsigned char params[130];
    unsigned char unk2[225];    // Seems screen grabber path here and some other path things
    unsigned int  task_id;	// Active task id
    unsigned int  last_error;	// last error
    unsigned char unk3[];
    unsigned char entries_count;
    unsigned char first;	// fist task
    tentrie  entries[MAXENTRIES];
  } tcontrol;

tcontrol far control;

void __interrupt __far interrupt_handler(union INTPACK r)
{
  int i;

    if (r.x.ax==0x4a05)   // TaskSwitcher API
    {
      switch (r.x.si)
      {
      case 0: // Init
	control.last_error=0;      
	control.entries_count=0;      
	control.first=0;
	for (i=0;i<=MAXENTRIES;i++)
        {
		control.entries[i].flags=F_UNUSED;
        }
	control.entries[0].flags=F_DOSSHELL;
	_fstrcpy(control.entries[0].name, "MS-DOS Shell");
	break;
      case 1: // Add Task
	for (i=0;i<=MAXENTRIES;i++)
        {
		if ((control.entries[i].flags & F_UNUSED)==F_UNUSED)
		{
			control.entries[i].flags=0;
			_fstrcpy(control.entries[i].name, MK_FP(r.x.es, r.x.bp));
			control.entries_count++;
			break;
		}
        }
	break;
      case 2: // Switch to next task by Z order
        break;
      case 3: // Switch to previous task by Z order
        break;
      case 4: // remove Task
	control.entries[r.h.bl].flags=F_UNUSED;
	break;
      case 5: // Go to next task in list
        break;
      case 6: // Get i-th program name entry
	r.x.si=FP_OFF(control.entries[r.h.bl].name);
	r.x.es=FP_SEG(control.entries[r.h.bl].name);
	break;
      case 7: // listlen
	r.x.ax=control.entries_count;
	break;
      case 8: // controldata
	r.x.dx=FP_OFF(&control);
	r.x.ax=FP_SEG(&control);
	break;
      case 9: // Get task list entry
        break;
      case 10: // Add parameters
        break;
      case 11: // LastError
	r.x.ax=control.last_error;
	break;
      case 12: //Put task on top of Z order
        break;
      default:
        _chain_intr(oldvect);
	break;
      }
    } else {
        _chain_intr(oldvect);
    }
}

void main(void)
{
    union REGS r;
    struct SREGS s;

    // Запоминаем адрес старого обработчика прерываний
    oldvect = _dos_getvect (0x2f);

    // Устанавливаем новый обработчик прерываний
    _dos_setvect(0x2f, interrupt_handler);

    // ЗАпускаем саму оболочку
    system("DOSSHELL.EXE");

    // Восстанавливаем старый обработчик прерываний
    _dos_setvect (0x2f, oldvect);

}
