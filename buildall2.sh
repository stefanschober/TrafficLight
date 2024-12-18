#! /bin/bash

declare -A platform=([Gnu]=Gnu [Pico]=Pico [Raspi]=Raspi [MinGW]=MinGW [Linux]=Linux [MSys]=MSys) # [mgwClang]="BuildMinGW/Clang")
declare -A tctpl=([Gnu]=armgnu [Pico]=none [Raspi]=armhfgnu [MinGW]=mingwgcc [Linux]="none" [MSys]=msys) # [mgwClang]=mingwclang)
declare -A container=([Gnu]=none [Pico]=none [Raspi]=armhf-cross [MinGW]=crosscompile [Linux]="none" [MSys]=crosscompile)
declare -A generator=([Make]="Make") # [Ninja]="Ninja")
declare -A cmakegen=([Make]="${CMAKE_GENERATOR:=none}") # [Ninja]="Ninja")
declare -A extracfg=([Gnu]="-DMCU=STM32F091xC" [Pico]="-DCONFIG_PICO=ON" [Raspi]="-DCONFIG_RASPI=ON -DCONFIG_RASPI_IO=ON -DCONFIG_PIGPIO=OFF -DCONFIG_GUI=ON" [MinGW]="-DCONFIG_GUI=ON" [Linux]="-DCONFIG_GUI=ON" [MSys]="-DCONFIG_GUI=ON")

DEFAULT_PRJ="TrafficLight"

function usage() {
    echo usge "buildall.sh -p platform [options] <project>
            default project = ${DEFAULT_PRJ}
            options:
                --platform    | -p <platform>
                    target platform, one out of (${!platform[@]})
                --source      | -s <source platform>
                    project source directory relativ to project base platform (CMakeLists.txt)
                --target      | -t <target>
                    build target [all]
                --container   | -C none
                    force local build []
                --prefix      | -P <prefix>
                    container prefix (e.g. 'troja:443')
                --app         | -a <containerapp>
                    define container manager [podman]
                --interactive | -i
                    container only: start in interactive mode 
                --qspy        | -q
                    enable qspy
                --clean       | -c
                    clean first
                --verbose     | -v
                    verbose build
                -D additional defines (-Dxxx=<value>) []"
    exit
}

unset _srcdir
unset _platform
unset _defines
unset _target
unset _container
unset _containerapp
unset _qspy
unset _clean
unset _verbose
unset _prefix
unset _interactive
function evalparameters() {
    while [[ "$1" == "-"* ]]; do
        case "$1" in
            --interactive | -i)
                _interactive=1
                ;;
            --source | -s)
                shift
                _srcdir="$1"
                ;;
            --platform | -p)
                shift
                _platform="$1"
                ;;
            -D*)
                _defines="$1 $_defines"
                ;;
            --target | -t)
                shift
                _target="$1"
                ;;
            --container | -C)
                shift
                _container="$1"
                if [ "${_container}" '!=' "none" ]; then
                    echo "only \"none\" allowed as value for \"container\""
                    usage
                fi
                ;;
            --prefix | -P )
                shift
                _prefix="$1"
                ;;
            --app | -a)
                shift
                _containerapp="$1"
                ;;
            --qspy | -q)
                _qspy=1
                ;;
            --clean | -c)
                _clean=1
                ;;
            --verbose | -v)
                _verbose=1
                ;;
            *)
                echo "unknown option $1"
                usage
                ;;
        esac
        shift
    done
    _prj="$1"
    _project="${_prj:-none}"

    if [ -z "${_project}" -o "${_project}" "=" "none" ]; then
        echo "No project given, using ${DEFAULT_PRJ}!"
        # usage
        _project="${DEFAULT_PRJ}"
    fi
}

function setupdatabase() {
    # configure the data base
    PLATFORM="${_platform:-none}"
    PRJ_BASE_DIR="${HOME}/Projects"
    BLD_BASE_DIR="${PRJ_BASE_DIR}/${_project}/Build"
    TC_BASE_DIR="${HOME}/cmake"
    CFG="-DTARGET=trafficlight"
    INTERACTIVE=""

    if [ -v _qspy ]; then
        CFG="${CFG} -DCONFIG_QSPY=ON"
    fi
    
    if [ -v _verbose ]; then
        TRACE="--trace"
        VERBOSE="--clean-first"
    else 
        if [ -v TRACE ]; then unset TRACE; fi
        if [ -v VERBOSE ]; then unset VERBOSE; fi
    fi

    if [ -v _clean ]; then
        VERBOSE="--clean-first"
    fi
}

