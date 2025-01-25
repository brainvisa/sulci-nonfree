
import sys
import six


class PluginError(BaseException):
    pass


class PluginManager(object):
    _plugins = {}
    _loaded_plugins = {}

    def __getitem__(self, name):
        if name in self._loaded_plugins:
            return self._loaded_plugins[name]
        elif name in self._plugins:
            raise PluginError("plugin '%s' not loaded" % name)
        else:
            raise KeyError("'%s'" % name)

    def loaded(cls): return cls._loaded_plugins
    loaded = classmethod(loaded)

    def __repr__(self):
        from datamind.tools import msg as msg
        if self._loaded_plugins == {}:
            msg.info("can't find any plugin :(")
        else:
            msg.info("loaded plugins : %s" %
                     str(list(self._loaded_plugins.keys())))

    def list(cls): return list(cls._plugins.keys())
    list = classmethod(list)

    def plugins_to_load(self):
        import datamind
        if isinstance(datamind.Settings.plugins, list):
            return datamind.Settings.plugins
        elif (datamind.Settings.plugins == 'all'):
            return list(self._plugins.keys())
        else:
            print('bad plugins settings, check datamind.Settings.plugins')

    def plugin_from_name(cls, name): return cls._plugins[name]
    plugin_from_name = classmethod(plugin_from_name)

    def _move_to_module(self, module, cls):
        module.__dict__[cls.__name__] = cls

    def _check_reader(self, plugin, r):
        try:
            r.__dict__['read']
        except KeyError:
            raise PluginError("plugin '%s': Reader "
                              "subclasses must define read(self, filename) "
                              "method." % plugin.name)

    def load_plugin(self, plugin):
        # For devel/debug, no exceptions catching
        def _get_info_debug(plugin):
            databases = plugin.databases()
            classifiers = plugin.classifiers()
            dimreductors = plugin.dimreductors()
            readers = plugin.readers()
            return databases, classifiers, dimreductors, readers

        def _get_info(plugin):
            try:
                databases = plugin.databases()
                classifiers = plugin.classifiers()
                dimreductors = plugin.dimreductors()
                readers = plugin.readers()
            except Exception as e:
                print('exception in load:', plugin, ':', e)
                import traceback
                traceback.print_exc()
                six.reraise(PluginError,
                            PluginError("Can't load plugin "
                                        "'%s'" % plugin.name),
                            sys.exc_info()[2])
            return databases, classifiers, dimreductors, readers

        import datamind
        if isinstance(plugin, str):
            try:
                plugin = self.plugin_from_name(plugin)
            except KeyError:
                from datamind.tools import msg
                msg.error("unknown plugin '%s'\n"
                          "Try one among : " + str(self.list()))
                raise

        if datamind.Settings.debug:
            databases, classifiers, dimreductors, readers = \
                _get_info_debug(plugin)
        else:
            databases, classifiers, dimreductors, readers = \
                _get_info(plugin)

        for db in databases:
            from datamind.ml import database
            self._move_to_module(database, db)
        for c in classifiers:
            from datamind.ml import classifier
            self._move_to_module(classifier, c)
        for d in dimreductors:
            from datamind.ml import dimreduction
            self._move_to_module(dimreduction, d)
        for r in readers:
            self._check_reader(plugin, r)
            from datamind.ml import reader
            self._move_to_module(reader, r)
        self._loaded_plugins[plugin.name] = plugin

    def load(self):
        from datamind.tools import msg
        for name in self.plugins_to_load():
            try:
                p = self.plugin_from_name(name)
            except KeyError:
                msg.warning("unknown plugin '%s', check "
                            "datamind.Settings.plugins.\n"
                            "Try one among %s" %
                            (name, str(self.list())))
                continue
            try:
                p.load()
            except PluginError as m:
                msg.warning(m)

    def register(self, plugin):
        self._plugins[plugin.name] = plugin()


class PluginRegister(type):

    def _check_new_class(cls):
        if cls.__name__ == 'Plugin':
            return
        if cls.name == "":
            raise ValueError('plugin class must define a name.')
        elif cls.name in PluginManager.list():
            plugin = PluginManager.plugin_from_name(cls.name)
            try:
                mod = plugin.__class__.__module__
            except:
                mod = '<unknown>'
            clsmod = cls.__module__
            #raise ValueError("several plugins " +
                             #("named '%s' :" % cls.name) +
                             #(" datamind.ml.plugins.%s and " % mod) +
                             #(" datamind.ml.plugins.%s" % clsmod))

    def __init__(cls, name, bases, dict):
        cls._check_new_class()
        if cls.__name__ != 'Plugin' and cls.autoregister:
            plugin_manager.register(cls)


@six.add_metaclass(PluginRegister)
class Plugin(object):
    name = ""
    autoregister = True

    def load(self): plugin_manager.load_plugin(self)

    def init(self): pass

    def quit(self): pass

    def classifiers(self): return []

    def dimreductors(self): return []

    def databases(self): return []

    def readers(self): return []

    def __del__(self): self.quit()


def init():
    import importlib
    import os
    dir = os.path.dirname(__file__)
    for p in os.listdir(dir):
        if not os.path.isdir(os.path.join(dir, p)) \
                or p.startswith('__'):
            continue
        importlib.import_module('.'.join([__package__, p]), __package__)


plugin_manager = PluginManager()
