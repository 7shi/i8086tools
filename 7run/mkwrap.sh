# sh mkwrap.sh /usr/local/bin/7run /usr/local/minix2 /usr/local/bin/m2 usr/bin/cc

# ex. /usr/local/bin/7run
run=$1

# ex. /usr/local/minix2
root=$2

# ex. /usr/local/bin/m2cc
cmd=$3`basename $4`

# ex. /usr/local/minix2/usr/bin/cc
wrap=$2/$4

echo "#!/bin/sh" > $cmd
echo "exec $run -r $root $wrap \$@" >> $cmd

chmod 755 $cmd