function setvariables() {
    local platform="$1"
    local gen="$2"

    # echo "setvariables $platform $gen"
    TC=""
    container="${_container:-${container[$platform]}}"
    target="${_target:-all}"
    srcdir="${_srcdir:-${PRJ_BASE_DIR}/${_project}/Source}"
    blddir="${BLD_BASE_DIR}/${platform[$platform]}_${gen}"
    toolchainfile="tc_${tctpl[$platform]}.cmake"
    GENERATOR="${cmakegen[${gen}]}"
    BAT=$( mktemp "${srcdir}/bld.XXX" )
    CFG="${CFG} ${extracfg[$platform]} ${_defines}"

    if [ -n "${container}" -a "${container}" "!=" "none" ]; then
        CONTAINERAPP="${_containerapp:-podman}"
        if [ -v _prefix -a -n "${_prefix}" -a "${_prefix}" "!=" "none" ]; then
            container="${_prefix}/${container}"
        fi
        if [ -n "${toolchainfile}" -a "${toolchainfile}" "!=" "tc_none.cmake" ]; then
            TC="--toolchain /cmake/${toolchainfile}"
        fi
        CBAT="/src/$( basename ${BAT} )"
        CSRC="/src"
        CBLD="."
    else
        if [ -n "${toolchainfile}" -a "${toolchainfile}" "!=" "tc_none.cmake" ]; then
            TC="-DCMAKE_TOOLCHAIN_FILE=${TC_BASE_DIR}/${toolchainfile}"    
        fi
        CBAT="${BAT}"
        CSRC="${srcdir}"
        CBLD="${blddir}"
    fi

    if [ -v _interactive ]; then
        interactive="-i -t"
        unset containercmd
    else
        containercmd="/bin/bash ${CBAT}"
    fi

    echo "umask 0" >>${BAT}
    
    if [ -n "${GENERATOR}" -a "${GENERATOR}" "!=" "none" ]; then
        echo "export CMAKE_GENERATOR=\"${GENERATOR}\"" >>"${BAT}"
    fi
    echo "cmake ${TRACE} -B. ${TC} ${CFG} ${CSRC}" >>"${BAT}"
    echo "cmake --build . --target ${target} ${VERBOSE}" >>"${BAT}"
    chmod +x "${BAT}"

    echo "Configuring and building target \"${target}\" of project \"${_project}\" in \"${platform}\" for generator \"${gen}\" with container \"${container}\""
    echo "SRCDIR = \"${srcdir}\""
    echo "BLDDIR = \"${blddir}\""
    echo "CFG = \"${CFG}\""
    echo "BAT = ${BAT}; CBAT = ${CBAT}"
    if [ -v _verbose ]; then
        echo ">>>>>>>>>>>>>>>>>>>>> BAT <<<<<<<<<<<<<<<<<<<<<"
        cat "${BAT}"
        echo ">>>>>>>>>>>>>>>>>>>>> EOF <<<<<<<<<<<<<<<<<<<<<"
    fi
}

function build() {
    local platform="$1"
    
    for gen in "${!generator[@]}"; do
        setvariables $platform $gen
        test -d "${blddir}" || mkdir -p "${blddir}"

        if [ -n "${container}" -a "${container}" "!=" "none" ]; then
            
            ${CONTAINERAPP} run  --rm --name="${container}" \
                            -v"${srcdir}":"/src":ro \
                            -v"${TC_BASE_DIR}":"/cmake":ro \
                            -v"${blddir}":"/bld" \
                            -e"PKG_CONFIG_PATH=/usr/lib/arm-linux-gnueabihf/pkgconfig" \
                            -w"/bld" \
                            ${interactive} "${container}" ${containercmd}
        else
            pushd "${blddir}"
            bash "${BAT}"
            popd
        fi
        rm -f "${BAT}"
    done    
}

function main() {
    local pf="${PLATFORM}"
    local _pf="none"

    if [ -z "${pf}" -o "${pf}" "=" "none" ]; then
         echo "No usable platform specified! (one out of: ${!platform[@]})"
         usage
#        for pf in "${!platform[@]}"; do
#            _pf="${platform[${pf}]}"
#            if [ -z "${_pf}" -o "${_pf}" = "none" ]; then
#                continue
#            else
#                build "${pf}"
#            fi
#        done
    else
        _pf="${platform[${pf}]}"
        if [ -z "${_pf}" -o "${_pf}" = "none" ]; then
            echo "Unknown platform ${pf}"
            usage
        else
            build "${pf}"
        fi
    fi
}

evalparameters $@
setupdatabase
main
