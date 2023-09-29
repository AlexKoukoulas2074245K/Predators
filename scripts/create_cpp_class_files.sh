CURR_DATE=$(date +%d/%m/%Y)
HEADER=$1.h
CPP=$1.cpp

TOP_LEVEL_DIR=$(git rev-parse --show-toplevel)

pushd .

cd $TOP_LEVEL_DIR
PROJECT=${PWD##*/} 

cd source
mkdir -p ${2:-.}
cd ${2:-.}

rm $1.cpp
rm $1.h

echo "///------------------------------------------------------------------------------------------------" >> $1.cpp
echo "///  ${CPP}                                                                                        " >> $1.cpp
echo "///  ${PROJECT}                                                                                            " >> $1.cpp
echo "///                                                                                                " >> $1.cpp
echo "///  Created by Alex Koukoulas on ${CURR_DATE}                                                       " >> $1.cpp
echo "///------------------------------------------------------------------------------------------------" >> $1.cpp
echo "\n#include <${2:-}/$1.h>" >> $1.cpp
echo "\n///------------------------------------------------------------------------------------------------" >> $1.cpp


echo "///------------------------------------------------------------------------------------------------" >> $1.h
echo "///  ${HEADER}                                                                                          " >> $1.h
echo "///  ${PROJECT}                                                                                            " >> $1.h
echo "///                                                                                                " >> $1.h
echo "///  Created by Alex Koukoulas on ${CURR_DATE}                                                       " >> $1.h
echo "///------------------------------------------------------------------------------------------------" >> $1.h
echo "\n#ifndef $1_h" >> $1.h
echo "#define $1_h" >> $1.h
echo "\n///------------------------------------------------------------------------------------------------" >> $1.h
echo "\n///------------------------------------------------------------------------------------------------" >> $1.h
echo "\n#endif /* $1_h */" >> $1.h

popd
