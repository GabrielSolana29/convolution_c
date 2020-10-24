#!bin/bash

# # is for comments

# $ es para obtener el valor de una variable

echo "Total arguments passed: " $#
echo "All arguments: " $@

# verificar que tengamos los argumentos antes de seguir 
if [ $# -ne 6 ]; then 
    echo "Incorrect number of arguments"
    exit 0
fi


# signo de pesos 1 es el primer agumento y así

echo "Number of threads: " $1
echo "Image selected: " $2
echo "Kernel: " $3
echo "Schedule:" $4
echo "Number of iterations used for validation: " $5
# Seguardan los argumentos

noThreads=$1
image=$2
kernel=$3
schedule=$4
noIterations=$5
prog=$6

echo "PROG IMP " $prog
# prog es el nombre del programa que vamos a ejecutar



## Estoy creando un archivo en blanco si ya existia uno con ese nombre lo va a desaparecer
#echo "" > "${noThreads}_${image}_${kernel}_${schedule}_${noIterations}.csv"
# Run the programm
## Concatenar en ese archivo los resultados
for ((i=1; i <= 3; i++)) # for three images
do
  for ((j=1; j <= $noThreads; j += 2)) # for each noThreads
  do 
    for ((e=1; e<=3; e++)) # Schedules
    do
      # Change the schedule of the environmental variable
      if [ $e -eq 1 ]; then
          echo "Schedule is static"
          export OMP_SCHEDULE="static,32768"
      elif [ $e -eq 2 ]; then 
          echo "Schedule is dynamic"
          export OMP_SCHEDULE="dynamic,32768"
      elif [ $e -eq 3 ]; then
          echo "Schedule is guided"
          export OMP_SCHEDULE="guided,32768"
      fi
      
      #echo "" > "${j}_${i}_${kernel}_${e}_${noIterations}_32768.csv"          
      # vamos a pasar los argumentos al programa prog
      res=`$prog $j $i $kernel $e $noIterations`
      #res=`$prog $noThreads $image $kernel $schedule $noIterations`
      #echo ${res} >> "${j}_${i}_${kernel}_${e}_${noIterations}_32768.csv"          
      echo ${res} >> "${noThreads}_${i}_${kernel}_${noIterations}_32768.csv"
     
    done
  done
done
