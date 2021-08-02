/* ADVLAND.C */

/* This file is a port of ADVEN1.BAS found on PC-SIG disk #203 */
/* The port is done by Morten Lohre (edb_morten@debet.nhh.no)  */
/* It has been successfully compiled with Borland Turbo C 2.0  */

/* It's not clean, but it works. Comments included in the file */
/* are made for my personal use. If you make modifications,    */
/* please mail me a copy stating the changes.                  */

#include <stdlib.h>
#include <div.h>
#include <advland_helpers.h>

signed int yes_no(void);
void empty_keyboardbuffer(void);
void welcome(void);
int get_input(void);
void look(void);
int get_item_string(int);
void turn(void);
void action(int ac, int *ip);
int get_action_variable(int *ip, int x);
void carry_drop(void);
int length(const char *s);
void copystring(unsigned char *dest, unsigned const char *source);
int comparestring(const char *s1, const char *s2);
int check_logics(void);
void prompt();

/* read static global data */
#include "advland.h"

/* dynamic global variables */
signed char     IA[IL];                 /* object locations */
signed int      NV[2];                  /* word numbers, NV[0] = first, NV[1] = second */
signed int      loadflag, endflag;      /* should we load or end? */
signed int      f,f3,f2;
signed int      r, lx, df, sf;
unsigned char   tps[80];                /* input string */
signed int      x,y;

/* externals:
   IA[], I2[], loadflag, endflag, lx, df, sf, r, NV[] */

void initAdvland(void)
{
  // omitted: 
  //  randomise
  //  welcome
  //  load saved game

  /* reset object locations */
  for (int i=0;i<IL;i++) IA[i]=I2[i]
    ;

  loadflag = 0;
  endflag = 0;
  
  r = AR;
  lx = LT;
  df = 0;
  sf = 0;

  clrscr();
  look();
  NV[0] = 0;
}

void stepAdvland() {
    if (!get_input()) 
    {
      turn();
      if (!loadflag && !endflag)
      {
		if (IA[9] == -1)
		{
		  lx--;
		  if (lx < 0)
		  {
			_printf("light has run out!\n");
			IA[9] = 0;
		  }
		  else if (lx < 25) _printf("light runs out in %u turns!",lx);
		}
		NV[0] = 0;
		turn();
      }
    }
  prompt();
}

void prompt() {
  _printf("\nTell me what to do? ");
}

/* Empty keyboard buffer */
void empty_keyboardbuffer(void)
{
  while (kbhit()!=0) getch();
}

/* Empty keyboard, get Y(es) or N(o), _printf character with carriage return */
int yes_no(void)
{
  int ch;

  empty_keyboardbuffer();  /* empty keyboardbuffer */
  do
  {
    ch = getch();
    if (ch > 96) ch = ch - 32;
  } while (ch!=89 && ch!=78);
  _printf("%c\n",ch);
  return(ch==89);  /* 1 if Y, 0 if N */
}

/* Welcome */
void welcome(void)
{
  clrscr();
  _printf(" W E L C O M E   T O \n A D V E N T U R E - 1 \n\n\n\n\n");
  _printf("The object of your adventure is to find treasures and return them\n");
  _printf("to the proper place for you to accumulate points.  I'm your clone.  Give me\n");
  _printf("commands that consist of a verb & noun, i.e. GO EAST, TAKE KEY, CLIMB TREE,\n");
  _printf("SAVE GAME, TAKE INVENTORY, FIND AXE, etc.\n\n");
  _printf("You'll need some special items to do some things, but I'm sure that you'll be\n");
  _printf("a good adventurer and figure these things out (which is most of the fun of\n");
  _printf("this game).\n\n");
  _printf("Note that going in the opposite direction won't always get you back to where\n");
  _printf("you were.\n\n\n");
  _printf("HAPPY ADVENTURING!!!\n\n\n\n\n");
  empty_keyboardbuffer();
  _printf("************************** Press any key to continue **************************");
  /* while (getch()==0); */
  /* clrscr(); */
}

/* Evaluate user input */
/* Externals:
   tps, NV[], NVS[][] */

