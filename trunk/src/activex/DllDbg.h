#ifndef UCANVCAM_DLLDBG_INC
#define UCANVCAM_DLLDBG_INC

#include <stdio.h>

// Comment out to disable debugging
//
#define DLL_DBG_ACTIVE

// Primitive log-to-file debugging.  Logs to:
//   c:/log/ucanvcam_*.txt
class DllDbg {
private:
	int initc;
	FILE *fout;
public:
	DllDbg() {
		initc = 0;
		fout = NULL;
	}

	~DllDbg() {
		if (initc) {
			initc = 1;
			fini();
		}
	}

	bool check() {
		return (initc>0)&&(fout!=NULL);
	}

	void done() {
#ifdef DLL_DBG_ACTIVE
		fflush(fout);
#endif
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
#ifdef DLL_DBG_ACTIVE
		if (initc == 0) {
			char buf[1000];
			sprintf_s(buf,sizeof(buf),"c:/log/ucanvcam_%s.txt",name);
			fout = NULL;
			fopen_s(&fout,buf,append?"a":"w");
		}
		initc++;
#endif
	}

	void fini() {
#ifdef DLL_DBG_ACTIVE
		initc--;
		if (initc==0) {
			fclose(fout);
			fout = NULL;
		}
#endif
	}
};

#endif
