#!/bin/bash
# Wrapper script around cmake commands

usage()
{
	printf "Usage: ./$(basename $0) [build] [install] [clean] [distclean]\n\n"
	printf "build     : Build or rebuild lagscope binary\n"
	printf "install   : Copy lagscope binary to install folder\n"
	printf "clean     : make clean\n"
	printf "distclean : Remove build folder\n"
	exit 1
}

[ $# -ne 1 ] && usage

returndir=$(pwd)

for i in "$@"
do
	case $i in
		build )
			mkdir -p build
			cd build
			cmake ../src
			cmake --build .
			break
			;;
		install )
			cd build
			cmake --build . --target install
			break
			;;
		clean )
			cd build
			cmake --build . --target clean
			break
			;;
		distclean )
			rm -rf build
			break
			;;
		* ) usage ;;
	esac
done

cd "$returndir"
exit 0
