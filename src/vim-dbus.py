# Vim DBus Bindings
#

import vim

import dbus
import dbus.service

DBUS_NAME= 'org.vim'
DBUS_PATH = '/org/editors/vim'

# A class that acts a daemon for Vim
class DBusDaemon(dbus.service.Object):
    def __init__(self, bus, bus_name, object_path):
        dbus.service.Object.__init__(self,
                dbus.service.BusName(bus_name,bus=bus),
                object_path)

    @dbus.service.method(dbus_interface=DBUS_NAME,
            in_signature='s', out_signature='s')
    def ExecuteCmd(self, cmd):
        result = ''
        try:
            result = vim.command(cmd)
            return result
        except vim.error:
            return 'ERROR'

    @dbus.service.method(dbus_interface=DBUS_NAME,
            in_signature='s', out_signature='s')
    def QueryExpr(self, expr):
        result = ''
        try:
            result = vim.eval(expr)
            return result
        except vim.error:
            return 'ERROR'
    
    @dbus.service.method(dbus_interface=DBUS_NAME,
            in_signature='uu', out_signature='s',
            sender_keyword='sender')
    def GetBufContents (start,end):
        result = ''
        try:
            result = vim.current.buffer[start:end]
            return result
        except vim.error:
            return 'ERROR'



    @dbus.service.method(dbus_interface=DBUS_NAME,
            in_signature='', out_signature='s',
            sender_keyword='sender')
    def SayHello(self,sender):
        return "Hello! %s" % sender

    @dbus.service.signal(dbus_interface=DBUS_NAME,
            signature='us')
    def BufChanged(self, bufno, uri):
        pass
    
    @dbus.service.signal(dbus_interface=DBUS_NAME,
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

    DBusGMainLoop (set_as_default=True)
    bus = dbus.SessionBus()

    loop = MainLoop()

    vim_anjuta = DBusDaemon (bus, DBUS_NAME,'%s/daemon'%DBUS_PATH)

    threads_init()
    thread.start_new_thread(run, ())

