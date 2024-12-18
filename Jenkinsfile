pipeline {
    agent any
    environment {
        SRC_DIR = "./Source"
    }
    stages {
        stage('build_cmake') {
            environment {
		PICO_SDK_PATH = "${HOME}/pico_sdk"
                CMAKE_TOOLCHAIN_FILE = "${HOME}/cmake/tc_armgnu.cmake"
                CMAKE_DEFINES = "-DCONFIG_KERNEL_QK=TRUE -DMCU=TLE9844_2QX"
                BLD_DIR = "Build/Make_Gnu"
            }
            steps {
                echo "build for ARM Cortex-M0 with arm-none-eabi-gcc"
                sh "cmake -B ${BLD_DIR} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} ${CMAKE_DEFINES} ${SRC_DIR}"
                sh "cmake --build ${BLD_DIR} --target all"
                echo "done'"
            }
        }
        stage('build_buildprj') {
            steps {
                echo "build all targets"
                sh "buildprj -p all"
                echo "done"
            }
        }
    }
}
