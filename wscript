#! /usr/bin/env python
# encoding: utf-8

APPNAME = 'mts'
VERSION = '7.1.0'


def build(bld):

    bld.env.append_unique(
        'DEFINES_STEINWURF_VERSION',
        'STEINWURF_MTS_VERSION="{}"'.format(
            VERSION))

    bld.recurse('src/mts')

    if bld.is_toplevel():

        # Only build tests when executed from the top-level wscript,
        # i.e. not when included as a dependency
        bld.recurse('test')
        bld.recurse('examples')
        bld.recurse('benchmark/parsing')
        bld.recurse('benchmark/packetizing')