int get_input(void)
{
  int i,j;              /* counting variables */
  char *word[2];        /* first and second string */
  const char  *s;

  _gets(tps);
  for (i=0;i<length(tps);i++) tps[i]=_toupper(tps[i]);
  i = 0;
  while (tps[i]==' ' && tps[i]!='\0') i++;  /* go to first word */
  word[0] = tps+i;
  while (tps[i]!=' ' && tps[i]!='\0') i++;  /* go to next space */
  while (tps[i]==' ' && tps[i]!='\0') i++;  /* go to next word */
  word[1] = tps+i;
  for (j=0;j<2;j++) /* no more than LN chars */
  {
    i = 0;
    while(word[j][i]!=' ' && word[j][i]!='\0' && i<LN) i++;
    word[j][i]='\0';
  }
  for (i=0;i<2;i++)
  {
    NV[i] = 0;
    if (word[i][0]!='\0')
    {
      for (j=0;j<NL;j++)
      {
	s = NVS[i][j];
	if (s[0] == '*') s++;  /* skip special char */
	if (comparestring(word[i],s)==0)
	{
	  NV[i] = j;
	  j = NL;
	  while (NVS[i][NV[i]][0] == '*') NV[i]--;
	}
      }
    }
  }
  if ((NV[0] < 1) || (word[1][0]!='\0' && NV[1] < 1))
  {
    _printf("You use word(s) I don't know.!\n");
    return(1);
  }
  else return(0);
}

/* Print location description, exits and visible items */
/* Externals:
   df, IA[], RSS[][], tps, r, RM[][], NVS[][] */

void look(void)
{
  int k;        /* Flag */
  int i,j;

  if (df && (IA[9]!=-1 && IA[9]!=r)) _printf("I can't see.  It's too dark!\n");
  else
  {
    if (RSS[r][0] == '*') _printf(RSS[r]+1);
    else
    {
      _printf("I'm in a %s",RSS[r]);
    }
    k = -1;
    for (i=0;i<IL;i++)
    {
      if (k && (IA[i]==r))
      {
	_printf("\n\nVISIBLE ITEMS HERE:\n");
	k = 0;
      }
      if (IA[i] == r)
      {
	j = get_item_string(i);
	if ((wherex() + j + 3) > MAXLINE) _printf("\n");
	_printf("%.*s.  ",j,IAS[i]);
      }
    }
    _printf("\n");
    k = -1;
    for (i=0;i<6;i++)
    {
      if (k && (RM[r][i]!=0))
      {
	_printf("\nObvious exits: \n");
	k = 0;
      }
      if (RM[r][i]!=0)
      {
	_printf("%s ",NVS[1][i + 1]);
      }
    }
    _printf("\n\n");
  }
}

void turn(void)
{
  int i,j,ac;

  if (NV[0] == 1 && NV[1] < 7)
  {
    i = (df) && (IA[9] != r) && (IA[9] != -1);
    if (i) _printf("Dangerous to move in the dark!\n");
    if (NV[1] < 1) _printf("Give me a direction too.\n");
    else
    {
      j = RM[r][NV[1] - 1];
      if (j == 0 && !i) _printf("I can't go in that direction.\n");
      else
      {
	if (j == 0 && i)
	{
	  _printf("I fell down and broke my neck.\n");
	  j = RL;
	  df = 0;
	}
	if (!i) clrscr();
	r = j;
	look();
      }
    }
  }
  else
  {
    f = -1;
    f2 = -1;
    f3 = 0;
    x = 0;
    do
    {
      i = divu(C[x][0], 150);
      if ((NV[0] == 0) && (i != 0)) x = CL;
      else
      {
	if (NV[0] == i)
	{
	  i = modu(C[x][0], 150);
	  if ((i == NV[1] || i == 0) || (NV[0] == 0 && divu(_rand(), 100)+1 <= i))
	  {
	    f = 0;
	    f3 = -1;
	    f2 = check_logics();
	    if (f2)
	    {
	      i = 0;
	      y = 1;
	      do
	      {
		j = divu(y - 1, 2)+6;
		ac = divu(C[x][j], 150);
		if (y == 2 || y == 4) ac = modu(C[x][j], 150);
		action(ac,&i);
		if (loadflag == 1 || endflag == 1)
		{
		  y = 10;
		  x = CL;
		}
		y++;
	      } while (y <= 4);
	      if (y < 10) if (NV[0]!= 0) x = CL;
	    }
	  }
	}
      }
      x++;
    } while (x < CL);
    if (NV[0] != 0) carry_drop();
  }
}

/* externals:
   MSS[], IA[], NV[], x, y, r, df, sf ... */
