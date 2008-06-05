# Vim DBus Bindings
#

import vim

import dbus
import dbus.service

DBUS_NAME_ANJUTA = 'org.anjuta'
DBUS_PATH_VIM = '/org/anjuta/vim/daemon'
DBUS_IFACE_EDITOR_REMOTE = 'org.editors.remote'

# A class that acts a daemon for Vim
class DBusDaemon(dbus.service.Object):
    def __init__(self, bus, bus_name, object_path):
        dbus.service.Object.__init__(self,
                dbus.service.BusName(bus_name,bus=bus),
                object_path)

    @dbus.service.method(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            in_signature='s', out_signature='s')
    def ExecuteCmd(self, cmd):
        result = ''
        try:
            result = vim.command(cmd)
            if result:
                return result
            else:
                return "None"
        except vim.error:
            return 'ERROR'

    @dbus.service.method(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            in_signature='s', out_signature='s')
    def QueryExpr(self, expr):
        result = ''
        try:
            result = vim.eval(expr)
            return result
        except vim.error:
            return 'ERROR'
    
    @dbus.service.method(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            in_signature='uu', out_signature='s')
    def GetBufContents (self, start, end):
        result = ''
        try:
            result = vim.current.buffer[start:end]
            return result
        except vim.error:
            return 'ERROR'

    @dbus.service.method(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            in_signature='', out_signature='s')
    def GetBufContentsFull (self):
        result = ''
        try:
            result = vim.current.buffer[:]
            return result
        except vim.error:
            return 'ERROR'

    @dbus.service.method(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            in_signature='', out_signature='s',
            sender_keyword='sender')
    def SayHello(self,sender):
        return "Hello! %s" % sender

    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='us')
    def BufChanged(self, bufno, uri):
        pass
    
    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='s')
    def BufSaved(self, uri):
        pass

# Run the daemon
if __name__ == "__main__":
    import thread
    from gobject import threads_init, MainLoop
    import dbus
    from dbus.mainloop.glib import DBusGMainLoop

    def run (*args, **kwargs):
        loop.run()

    DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
#    loop = MainLoop()

    vim_anjuta = DBusDaemon (bus, DBUS_NAME_ANJUTA,'%s/daemon'%DBUS_PATH_VIM)

#    threads_init()
#    thread.start_new_thread(run, ())

