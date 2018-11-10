#!/bin/bash
#SBATCH --ntasks-per-node 28              # total number of mpi tasks requested
#SBATCH -N 1 
#SBATCH -p normal     # queue (partition) -- normal, development, etc.
#SBATCH -t 48:00:00        # run time (hh:mm:ss) - 1.5 hours
#SBATCH -p RM
#SBATCH --mail-type=ALL
#SBATCH --mail-user=140137084@aluno.unb.br

if [ $# -ne 10 ]
then
	echo "Numero de parametros errado"
	exit $E_BADARGS
fi

NP=$1
I=$2
B=$3
C=$4
DKT=$5
ALG=$6
DOUT=$7
RPT=$8

LMEN=$9
LTIME=${10}

for rep in `seq 1 $RPT`
do
	printf 'i%b b%b c%b r%b\n' $I $B $C $rep
	echo "/usr/bin/time -a -f "\t%E" -o $LTIME /usr/bin/time -a -f "\t%M" -o $LMEM \
mpirun -np $NP ./PipelineRTFS-NS-Diff-FGO -b $B -c $C -i $I -dkt $DKT -ma $ALG "
#1>$DOUT/cout_i${I}b${B}c${C}rep${rep}.txt 2>$DOUT/cerr_i${I}b${B}c${C}rep${rep}.txt"

	#printf %s "$(cat $LTIME)" > $LTIME
	#printf %s "$(cat $LMEM)" > $LMEM
done
