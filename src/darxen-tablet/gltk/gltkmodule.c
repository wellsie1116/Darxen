
#include <pygobject.h>

void gltk_register_classes (PyObject* d);
void gltk_add_constants(PyObject *module, const gchar *strip_prefix);
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
	gltk_add_constants(m, "GLTK_");

	if (PyErr_Occurred())
   	{
		Py_FatalError("Can't initialize module gltk");
	}

}
