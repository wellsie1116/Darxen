
#include <pygobject.h>

void gltk_register_classes (PyObject* d);
extern PyMethodDef gltk_functions[];

DL_EXPORT(void)
initgltk(void)
{
	PyObject* m;
	PyObject* d;

	init_pygobject();

	m = Py_InitModule("gltk", gltk_functions);
	d = PyModule_GetDict(m);

	gltk_register_classes(d);

	if (PyErr_Occurred())
   	{
		Py_FatalError("Can't initialize module gltk");
	}

}
