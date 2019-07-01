#ifndef _EHBANANA_H_
#define _EHBANANA_H_

#ifdef COMPILING_DLL
#define EHBANANA_EXPORTS __declspec(dllexport)
#else
#define EHBANANA_EXPORTS __declspec(dllimport)
#endif

extern "C" EHBANANA_EXPORTS void init();

#endif /* _EHBANANA_H_ */