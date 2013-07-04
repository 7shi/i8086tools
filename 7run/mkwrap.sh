run=$1
root=$2
cmd=$3`basename $4`
m2cmd=$2/usr/$4

echo "#!/bin/sh" > $cmd
echo "exec $run -r $root $m2cmd \$@" >> $cmd

chmod 755 $cmd
