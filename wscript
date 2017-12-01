# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('satellites-blockchain', ['core', 'network', 'internet', 'mobility'])
    module.source = [
        'model/satellite-channel.cc',
        'model/satellite-net-device.cc',
        'helper/satellites-blockchain-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('satellites-blockchain')
    module_test.source = [
        'test/satellites-blockchain-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'satellites-blockchain'
    headers.source = [
        'model/satellite-channel.h',
        'model/satellite-blockchain-device.h',
        'helper/satellites-blockchain-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

