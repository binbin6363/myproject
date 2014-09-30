echo 'batch run...'

for((i=1;i<1000;i++));do
	echo $i
	sh run.sh
done
echo 'done'
