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
    unsigned char id;		// Swither id
    unsigned char unk1[2+2];
    unsigned char params[130];
    unsigned char unk2[80+68+68+2+2+2+1+2];
    unsigned int  task_id;	// Active task id
    unsigned int  last_error;	// last error
    unsigned char unk3[1+1+1+1+1];
    unsigned char entries_count;
    unsigned char first;	// fist task
    tentrie  entries[MAXENTRIES];
  } tcontrol;

tcontrol far control;

/* char far shell[]="DOSSHELL.EXE";
par_blk  dw     0               ; use current environment
cmd_loc  dw     COMMANDLINEOUT  ; command-line address
cmd_seg  dw     0               ; fill in at initialization
         dw     offset fcb1     ; default FCB #1
fcb1_seg dw     0               ; fill in at initialization
         dw     offset fcb2     ; default FCB #2
fcb2_seg dw     0               ; fill in at initialization

fcb1     db     0
         db     11 dup (' ')
         db     25 dup ( 0 )
fcb2     db     0
         db     11 dup (' ')
         db     25 dup ( 0 )    */

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
      case 4: // remove Task
	control.entries[r.h.bl].flags=F_UNUSED;
	break;
      case 6: // ithprogramstring
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
      case 11: // LastError
	r.x.ax=control.last_error;
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

//        r.x.ax = 0x4a05;  
//        r.x.si = 0;  
//        int86x( 0x2f, &r, &r, &s );

    // Главный цикл
//        r.h.l = 0;  
//        r.x.ds = FP_SEG(shell);
//        r.x.dx = FP_OFF(shell);

//        intdos(0x4b, &r, &r);
	system("DOSSHELL.EXE");

//test();
                /*
        r.x.ax = 0x4a05;  
        r.x.si = 11;  
        int86x( 0x2f, &r, &r, &s );
        printf( "mouse handler address=%4.4x:%4.4x\n",
                s.es, r.x.ax );    

	r.x.bx=r.x.ax;
        r.x.ax = 0x4a05; 
        r.x.si = 6;  
        int86x( 0x2f, &r, &r, &s );
        printf( "mouse handler address=%Fs\n",
                MK_FP(s.es, r.x.si) );    
                     */
    // Восстанавливаем старый обработчик прерываний
    _dos_setvect (0x2f, oldvect);

}