void action(int ac, int *ip)
{
  // FILE *fd;
  int i,j,p;

  if (ac > 101) _printf("%s\n",MSS[ac - 50]);    /* Messages 52 and up */
  if (ac > 0 && ac < 52) _printf("%s\n",MSS[ac]);  /* Messages 1 - 51 */
  if (ac == 52)
  {
    j = 0;
    for (i=1;i<IL;i++) if (IA[i] == -1) j++;
    if (j >= MX)
    {
      _printf("I've too much to carry!\n");
      if (NV[0] != 0) x = CL;
      y = 10;
    }
    else IA[get_action_variable(ip,x)] = -1;
  }
  if (ac == 53) IA[get_action_variable(ip,x)] = r;
  if (ac == 54) r = get_action_variable(ip,x);
  if (ac == 55 || ac == 59) IA[get_action_variable(ip,x)] = 0;
  if (ac == 56) df = -1;
  if (ac == 57) df = 0;
  if (ac == 58) sf = 1<<get_action_variable(ip,x) | sf;
  if (ac == 60) sf = sf ^ 1<<get_action_variable(ip,x);
  if (ac == 61)
  {
    _printf("I'm dead...\n");
    r = RL-1;
    df = 0;
    look();
  }
  if (ac == 62)
  {
    i = get_action_variable(ip,x);
    IA[i] = (get_action_variable(ip,x));
  }
  if (ac == 63)
  {
    _printf("The game is now over.\nAnother game? ");
    if (!yes_no())  /* No */ endflag = 1;
    else /* Yes */
    {
      for (i=0;i<IL;i++) IA[i] = I2[i];
      loadflag = 1;
    }
  }
  if (ac == 64) look();
  if (ac == 65)
  {
    j = 0;
    for (i=1;i<IL;i++) if (IA[i] == TR) if (IAS[i][0] == '*') j++;
    _printf("I've stored %u treasures.  On a scale\nof 0 to 100, that rates a %u.\n",j,j*divu(100,TT));
    if (j == TT)
    {
      _printf("Well done.\nThe game is now over.\nAnother game? ");
      if (!yes_no())  /* No */ endflag = 1;
      else
      {
	for (i=0;i<IL;i++) IA[i] = I2[i];
	loadflag = 1;
      }
    }
  }
  if (ac == 66)
  {
    _printf("I'm carrying:\n");
    j = -1;
    for (i=0;i<IL;i++)
    {
      if (IA[i] == -1)
      {
	p = get_item_string(i);
	if ((p + wherex() + 2) > MAXLINE) _printf("\n");
	_printf("%.*s. ",p,IAS[i]);
	j = 0;
      }
    }
    if (j) _printf("Nothing!\n");
  }
  if (ac == 67) sf = 1 | sf;
  if (ac == 68) sf = sf ^ 1;
  if (ac == 69)
  {
    lx = LT;
    IA[9] = -1;
  }
  if (ac == 70) clrscr();
  if (ac == 71)
  {
    _printf("Save is not supported\n");
    /* printf("Is the current drive ready to receive the saved game? "); */
    /* if (yes_no()) */
    /* { */
    /*   fd = fopen("ADVEN-1.DAT","wb"); */
    /*   putw(sf,fd); */
    /*   putw(lx,fd); */
    /*   putw(df,fd); */
    /*   putw(r,fd); */
    /*   for (i=0;i<IL;i++) putw(IA[i],fd); */
    /*   fclose(fd); */
    /* } */
    /* printf("\n"); */
  }
  if (ac == 72)
  {
    j = get_action_variable(ip,x);
    p = get_action_variable(ip,x);
    i = IA[j];
    IA[j] = IA[p];
    IA[p] = i;
  }
}

/* Discard unwanted string at end of item description */
/* Externals:
   IAS[] */
/* Returns number of printable characters in item description */

int get_item_string(int i)
{
  int p;

  p = length(IAS[i]); /* points to back of string */
  if (IAS[i][p-1] == '/')
  {
    do
      p--;
    while (p>0 && IAS[i][p-1]!='/');
    if (IAS[i][p-1]!='/') p = length(IAS[i]); else p--;
  }
  return(p);
}

/* Externals: C[][] */
int get_action_variable(int *p, int x)
{
  do
  {
    (*p)++;
    if (*p < 1 || *p > 5) *p = 1;
  } while (modu(C[x][*p], 20) != 0);
  return(divu(C[x][*p], 20));
}

