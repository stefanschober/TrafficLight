#! /bin/bash

# createTgtCpl.sh
# create the following files from
# 	1) the executable application and
#	2) the eeprom data files (parameter, profile) in binary format (*.bin)
# Output files:
#	1) application as HEX file
#	2) eeprom data as HEX file
#	3) application + eeprom data integrated into 1 HEX file (<tgt>.cpl.hex)

# command line parameters to this script are
#	1) toolchain - GNU or KEIL
#	2) object copy executable (GNU: objcopy; KEIL: fromelf - complete path/file name)
#	3) application (complete path/file name)
#	4) parameter data file (complete path/file name)
#	5) profile data file (complete path/file name)

TOOLCHAIN="${1}"
OBJCOPY="${2}"
APP="${3}"
PARAM_IN="${4}"
PROFILE_IN="${5}"
EEPROM_DATA_FILE=eepromData.hex

if [ "${TOOLCHAIN}" = "KEIL" ]; then
	 AWKPARREGEX="/eepromParameter[ ]*0x/"
	 AWKPRFREGEX="/eepromVariant[ ]*0x/"
	 AWKCMD='{ print $2 }'
elif [ "${TOOLCHAIN}" = "GNU" ]; then
	 AWKPARREGEX="/0x[0-9a-fA-F]+\\s+eepromParameter/"
	 AWKPRFREGEX="/0x[0-9a-fA-F]+\\s+eepromVariant/"
	 AWKCMD='{ print $1 }'
else
	echo "Unknown toolchain ${TOOLCHAIN}"
	return 1
fi


# 1) create the hex file from the application; toolchain dependent
echo "Using ${OBJCOPY} to create ${APP}.hex from ${APP}"
if [ -x "${OBJCOPY}" ] && [ -f "${APP}" ]; then
	if [ "${TOOLCHAIN}" = "KEIL" ]; then
		 "${OBJCOPY}" --i32combined --output="${APP}.hex" "${APP}"
	elif [ "${TOOLCHAIN}" = "GNU" ]; then
		"${OBJCOPY}" -O ihex "${APP}" "${APP}.hex"
	else
		echo "Unknown toolchain ${TOOLCHAIN}"
		return 1
	fi
else
	echo "command failed: ${OBJCOPY} ${APP} => ${APP}.hex"
	return 1
fi

# 2) creating eepromData.hex from parameter and profile data
if [ -f "${APP}.map" ]; then
	PAROFFS=$( awk "${AWKPARREGEX} ${AWKCMD}" ${APP}.map )
	PRFOFFS=$( awk "${AWKPRFREGEX} ${AWKCMD}" ${APP}.map )
	echo "Using srec_cat to create eepromData.hex from ${PARAM_IN} and ${PROFILE_IN}"
	srec_cat "${PARAM_IN}"   -bin -offset ${PAROFFS} \
	         "${PROFILE_IN}" -bin -offset ${PRFOFFS} \
	                         -o "${EEPROM_DATA_FILE}" -intel
	                       
	srec_cat "${APP}.hex" -intel -exclude -over "${EEPROM_DATA_FILE}" -intel "${EEPROM_DATA_FILE}" -intel -o __tmp.s19 -intel
	srec_cat __tmp.s19  -intel -fill 0x00 -over __tmp.s19 -intel -o "${APP}.cpl.hex" -intel
	rm -f __tmp.s19
else
	echo "Map file ${APP}.map not found"
	return 1
fi