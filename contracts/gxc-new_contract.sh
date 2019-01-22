#!/bin/bash

if [[ $# < 2 ]]; then
   echo Usage: $0 TEMPLATE CONTRACT [ACTION]
   echo Create new project for gxc.contracts
   exit 1
fi

TEMPLATE=$1
if [[ $TEMPLATE -ne 0 && $TEMPLATE -ne 1 ]]; then
   echo ERROR: not supported template type [0-1]
   exit 1
fi

CONTRACT=$2
if [[ -z $CONTRACT ]]; then
   echo ERROR: contract name not specified
   exit 1
fi
if [[ -f $CONTRACT ]]; then
   echo ERROR: cannot create directory ‘$CONTRACT’: File exists
   exit 1
fi

ACTION=$3
if [[ -z $ACTION ]]; then
   ACTION="hi"
fi

IN=(${CONTRACT//./ })
if [[ ${#IN[@]} > 1 ]]; then
   NAMESPACE=${IN[0]}
   CLASS=${IN[1]}
else
   CLASS=$CONTRACT
fi

echo Copying template...
cp -R template/$TEMPLATE $CONTRACT

echo Renaming files/directories...
mv $CONTRACT/include/template $CONTRACT/include/$CONTRACT

for file in `find $CONTRACT -type f -name '*template*'`
do
   mv $file `sed s/template/$CONTRACT/g <<< $file`
done

echo Renaming in files...
if [[ -z $NAMESPACE ]]; then
   find $CONTRACT -type f -exec sed -i "s/@CONTRACT/$CONTRACT/g" {} \; \
                          -exec sed -i "s/@ACTION/$ACTION/g" {} \; \
                          -exec sed -i "/@NAMESPACE_BEGIN/d" {} \; \
                          -exec sed -i "/@NAMESPACE_END/d" {} \; \
                          -exec sed -i "s/@NAMESPACE_PREFIX//g" {} \; \
                          -exec sed -i "s/@CLASS/$CLASS/g" {} \;
else
   if [[ $TEMPLATE -eq 0 ]]; then
      find $CONTRACT -type f -exec sed -i "s/@CONTRACT/$CONTRACT/g" {} \; \
                             -exec sed -i "s/@ACTION/$ACTION/g" {} \; \
                             -exec sed -i "s/@NAMESPACE_BEGIN/namespace $NAMESPACE {\n/g" {} \; \
                             -exec sed -i "s/@NAMESPACE_END/\n}/g" {} \; \
                             -exec sed -i "s/@NAMESPACE_PREFIX/$NAMESPACE::/g" {} \; \
                             -exec sed -i "s/@CLASS/$CLASS/g" {} \;
   else
      find $CONTRACT -type f -exec sed -i "s/@CONTRACT/$CONTRACT/g" {} \; \
                             -exec sed -i "s/@ACTION/$ACTION/g" {} \; \
                             -exec sed -i "s/@NAMESPACE_BEGIN/namespace $NAMESPACE { namespace $CLASS {\n/g" {} \; \
                             -exec sed -i "s/@NAMESPACE_END/\n} }/g" {} \; \
                             -exec sed -i "s/@NAMESPACE_PREFIX/$NAMESPACE::/g" {} \; \
                             -exec sed -i "s/@CLASS/$CLASS/g" {} \;
   fi
fi

echo "Done! $CONTRACT is ready. Don't forget to add it to CMakeLists.txt!"
