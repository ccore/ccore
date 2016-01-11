import os

env=Environment(CC='gcc', CCFLAGS='-Iinclude/')

sources=[Glob('src/ccore/linux/*/*.c'), Glob('src/ccore/common/*/*.c'), Glob('src/ccore/x11/*/*.c')]
libs=['X11', 'Xrandr', 'Xinerama', 'Xi', 'GL', 'GLU', 'pthread']
libpaths=['/usr/lib', '/usr/local/lib', '.']

opts=Variables('custom.py', ARGUMENTS)
opts.Add('target', 'Compile Target (debug/release)', 'debug', allowed_values=('debug', 'release'))
opts.Add('test', 'Create test files in the bin folder', 'no', allowed_values=('yes', 'feature', 'no'))
opts.Update(env)

env.Append(CCFLAGS=['-DCC_USE_ALL'])

if(env['target']=='debug'):
    env.Append(CCFLAGS=['-D_DEBUG'])
    env.Append(CCFLAGS=['-g'])
    env.Append(CCFLAGS=['-Wall'])

if(env['target']=='release'):
    env.Append(CCFLAGS=['-O3'])

staticLibrary=env.Library(target='lib/ccore', source=sources, LIBS=libs, LIBPATH=libpaths)
if(env['test']=='yes' or env['test']=='all' or env['test']=='feature'):
    env.Append(CCFLAGS=['-D_DEBUG'])
    env.Program(target='bin/test', source=['test/test.c', 'test/icon.c'], LIBS=[staticLibrary, libs], LIBPATH=libpaths)
    env.Program(target='bin/opengl', source=['example/opengl.c'], LIBS=[staticLibrary, libs], LIBPATH=libpaths)

Help(opts.GenerateHelpText(env))
