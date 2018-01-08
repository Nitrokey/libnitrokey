project(
  'libnitrokey', 'cpp',
  version : '3.1.0',
  license : 'LGPL-3.0+',
  default_options : [
    'cpp_std=c++14'
  ],
  meson_version : '>= 0.43.0',
)
cxx = meson.get_compiler('cpp')
host_system = host_machine.system()
pkg = import('pkgconfig')

test_cflags = [
  '-Wno-unused-function',
  '-Wcast-qual',
  '-Woverloaded-virtual',
]
add_project_arguments(cxx.get_supported_arguments(test_cflags), language : 'cpp')

dep_hidapi = dependency('hidapi-libusb')

inc_libnitrokey = include_directories('include')
libnitrokey_args = []
if not get_option('log')
  libnitrokey_args += ['-DNO_LOG']
endif
if get_option('log-volatile-data')
  libnitrokey_args += ['-DLOG_VOLATILE_DATA']
endif
libnitrokey = library(
  'nitokey',
  sources : [
    'command_id.cc',
    'device.cc',
    'log.cc',
    'misc.cc',
    'NitrokeyManager.cc',
    'NK_C_API.cc',
    'DeviceCommunicationExceptions.cpp',
  ],
  include_directories : [
    inc_libnitrokey,
  ],
  dependencies : [
    dep_hidapi,
  ],
  cpp_args : libnitrokey_args,
  version : meson.project_version(),
  install : true,
)
install_headers(
  'include/CommandFailedException.h',
  'include/command.h',
  'include/command_id.h',
  'include/cxx_semantics.h',
  'include/DeviceCommunicationExceptions.h',
  'include/device.h',
  'include/device_proto.h',
  'include/dissect.h',
  'include/inttypes.h',
  'include/LibraryException.h',
  'include/log.h',
  'include/LongOperationInProgressException.h',
  'include/misc.h',
  'include/NitrokeyManager.h',
  'include/stick10_commands_0.8.h',
  'include/stick10_commands.h',
  'include/stick20_commands.h',
  subdir : meson.project_name(),
)

ext_libnitrokey = declare_dependency(
  link_with : libnitrokey,
  include_directories : inc_libnitrokey,
)

pkg.generate(
  name : meson.project_name(),
  filebase : 'libnitrokey-1',
  libraries : libnitrokey,
  version : meson.project_version(),
  requires_private : 'hidapi-libusb',
  description : 'Library for communicating with Nitrokey in a clean and easy manner',
  install : true,
)

if get_option('tests') or get_option('offline-tests')
  # FIXME
  _inc_catch = include_directories('/usr/include/catch')
  if not cxx.has_header('catch.hpp', include_directories : _inc_catch)
    error('Can\'t find catch.hpp')
  endif
  _catch = static_library(
    'catch',
    sources : [
      'unittest/catch_main.cpp',
    ],
    include_directories : [
      _inc_catch,
    ],
  )
  _dep_catch = declare_dependency(
    link_with : _catch,
    include_directories : _inc_catch,
  )
endif

tests = []
if get_option('offline-tests')
  tests += [
    ['test_offline', 'test_offline.cc'],
  ]
endif
if get_option('tests')
  tests += [
    ['test_C_API',  'test_C_API.cpp'],
    ['test1',       'test.cc'],
    ['test2',       'test2.cc'],
    ['test3',       'test3.cc'],
    ['test_HOTP',   'test_HOTP.cc'],
    ['test_issues', 'test_issues.cc'],
  ]
endif
foreach tst : tests
  test(
    tst[0],
    executable(
      tst[0],
      sources : 'unittest/@0@'.format(tst[1]),
      dependencies : [
        ext_libnitrokey,
        _dep_catch,
      ],
    )
  )
endforeach
