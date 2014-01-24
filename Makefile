
archive:
	cd ..; tar -pczf $(date +%F_%T)-bark.tar.gz webcam

clean:
	rm -rf arch sounds *.jpg
	rm -rf sounds
	> bark.txt
