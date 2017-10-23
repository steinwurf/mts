#!/usr/bin/env python
# encoding: utf-8

import traceback
import sys

try:
    input = raw_input
except NameError:
    pass

project_name = 'mts'
project_dependencies = \
    [
        'waf-tools',
        'gtest',
    ]


def importCode(code, name, add_to_sys_modules=0):
    """
    Import dynamically generated code as a module.
    Python recipe from http://code.activestate.com/recipes/82234
    Code is the object containing the code (a string, a file handle or an
    actual compiled code object, same types as accepted by an
    exec statement). The name is the name to give to the module,
    and the final argument says wheter to add it to sys.modules
    or not. If it is added, a subsequent import statement using
    name will return this module. If it is not added to sys.modules
    import will try to load it in the normal fashion.

    import foo

    is equivalent to

    foofile = open("/path/to/foo.py")
    foo = importCode(foofile,"foo",1)

    Returns a newly generated module.
    """
    import imp

    module = imp.new_module(name)

    exec(code, module.__dict__)
    if add_to_sys_modules:
        sys.modules[name] = module

    return module


if __name__ == '__main__':
    print('Updating Smart Project Config Tool...')

    url = "https://raw.github.com/steinwurf/steinwurf-labs/" \
          "master/config_helper/config-impl.py"

    try:
        from urllib.request import urlopen, Request
    except ImportError:
        from urllib2 import urlopen, Request

    try:
        # Fetch the code file from the given url
        req = Request(url)
        response = urlopen(req)
        code = response.read()
        print("Update complete. Code size: {}\n".format(len(code)))
        try:
            # Import the code string as a module
            mod = importCode(code, "config_helper")
            # Run the actual config tool from the dynamic module
            mod.config_tool(project_dependencies)
        except Exception:
            print("Unexpected error:")
            print(traceback.format_exc())
    except Exception as e:
        print("Could not fetch code file from:\n\t{}".format(url))
        print(e)

    input('Press ENTER to exit...')
