##
#  Build file for gridfs
# @author : Devresse Adrien
# @version 1.0_beta
# @date 29/06/2011

import os

main_core = False


## pre-defined vars for local build only
etics_build_dir= "/home/adevress/workspace"
glib_location = etics_build_dir+ "/repository/externals/glib2-devel/2.12.3/sl5_x86_64_gcc412"

version = "1.0"
package_version= "2.3_preview"

	
## generic function to get conf value
def get_depconf(key_value, include_path='/include/', lib_path='/lib/', lib64_path='/lib64/', etics_suffix="/stage/"):
	if ARGUMENTS.get(key_value,'0') !='0':
		tmp_path = ARGUMENTS.get(key_value,'0')
	else:
		tmp_path= etics_build_dir+ etics_suffix 
	return ([ tmp_path+ include_path],[ tmp_path + lib64_path, tmp_path + lib_path ] )


gfal_headers_dir, gfal_lib_dir = get_depconf('gfal_path', include_path="/include/gfal2/", etics_suffix="stage/")
fuse_headers_dir, fuse_lib_dir = get_depconf('fuse_path', include_path="/include/", etics_suffix="stage/")



src_all = Glob("src/*.c");
resu = "gfalFS";

env = Environment(tools=['default', 'packaging']);


libs=[]
libs_path=[]
headers=[]
if ARGUMENTS.get("epel", "no") == "yes":
	env.ParseConfig('pkg-config --cflags --libs fuse')
	env.ParseConfig('pkg-config --cflags --libs gfal2')
else:
	libs = ['fuse', 'glib-2.0','gfal2'];
	libs_path= gfal_lib_dir + ["../gfal/build/libs/"] + fuse_lib_dir
	headers = gfal_headers_dir  + ["../gfal/src/"] + fuse_headers_dir


# debug mode
if ARGUMENTS.get('debug','0') =='yes':
	print "DEBUG MODE"
	env.Append(CFLAGS='-g')
	
# prod mode
if ARGUMENTS.get('production','0') =='yes':
	print "prod MODE"
	env.Append(CFLAGS='-O3')


if ARGUMENTS.get('main_core','no') =='yes':
	main_core=True

r = os.getenv('LD_LIBRARY_PATH')	# get ld path
env['ENV']['LD_LIBRARY_PATH'] = (r is not None) and r or "" # set ld path or empty one if not exist
env.Append(CFLAGS=['-D_FILE_OFFSET_BITS=64', '-Wall',  "-D_GRIDFS_VERSION=\\\"1.0\\\""], LIBS=libs, CPPPATH=headers, LIBPATH=libs_path)
env.ParseConfig('pkg-config --cflags --libs glib-2.0')
env.ParseConfig('pkg-config --cflags --libs gthread-2.0')


prog = env.Program(resu, src_all);
env.Depends(prog, Glob("src/*.h"))

install_list = []
package_list = []
comp_list = []

def define_rpm_install(opt):
	return 'scons -j 8 '+ opt+ ' --install-sandbox="$RPM_BUILD_ROOT" "$RPM_BUILD_ROOT" '


def arguments_to_str():
	ret = ' ';
	for arg, value in ARGUMENTS.iteritems():
		ret += arg+ '=' +value+ ' '
	return ret

if(main_core):
	comp_list += prog
	i = env.Install("/usr/bin/", prog)
	i2 = env.Install("/usr/bin/", "gfalFS_umount")
	install_list += [ i, i2 ]
	x_rpm_install = define_rpm_install(arguments_to_str());
	package_list += env.Package( 
			 NAME     = 'gfalFS',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = 'Apache-2.0',
			 SUMMARY        = 'filesystem for the grid',
			 DESCRIPTION    = 'Fuse filesystem for the grid, based on gfal, support the same url type than gfal',
			 X_RPM_GROUP    = 'lcg/grid',
			 X_RPM_INSTALL= x_rpm_install,
			 X_RPM_REQUIRES = 'glib2, fuse, gfal2-core',
			 source= [i, i2] 
			 )

env.Alias("build", comp_list);	
env.Alias("install", install_list)
env.Alias("package_generator", package_list)
