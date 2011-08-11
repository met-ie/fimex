#! /bin/sh
echo "testing conversion of pressure to pressure"
../src/binSrc/fimex --input.file=verticalPressure.nc --verticalInterpolate.type=pressure --verticalInterpolate.method=log --verticalInterpolate.level1=1000,850,500,300,100,50 --output.file=out.nc
if [ $? != 0 ]; then
  echo "failed converting pressure to pressure"
  rm -f out.nc
  exit 1
fi
diff verticalPressurePressure.nc out.nc 
if [ $? != 0 ]; then
  echo "failed diff pressure to pressure"
  rm -f out.nc
  exit 1
fi
rm -f out.nc
echo "success"
exit 0