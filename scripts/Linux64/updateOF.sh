#!/bin/bash

# path to openFrameworks base folder
ofPath=`cd "../../../.."; pwd`

# paths to be updated
paths=(
"${ofPath}/libs/kiss"
"${ofPath}/libs/tess2"
"${ofPath}/libs/openFrameworksCompiled/project/linux64/config.linux64.default.mk"
)

# starting index
if [ $(uname -m) == 'x86_64' ]; then
startIndex=0
else
startIndex=3
fi

# number of paths
numPaths="${#paths[@]}"

# update paths
for ((i=startIndex; i<"${numPaths}"; i++));

do
path="${paths[$i]}"
srcPath=$(basename "${path}")
destPath=$(dirname "${path}")

# check if the update is successful or not
if yes | cp -rp "updateOF/${srcPath}" "${destPath}"; then
    echo "Updating: ${path}"
else
    echo "Failed updating: ${path}, exit status $?"
    exit 1
fi
done

echo "The update has been completed successfully"
