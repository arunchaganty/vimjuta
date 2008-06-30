# Vim DBus Bindings
#

import vim

import dbus
import dbus.service

DBUS_NAME_ANJUTA = 'org.anjuta'
DBUS_PATH_VIM = '/org/anjuta/vim'
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
            in_signature='', out_signature='s',
            sender_keyword='sender')
    def SayHello(self,sender):
        return "Hello! %s" % sender

    # Signals. Follow Vim event name conventions

    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='u')
    def BufNewFile(self, bufno):
        pass

    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='us')
    def BufRead(self, bufno, filename):
        pass
    
    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='us')
    def BufWrite(self, bufno, filename):
        pass

    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='us')
    def BufAdd(self, bufno, filename):
        pass

    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='u')
    def BufDelete(self, bufno):
        pass

    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='us')
    def BufFilePost(self, bufno, filename):
        pass

    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='us')
    def BufEnter(self, bufno, filename):
        pass

    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='u')
    def BufLeave(self, bufno):
        pass

    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='')
    def VimLeave(self):
        pass

    @dbus.service.signal(dbus_interface=DBUS_IFACE_EDITOR_REMOTE,
            signature='u')
    def MenuPopup(self, bufno):
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

    # TODO: Use an arbitrarily set variable g:anjuta to make the bus unique.
    daemon = DBusDaemon (bus, DBUS_NAME_ANJUTA,'%s/daemon'%DBUS_PATH_VIM)


