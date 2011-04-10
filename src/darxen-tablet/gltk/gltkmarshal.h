
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

/* NONE:BOOL (gltkmarshal.list:3) */
#define g_cclosure_user_marshal_VOID__BOOLEAN	g_cclosure_marshal_VOID__BOOLEAN
#define g_cclosure_user_marshal_NONE__BOOL	g_cclosure_user_marshal_VOID__BOOLEAN

/* NONE:POINTER,POINTER (gltkmarshal.list:4) */
extern void g_cclosure_user_marshal_VOID__POINTER_POINTER (GClosure     *closure,
                                                           GValue       *return_value,
                                                           guint         n_param_values,
                                                           const GValue *param_values,
                                                           gpointer      invocation_hint,
                                                           gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__POINTER_POINTER	g_cclosure_user_marshal_VOID__POINTER_POINTER

/* NONE:STRING,POINTER (gltkmarshal.list:5) */
extern void g_cclosure_user_marshal_VOID__STRING_POINTER (GClosure     *closure,
                                                          GValue       *return_value,
                                                          guint         n_param_values,
                                                          const GValue *param_values,
                                                          gpointer      invocation_hint,
                                                          gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__STRING_POINTER	g_cclosure_user_marshal_VOID__STRING_POINTER

/* NONE:STRING,STRING,POINTER (gltkmarshal.list:6) */
extern void g_cclosure_user_marshal_VOID__STRING_STRING_POINTER (GClosure     *closure,
                                                                 GValue       *return_value,
                                                                 guint         n_param_values,
                                                                 const GValue *param_values,
                                                                 gpointer      invocation_hint,
                                                                 gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__STRING_STRING_POINTER	g_cclosure_user_marshal_VOID__STRING_STRING_POINTER

/* NONE:STRING,POINTER,STRING (gltkmarshal.list:7) */
extern void g_cclosure_user_marshal_VOID__STRING_POINTER_STRING (GClosure     *closure,
                                                                 GValue       *return_value,
                                                                 guint         n_param_values,
                                                                 const GValue *param_values,
                                                                 gpointer      invocation_hint,
                                                                 gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__STRING_POINTER_STRING	g_cclosure_user_marshal_VOID__STRING_POINTER_STRING

/* NONE:NONE (gltkmarshal.list:8) */
#define g_cclosure_user_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID
#define g_cclosure_user_marshal_NONE__NONE	g_cclosure_user_marshal_VOID__VOID

/* POINTER:INT,INT (gltkmarshal.list:9) */
extern void g_cclosure_user_marshal_POINTER__INT_INT (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);

G_END_DECLS

#endif /* __g_cclosure_user_marshal_MARSHAL_H__ */

