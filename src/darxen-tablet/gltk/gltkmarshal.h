
#ifndef __g_cclosure_user_marshal_MARSHAL_H__
#define __g_cclosure_user_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOL:POINTER (gltkmarshal.list:1) */
extern void g_cclosure_user_marshal_BOOLEAN__POINTER (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);
#define g_cclosure_user_marshal_BOOL__POINTER	g_cclosure_user_marshal_BOOLEAN__POINTER

/* NONE:POINTER (gltkmarshal.list:2) */
#define g_cclosure_user_marshal_VOID__POINTER	g_cclosure_marshal_VOID__POINTER
#define g_cclosure_user_marshal_NONE__POINTER	g_cclosure_user_marshal_VOID__POINTER

/* NONE:POINTER,POINTER (gltkmarshal.list:3) */
extern void g_cclosure_user_marshal_VOID__POINTER_POINTER (GClosure     *closure,
                                                           GValue       *return_value,
                                                           guint         n_param_values,
                                                           const GValue *param_values,
                                                           gpointer      invocation_hint,
                                                           gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__POINTER_POINTER	g_cclosure_user_marshal_VOID__POINTER_POINTER

/* NONE:NONE (gltkmarshal.list:4) */
#define g_cclosure_user_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID
#define g_cclosure_user_marshal_NONE__NONE	g_cclosure_user_marshal_VOID__VOID

G_END_DECLS

#endif /* __g_cclosure_user_marshal_MARSHAL_H__ */

