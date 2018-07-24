project(
  'libnitrokey', 'cpp',
  version : '3.4.1',
  license : 'LGPL-3.0+',
  default_options : [
    'cpp_std=c++14'
  ],
  meson_version : '>= 0.44.0',
)
cxx = meson.get_compiler('cpp')
host_system = host_machine.system()
pkg = import('pkgconfig')

common_flags = [
  '-Wno-unused-function',
  '-Wcast-qual',
]
test_cxxflags = common_flags + [
  '-Woverloaded-virtual',
]
test_cflags = common_flags
add_project_arguments(cxx.get_supported_arguments(test_cxxflags), language : 'cpp')
if get_option('offline-tests')
  add_languages('c', required: get_option('offline-tests'))
  c = meson.get_compiler('c')
  add_project_arguments(c.get_supported_arguments(test_cflags), language : 'c')
endif

dep_hidapi = dependency('hidapi-libusb')

inc_libnitrokey = include_directories('libnitrokey')
libnitrokey_args = []
if not get_option('log')
  libnitrokey_args += ['-DNO_LOG']
endif
if get_option('log-volatile-data')
  libnitrokey_args += ['-DLOG_VOLATILE_DATA']
endif

version_array = meson.project_version().split('.')
version_major = version_array[0].to_int()
version_minor = version_array[1].to_int()
version_data = configuration_data()
version_data.set('PROJECT_VERSION_MAJOR', version_major)
version_data.set('PROJECT_VERSION_MINOR', version_minor)
# We don't want to substitute it by noop
version_data.set('PROJECT_VERSION_GIT', '@VCS_TAG@')
version_cc_in = configure_file(
  input : 'version.cc.in',
  output : 'version.cc.in',
  configuration : version_data,
)
version_cc = vcs_tag(
  input : version_cc_in,
  output : 'version.cc',
  fallback : 'v@0@'.format(meson.project_version()),
)
libnitrokey = library(
  'nitrokey',
  sources : [
    'command_id.cc',
    'device.cc',
    'log.cc',
    version_cc,
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
  'libnitrokey/CommandFailedException.h',
  'libnitrokey/command.h',
  'libnitrokey/command_id.h',
  'libnitrokey/cxx_semantics.h',
  'libnitrokey/DeviceCommunicationExceptions.h',
  'libnitrokey/device.h',
  'libnitrokey/device_proto.h',
  'libnitrokey/dissect.h',
  'libnitrokey/LibraryException.h',
  'libnitrokey/log.h',
  'libnitrokey/LongOperationInProgressException.h',
  'libnitrokey/misc.h',
  'libnitrokey/version.h',
  'libnitrokey/NitrokeyManager.h',
  'libnitrokey/stick10_commands_0.8.h',
  'libnitrokey/stick10_commands.h',
  'libnitrokey/stick20_commands.h',
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

dep_udev = dependency('udev')
install_data(
  'data/41-nitrokey.rules',
  install_dir : '@0@/rules.d'.format(dep_udev.get_pkgconfig_variable('udevdir')),
)

if get_option('tests') or get_option('offline-tests')
  dep_catch = dependency('catch2', version : '>=2.3.0', required : false)
  if not dep_catch.found()
    dep_catch = declare_dependency(
      include_directories : include_directories('unittest/Catch/single_include')
    )
  endif
  _catch = static_library(
    'catch',
    sources : [
      'unittest/catch_main.cpp',
    ],
    dependencies : [
      dep_catch,
    ],
  )
  _dep_catch = declare_dependency(
    link_with : _catch,
    dependencies : dep_catch,
  )
endif

tests = []
if get_option('offline-tests')
  tests += [
    ['test_offline', 'test_offline.cc'],
    ['test_minimal', 'test_minimal.c'],
  ]
endif
if get_option('tests')
  tests += [
    ['test_C_API',  'test_C_API.cpp'],
    ['test1',       'test1.cc'],
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
