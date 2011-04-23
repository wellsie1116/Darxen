
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

/* NONE:POINTER,INT (gltkmarshal.list:5) */
extern void g_cclosure_user_marshal_VOID__POINTER_INT (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__POINTER_INT	g_cclosure_user_marshal_VOID__POINTER_INT

/* NONE:STRING (gltkmarshal.list:6) */
#define g_cclosure_user_marshal_VOID__STRING	g_cclosure_marshal_VOID__STRING
#define g_cclosure_user_marshal_NONE__STRING	g_cclosure_user_marshal_VOID__STRING

/* NONE:STRING,INT (gltkmarshal.list:7) */
extern void g_cclosure_user_marshal_VOID__STRING_INT (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__STRING_INT	g_cclosure_user_marshal_VOID__STRING_INT

/* NONE:STRING,STRING (gltkmarshal.list:8) */
extern void g_cclosure_user_marshal_VOID__STRING_STRING (GClosure     *closure,
                                                         GValue       *return_value,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint,
                                                         gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__STRING_STRING	g_cclosure_user_marshal_VOID__STRING_STRING

/* NONE:STRING,STRING,INT (gltkmarshal.list:9) */
extern void g_cclosure_user_marshal_VOID__STRING_STRING_INT (GClosure     *closure,
                                                             GValue       *return_value,
                                                             guint         n_param_values,
                                                             const GValue *param_values,
                                                             gpointer      invocation_hint,
                                                             gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__STRING_STRING_INT	g_cclosure_user_marshal_VOID__STRING_STRING_INT

/* NONE:STRING,POINTER (gltkmarshal.list:10) */
extern void g_cclosure_user_marshal_VOID__STRING_POINTER (GClosure     *closure,
                                                          GValue       *return_value,
                                                          guint         n_param_values,
                                                          const GValue *param_values,
                                                          gpointer      invocation_hint,
                                                          gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__STRING_POINTER	g_cclosure_user_marshal_VOID__STRING_POINTER

/* NONE:STRING,STRING,POINTER (gltkmarshal.list:11) */
extern void g_cclosure_user_marshal_VOID__STRING_STRING_POINTER (GClosure     *closure,
                                                                 GValue       *return_value,
                                                                 guint         n_param_values,
                                                                 const GValue *param_values,
                                                                 gpointer      invocation_hint,
                                                                 gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__STRING_STRING_POINTER	g_cclosure_user_marshal_VOID__STRING_STRING_POINTER

/* NONE:STRING,POINTER,STRING (gltkmarshal.list:12) */
extern void g_cclosure_user_marshal_VOID__STRING_POINTER_STRING (GClosure     *closure,
                                                                 GValue       *return_value,
                                                                 guint         n_param_values,
                                                                 const GValue *param_values,
                                                                 gpointer      invocation_hint,
                                                                 gpointer      marshal_data);
#define g_cclosure_user_marshal_NONE__STRING_POINTER_STRING	g_cclosure_user_marshal_VOID__STRING_POINTER_STRING

/* NONE:NONE (gltkmarshal.list:13) */
#define g_cclosure_user_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID
#define g_cclosure_user_marshal_NONE__NONE	g_cclosure_user_marshal_VOID__VOID

/* POINTER:INT,INT (gltkmarshal.list:14) */
extern void g_cclosure_user_marshal_POINTER__INT_INT (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);

/* POINTER:STRING (gltkmarshal.list:15) */
extern void g_cclosure_user_marshal_POINTER__STRING (GClosure     *closure,
                                                     GValue       *return_value,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint,
                                                     gpointer      marshal_data);

/* POINTER:STRING,POINTER (gltkmarshal.list:16) */
extern void g_cclosure_user_marshal_POINTER__STRING_POINTER (GClosure     *closure,
                                                             GValue       *return_value,
                                                             guint         n_param_values,
                                                             const GValue *param_values,
                                                             gpointer      invocation_hint,
                                                             gpointer      marshal_data);

/* POINTER:STRING,BOXED (gltkmarshal.list:17) */
extern void g_cclosure_user_marshal_POINTER__STRING_BOXED (GClosure     *closure,
                                                           GValue       *return_value,
                                                           guint         n_param_values,
                                                           const GValue *param_values,
                                                           gpointer      invocation_hint,
                                                           gpointer      marshal_data);

/* NONE:STRING,POINTER (gltkmarshal.list:18) */

G_END_DECLS

#endif /* __g_cclosure_user_marshal_MARSHAL_H__ */

