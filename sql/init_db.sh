for file in *.sql
do
 echo "Importing $file" & mysql muddbtwo -u m241dan -pGrc937! < $file
done
echo "Done, enjoy!"
