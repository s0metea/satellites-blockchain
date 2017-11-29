# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('sat-net-device', ['core', 'network', 'internet', 'mobility'])
    module.source = [
        'model/sat-channel.cc',
        'model/sat-net-device.cc',
        'helper/sat-net-device-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('sat-net-device')
    module_test.source = [
        'test/sat-net-device-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'sat-net-device'
    headers.source = [
        'model/sat-channel.h',
        'model/sat-net-device.h',
        'helper/sat-net-device-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

