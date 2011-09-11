#ifndef YARPOUT_INC
#define YARPOUT_INC

#include <stdio.h>

#define GOTCHA(x) {YarpOut gotcha; gotcha.init("gotcha",true); gotcha.say(__FILE__,__LINE__,x); }


//#define USE_NETWORK
//#define USE_FILE
#define USE_DUMMY

#ifdef USE_NETWORK

class YarpOut {
private:
    int initc;
    Port p;
public:
    YarpOut() {
        initc = 0;
    }
    void say(Bottle& b) {
        if (initc>0) {
            Bottle b2("ver5");
            b2.append(b);
            p.write(b2);
        }
    }

    void init() {
        if (initc == 0) {
            Network::init();
            p.open("/testing");
            Network::connect("/testing","/log");
        }
        initc++;
    }

    void fini() {
        initc--;
        if (initc==0) {
            p.close();
            Network::fini();
        }
    }
};

#endif

#ifdef USE_FILE

class YarpOut {
private:
    int initc;
    FILE *fout;
public:
    YarpOut() {
        initc = 0;
        fout = NULL;
    }

    ~YarpOut() {
      if (initc) {
	initc = 1;
	fini();
      }
    }

    bool check() {
      return (initc>0)&&(fout!=NULL);
    }

    void done() {
      fflush(fout);
    }

    void say(const char *str) {
      if (!check()) return;
      fprintf(fout,"*** %s\n", str);
      done();
    }

    void say(const char *str, int a, int b) {
      if (!check()) return;
      fprintf(fout,"*** %s %d %d\n", str, a, b);
      done();
    }

    void init(const char *name, bool append = false) {
        if (initc == 0) {
	  char buf[1000];
	  sprintf(buf,"c://log//ucanvcam_%s.txt",name);
	  fout = fopen(buf,append?"a":"w");
	  /*
	  if (fout!=NULL) {
	    //fprintf(fout,"STARTING!\n");
	    fflush(fout);
	  }
	  */
        }
        initc++;
    }

    void fini() {
        initc--;
        if (initc==0) {
            fclose(fout);
            fout = NULL;
        }
    }
};

#endif


#ifdef USE_DUMMY
class YarpOut {
public:
    YarpOut() {
    }

    ~YarpOut() {
    }

    void say(const char *str) {
    }

    void say(const char *str, int a, int b) {
    }

    void init(const char *name, bool append = false) {
    }

    void fini() {
    }
};
#endif

#endif