/* Can I carry or drop it? If so, do it. */
void carry_drop(void)
{
  int ll,i,j,l,k;

  if ((NV[0] == 10 || NV[0] == 18) && (!f3))
  {
    if (NV[1] == 0)
    {
      _printf("What?\n");
      f = 0;
    }
    else
    {
      if (NV[0] == 10)
      {
	l = 0;
	for (i=0;i<IL;i++) if (IA[i] == -1) l++;
      }
      if (NV[0] == 10 && l >= MX)
      {
	_printf("I've too much to carry!\n");
	f = 0;
      }
      else
      {
	k = 0;
	for (j=0;j<IL;j++)
	{
	  ll = length(IAS[j]) - 1;
	  if (IAS[j][ll] == '/')
	  {
	    copystring(tps,IAS[j]);
	    tps[ll] = '\0';     /* get one shorter */
	    while (ll>1)
	    {
	      ll--;
	      if (tps[ll] == '/')
	      {
		ll++;   /* first char in item name */
		tps[ll+LN] = '\0';      /* make sure string is no longer than 3 chars */
		if (comparestring(tps+ll,NVS[1][NV[1]])==0)
		{
		  if (NV[0] == 10)
		  {
		    if (IA[j] != r) k = 2;
		    else
		    {
		      IA[j] = -1;
		      k = 3;
		      _printf("OK, \n");
		      j = IL;
		    }
		  }
		  else
		  {
		    if (IA[j] != -1) k=1;
		    else
		    {
		      IA[j] = r;
		      k = 3;
		      _printf("OK, \n");
		      j = IL;
		    }
		  }
		}
		ll = 1;
	      }
	    }
	  }
	}
	if (k == 1) _printf("I'm not carrying it!\n");
	if (k == 2) _printf("I don't see it here.\n");
	if (k == 0)
	{
	  if (!f3)
	  {
	    _printf("It's beyond my power to do that.\n");
	    f = 0;
	  }
	}
	else f = 0;
      }
    }
  }
  if (f) _printf("I don't understand your command.\n");
  else if (!f2) _printf("I can't do that yet.\n");
}

int length(const char *s)
{
  int i;

  i = 0;
  while (s[i] != '\0') i++;
  return(i);
}

void copystring(unsigned char *dest, unsigned const char *source)
{
  int i;

  i = 0;
  while (source[i]!='\0')
  {
    dest[i] = source[i];
    i++;
  }
  dest[i] = '\0';
}

int comparestring(const char *s1, const char *s2)
{
  int i;

  i = 0;
  while (s1[i]==s2[i] && s1[i]!='\0') i++;
  if (s1[i]=='\0' || s2[i]=='\0') return(0); else return(1);
}

/* externals:
   C[][], x, r, IA[], sf, f2 */
int check_logics(void)
{
  int y,ll,k,i,f1;

  f2 = -1;
  y = 1;
  do
  {
    ll = divu(C[x][y], 20);
    k = modu(C[x][y], 20);
    f1 = -1;
    if (k == 1) f1 = -(IA[ll] == -1);
    if (k == 2) f1 = -(IA[ll] == r);
    if (k == 3) f1 = -(IA[ll] == r || IA[ll] == -1);
    if (k == 4) f1 = -(r == ll);
    if (k == 5) f1 = -(IA[ll] != r);
    if (k == 6) f1 = -(IA[ll] != -1);
    if (k == 7) f1 = -(r != ll);
    if (k == 8)
    {
      f1 = sf & 1<<ll;
      f1 = -(f1 != 0);
    }
    if (k == 9)
    {
      f1 = sf & 1<<ll;
      f1 = -(f1 == 0);
    }
    if (k == 10)
    {
      f1 = 0;
      for (i=0;i<IL;i++)
      {
	if (IA[i] == -1)
	{
	  f1 = -1;
	  i = IL;
	}
      }
    }
    if (k == 11)
    {
      f1 = -1;
      for (i=0;i<IL;i++)
      {
	if (IA[i] == -1)
	{
	  f1 = 0;
	  i = IL;
	}
      }
    }
    if (k == 12) f1 = -(IA[ll] != -1 && IA[ll] != r);
    if (k == 13) f1 = -(IA[ll] != 0);
    if (k == 14) f1 = -(IA[ll] == 0);
    f2 = -(f2 && f1);
    y++;
  } while ((y <= 5) && f2);
  return(f2);
}
