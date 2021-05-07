#!/bin/bash -e

OUTPUT_DIR=set_01
mkdir -p ${OUTPUT_DIR}

for DIR in ~/nn/DarkPlate/empty_images ~/nn/DarkPlate/video_import_* ; do
	echo "${DIR}..."

	for IMAGE in ${DIR}/*.jpg ; do

		TXT=${DIR}/$(basename ${IMAGE} .jpg).txt
		if [ -e ${TXT} ]; then
			cp ${IMAGE} ${OUTPUT_DIR}
			cp ${TXT} ${OUTPUT_DIR}
		fi
	done
done

