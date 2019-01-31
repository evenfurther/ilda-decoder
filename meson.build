project('ilda-decoder', 'c',
  default_options : 'c_std=c11',
  version : '1.0.0',
  license : 'MIT')

sdl2_dep = dependency('sdl2', required : false)

install_headers('ilda-decoder.h')

ilda_decoder_lib = both_libraries('ilda-decoder',
  'ilda-decoder.c',
  pic : false,
  install : true)

pkg = import('pkgconfig')
pkg.generate(libraries : ilda_decoder_lib,
             version : '1.0',
             name : 'libilda-decoder',
             filebase : 'ilda-decoder',
             description : 'A library to decode ILDA files.')

ilda_dump = executable('ilda-dump', 'ilda-dump.c',
  link_with : ilda_decoder_lib,
  install : true)

if sdl2_dep.found()
  ilda_display = executable('ilda-display', 'ilda-display.c',
    dependencies : sdl2_dep,
    link_with : ilda_decoder_lib,
    install : true)
endif

ilda_max_records = executable('ilda-max-records', 'ilda-max-records.c',
  link_with : ilda_decoder_lib,
  install : true)

ilda_check_liveness = executable('ilda-check-liveness', 'ilda-check-liveness.c',
  link_with : ilda_decoder_lib,
  install : true)
