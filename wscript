# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('satellites-blockchain', ['core', 'network', 'internet', 'mobility', 'low-resolution-radio'])
    module.source = [
        'model/satellite-channel.cc',
        'model/satellite-net-device.cc',
        'helper/satellites-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('satellites-blockchain')
    module_test.source = []

    headers = bld(features='ns3header')
    headers.module = 'satellites-blockchain'
    headers.source = [
        'model/satellite-channel.h',
        'model/satellite-net-device.h',
        'helper/satellites-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

