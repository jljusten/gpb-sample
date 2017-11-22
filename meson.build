project('gpb-sample', 'c')

c_compiler = meson.get_compiler('c')

deps = []
deps += dependency('waffle-1')

srcs = 'main.c gl_defs.h gbm_ctx.c'
srcs = srcs.split()

executable('gpb-sample', srcs, dependencies: deps)
