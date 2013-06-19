TARGETS = lib ld ar

all clean:
	for dir in $(TARGETS); do (make $@ -C $$dir || exit 1); done
