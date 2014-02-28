all:
	@/bin/echo -e "{\n\t\"data\": [" | cat - bark.txt > tmpfile && mv tmpfile bark.txt
	head -n -1 bark.txt > tmpfile && mv tmpfile bark.txt
	@/bin/echo -e "\t\t}" >> bark.txt
	@/bin/echo -e "\t]\n}" >> bark.txt

archive:
	cd ..; tar -pczf $(shell date +%F)-bark.tar.gz dogwatch

clean:
	rm -rf arch sounds *.jpg
	rm -rf sounds
	> bark.txt
