
#ifndef __g_cclosure_user_marshal_MARSHAL_H__
#define __g_cclosure_user_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:UINT (vim-signal.marshall:2) */
#define g_cclosure_user_marshal_VOID__UINT	g_cclosure_marshal_VOID__UINT

/* VOID:UINT,STRING (vim-signal.marshall:3) */
extern void g_cclosure_user_marshal_VOID__UINT_STRING (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);

G_END_DECLS

#endif /* __g_cclosure_user_marshal_MARSHAL_H__ */

