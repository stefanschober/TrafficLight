# also update assembler options to include important #define-s
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -A--cpreproc_opts=-DSTACKSIZE=${STACKSIZE}")
if(HEAPSIZE)
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS},-DHEAPSIZE=${HEAPSIZE}")
endif(HEAPSIZE)

target_compile_definitions(${TGT} PUBLIC MICROLIB)

# set default compiler options, valid for all USC2 projects
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --cpu=Cortex-M0 --thumb")

# set default linker options, valid for all USC2 projects
target_link_options(${TGT}
	PUBLIC
        --strict
	    --callgraph					                                # list a static callgraph of functions (HTML)
        $<$<CONFIG:Debug,Spy>:--debug>
	    --entry=Reset_Handler			                            # define global entry point to the application
	    --library_type=microlib		                                # link vs barebone system C library
	    --remove						                            # remove unused input sections from the final image
	    --map							                            # print the memory map of the image
#	    --list=$<TARGET_NAME:${TGT}>.map
	    --xref						                                # list all cross references between input sections
	    --symbols						                            # Output the local and global symbols
	    --info=architecture,compression,sizes,stack,summarystack,totals,summarysizes,unused,veneers	# output size, stack and compression information
	    --scatter=${SCATTER_FILE}		                            # the scatter file defines the memory assignment of the image
	    --strict_relocations
)
